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
  \file bdd.hpp
  \brief Binary decision diagram

  \author Heinz Riener
*/

#pragma once

#include <vector>
#include <stack>
#include <unordered_map>

namespace easy
{

class BDD
{
private:
  struct node_t
  {
    node_t( uint32_t var, uint32_t lo, uint32_t hi )
      : var( var )
      , ref( 0 )
      , dead( 0 )
      , lo( lo )
      , hi( hi )
    {}

    uint64_t var : 12;
    uint64_t ref : 11;
    uint64_t dead : 1;
    uint64_t lo : 20;
    uint64_t hi : 20;
  }; /* node_t */

  enum op_t
  {
    bdd_and
  , bdd_xor
  };

private:
  struct unique_table_hash
  {
    std::size_t operator()( std::pair<uint32_t, uint32_t> const& p ) const
    {
      return 12582917 * p.first + 4256249 * p.second;
    }
  }; /* unique_table_hash */

  struct compute_table_hash
  {
    std::size_t operator()( std::tuple<uint32_t, uint32_t, op_t> const& p ) const
    {
      return 12582917 * std::get<0>( p ) + 4256249 * std::get<1>( p ) + 741457 * static_cast<uint32_t>( std::get<2>( p ) );
    }
  }; /* compute_table_hash */

public:
  using node = uint32_t;
    
  explicit BDD( uint32_t num_vars, uint32_t log_num_objs = 16 )
    : unique_table( num_vars )
  {
    nodes.reserve( 1u << log_num_objs );

    nodes.emplace_back( num_vars, 0u, 0u );
    nodes.emplace_back( num_vars, 1u, 1u );

    for ( auto v = 0u; v < num_vars; ++v )
      ref( unique( v, 0, 1 ) );
  }

  node bot() const
  {
    return 0u;
  }

  node top() const
  {
    return 1u;
  }

  node get_ith_var( uint32_t const i ) const
  {
    return 2u + i;
  }

  void ref( node const& n )
  {
    if ( n > 1u )
    {
      ++(nodes[n].ref);
    }
  }

  void deref( node const& n )
  {
    if ( n > 1u && nodes[n].ref > 0u )
    {
      --(nodes[n].ref);
    }
  }

  uint64_t num_nodes() const
  {
    return nodes.size() - 2u - free.size();
  }

  node compute_and( node f, node g )
  {
    /* trivial cases */
    if ( f == 0 || g == 0 ) return 0;
    if ( f == 1 ) return g;
    if ( g == 1 ) return f;
    if ( f == g ) return f;
    if ( f == !f ) return 0;
    
    if ( f > g )
    {
      std::swap( f, g );
    }
    
    /* lookup in computed cache */
    auto const it = compute_table.find( {f, g, bdd_and} );
    if ( it != compute_table.end() )
    {
      assert( !nodes[it->second].dead );
      return it->second;
    }

    auto const& F = nodes[f];
    auto const& G = nodes[g];

    node r_lo, r_hi;
    if ( F.var < G.var )
    {
      r_lo = compute_and( F.lo, g );
      r_hi = compute_and( F.hi, g );
    }
    else if ( F.var > G.var )
    {
      r_lo = compute_and( f, G.lo );
      r_hi = compute_and( f, G.hi );
    }
    else
    {
      r_lo = compute_and( F.lo, G.lo );
      r_hi = compute_and( F.hi, G.hi );
    }

    auto const var = std::min( F.var, G.var );
    return compute_table[{f,g,bdd_and}] = unique( var, r_lo, r_hi );      
  }

  node compute_xor( node f, node g )
  {
    /* trivial cases */
    if ( f == 1 ) return !g;
    if ( g == 1 ) return !f;
    if ( f == 0 ) return g;
    if ( g == 0 ) return f;
    if ( f == g ) return 0;
    if ( f == !g ) return 1;
    
    if ( f > g )
    {
      std::swap( f, g );
    }
    
    /* lookup in computed cache */
    auto const it = compute_table.find( {f, g, bdd_and} );
    if ( it != compute_table.end() )
    {
      assert( !nodes[it->second].dead );
      return it->second;
    }

    auto const& F = nodes[f];
    auto const& G = nodes[g];

    node r_lo, r_hi;
    if ( F.var < G.var )
    {
      r_lo = compute_xor( F.lo, g );
      r_hi = compute_xor( F.hi, g );
    }
    else if ( F.var > G.var )
    {
      r_lo = compute_xor( f, G.lo );
      r_hi = compute_xor( f, G.hi );
    }
    else
    {
      r_lo = compute_xor( F.lo, G.lo );
      r_hi = compute_xor( F.hi, G.hi );
    }

    auto const var = std::min( F.var, G.var );
    return compute_table[{f,g,bdd_and}] = unique( var, r_lo, r_hi );      
  }

private:
  node unique( uint32_t var, uint32_t lo, uint32_t hi )
  {
    /* BDD reduction rules */
    if ( lo == hi )
    {
      return lo;
    }

    assert( nodes[lo].var > var );
    assert( nodes[hi].var > var );
    
    /* unique table lookup */
    auto const it = unique_table[var].find( {lo,hi} );
    if ( it != unique_table[var].end() )
    {
      assert( !nodes[it->second].dead );
      return it->second;
    }

    /* create new node */
    node r;

    if ( !free.empty() )
    {
      r = free.top();
      free.pop();
      nodes[r].ref = nodes[r].dead = 0;
      nodes[r] = {var,lo,hi};
    }
    else if ( nodes.size() < nodes.capacity() )
    {
      r = nodes.size();
      nodes.emplace_back( var, lo, hi );
    }
    else
    {
      std::cerr << "[e] no more space for new nodes available" << std::endl;
      exit( 1 );
    }

    /* increase ref counts */
    if ( lo > 1 )
    {
      ++(nodes[lo].ref);
    }
    else if ( hi > 1 )
    {
      ++(nodes[hi].ref);
    }
    return unique_table[var][{lo,hi}] = r;
  }
  
private:
  std::vector<node_t> nodes;
  std::stack<node> free;
  std::vector<std::unordered_map<std::pair<uint32_t, uint32_t>, node, unique_table_hash>> unique_table;
  std::unordered_map<std::tuple<uint32_t, uint32_t, op_t>, node, compute_table_hash> compute_table;
}; /* bdd_base */

} /* namespace easy */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
