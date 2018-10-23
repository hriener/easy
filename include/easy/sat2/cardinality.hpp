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

/*!
  \file cardinality.hpp
  \brief Totalizer cardinality constraints

  \author Heinz Riener
*/

#pragma once

#include <deque>

namespace easy::sat2
{

struct totalizer_tree
{
  std::vector<int> vars;
  uint32_t num_inputs;
  std::shared_ptr<totalizer_tree> left;
  std::shared_ptr<totalizer_tree> right; 
}; /* totalizer_tree */

namespace detail
{

inline void add_clause( std::vector<std::vector<int>>& clauses, std::vector<int> const& cl )
{
  clauses.emplace_back( cl );
}

inline void create_totalizer_internal( std::vector<std::vector<int>>& dest, int& sid, std::vector<int> const& ov, uint32_t rhs, std::vector<int> const& av, std::vector<int> const& bv )
{
  /* i = 0 */
  uint32_t kmin = std::min( rhs, uint32_t( bv.size() ) );
  for ( auto j = 0; j < kmin; ++j )
  {
    add_clause( dest, { -bv[j], ov[j] } );
  }

  /* j = 0 */
  kmin = std::min( rhs, uint32_t( av.size() ) );
  for ( auto i = 0; i < kmin; ++i )
  {
    add_clause( dest, { -av[i], ov[i] } );
  }
  
  /* i, j > 0 */
  for ( auto i = 1; i <= kmin; ++i )
  {
    auto min_j = std::min( rhs-i, uint32_t( bv.size() ) );
    for ( auto j = 1; j <= min_j; ++j )
    {
      add_clause( dest, { -av[i-1], -bv[j-1], ov[i+j-1] } );
    }
  }  
}

} /* detail */

inline std::shared_ptr<totalizer_tree> create_totalizer( std::vector<std::vector<int>>& dest, int& sid, std::vector<int> const& lhs, uint32_t rhs )
{
  auto const n = lhs.size();

  std::deque<std::shared_ptr<totalizer_tree>> queue;
  for ( auto i = 0; i < n; ++i )
  {
    std::shared_ptr<totalizer_tree> t( new totalizer_tree() );
    t->vars.resize( 1 );
    t->vars[0] = lhs[i];
    t->num_inputs = 1;

    queue.push_back( t );
  }

  while ( queue.size() > 1 )
  {
    auto const le = queue.front();
    queue.pop_front();

    auto const ri = queue.front();
    queue.pop_front();

    std::shared_ptr<totalizer_tree> t( new totalizer_tree() );
    t->num_inputs = le->num_inputs + ri->num_inputs;
    t->left = le;
    t->right = ri;

    uint32_t kmin = std::min( rhs+1, t->num_inputs );
    t->vars.resize( kmin );
    for ( auto i = 0; i < kmin; ++i )
    {
      t->vars[i] = sid++;
    }
    detail::create_totalizer_internal( dest, sid, t->vars, kmin, le->vars, ri->vars );
    queue.push_back( t );
  }

  return queue.front();
}

} /* namespace easy::sat2 */
