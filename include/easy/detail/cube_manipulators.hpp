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
  \file constructors.hpp
  \brief Implements constructors for exclusive-or sum-of-product forms.

  \author Mathias Soeken
  \author Winston Haaswijk
  \author Heinz Riener
*/

#pragma once

#include <kitty/cube.hpp>
#include <unordered_set>
#include <cassert>

namespace easy::detail
{

inline void add_to_cubes( std::unordered_set<kitty::cube, kitty::hash<kitty::cube>>& pkrm, const kitty::cube& c, bool distance_one_merging = true )
{
  /* first check whether cube is already contained; if so, delete it */
  const auto it = pkrm.find( c );
  if ( it != pkrm.end() )
  {
    pkrm.erase( it );
    return;
  }

  /* otherwise, check if there is a distance-1 cube; if so, merge it */
  if ( distance_one_merging )
  {
    for ( auto it = pkrm.begin(); it != pkrm.end(); ++it )
    {
      if ( c.distance( *it ) == 1 )
      {
        auto new_cube = c.merge( *it );
        pkrm.erase( it );
        add_to_cubes( pkrm, new_cube );
        return;
      }
    }
  }

  /* otherwise, just add the cube */
  pkrm.insert( c );
}

inline kitty::cube with_literal( const kitty::cube& c, uint8_t var_index, bool polarity )
{
  auto copy = c;
  copy.add_literal( var_index, polarity );
  return copy;
}

/*! \brief Increment cube

  Increment cube for tenerary counting, e.g.,
    ...00 -> ...01 -> ...0- -> ...10 -> ...1- -> ...-0 -> ...-1 -> ...--

  \param m Cube
  \param num_vars Number of variables
*/
inline void incr_cube( kitty::cube& m, uint32_t num_vars )
{
  for ( auto i = 0u; i < num_vars; ++i )
  {
    auto const pos = num_vars - i - 1;
    auto const bit = m.get_bit( pos );
    auto const mask = m.get_mask( pos );

    if ( mask )
    {
      if ( !bit )
      {
        m.set_bit( pos );
      }
      else
      {
        m.clear_bit( pos );
        m.clear_mask( pos );
      }
      break;
    }
    else
    {
      assert( !mask && !bit && "unsupported case treated as don't care" );
      m.clear_bit( pos );
      m.set_mask( pos );
      continue;
    }
  }
}

/*! \brief Greater equal for ternary cubes.

  Compares two cubes bit-wise wrt. binary relation R = { (0,0), (1,1),
  (-,-), (-,0), (-,1) }.

  \param a Cube
  \param b Cube
  \param num_vars Number of variables
*/
inline bool compare( kitty::cube const& a, kitty::cube const& b, uint32_t num_vars )
{
  for ( auto i = 0u; i < num_vars; ++i )
  {
    auto const mask_a = a.get_mask( num_vars - i - 1 );
    auto const mask_b = b.get_mask( num_vars - i - 1 );
    if ( !mask_a )
      continue;

    auto const value_a = a.get_bit( num_vars - i - 1 );
    auto const value_b = b.get_bit( num_vars - i - 1 );
    if ( mask_b && ( value_a == value_b ) )
      continue;

    return false;
  }
  return true;
}

/*! \brief Combine partitioned cubes

  Combines two partitioned cubes of (n-r)-variables and a r-variables
  into an n-variable cube.

  \param a Cube
  \param b Cube
  \param n Integer
  \param r Integer
  \param num_vars Number of variables
*/
inline kitty::cube combine( kitty::cube const& a, kitty::cube const& b, uint32_t n, uint32_t r )
{
  kitty::cube combined( a );
  for ( auto i = 0u; i < r; ++i )
  {
    auto const v = b.get_bit( i );
    auto const m = b.get_mask( i );
    if ( m )
    {
      if ( v )
      {
        combined.set_bit( n - r + i );
        combined.set_mask( n - r + i );
      }
      else
      {
        combined.clear_bit( n - r + i );
        combined.set_mask( n - r + i );
      }
    }
    else
    {
      combined.clear_bit( n - r + i );
      combined.clear_mask( n - r + i );
    }
  }
  return combined;
}

} // namespace easy::detail
