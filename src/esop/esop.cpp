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

#include <esop/esop.hpp>

namespace esop
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/
static constexpr auto XOR_SYMBOL = "\u2295";

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void print_esop_as_exprs( const esop_t& esop, unsigned num_vars, std::ostream& os )
{
  assert( num_vars <= 32 );
  os << esop.size() << ' ';
  for ( auto i = 0u; i < esop.size(); ++i )
  {
    const auto& c = esop[i];
    auto lit_count = c.num_literals();
    if ( lit_count == 0 )
    {
      os << "(1)";
    }
    else
    {
      os << "(";
      for ( auto j = 0u; j < num_vars; ++j )
      {
	if ( ( c._mask >> j ) & 1 )
	{
	  os << ( ( ( c._bits >> j ) & 1 ) ? "x" : "~x" ) << j;
	  --lit_count;
	  if ( lit_count != 0 )
	  {
	    os << "*";
	  }
	}
      }
      os << ")";
    }
    if ( i+1 < esop.size() )
    {
      os << XOR_SYMBOL;
    }
  }
  os << '\n';
}

void print_esop_as_cubes( const esop::esop_t& esop, unsigned num_vars, std::ostream& os )
{
  assert( num_vars <= 32 );
  for ( const auto& c : esop )
  {
    c.print( num_vars, os );
    os << ' ';
  }
  os << '\n';
}

bool verify_esop( const esop::esop_t& esop, const std::string& bits, const std::string& care )
{
  assert( bits.size() == care.size() );
  const auto number_of_variables = unsigned( log2( bits.size() ) );

  kitty::dynamic_truth_table tt( number_of_variables );
  kitty::create_from_cubes( tt, esop, true );

  for ( auto i = 0; i < bits.size(); ++i )
  {
    if ( care[i] && bits[i] != '0' + get_bit( tt, i ) )
    {
      return false;
    }
  }
  return true;
}

} /* esop */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
