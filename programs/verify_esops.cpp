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

#include <easy/easy.hpp>
#include <easy/utils/string_utils.hpp>
#include <kitty/kitty.hpp>
#include <args/args.hxx>
#include <rang/rang.hpp>
#include <boost/algorithm/string.hpp>
#include <bitset>
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

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

int main(int argc, char **argv)
{
  args::ArgumentParser parser( "Verify ESOP expressions", "" );

  args::HelpFlag                   help(      parser, "help",        "Display this help message", { 'h', "help" } );
  args::Flag                       echo(      parser, "echo",        "echo verification",         { 'e', "echo" } );
  args::Flag                       summary(   parser, "summary",     "print summary",             { 's', "summary" } );

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
    std::cerr << rang::style::bold << rang::fg::red << e.what() << rang::style::reset << std::endl;
    std::cerr << parser;
    return 1;
  }
  catch (args::ValidationError e)
  {
    std::cerr << rang::style::bold << rang::fg::red << e.what() << rang::style::reset << std::endl;
    std::cerr << parser;
    return 1;
  }

  auto errors = 0u;

  std::string line;
  while ( std::getline( std::cin, line ) )
  {
    /* skip empty and comment lines */
    utils::trim( line );
    if ( line == "" || line[0] == '#' ) continue;

    std::vector<std::string> tokens;
    boost::split( tokens, line, boost::is_any_of( " " ) );

    const auto size = tokens[0].size();
    const auto bits = std::bitset<1024>( tokens[0] );
    const auto care = std::bitset<1024>( tokens[1] );

    std::vector<kitty::cube> cubes;
    for ( auto i = 2u; i < tokens.size(); ++i )
    {
      cubes.emplace_back( tokens[i] );
    }

    kitty::dynamic_truth_table tt( int(log2(size)) );
    kitty::create_from_cubes( tt, cubes, true );

    assert( size <= bits.size() );
    assert( size <= care.size() );

    auto eq = true;
    for ( auto i = 0; i < size; ++i )
    {
      if ( care[i] && bits[i] != get_bit( tt, i ) )
      {
	eq = false;
	break;
      }
    }

    if ( !eq )
    {
      ++errors;
    }

    if ( echo )
    {
      std::cout << line << ' ';
      if ( eq )
      {
        std::cout << rang::style::bold << rang::fg::blue << "OK" << rang::style::reset << std::endl;
      }
      else
      {
        std::cout << rang::style::bold << rang::fg::red << "ERROR" << rang::style::reset << std::endl;
      }
    }
  }

  if ( summary )
  {
    std::cout << "[i] total number of errors: " << errors << std::endl;
  }

  if ( errors == 0 )
  {
    return 0;
  }
  else
  {
    return -1;
  }
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
