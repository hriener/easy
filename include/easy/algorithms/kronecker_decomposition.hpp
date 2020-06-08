/* easy: C++ ESOP library
 * Copyright (C) 2018-2020  EPFL
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

#include <kitty/kitty.hpp>

#include <unordered_set>
#include <vector>

enum class decomposition_type
{
  positive_davio,
  negative_davio,
  shannon
};

namespace detail
{

template<typename TT>
inline void kronecker_decomposition_rec( std::unordered_set<kitty::cube, kitty::hash<kitty::cube>>& esop, TT const& tt, std::vector<decomposition_type> const& decomps,
                                         uint8_t var_index, kitty::cube const& c )
{
  /* terminal cases */
  if ( is_const0( tt ) )
  {
    return;
  }
  if ( is_const0( ~tt ) )
  {
    kitty::detail::add_to_cubes( esop, c );
    return;
  }

  auto const tt0 = cofactor0( tt, var_index );
  auto const tt1 = cofactor1( tt, var_index );

  switch ( decomps[var_index] )
  {
  case decomposition_type::positive_davio:
    kronecker_decomposition_rec( esop, tt0, decomps, var_index + 1, c );
    kronecker_decomposition_rec( esop, tt0 ^ tt1, decomps, var_index + 1, kitty::detail::with_literal( c, var_index, true ) );
    break;
  case decomposition_type::negative_davio:
    kronecker_decomposition_rec( esop, tt1, decomps, var_index + 1, c );
    kronecker_decomposition_rec( esop, tt0 ^ tt1, decomps, var_index + 1, kitty::detail::with_literal( c, var_index, false ) );
    break;
  case decomposition_type::shannon:
    kronecker_decomposition_rec( esop, tt0, decomps, var_index + 1, kitty::detail::with_literal( c, var_index, false ) );
    kronecker_decomposition_rec( esop, tt1, decomps, var_index + 1, kitty::detail::with_literal( c, var_index, true ) );
    break;
  }
}

}
template<typename TT>
inline std::vector<kitty::cube> kronecker_decomposition( TT const& tt, std::vector<decomposition_type> const& decomps )
{
  assert( tt.num_vars() == decomps.size() );

  std::unordered_set<kitty::cube, kitty::hash<kitty::cube>> cubes;
  detail::kronecker_decomposition_rec( cubes, tt, decomps, 0, kitty::cube() );
  return std::vector<kitty::cube>( cubes.begin(), cubes.end() );
}
