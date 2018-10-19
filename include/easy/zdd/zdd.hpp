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
  \file zdd.hpp
  \brief Implementation of ZDDs (highly inspired by the implementation in ABC)

  \author Heinz Riener
*/

#pragma once

#include <iostream>
#include <numeric>

namespace easy::zdd
{

/* https://stackoverflow.com/questions/21438631/fastest-way-of-computing-the-power-that-a-power-of-2-number-used */
int ilog2( int value )
{
  static constexpr int twos[] = {
    1<<0,  1<<1,  1<<2,  1<<3,  1<<4,  1<<5,  1<<6,  1<<7,
    1<<8,  1<<9,  1<<10, 1<<11, 1<<12, 1<<13, 1<<14, 1<<15,
    1<<16, 1<<17, 1<<18, 1<<19, 1<<20, 1<<21, 1<<22, 1<<23,
    1<<24, 1<<25, 1<<26, 1<<27, 1<<28, 1<<29, 1<<30, 1<<31
  };

  return std::lower_bound( std::begin( twos ), std::end( twos ), value ) - std::begin( twos );
}

struct zdd_object
{
  uint32_t var  : 31;
  uint32_t mark : 1;
  uint32_t t;
  uint32_t f;
}; // zdd_object

struct zdd_cache_entry
{
  int32_t arg0;
  int32_t arg1;
  int32_t arg2;
  int32_t res;
}; // zdd_cache_entry

uint64_t zdd_hash( int32_t arg0, int32_t arg1, int32_t arg2 )
{
  return uint64_t(12582917) * arg0 + 4256249 * arg1 + 741457 * arg2;
}

struct zdd_statistics
{
  uint64_t num_cache_lookups = 0;
  uint64_t num_cache_misses = 0;
};

class Zdd
{
public:
  enum op
  {
    ZDD_DIFF,
    ZDD_UNION,
    ZDD_INTERSECT,
    ZDD_DOT_PRODUCT,
    ZDD_PATHS
  };

  explicit Zdd( uint32_t alloc_objects )
    : num_variables( 0u )
    , num_objects( 0u )
    , unique_mask((1 << ilog2(alloc_objects))-1)
    , cache_mask((1 << ilog2(alloc_objects))-1)
    , unique( unique_mask + 1 )
    , next( alloc_objects )
    , cache( cache_mask + 1 )
    , objects( alloc_objects )
  {
    /* create constants */
    objects[0].var = num_variables;
    objects[1].var = num_variables;
    num_objects = 2u;

    assert( num_objects == 2u );
  }

  explicit Zdd( uint32_t alloc_variables, uint32_t alloc_objects )
    : num_variables( alloc_variables )
    , num_objects( 0u )
    , unique_mask((1 << ilog2(alloc_objects))-1)
    , cache_mask((1 << ilog2(alloc_objects))-1)
    , unique( unique_mask + 1 )
    , next( alloc_objects )
    , cache( cache_mask + 1 )
    , objects( alloc_objects )
  {
    /* create constants */
    objects[0].var = num_variables;
    objects[1].var = num_variables;
    num_objects = 2u;

    /* create variables */
    for ( auto i = 0; i < num_variables; ++i )
      unique_create( i, 1, 0 );

    assert( num_objects == num_variables + 2u );
  }

  int32_t get_constant( bool value = false )
  {
    assert( num_objects > int32_t( value ) );
    return value ? 1 : 0;
  }

  int32_t get_ith_var( int32_t index )
  {
    assert( num_objects > 2u + index );
    return 2u + index;
  }

  zdd_object& zdd_from_index( int32_t index )
  {
    return objects.at( index );
  }

  zdd_object const& zdd_from_index( int32_t index ) const
  {
    return objects.at( index );
  }

  int32_t index_from_zdd( zdd_object const& z )
  {
    return std::distance( std::begin( objects ), std::find_if( std::begin( objects ), std::end( objects ),
                          [&z]( const auto& arg ){ return z.var == arg.var; } ) );
  }

  int32_t unique_create( int32_t var, int32_t t, int32_t f )
  {
    assert( var >= 0 && var < num_variables );
    assert( var < zdd_from_index( t ).var );
    assert( var < zdd_from_index( f ).var );

    if ( t == 0 )
      return f;

    int32_t q = unique.at( zdd_hash( var, t, f ) & unique_mask );
    for ( ; q > 0; q = next.at( q ) )
    {
      auto const obj = objects.at( q );
      if ( obj.var == uint32_t( var ) &&
           obj.t == uint32_t( t ) &&
           obj.f == uint32_t( f ) )
        return q;
    }

    /* abort because the number of nodes exceeded */
    if ( num_objects == objects.size() )
    {
      std::cout << "[w] aborting because the number of nodes exceeded "
                << num_objects << std::endl;
    }

    assert( num_objects < objects.size() );

    auto const n = num_objects++;
    zdd_object& obj = objects[n];
    obj.var = var;
    obj.t = t;
    obj.f = f;

    // std::cout << "[i] added node " << n << ":"
    //           << " Var = " << var
    //           << " True = " << t
    //           << " False = " << f
    //           << std::endl;

    return n;
  }

  int32_t zdd_union( int32_t a, int32_t b )
  {
    int32_t r0, r1, r;
    if ( a == 0 ) return b;
    if ( b == 0 ) return a;
    if ( a == b ) return a;
    if ( a > b ) return zdd_union( b, a );
    if ( ( r = cache_lookup( a, b, op::ZDD_UNION ) ) >= 0 )
      return r;
    auto zdd_a = objects.at( a );
    auto zdd_b = objects.at( b );
    if ( zdd_a.var < zdd_b.var )
    {
      r0 = zdd_union( zdd_a.f, b );
      r1 = zdd_a.t;
    }
    else if ( zdd_a.var > zdd_b.var )
    {
      r0 = zdd_union( a, zdd_b.f );
      r1 = zdd_b.t;
    }
    else
    {
      r0 = zdd_union( zdd_a.f, zdd_b.f );
      r1 = zdd_union( zdd_a.t, zdd_b.t );
    }
    r = unique_create( std::min( zdd_a.var, zdd_b.var ), r1, r0 );
    return cache_insert( a, b, op::ZDD_UNION, r );
  }

  int32_t zdd_union( std::vector<int32_t> const& vs )
  {
    return std::reduce( vs.begin(), vs.end(), get_constant( false ),
                        [&]( const auto& a, const auto& b ){ return zdd_union( a, b ); } );
  }

  int32_t zdd_intersect( int32_t a, int32_t b )
  {
    int32_t r0, r1, r;
    if ( a == 0 ) return 0;
    if ( b == 0 ) return 0;
    if ( a == b ) return a;
    if ( a > b ) return zdd_intersect( b, a );
    if ( ( r = cache_lookup( a, b, ZDD_INTERSECT ) ) >= 0 )
      return r;
    auto zdd_a = objects.at( a );
    auto zdd_b = objects.at( b );
    if ( zdd_a.var < zdd_b.var )
    {
      r0 = zdd_intersect( zdd_a.var, b );
      r1 = zdd_a.t;
    }
    else if ( zdd_a.var > zdd_b.var )
    {
      r0 = zdd_intersect( a, zdd_b.f );
      r1 = zdd_b.t;
    }
    else
    {
      r0 = zdd_intersect( zdd_a.f, zdd_b.f );
      r1 = zdd_intersect( zdd_a.t, zdd_b.t );
    }
    r = unique_create( std::min( zdd_a.var, zdd_b.var ), r1, r0 );
    return cache_insert( a, b, ZDD_INTERSECT, r );
  }

  int32_t zdd_intersect( std::vector<int32_t> const& vs )
  {
    if ( vs.begin() == vs.end() )
    {
      return get_constant( false );
    }
    return std::reduce( vs.begin()+1, vs.end(), vs[0],
                        [&]( const auto& a, const auto& b ){ return zdd_intersect( a, b ); } );
  }

  int32_t zdd_dot_product( int32_t a, int32_t b )
  {
    int32_t r0, r1, r;
    if ( a == 0 ) return 0;
    if ( b == 0 ) return 0;
    if ( a == 1 ) return b;
    if ( b == 1 ) return a;
    if ( a > b ) return zdd_dot_product( b, a );
    if ( ( r = cache_lookup( a, b, op::ZDD_DOT_PRODUCT ) ) >= 0 )
      return r;
    auto zdd_a = objects.at( a );
    auto zdd_b = objects.at( b );
    if ( zdd_a.var < zdd_b.var )
    {
      r0 = zdd_dot_product( zdd_a.f, b );
      r1 = zdd_dot_product( zdd_a.t, b );
    }
    else if ( zdd_a.var > zdd_b.var )
    {
      r0 = zdd_dot_product( a, zdd_b.f );
      r1 = zdd_dot_product( a, zdd_b.t );
    }
    else
    {
      r0 = zdd_dot_product( zdd_a.f, zdd_b.f );
      auto const b2 = zdd_union( zdd_b.f, zdd_b.t );
      auto const t1 = zdd_dot_product( zdd_a.t, b2 );
      auto const t2 = zdd_dot_product( zdd_a.f, zdd_b.t );
      r1 = zdd_union( t1, t2 );
    }
    r = unique_create( std::min( zdd_a.var, zdd_b.var ), r1, r0 );
    return cache_insert( a, b, ZDD_DOT_PRODUCT, r );
  }

  int32_t zdd_dot_product( std::vector<int32_t> const& vs )
  {
    return std::reduce( vs.begin(), vs.end(), get_constant( false ),
                        [&]( const auto& a, const auto& b ){ return zdd_dot_product( a, b ); } );
  }

  int32_t zdd_diff( int32_t a, int32_t b )
  {
    int32_t r0, r1, r;
    if ( a == 0 ) return 0;
    if ( b == 0 ) return a;
    if ( a == b ) return 0;
    if ( ( r = cache_lookup( a, b, ZDD_DIFF ) >= 0 ) )
      return r;
    auto zdd_a = objects.at( a );
    auto zdd_b = objects.at( b );
    if ( zdd_a.var < zdd_b.var )
    {
      r0 = zdd_diff( zdd_a.f, b );
      r = unique_create( zdd_a.var, zdd_a.t, r0 );
    }
    else if ( zdd_a.var > zdd_b.var )
    {
      r = zdd_diff( a, zdd_b.f );
    }
    else
    {
      r0 = zdd_diff( zdd_a.f, zdd_b.f );
      r1 = zdd_diff( zdd_a.t, zdd_b.t );
      r = unique_create( zdd_a.var, zdd_a.t, r0 );
    }
    return cache_insert( a, b, ZDD_DIFF, r );
  }

  int32_t zdd_sym_diff( int32_t a, int32_t b )
  {
    return zdd_union( zdd_diff( a, b ), zdd_diff( b, a ) );
  }

  int32_t zdd_sym_diff( std::vector<int32_t> const& vs )
  {
    return std::reduce( vs.begin(), vs.end(), get_constant( false ),
                        [&]( const auto& a, const auto& b ){ return zdd_sym_diff( a, b ); } );
  }

  void print_cover_recur( int32_t a, std::string current )
  {
    if ( a == 1 )
    {
      // std::cout << "[+]: " << current << std::endl;
      std::cout << current << ' ';
      return;
    }
    else if ( a == 0 )
    {
      // std::cout << "[-]: " << current << std::endl;
      return;
    }

    auto zdd_a = objects.at( a );
    current[int32_t( zdd_a.var / 2 )] = ( zdd_a.var % 2 ) == 1 ? '1' : '-';
    print_cover_recur( zdd_a.f, current );
    current[int32_t( zdd_a.var / 2 )] = ( zdd_a.var % 2 ) == 0 ? '1' : '0';
    print_cover_recur( zdd_a.t, current );
  }

  void print_cover( int32_t a, uint32_t num_vars )
  {
    std::cout << "{ ";
    print_cover_recur( a, std::string( '-', num_vars ) );
    std::cout << "}";
  }

  int32_t zdd_count_paths( int32_t a )
  {
    int r;
    if ( a < 2 ) return a;
    if ( ( r = cache_lookup( a, 0, ZDD_PATHS ) ) >= 0 )
      return r;
    auto zdd_a = objects.at( a );
    r = zdd_count_paths( zdd_a.f ) + zdd_count_paths( zdd_a.t );
    return cache_insert( a, 0, ZDD_PATHS, r );
  }

  int32_t zdd_count_nodes( int32_t a )
  {
    int32_t count = zdd_count_rec( a );
    zdd_unmark_rec( a );
    return count;
  }

private:
  int32_t zdd_count_rec( int32_t a )
  {
    if ( a < 2 )
      return 0;
    auto zdd_a = objects.at( a );
    if ( zdd_a.mark )
      return 0;
    zdd_a.mark = 1;
    return 1 + zdd_count_rec( zdd_a.f ) + zdd_count_rec( zdd_a.t );
  }

  void zdd_unmark_rec( int32_t a )
  {
    if ( a < 2 )
      return;
    auto zdd_a = objects.at( a );
    if ( !zdd_a.mark )
      return;
    zdd_a.mark = 0;
    zdd_unmark_rec( zdd_a.f );
    zdd_unmark_rec( zdd_a.t );
  }

  int32_t cache_lookup( int32_t arg0, int32_t arg1, int32_t arg2 )
  {
    zdd_cache_entry const& entry = cache.at( zdd_hash( arg0, arg1, arg2 ) & cache_mask );
    ++stats.num_cache_lookups;
    return ( entry.arg0 == arg0 && entry.arg1 == arg1 && entry.arg2 == arg2 ) ? entry.res : -1;
  }

  int32_t cache_insert( int32_t arg0, int32_t arg1, int32_t arg2, int32_t res )
  {
    assert( res >= 0 );
    cache[zdd_hash( arg0, arg1, arg2 ) & cache_mask] = zdd_cache_entry{arg0,arg1,arg2,res};
    ++stats.num_cache_misses;
    return res;
  }

#if 0
  int32_t cache_unique_lookup( int32_t var, int32_t t, int32_t f )
  {
    for ( int32_t q = unique.at( ( zdd_hash( var, t, f ) & unique_mask ) );
          q > 0; q = next.at( q ) )
    {
      auto const obj = objects.at( q );
      if ( obj.var == uint32_t( var ) &&
           obj.t == uint32_t( t ) &&
           obj.f == uint32_t( f ) )
        return q;
    }

    return 0;
  }
#endif

private:
  zdd_statistics stats;

  uint32_t num_variables;
  uint32_t num_objects;
  uint32_t cache_mask;
  uint32_t unique_mask;
  std::vector<int32_t> unique;
  std::vector<int32_t> next;
  std::vector<zdd_cache_entry> cache;
  std::vector<zdd_object> objects;
};

} // namespace easy::zdd

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
