/* ESOP
 * Copyright (C) 2018  EPFL
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <sat/cnf_symmetry_breaking.hpp>
#include <src/Graph.hpp>
#include <src/global.hpp>
#include <src/Algebraic.hpp>
#include <src/Theory.hpp>
#include <src/Breaking.hpp>

namespace sat
{

cnf_symmetry_breaking::cnf_symmetry_breaking( int& sid )
  : _sid( sid )
{}

void cnf_symmetry_breaking::apply( constraints& constraints )
{
  std::unordered_set<sptr<Clause>, UVecHash, UvecEqual> clauses;
  for ( const auto& c : constraints._clauses )
  {
    std::set<unsigned> clause;
    for ( const auto& l : c )
    {
      clause.insert( encode( l ) );
      if ((uint) abs(l) > nVars) {
        nVars = abs(l);
      }
    }
    clauses.insert( std::make_shared<Clause>( clause ) );
  }

  auto theory = std::make_shared<CNF>( clauses );

  std::vector<sptr<Group> > subgroups;
  theory->getGroup()->getDisjointGenerators(subgroups);
  theory->cleanUp(); // improve some memory overhead

  uint totalNbMatrices = 0;
  uint totalNbRowSwaps = 0;

  Breaker brkr( theory );
  for (auto grp : subgroups) {
    if (grp->getSize() > 1) {
      theory->setSubTheory(grp);
      grp->addMatrices();
      totalNbMatrices += grp->getNbMatrices();
      totalNbRowSwaps += grp->getNbRowSwaps();
    }
    grp->addBreakingClausesTo(brkr);
    grp.reset();
  }

  auto cls = brkr.get_clauses();
  for ( const auto& c : cls )
  {
    constraints._clauses.push_back( c );
  }
  constraints._num_variables = brkr.getTotalNbVars();
  _sid = brkr.getTotalNbVars();
}

} /* sat */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
