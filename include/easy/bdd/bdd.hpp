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
#include <iostream>

namespace easy
{

class bdd_function
{
public:
  explicit bdd_function( uint32_t node, bool complemented )
    : cmpl( complemented )
    , node( node )
  {
  }

  bool operator==( bdd_function const& other ) const
  {
    return value == other.value;
  }

  bool operator!=( bdd_function const& other ) const
  {
    return this->operator==( other );
  }

  bdd_function operator!() const
  {
    return bdd_function{node, !cmpl};
  }

public:
  union
  {
    uint32_t value;
    struct
    {
      uint32_t cmpl :  1;
      uint32_t node : 19;
    };
  };
}; /* bdd_function */

class bdd_node
{
public:
  explicit bdd_node( uint32_t var, bdd_function lo, bdd_function hi )
    : var( var )
    , ref( 0 )
    , lo( lo.value )
    , hi( hi.value )
  {
  }

  bdd_function get_lo() const
  {
    return bdd_function{ uint32_t( lo ) >> 1u, bool( lo & 1u ) };
  }

  bdd_function get_hi() const
  {
    return bdd_function{ uint32_t( hi ) >> 1u, bool( hi & 1u ) };
  }

public:
  union
  {
    uint64_t value;
    struct
    {
      uint64_t var : 12;
      uint64_t ref : 12;
      uint64_t lo  : 20; /* else */
      uint64_t hi  : 20; /* then */
    };
  };
}; /* bdd_node */

class BDD
{
public:
  using index_t = uint32_t;

private:
  enum op_t
  {
    bdd_and
  , bdd_xor
  };

  struct unique_table_hash
  {
    std::size_t operator()( std::pair<bdd_function, bdd_function> const& p ) const
    {
      return 12582917 * p.first.value + 4256249 * p.second.value;
    }
  }; /* unique_table_hash */

  struct compute_table_hash
  {
    std::size_t operator()( std::tuple<bdd_function, bdd_function, op_t> const& p ) const
    {
      return 12582917 * std::get<0>( p ).value + 4256249 * std::get<1>( p ).value + 741457 * static_cast<uint32_t>( std::get<2>( p ) );
    }
  }; /* compute_table_hash */

public:
  explicit BDD( uint32_t num_vars, uint32_t size )
    : unique_table( num_vars )
  {
    nodes.reserve( 1u << size );
    nodes.emplace_back( num_vars, bot(), bot() );

    for ( auto v = 0u; v < num_vars; ++v )
    {
      ref( unique( v, bot(), top() ) );
    }
  }

  bdd_function bot() const
  {
    return bdd_function{0u, false};
  }

  bdd_function top() const
  {
    return bdd_function{0u, true};
  }

  bdd_function ith_var( uint32_t const i ) const
  {
    return bdd_function{1u + i, false};
  }

  void ref( index_t const& index )
  {
    if ( index > 0u )
    {
      ++( nodes[index].ref );
    }
  }

  void ref( bdd_function const& f )
  {
    ref( f.node );
  }

  void deref( index_t const& index )
  {
    if ( index > 0u && nodes[index].ref > 0u )
    {
      --( nodes[index].ref );
    }
  }

  void deref( bdd_function const& f )
  {
    deref( f.node );
  }

  uint64_t num_nodes() const
  {
    return nodes.size() - 1u;
  }

  bdd_function unique( uint32_t var, bdd_function lo, bdd_function hi )
  {
    /* BDD reduction rules */
    if ( lo == hi )
    {
      return lo;
    }

    assert( nodes[lo.node].var > var );
    assert( nodes[hi.node].var > var );

    /* normalize complemented ELSE-edges */
    bool complemented;
    if ( lo.cmpl )
    {
      complemented = true;
      lo.cmpl = false;
      hi.cmpl = !hi.cmpl;
    }
    else
    {
      complemented = false;
    }

    /* look-up in unique table */
    auto const it = unique_table[var].find( {lo,hi} );
    if ( it != unique_table[var].end() )
    {
      return bdd_function{it->second, complemented};
    }

    index_t r;
    if ( nodes.size() < nodes.capacity() )
    {
      r = nodes.size();
      nodes.emplace_back( var, lo, hi );
    }
    else
    {
      std::cerr << "[e] no more space for new nodes available" << std::endl;
      exit( 1 );
    }

    ref( lo.node );
    ref( hi.node );

    unique_table[var][{lo,hi}] = r;
    return bdd_function{r, complemented};
  }

  bdd_function create_and( bdd_function f, bdd_function g )
  {
    /* trivial cases */
    if ( f == bot() || g == bot() ) return bot();
    if ( f == top() ) return g;
    if ( g == top() ) return f;
    if ( f == g ) return f;
    if ( f == !f ) return bot();

    if ( f.node > g.node )
    {
      std::swap( f, g );
    }

    /* normalize complemented ELSE-edges */
    bool complemented;
    if ( f.cmpl )
    {
      complemented = true;
      f.cmpl = !f.cmpl;
      g.cmpl = false;
    }
    else
    {
      complemented = false;
    }

    /* look-up in computed cache */
    auto const it = compute_table.find( {f, g, bdd_and} );
    if ( it != compute_table.end() )
    {
      return bdd_function{it->second, complemented};
    }

    auto const& F = nodes[f.node];
    auto const& G = nodes[g.node];

    bdd_function lo = bot(), hi = bot();
    if ( F.var < G.var )
    {
      lo = create_and( F.get_lo(), g );
      hi = create_and( F.get_hi(), g );
    }
    else if ( F.var > G.var )
    {
      lo = create_and( f, G.get_lo() );
      hi = create_and( f, G.get_hi() );
    }
    else
    {
      lo = create_and( F.get_lo(), G.get_lo() );
      hi = create_and( F.get_hi(), G.get_hi() );
    }

    auto const var = std::min( F.var, G.var );

    auto h = unique( var, lo, hi );

    compute_table[{f, g, bdd_and}] = h.node;
    h.cmpl = h.cmpl ^ complemented;
    return h;
  }

  bdd_function create_xor( bdd_function f, bdd_function g )
  {
    /* trivial cases */
    if ( f == top() ) return !g;
    if ( g == top() ) return !f;
    if ( f == bot() ) return g;
    if ( g == bot() ) return f;

    if ( f == g ) return bot();
    if ( f == !g ) return top();

    if ( f.node > g.node )
    {
      std::swap( f, g );
    }

    /* normalize complemented EDSE-edges */
    bool complemented;
    if ( f.cmpl )
    {
      complemented = true;
      f.cmpl = !f.cmpl;
      g.cmpl = false;
    }
    else
    {
      complemented = false;
    }

    /* look-up in computed cache */
    auto const it = compute_table.find( {f, g, bdd_xor} );
    if ( it != compute_table.end() )
    {
      return bdd_function{it->second, complemented};
    }

    auto const& F = nodes[f.node];
    auto const& G = nodes[g.node];

    bdd_function lo = bot(), hi = bot();
    if ( F.var < G.var )
    {
      lo = create_xor( F.get_lo(), g );
      hi = create_xor( F.get_hi(), g );
    }
    else if ( F.var > G.var )
    {
      lo = create_xor( f, G.get_lo() );
      hi = create_xor( f, G.get_hi() );
    }
    else
    {
      lo = create_xor( F.get_lo(), G.get_lo() );
      hi = create_xor( F.get_hi(), G.get_hi() );
    }

    auto const var = std::min( F.var, G.var );

    auto h = unique( var, lo, hi );

    compute_table[{f, g, bdd_xor}] = h.node;
    h.cmpl = h.cmpl ^ complemented;
    return h;
  }

protected:
  std::vector<bdd_node> nodes;
  std::vector<std::unordered_map<std::pair<bdd_function,bdd_function>, index_t, unique_table_hash>> unique_table;
  std::unordered_map<std::tuple<bdd_function, bdd_function, op_t>, index_t, compute_table_hash> compute_table;
}; /* BDD */

} /* namespace easy */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
