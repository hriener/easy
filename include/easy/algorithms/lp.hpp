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

#include <algorithm>
#include <vector>

namespace detail
{

static constexpr uint32_t pow3[] = { 1u, 3u, 9u, 27u, 81u, 243u, 729u };

class extended_truth_table
{
public:
  extended_truth_table( uint32_t num_vars )
    : _num_vars( num_vars )
  {
    assert( num_vars > 0u && num_vars <= 6u );
    _bits.resize( 1u + ( pow3[num_vars] >> 6u ) );
  }

  uint64_t num_packets() const
  {
    return _bits.size();
  }

  uint64_t num_vars() const
  {
    return _num_vars;
  }

  uint64_t num_bits() const
  {
    return pow3[_num_vars];
  }

  bool get_bit( uint64_t index ) const
  {
    assert( index < num_bits() );
    uint64_t const packet = index >> 6u;
    uint64_t const offset = index % 64u;
    return ( _bits[packet] >> offset ) & 1u;
  }

  void set_bit( uint64_t index )
  {
    assert( index < num_bits() );
    uint64_t const packet = index >> 6u;;
    uint64_t const offset = index % 64ul;
    _bits[packet] |= ( 1ul << offset );
  }

  void clear_bit( uint64_t index )
  {
    assert( index < num_bits() );
    uint64_t const packet = index >> 6u;
    uint64_t const offset = index % 64u;
    _bits[packet] &= ~( 1ul << offset );
  }

  void print_binary() const
  {
    for ( auto i = 0u; i < num_bits(); ++i )
    {
      std::cout << get_bit( i );
    }
  }

public:
  uint64_t _num_vars;
  std::vector<uint64_t> _bits;
};

template<typename TT>
inline extended_truth_table create_extended_truth_table( TT const& tt )
{
  auto const num_vars = tt.num_vars();
  extended_truth_table ett( num_vars );

  /* sort ett indices by the number of 2s */
  std::vector<std::vector<uint32_t>> indices( num_vars+1u );
  for ( auto i = 0u; i < ( pow3[num_vars] ); ++i )
  {
    uint32_t count_2s = 0u;
    uint32_t value = i;
    while ( value > 0u )
    {
      if ( ( value % 3 ) == 2u )
      {
        ++count_2s;
      }
      value = value / 3;
    }

    indices[count_2s].emplace_back( i );
  }

  /* fill entries with no 2s */
  for ( auto i = 0u; i < ( 1ul << num_vars ); ++i )
  {
    uint64_t tt_index = 0u;
    uint64_t ett_index = 0u;

    for ( auto j = 0u; j < num_vars; ++j )
    {
      bool const entry = ( i >> j ) & 1u;
      if ( entry )
      {
        ett_index += pow3[j];
        tt_index += 1ul << j;
      }
    }

    if ( kitty::get_bit( tt, tt_index ) )
    {
      ett.set_bit( ett_index );
    }
    else
    {
      ett.clear_bit( ett_index );
    }
  }

  /* fill other entries in ascending order, first those with 1x 2, then those with 2x 2, and so forth */
  for ( auto i = 1u; i < num_vars + 1u; ++i )
  {
    for ( auto const& ett_index : indices[i] )
    {
      uint64_t ett_index0{0u};
      uint64_t ett_index1{0u};

      uint32_t value = ett_index;
      uint32_t var_index = 0u;

      bool first_two = true;
      while ( value > 0u )
      {
        if ( ( value % 3 ) == 2u )
        {
          if ( first_two )
          {
            ett_index0 += 0*(pow3[var_index]);
            ett_index1 += 1*(pow3[var_index]);
            first_two = false;
          }
          else
          {
            ett_index0 += 2u*pow3[var_index];
            ett_index1 += 2u*pow3[var_index];
          }
        }
        else if ( ( value % 3 ) == 1u )
        {
          ett_index0 += pow3[var_index];
          ett_index1 += pow3[var_index];
        }

        value /= 3;
        ++var_index;
      }

      if ( ett.get_bit( ett_index0 ) ^ ett.get_bit( ett_index1 ) )
      {
        ett.set_bit( ett_index );
      }
    }
  }

  return ett;
}

inline std::vector<uint8_t> create_extended_weight_table( extended_truth_table const& ett )
{
  std::vector<uint8_t> w( ett.num_bits() );
  std::vector<uint8_t> w_tmp( ett.num_bits() );

  for ( auto i = 0u; i < ett.num_bits(); ++i )
  {
    if ( ett.get_bit( i ) )
    {
      w[i] = 1;
    }
    else
    {
      w[i] = 0;
    }
  }

  auto const n = ett.num_vars();
  for ( auto k = 0u; k < n; ++k )
  {
    for ( auto j = 0u; j < pow3[n - k - 1]; ++j )
    {
      for ( auto i = 0u; i < pow3[k]; ++i )
      {
        auto const i0 = pow3[k] * (3*j + 0) + i;
        auto const i1 = pow3[k] * (3*j + 1) + i;
        auto const i2 = pow3[k] * (3*j + 2) + i;

        w_tmp[i0] = w[i0] + w[i2];
        w_tmp[i1] = w[i1] + w[i2];
        w_tmp[i2] = w[i0] + w[i1];
      }
    }

    w = w_tmp;
  }

  return w;
}

} /* detail */

/*! \brief LP characteristic vector

   This algorithm computes the LP characteristic vector of a truth
   table using the method described in [N. Koda and T. Sasao, RM
   Workshop, 1993].

   \param Truth table
   \return LP characteristic vector
 */
template<typename TT>
std::vector<uint8_t> lp_characteristic_vector( TT const& tt )
{
  // static_assert( is_complete_truth_table<TT>::value, "Can only be applied on complete truth tables." );

  auto const ett = detail::create_extended_truth_table( tt );
  auto ewt = detail::create_extended_weight_table( ett );
  std::sort( std::begin( ewt ), std::end( ewt ) );
  return ewt;
}
