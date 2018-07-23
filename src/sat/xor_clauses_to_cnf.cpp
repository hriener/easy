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

#include <sat/xor_clauses_to_cnf.hpp>
#include <queue>

namespace sat
{

xor_clauses_to_cnf::xor_clauses_to_cnf( int& sid )
  : _sid( sid )
{}

void xor_clauses_to_cnf::add_xor_clause( constraints& constraints, const std::vector<int>& xor_clause, bool value )
{
  assert( !clause.first.empty() && "clause must not be empty" );

  std::queue<int> lits;
  for ( const auto& l : xor_clause )
  {
    lits.push( l );
  }

  while ( lits.size() > 1 )
  {
    auto a = lits.front();
    lits.pop();
    auto b = lits.front();
    lits.pop();

    int c = _sid++;
    constraints.add_clause( { -a, -b, -c } );
    constraints.add_clause( {  a,  b, -c } );
    constraints.add_clause( {  a, -b,  c } );
    constraints.add_clause( { -a,  b,  c } );

    lits.push( c );
  }

  assert( lits.size() == 1u );
  if ( value )
  {
    constraints.add_clause( { lits.front() } );
  }
  else
  {
    constraints.add_clause( { -lits.front() } );
  }
}

void xor_clauses_to_cnf::apply( constraints& constraints )
{
  for ( const auto& clause : constraints._xor_clauses )
  {
    add_xor_clause( constraints, clause.first, clause.second );
  }
  constraints._xor_clauses.clear();
}

} /* sat */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
