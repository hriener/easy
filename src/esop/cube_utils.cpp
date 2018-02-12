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

#if defined(KITTY_EXTENSION)

#include <esop/cube_utils.hpp>

namespace esop
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

static uint64_t pow3[32] = {
    0x1
  , 0x3
  , 0x9
  , 0x1b
  , 0x51
  , 0xf3
  , 0x2d9
  , 0x88b
  , 0x19a1
  , 0x4ce3
  , 0xe6a9
  , 0x2b3fb
  , 0x81bf1
  , 0x1853d3
  , 0x48fb79
  , 0xdaf26b
  , 0x290d741
  , 0x7b285c3
  , 0x17179149
  , 0x4546b3db
  , 0xcfd41b91
  , 0x26f7c52b3
  , 0x74e74f819
  , 0x15eb5ee84b
  , 0x41c21cb8e1
  , 0xc546562aa3
  , 0x24fd3027fe9
  , 0x6ef79077fbb
  , 0x14ce6b167f31
  , 0x3e6b41437d93
  , 0xbb41c3ca78b9
  , 0x231c54b5f6a2b
}; /* pow3 */

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

uint64_t cube_weight( const kitty::cube& c, unsigned num_vars )
{
  uint64_t value = 0;
  for ( auto i = 0u; i < num_vars; ++i )
  {
    value += c.get_mask( i ) ? ( c.get_bit( i ) ? pow3[ i ] : 0 ) : 2*pow3[ i ];
  }
  return value;
}

cube_weight_compare::cube_weight_compare( unsigned num_vars )
  : _num_vars( num_vars )
{}

bool cube_weight_compare::operator()( const kitty::cube& a, const kitty::cube& b ) const
{
  return cube_weight( a, _num_vars ) < cube_weight( b, _num_vars );
}

std::vector<std::vector<kitty::cube>> combinations( const std::vector<kitty::cube>& e, std::size_t t )
{
  const auto n = e.size();
  if ( n == t ) return { e };

  assert( n > t );

  std::vector<std::vector<kitty::cube>> result;

  // T1 [initialize.]
  int j, x;
  std::vector<int> c( t + 2 );
  for ( auto j = 0u; j < t; ++j )
  {
    c[ j ] = j;
  }
  c[ t ] = n;
  c[ t+1 ] = 0;

  j = t;

  while ( true )
  {
    // T2 [visit.]
    std::vector<kitty::cube> v;
    for ( auto i = 0u; i < t; ++i )
    {
      v.push_back( e[ c[ i ] ] );
    }
    result.push_back( v );

    if ( j > 0 )
    {
      x = j;

      // T6 [increase c_j.]
      c[ j-1 ] = x;
      j = j-1;
      continue;
    }

    // T3 [easy case.]
    if ( c[ 0 ] + 1 < c[ 1 ] )
    {
      c[ 0 ] = c[ 0 ] + 1;
      continue;
    }
    else
    {
      j = 2;
    }

    // T4 [find j.]
    while ( true )
    {
      c[ j-2 ] = j - 2;
      x = c[ j-1 ] + 1;
      if ( x != c[ j ] )
      {
	break;
      }
      j = j + 1;
    }

    // T5 [done?]
    if ( j > int(t) )
    {
      break;
    }

    c[ j-1 ] = x;
    j = j - 1;
  }

  return result;
}

} /* esop */

#endif /* KITTY_EXTENSION */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
