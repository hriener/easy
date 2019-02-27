/* easy: C++ ESOP library
 * Copyright (C) 2018-2019  EPFL
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

/*!
  \file esop_from_rcf.hpp
  \brief Implements constructors for exclusive-or sum-of-product forms.

  \author Heinz Riener
*/

#pragma once

#include <easy/sat/sat_solver.hpp>
#include <easy/detail/cube_manipulators.hpp>
#include <kitty/cube.hpp>
#include <kitty/operations.hpp>
#include <kitty/print.hpp>
#include <queue>
#include <map>

namespace easy
{

namespace detail
{

/*
 * 3^x for 0 <= x <= 10
 */
uint64_t pow3[] =
{
  1,
  3,
  9,
  27,
  81,
  243,
  2187,
  6561,
  19683,
  59049,
  177147,
}; /* pow3 */

class rcf_sat_engine
{
public:
  using map_t = std::map<uint64_t,int>;

public:
  explicit rcf_sat_engine( sat_solver& solver )
    : _solver( solver )
  {}

  int add_variable( kitty::cube const& c )
  {
    auto it = cube_to_sid.find( c._value );
    if ( it != cube_to_sid.end() )
    {
      return it->second;
    }
    else
    {
      auto const new_id = _sid++;
      cube_to_sid.emplace( std::make_pair( c._value, new_id ) );
      return new_id;
    }
  }

  void add_xor_clause( std::vector<int> const& clause, bool value )
  {
    assert( !clause.empty() && "clause must not be empty" );

    std::queue<int> lits;
    for ( const auto& l : clause )
      lits.push( l );

    while ( lits.size() > 1 )
    {
      auto a = lits.front();
      lits.pop();
      auto b = lits.front();
      lits.pop();

      int c = _sid++;
      _solver.add_clause( { -a, -b, -c } );
      _solver.add_clause( {  a,  b, -c } );
      _solver.add_clause( {  a, -b,  c } );
      _solver.add_clause( { -a,  b,  c } );
      lits.push( c );
    }

    assert( lits.size() == 1u );
    if ( value )
    {
      _solver.add_clause( {-lits.front()} );
    }
    else
    {
      _solver.add_clause( {lits.front()} );
    }    
  }

  bool solve( std::vector<kitty::cube>& esop )
  {
    auto const result = _solver.solve();
    if ( result == sat_solver::state::sat )
    {
      auto const model = _solver.get_model();
      for ( const auto& p : cube_to_sid )
      {
        auto const id = p.second;
        if ( model[id] )
        {
          kitty::cube c;
          c._value = p.first;
          esop.emplace_back( c );
        }
      }
      return true;
    }

    std::cout << "UNSAT" << std::endl;
    return false;
  }
  
  sat_solver& _solver;
  int _sid{1};

  /* cube to sid */
  map_t cube_to_sid;
}; /* rcf_sat_engine */

template<typename TT>
void generate_rcf_constraints( rcf_sat_engine& rcf, TT const& tt, uint32_t const r = 0 )
{
  auto const n = tt.num_vars();
  assert( uint32_t(n) > r );
  
  kitty::cube d = kitty::cube::neg_cube( r );
  for ( auto k = 0u; k < ( 1u << r ); ++k ) // d
  {
    kitty::cube b = kitty::cube::neg_cube( n - r );
    for ( auto i = 0u; i < ( 1u << ( n - r ) ) ; ++i ) // b
    {
      std::vector<int> xor_clause;
      kitty::cube c = kitty::cube::neg_cube( n - r );

      for ( auto j = 0u; j < pow3[ n - r ]; ++j ) // c
      {
        auto const cmp = compare( c, b, n - r );

        /* (c,b) in R */
        if ( cmp )
        {
          /* combine c and d into one cube of size n */
          auto cd = combine( c, d, n, r );

          // std::cout << "cd = "; cd.print( n ); std::cout << std::endl;

          /* unique push */
          auto const v = rcf.add_variable( cd );
          if ( std::find( std::begin( xor_clause ), std::end( xor_clause ), v ) == std::end( xor_clause ) )
          {
            // std::cout << v << ' ';
            xor_clause.emplace_back( v );
          }
        }

        // std::cout << std::endl;        
        incr_cube( c, n - r );
      }

      /* combine b and d into one cube of size n */
      auto const bd = combine( b, d, n, r );
      rcf.add_xor_clause( xor_clause, !kitty::get_bit( tt, bd._bits ) );
      ++b._bits;
    }

    ++d._bits;
  }
}

} /* namespace detail */

/*! \brief Computes ESOP representation using RCF

  \param tt Truth table
*/
template<typename TT>
inline std::vector<kitty::cube> esop_from_rcf( TT const& tt, uint32_t r = 0 )
{
  sat_solver_statistics stats;
  sat_solver_params params;
  sat_solver solver( stats, params );

  detail::rcf_sat_engine rcf_engine( solver );
  detail::generate_rcf_constraints( rcf_engine, tt, r );

  std::vector<kitty::cube> esop;
  rcf_engine.solve( esop );
  return esop;
}

} /* namespace easy */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
