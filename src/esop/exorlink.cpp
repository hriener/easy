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

#include <esop/exorlink.hpp>
#include <cassert>

namespace esop
{

/**
 * The implementation is based on Bruno Schmitt's implementation of ``exorlink`` in
 *   https://raw.githubusercontent.com/boschmitt/exorcism/master/source/cube32.cpp
 */
std::array<kitty::cube,4> exorlink( kitty::cube c0, kitty::cube c1, std::uint32_t distance, std::uint32_t *group )
{
  assert( distance <= 4 );
  const auto diff = c0.difference( c1 );

  std::array<kitty::cube, 4> result;
  if ( c1 < c0 ) std::swap( c0, c1 );

  const auto bits = ~( c0._bits ) & ~( c1._bits );
  const auto mask = c0._mask ^ c1._mask;

  for ( auto i = 0; i < distance; ++i )
  {
    auto tmp_bits = c0._bits;
    auto tmp_mask = c0._mask;
    auto tmp_pos = diff;

    for ( auto j = 0; j < distance; ++j )
    {
      /* compute next position */
      std::uint64_t p = tmp_pos & -tmp_pos;
      tmp_pos &= tmp_pos - 1;
      switch (*group++)
      {
      case 0:
        /* take from c0 */
        break;
      case 1:
        /* take from c1 */
        tmp_bits ^= ((c1._bits & p) ^ tmp_bits) & p;
        tmp_mask ^= ((c1._mask & p) ^ tmp_mask) & p;
        break;
      case 2:
        /* take other */
        tmp_bits ^= ((bits & p) ^ tmp_bits) & p;
        tmp_mask ^= ((mask & p) ^ tmp_mask) & p;
        break;
      }
    }
    result[i]._bits = tmp_bits;
    result[i]._mask = tmp_mask;
  }

  return result;
}

std::array<kitty::cube,4> exorlink4( const kitty::cube& c0, const kitty::cube& c1, uint32_t offset )
{
  std::uint32_t *group = &cube_groups4[ offset ];
  const auto diff = c0.difference( c1 );

  std::array<kitty::cube, 4> result;
  const auto bits = ~( c0._bits ) & ~( c1._bits );
  const auto mask = c0._mask ^ c1._mask;

  if ( c0 < c1 )
  {
    for ( auto i = 0; i < 4; ++i )
    {
      auto tmp_bits = c0._bits;
      auto tmp_mask = c0._mask;
      auto tmp_pos = diff;

      for ( auto j = 0; j < 4; ++j )
      {
        /* compute next position */
        std::uint64_t p = tmp_pos & -tmp_pos;
        tmp_pos &= tmp_pos - 1;
        switch ( *group++ )
        {
        case 0:
          /* take from c0 */
          break;
        case 1:
          {
          /* take from c1 */
          tmp_bits ^= ((c1._bits & p) ^ tmp_bits) & p;
          tmp_mask ^= ((c1._mask & p) ^ tmp_mask) & p;
          }
          break;
        case 2:
          /* take other */
          tmp_bits ^= ((bits & p) ^ tmp_bits) & p;
          tmp_mask ^= ((mask & p) ^ tmp_mask) & p;
          break;
        }
      }
      result[i]._bits = tmp_bits;
      result[i]._mask = tmp_mask;
    }
  }
  else
  {
    for ( auto i = 0; i < 4; ++i )
    {
      auto tmp_bits = c1._bits;
      auto tmp_mask = c1._mask;
      auto tmp_pos = diff;

      for ( auto j = 0; j < 4; ++j )
      {
        /* compute next position */
        std::uint64_t p = tmp_pos & -tmp_pos;
        tmp_pos &= tmp_pos - 1;
        switch ( *group++ )
        {
        case 0:
          /* take from c0 */
          break;
        case 1:
          /* take from c1 */
          tmp_bits ^= ((c0._bits & p) ^ tmp_bits) & p;
          tmp_mask ^= ((c0._mask & p) ^ tmp_mask) & p;
          break;
        case 2:
          /* take other */
          tmp_bits ^= ((bits & p) ^ tmp_bits) & p;
          tmp_mask ^= ((mask & p) ^ tmp_mask) & p;
          break;
        }
      }
      result[i]._bits = tmp_bits;
      result[i]._mask = tmp_mask;
    }
  }

  return result;
}

} /* esop */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
