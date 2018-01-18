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

#if defined(CRYPTOMINISAT_EXTENSION) && defined(KITTY_EXTENSION)

#include <esop/exact_synthesis.hpp>
#include <sat/sat_solver.hpp>

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

inline uint64_t cube_weight( const kitty::cube& c, unsigned num_vars )
{
  uint64_t value = 0;
  for ( auto i = 0u; i < num_vars; ++i )
  {
    value += c.get_mask( i ) ? ( c.get_bit( i ) ? pow3[ i ] : 0 ) : 2*pow3[ i ];
  }
  return value;
}

struct cube_weight_compare
{
  cube_weight_compare( unsigned num_vars = 32 )
    : _num_vars( num_vars )
  {}

  bool operator()( const kitty::cube& a, const kitty::cube& b ) const
  {
    return cube_weight( a, _num_vars ) < cube_weight( b, _num_vars );
  }

  unsigned _num_vars;
};

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

esops_t exact_synthesis_from_binary_string( const std::string& binary, unsigned max_number_of_cubes )
{
  const int num_vars = log2( binary.size() );
  assert( binary.size() == (1ull << num_vars) && "bit-width is not a power of 2" );

  assert( num_vars <= 32 && "cube data structure cannot store more than 32 variables" );

  esop::esops_t esops;
  for ( auto k = 1u; k <= max_number_of_cubes; ++k )
  {
    // std::cout << "[i] bounded synthesis for k = " << k << std::endl;

    sat::sat_solver solver;

    /* add constraints */
    kitty::cube minterm = kitty::cube::neg_cube( num_vars );

    auto sample_counter = 0u;
    do
    {
      /* skip don't cares */
      if ( binary[ minterm._bits ] != '0' && binary[ minterm._bits ] != '1' )
      {
	++minterm._bits;
	continue;
      }

      std::vector<int> xor_clause;

      for ( auto j = 0u; j < k; ++j )
      {
	const int z = 1 + 2*num_vars*k + sample_counter*k + j;
	xor_clause.push_back( z );

	// positive
	for ( auto l = 0; l < num_vars; ++l )
	{
	  if ( minterm.get_bit( l ) )
	  {
	    std::vector<int> clause;
	    clause.push_back( -z ); // - z_j
	    clause.push_back( -( 1 + num_vars*k + num_vars*j + l ) ); // -q_j,l

	    solver.add_clause( clause );
	  }
	  else
	  {
	    std::vector<int> clause;
	    clause.push_back( -z ); // -z_j
	    clause.push_back( -( 1 + num_vars*j + l ) ); // -p_j,l

	    solver.add_clause( clause );
	  }
	}
      }

      for ( auto j = 0u; j < k; ++j )
      {
	const int z = 1 + 2*num_vars*k + sample_counter*k + j;

	// negative
	std::vector<int> clause = { z };
	for ( auto l = 0; l < num_vars; ++l )
	{
	  if ( minterm.get_bit( l ) )
	  {
	    clause.push_back( 1 + num_vars*k + num_vars*j + l ); // q_j,l
	  }
	  else
	  {
	    clause.push_back( 1 + num_vars*j + l ); // p_j,l
	  }
	}

	solver.add_clause( clause );
      }

      solver.add_xor_clause( xor_clause, binary[ minterm._bits ] == '1' );

      ++sample_counter;
      ++minterm._bits;
    } while( minterm._bits < (1 << num_vars) );

    while ( auto result = solver.solve() )
    {
      esop::esop_t esop;

      std::vector<int> blocking_clause;
      for ( auto j = 0u; j < k; ++j )
      {
	kitty::cube c;
	bool cancel_cube = false;
	for ( auto l = 0; l < num_vars; ++l )
	{
	  const auto p_value = result.model[ j*num_vars + l ] == CMSat::l_True;
	  const auto q_value = result.model[ num_vars*k + j*num_vars + l ] == CMSat::l_True;

	  blocking_clause.push_back( p_value ? -(1 + j*num_vars + l) : (1 + j*num_vars + l) );
	  blocking_clause.push_back( q_value ? -(1 + num_vars*k + j*num_vars + l) : (1 + num_vars*k + j*num_vars + l) );

	  if ( p_value && q_value )
	  {
	    cancel_cube = true;
	  }
	  else if ( p_value )
	  {
	    c.add_literal( l, true );
	  }
	  else if ( q_value )
	  {
	    c.add_literal( l, false );
	  }
	}

	if ( cancel_cube )
	{
	  continue;
	}

	esop.push_back( c );
      }

      /* special case: if the empty ESOP, i.e., false, is a possible solution, the immediately return */
      if ( esop.empty() )
      {
	return { esop };
      }

      std::sort( esop.begin(), esop.end(), cube_weight_compare( num_vars ) );
      esops.push_back( esop );

      solver.add_clause( blocking_clause );
    }

    if ( esops.size() > 0 )
    {
      break;
    }
  }

  return esops;
}

} /* esop */

#endif /* CRYPTOMINISAT_EXTENSION && KITTY_EXTENSION */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
