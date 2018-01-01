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

#include <sat/sat_solver.hpp>
#include <cassert>

#ifdef CRYPTOMINISAT_EXTENSION

namespace sat
{

sat_solver::sat_solver()
{
  /* default solver settings */
  _solver.set_default_polarity( false );
  _solver.set_allow_otf_gauss();
}

sat_solver::result sat_solver::solve( const std::vector<int>& assumptions )
{
  CMSat::lbool sat;

  if ( assumptions.size() > 0 )
  {
    std::vector<CMSat::Lit> assume;
    for ( const auto& v : assumptions )
    {
      const unsigned var = abs(v)-1;
      while ( _num_vars <= var )
      {
	_solver.new_var();
	++_num_vars;
      }
      assume.push_back( CMSat::Lit( var, v < 0 ) );
    }
    sat = _solver.solve( &assume );
  }
  else
  {
    sat = _solver.solve();
  }

  if ( sat == CMSat::l_True )
  {
    return result( _solver.get_model() );
  }
  else
  {
    return result( sat );
  }
}

void sat_solver::add_clause( const clause_t& c )
{
  std::vector<CMSat::Lit> clause;
  for ( const auto& v : c )
  {
    const unsigned var = abs(v)-1;
    while ( _num_vars <= var )
    {
      _solver.new_var();
      ++_num_vars;
    }
    clause.push_back( CMSat::Lit( var, v < 0 ) );
  }
  _solver.add_clause( clause );
}

void sat_solver::add_xor_clause( const clause_t& c, bool value )
{
  std::vector<unsigned> clause;
  for ( const auto& v : c )
  {
    assert( v > 0 );
    const unsigned var = v-1;
    while ( _num_vars <= var )
    {
      _solver.new_var();
      ++_num_vars;
    }
    clause.push_back( var );
  }
  _solver.add_xor_clause( clause, value );
}

} /* sat */

#endif /* CRYPTOMINISAT_EXTENSION */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
