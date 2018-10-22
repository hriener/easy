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
  \file dynamic_bitset.hpp
  \brief A minimalistic excerpt of boost::dynamic_bitset.

  \author Heinz Riener
*/

#pragma once

// #include <iostream>

namespace easy::utils
{

template <typename Block = uint32_t, typename Allocator = std::allocator<Block>>
class dynamic_bitset;

template<typename Block,typename Allocator>
class dynamic_bitset
{
public:
  using block_type = Block;
  using allocator_type = Allocator;
  using size_type = std::size_t;
  using block_width_type = block_type;

  static constexpr block_width_type bits_per_block = std::numeric_limits<Block>::digits;
  static constexpr size_type npos = static_cast<size_type>( -1 );

public:
  explicit dynamic_bitset( const Allocator& alloc = Allocator() )
    : _bits( alloc )
  {
  }

  /*** basic bit operations ***/
  dynamic_bitset& set( size_type pos, bool val = true )
  {
    assert( pos < _num_bits );
    if ( val )
    {
      _bits[block_index( pos )] |= bit_mask( pos );      
    }
    else
    {
      reset( pos );
    }
    return *this;
  }

  dynamic_bitset& reset( size_type pos )
  {
    assert( pos < _num_bits );
    _bits[block_index( pos )] &= ~bit_mask( pos );
    return *this;
  }

  dynamic_bitset& reset()
  {
    std::fill( _bits.begin(), _bits.end(), Block( 0 ) );
    return *this;
  }

  dynamic_bitset& flip( size_type pos )
  {
    assert( pos < _num_bits );
    _bits[block_index( pos )] ^= bit_mask( pos );
    return *this;
  }

  bool _unchecked_test( size_type pos ) const
  {
    return ( _bits[block_index( pos )] & bit_mask( pos ) ) != 0;
  }

  bool test( size_type pos ) const
  {
    assert( pos < _num_bits );
    return _unchecked_test( pos );
  }

  bool operator[]( size_type pos ) const
  {
    return test( pos );
  }

  size_type size() const noexcept
  {
    return _num_bits;
  }

  size_type num_blocks() const noexcept
  {
    return _bits.size();
  }

  bool empty() const noexcept
  {
    return size() == 0;
  }
  
  size_type capacity() const noexcept
  {
    return _bits.capacity() * bits_per_block;
  }

  void reserve( size_type num_bits )
  {
    _bits.reserve( calc_num_blocks( num_bits ) );
  }

  /*** size changing operations ***/
  void resize( size_type num_bits, bool value = false )
  {
    const size_type old_num_blocks = num_blocks();
    const size_type required_blocks = calc_num_blocks( num_bits );

    const block_type v = value ? ~Block( 0 ) : Block( 0 );

    if ( required_blocks != old_num_blocks )
    {
      _bits.resize( required_blocks, v );
    }

    // At this point:
    //
    //  - if the buffer was shrunk, we have nothing more to do,
    //    except a call to m_zero_unused_bits()
    //
    //  - if it was enlarged, all the (used) bits in the new blocks have
    //    the correct value, but we have not yet touched those bits, if
    //    any, that were 'unused bits' before enlarging: if value == true,
    //    they must be set.

    if ( value && ( num_bits > _num_bits ) )
    {
      const block_width_type extra_bits = count_extra_bits();
      if ( extra_bits )
      {
        assert( old_num_blocks >= 1 && old_num_blocks <= _bits.size() );

        // Set them.
        _bits[old_num_blocks - 1] |= ( v << extra_bits );
      }
    }

    _num_bits = num_bits;
    _zero_unused_bits();
  }

  void clear() // no throw
  {
    _bits.clear();
    _num_bits = 0;
  }

  void push_back( bool bit )
  {
    const size_type sz = size();
    resize( sz + 1 );
    set( sz, bit );
  }

  void append(Block value)
  {
    const block_width_type r = count_extra_bits();

    if ( r == 0 ) {
      // the buffer is empty, or all blocks are filled
      _bits.push_back( value );
    }
    else {
      _bits.push_back( value >> ( bits_per_block - r ) );
      _bits[_bits.size() - 2] |= ( value << r ); // _bits.size() >= 2
    }

    _num_bits += bits_per_block;
  }

private:
  block_width_type count_extra_bits() const
  {
    return bit_index( size() );
  }
  
  static size_type block_index( size_type pos )
  {
    return pos / bits_per_block;
  }

  static block_width_type bit_index( size_type pos )
  {
    return static_cast<block_width_type>( pos % bits_per_block );
  }
  
  static Block bit_mask( size_type pos )
  {
    return Block( 1 ) << bit_index( pos );
  }
  
  static size_type calc_num_blocks( size_type num_bits )
  {
    return num_bits / bits_per_block + static_cast<size_type>( num_bits % bits_per_block != 0 );
  }

  Block& _highest_block()
  {
    return const_cast<Block &>( static_cast<const dynamic_bitset *>( this )->_highest_block() );
  }

  const Block& _highest_block() const
  {
    assert( size() > 0 && num_blocks() > 0 );
    return _bits.back();
  }

  /* If size() is not a multiple of bits_per_block then not all the
   * bits in the last block are used.  This function resets the unused
   * bits (convenient for the implementation of many member functions)
   */
  void _zero_unused_bits()
  {
    assert ( num_blocks() == calc_num_blocks( _num_bits ) );

    // if != 0 this is the number of bits used in the last block
    const block_width_type extra_bits = count_extra_bits();

    if (extra_bits != 0)
    {
      _highest_block() &= ~( ~static_cast<Block>( 0 ) << extra_bits );
    }
  }

public:
  std::vector<Block> _bits;
  size_type _num_bits{0};
}; /* dynamic_bitset */

} // namespace easy::utils
