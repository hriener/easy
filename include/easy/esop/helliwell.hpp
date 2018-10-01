/* easy: C++ ESOP library
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

#pragma once

/***
 *
 * SAT-based ESOP enumeration algorithm that solves the Helliwell decision problem using CryptoMiniSAT.
 *
 * Marek Perkowski and Malgorzata Chrzanowska-Jeske, An exact algorithm to minimize mixed-radix exclusive
 * sums of products for incompletely specified Boolean funcions, IEEE International Symposium on Circuits
 * and Systems, 1990.
 */

#include <esop/esop.hpp>
#include <sat/sat_solver.hpp>
#include <cassert>
#include <cstring>
#include <unordered_map>
#include <vector>

namespace easy::esop
{

namespace detail
{

struct vars
{
  inline int operator[]( const kitty::cube& c )
  {
    return get_or_create_var( c );
  }

  int get_or_create_var( const kitty::cube& c )
  {
    auto it = cube_to_var.find( c );
    int variable;
    if ( it == cube_to_var.end() )
    {
      variable = sid++;
      cube_to_var.insert( std::make_pair( c, variable ) );
      var_to_cube.insert( std::make_pair( variable, c ) );
    }
    else
    {
      variable = it->second;
    }
    return variable;
  }

  int lookup_var( const kitty::cube& c ) const
  {
    return cube_to_var.at( c );
  }

  const kitty::cube& lookup_cube( int variable ) const
  {
    return var_to_cube.at( variable );
  }

  int sid = 1;
  std::unordered_map<kitty::cube, int, kitty::hash<kitty::cube>> cube_to_var;
  std::unordered_map<int, kitty::cube> var_to_cube;
}; /* vars */

std::vector<unsigned> compute_flips( unsigned n )
{
  const auto total_flips = ( 1u << n ) - 1;
  std::vector<unsigned> flip_array( total_flips );

  auto graynumber = 0u;
  auto temp = 0u;
  for ( auto i = 1u; i <= total_flips; ++i )
  {
    graynumber = i ^ ( i >> 1 );
    flip_array[total_flips - i] = ffs( temp ^ graynumber ) - 1u;
    temp = graynumber;
  }

  return flip_array;
}

std::vector<kitty::cube> derive_product_group( const kitty::cube& c, unsigned num_vars )
{
  const auto flips = compute_flips( num_vars );

  std::vector<kitty::cube> group = {c};
  auto copy = c;
  for ( auto i = 0u; i < flips.size(); ++i )
  {
    if ( copy.get_mask( flips[i] ) )
    {
      copy.clear_bit( flips[i] );
      copy.clear_mask( flips[i] );
    }
    else
    {
      copy.set_mask( flips[i] );
      if ( c.get_bit( flips[i] ) )
      {
        copy.set_bit( flips[i] );
      }
      else
      {
        copy.clear_bit( flips[i] );
      }
    }
    group.push_back( copy );
  }

  return group;
}

} // namespace detail

esops_t synthesis_from_binary_string( std::string const& binary )
{
  const int num_vars = log2( binary.size() );
  assert( binary.size() == ( 1ull << num_vars ) && "bit-width is not a power of 2" );

  esops_t esops;

  sat::constraints constraints;
  sat::sat_solver solver;
  vars g;
  assert( num_vars <= 32 && "cube data structure cannot store more than 32 variables" );
  kitty::cube minterm;
  for ( auto i = 0; i < num_vars; ++i )
  {
    minterm.set_mask( i );
  }

  do
  {
    if ( binary[minterm._bits] == '0' || binary[minterm._bits] == '1' )
    {
      std::vector<int> clause;
      for ( const auto& t : derive_product_group( minterm, num_vars ) )
      {
        clause.push_back( g[t] );
      }
      constraints.add_xor_clause( clause, binary[minterm._bits] == '1' );
    }

    ++minterm._bits;
  } while ( minterm._bits < ( 1 << num_vars ) );

  sat::sat_solver::result result;
  while ( ( result = solver.solve( constraints ) ) )
  {
    esop_t esop;
    std::vector<int> blocking_clause;
    for ( auto i = 0u; i != solver._solver->nVars(); ++i )
    {
      const auto var = i + 1;
      if ( result.model[i] == l_True )
      {
        blocking_clause.push_back( -var );
        esop.push_back( g.lookup_cube( var ) );
      }
      if ( result.model[i] == l_False )
      {
        blocking_clause.push_back( var );
      }
    }
    constraints.add_clause( blocking_clause );
    esops.push_back( esop );

    if ( esops.size() >= 100 )
    {
      std::cout << "[w] terminated " << binary << ": 100 ESOPs have been enumerated" << std::endl;
      break;
    }
  }
  return esops;
}

} // namespace easy::esop

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
