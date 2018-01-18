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

#include <esop/print.hpp>
#include <esop/helliwell.hpp>
#include <esop/exact_synthesis.hpp>
#include <utils/string_utils.hpp>
#include <kitty/kitty.hpp>
#include <args.hxx>
#include <algorithm>
#include <fstream>
#include <iostream>

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

enum class esop_representation_enum
{
  xor_expression = 0
, cube_vector = 1
};

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

void sort_by_number_of_product_terms( esop::esops_t& esops )
{
  const auto compare = []( const esop::esop_t& a, const esop::esop_t& b )
    { return a.size() < b.size(); };
  std::sort( esops.begin(), esops.end(), compare );
}

std::string binary_string_from_hex_string( const std::string& hex )
{
  std::string result = "";
  for ( auto i = 0u; i < hex.length(); ++i )
  {
    switch ( hex[i] )
    {
    case '0': result.append ("0000"); break;
    case '1': result.append ("0001"); break;
    case '2': result.append ("0010"); break;
    case '3': result.append ("0011"); break;
    case '4': result.append ("0100"); break;
    case '5': result.append ("0101"); break;
    case '6': result.append ("0110"); break;
    case '7': result.append ("0111"); break;
    case '8': result.append ("1000"); break;
    case '9': result.append ("1001"); break;
    case 'a': result.append ("1010"); break;
    case 'b': result.append ("1011"); break;
    case 'c': result.append ("1100"); break;
    case 'd': result.append ("1101"); break;
    case 'e': result.append ("1110"); break;
    case 'f': result.append ("1111"); break;
    }
  }
  return result;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

int main(int argc, char **argv)
{
  args::ArgumentParser parser( "Enumerate ESOP expressions", "" );

  args::HelpFlag                   help(      parser, "help",        "Display this help message",      { 'h', "help" } );
  args::Flag                       reverse(   parser, "reverse",     "reverse input functions",        { 'r', "reverse" } );
  args::Flag                       echo(      parser, "echo",        "echo input function",            { 'e', "echo" } );

  std::unordered_map<std::string, esop_representation_enum> map{
    {"expr", esop_representation_enum::xor_expression},
    {"cube", esop_representation_enum::cube_vector}};

  args::MapFlag<std::string,esop_representation_enum> repr( parser, "expr|cube", "Representation of the computed ESOP (default:expr)",
							    { "repr" }, map, esop_representation_enum::xor_expression );

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

  /* configure output printer */
  std::function<void(const esop::esop_t&,unsigned,std::ostream&)> print_function;
  switch( args::get( repr ) )
  {
  case esop_representation_enum::xor_expression:
    print_function = esop::print_esop_expression;
    break;
  case esop_representation_enum::cube_vector:
    print_function = esop::print_esop_cubes;
    break;
  default:
    assert( false && "unsupported output representation" );
  }

  std::string line;
  while ( std::getline( std::cin, line ) )
  {
    /* remove trailing space and skip empty lines */
    utils::trim( line );
    if ( line == "" ) continue;

    /* echo */
    if ( echo )
    {
      std::cout << "[i] synthesize ESOPs for " << line << std::endl;
    }

    const auto prefix = line.substr( 0, 2 );
    if ( prefix == "0x" )
    {
      /* remove prefix */
      line = line.substr( 2, line.size() - 2 );
      line = binary_string_from_hex_string( line );
    }
    else if ( prefix == "0b" )
    {
      /* remove prefix */
      line = line.substr( 2, line.size() - 3 );
    }

    if ( reverse )
    {
      std::reverse( line.begin(), line.end() );
    }

    /* solve */
    esop::esops_t esops = esop::exact_synthesis_from_binary_string( line );
    sort_by_number_of_product_terms( esops );

    /* print result */
    const auto number_of_variables = int( log2( line.size() ) );
    for ( const auto& e : esops )
    {
      std::cout << line << ' ';
      print_function( e, number_of_variables, std::cout );
    }
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
