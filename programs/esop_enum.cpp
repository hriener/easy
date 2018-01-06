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

#if defined(CRYPTOMINISAT_EXTENSION) && defined(KITTY_EXTENSION) && defined(ARGS_EXTENSION)

#include <esop/helliwell.hpp>
#include <esop/exact_synthesis.hpp>
#include <kitty/kitty.hpp>
#include <args.hxx>
#include <algorithm>
#include <fstream>
#include <iostream>

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

void print_esop( const esop::esop_t& esop, unsigned num_vars, std::ostream& os = std::cout )
{
  assert( num_vars < 32 );
  static const auto SYMBOL_XOR = "\u2295";
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
      os<< SYMBOL_XOR;
    }
  }
  os << std::endl;
}

void sort_by_number_of_product_terms( esop::esops_t& esops )
{
  const auto compare = []( const esop::esop_t& a, const esop::esop_t& b )
    { return a.size() < b.size(); };
  std::sort( esops.begin(), esops.end(), compare );
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

int main(int argc, char **argv)
{
  args::ArgumentParser parser( "Enumerate ESOP expressions", "" );

  args::HelpFlag                   help(      parser, "help",        "Display this help message",      { 'h', "help" } );
  args::Flag                       reverse(   parser, "reverse",     "reverse truth tables",           { 'r', "reverse" } );
  args::ValueFlagList<std::string> functions( parser, "truth_table", "truth table (as binary string)", { 'b' } );
  args::ValueFlag<std::string>     output(    parser, "filename",    "output file (default: stdout)",  { 'o' } );
  args::ValueFlag<int>             method(    parser, "method",      "0 ... helliwell decision problem\n"
					                             "1 ... exact synthesis",          { 'm' } );

  try
  {
    parser.ParseCLI(argc, argv);
  }
  catch (args::Help)
  {
    std::cout << parser;
    return 0;
  }
  catch (args::ParseError e)
  {
    std::cerr << e.what() << std::endl;
    std::cerr << parser;
    return 1;
  }
  catch (args::ValidationError e)
  {
    std::cerr << e.what() << std::endl;
    std::cerr << parser;
    return 1;
  }

  /* if output file is set, redirect std::cout to file */
  std::streambuf *cout_buf = std::cout.rdbuf();
  std::ofstream ofs;
  if ( output )
  {
    const auto filename = args::get( output );
    ofs.open( filename );
    if ( !ofs.is_open() )
    {
      std::cerr << "[e] could not open file " << filename << std::endl;
      return 1;
    }
    std::cout.rdbuf( ofs.rdbuf() );
  }
  
  if ( functions )
  {
    for ( auto binary : args::get( functions ) )
    {
      const auto num_vars = int( log2( binary.size() ) );

      /* check if num_vars is a power of 2 */
      if ( binary.size() != (1ull << num_vars) )
      {
	std::cout << "[w] skipped " << binary << ": bit-width is not a power of 2" << std::endl;
	continue;
      }

      /* if reverse flag is set, reverse the binary string */
      if ( reverse )
      {
	std::reverse( binary.begin(), binary.end() );
      }
      
      std::cout << "[i] compute ESOPs for " << binary << std::endl;
      esop::esops_t esops;

      const auto m = method ? args::get( method ) : /* default */0;
      if ( m == 0 )
      {
	std::cout << "[i] method: SAT-based synthesis (Helliwell decision problem)" << std::endl;
	esops = esop::synthesis_from_binary_string( binary );
      }
      else if ( m == 1 )
      {
	std::cout << "[i] method: SAT-based exact synthesis" << std::endl;
	esops = esop::exact_synthesis_from_binary_string( binary, 10 );
      }
      else
      {
	std::cout << "[e] unknown method" << std::endl;
	return 1;
      }

      sort_by_number_of_product_terms( esops );
      for ( const auto& e : esops )
      {
	print_esop( e, num_vars, std::cout );
      }
    }
  }

  /* restore std::cout */
  if ( output )
  {
    std::cout.rdbuf( cout_buf );
    ofs.close();
  }
  
  return 0;
}

#else

#warning "missing compile-time dependencies: cryptominisat, kitty and args are required"

#include <iostream>

int main( int argc, char *argv[] )
{
  (void)(argc);
  std::cerr << "[e] missing compile-time dependencies for " << argv[0] << std::endl;
  std::cout << "[e] cryptominisat, kitty and args are required" << std::endl;
  return 1;
}

#endif /* CRYPTOMINISAT_EXTENSION && KITTY_EXTENSION && ARGS_EXTENSION */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
