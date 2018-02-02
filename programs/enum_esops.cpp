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

#if defined(GLUCOSE_EXTENSION) && defined(KITTY_EXTENSION) && defined(ARGS_EXTENSION)

#include <esop/print.hpp>
#include <esop/exact_synthesis.hpp>
#include <utils/string_utils.hpp>
#include <kitty/kitty.hpp>
#include <json/json.hpp>
#include <args.hxx>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <regex>
#include <stdexcept>

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

std::string parse_function( std::string s )
{
  std::transform( s.begin(), s.end(), s.begin(), ::tolower );

  std::regex binary_number( "(0b)?([01]*)" );
  std::regex hex_number( "(0x)?([0123456789abcdef]*)" );

  std::smatch match;
  if ( std::regex_match( s, match, binary_number ) )
  {
    assert( match.size() == 3 );
    return match[2];
  }
  else if ( std::regex_match( s, match, hex_number ) )
  {
    assert( match.size() == 3 );
    return utils::binary_string_from_hex_string( match[2] );
  }
  else
  {
    assert( false && "unreachable" );
    throw std::invalid_argument( "function must be a binary or hexadecimal number" );
  }
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

int main(int argc, char **argv)
{
  args::ArgumentParser parser( "Enumerate ESOP expressions", "" );
  args::HelpFlag       help(      parser, "help",        "display this help message",      { 'h', "help" } );
  args::Flag           reverse(   parser, "reverse",     "reverse input functions",        { 'r', "reverse" } );
  args::Flag           echo(      parser, "echo",        "echo input function",            { 'e', "echo" } );
  args::Flag           dump(      parser, "cnf",         "dump intermediate CNF files",    { "cnf" } );
  args::Flag           all(       parser, "all",         "enumerate all exact ESOPs",      { "all" } );

  std::unordered_map<std::string, esop_representation_enum> map{
    {"expr", esop_representation_enum::xor_expression},
    {"cube", esop_representation_enum::cube_vector}};

  args::MapFlag<std::string,esop_representation_enum> repr( parser, "expr|cube", "representation of the computed ESOP (default:expr)",
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

  /* configure algorithm */
  nlohmann::json config;
  config["maximum_cubes"] = 10;
  config["dump_cnf"] = bool(dump);

  if ( all )
  {
    config["one_esop"] = false;
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

    auto binary = parse_function( line );
    if ( reverse )
    {
      std::reverse( binary.begin(), binary.end() );
    }

    /* solve */
    std::reverse( binary.begin(), binary.end() );
    esop::esops_t esops = esop::exact_synthesis_from_binary_string( binary, config );
    std::reverse( binary.begin(), binary.end() );
    sort_by_number_of_product_terms( esops );

    /* print result */
    const auto number_of_variables = int( log2( binary.size() ) );
    for ( const auto& e : esops )
    {
      std::cout << binary << ' ';
      print_function( e, number_of_variables, std::cout );
    }
  }

  return 0;
}

#else

#warning "missing compile-time dependencies: glucose, kitty and args are required"

#include <iostream>

int main( int argc, char *argv[] )
{
  (void)(argc);
  std::cerr << "[e] missing compile-time dependencies for " << argv[0] << std::endl;
  std::cout << "[e] glucose, kitty and args are required" << std::endl;
  return 1;
}

#endif /* GLUCOSE_EXTENSION && KITTY_EXTENSION && ARGS_EXTENSION */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
