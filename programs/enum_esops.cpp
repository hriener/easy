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

#if defined(GLUCOSE_EXTENSION)

#include <easy/esop/exact_synthesis.hpp>
#include <easy/esop/synthesis.hpp>
#include <easy/utils/string_utils.hpp>
#include <easy/utils/stopwatch.hpp>
#include <kitty/kitty.hpp>
#include <json/json.hpp>
#include <args/args.hxx>
#include <boost/algorithm/string.hpp>
#include <fmt/format.h>
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

  std::regex binary_number( "(0b)?([01]+)" );
  std::regex hex_number( "(0x)?([0123456789abcdef]+)" );

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
  args::HelpFlag       help(          parser, "help",          "display this help message",                      { 'h', "help" } );
  args::Flag           reverse(       parser, "reverse",       "reverse input functions",                        { 'r', "reverse" } );
  args::Flag           echo(          parser, "echo",          "echo input function",                            { 'e', "echo" } );
  args::ValueFlag<int> num_terms(     parser, "num_terms",     "number of terms (default:8)",                    { 't', "terms" } );
  args::ValueFlag<int> num_conflicts( parser, "num_conflicts", "conflict limit (default:-1 for unlimited)",      { 'c', "conflicts" } );
  args::ValueFlag<int> strategy(      parser, "strategy",      "synthesis strategy (default:0)\n"
                                                               "\t0 fixed-size\n"
                                                               "\t1 downward search\n"
                                                               "\t2 upward search\n",                            { 's', "strategy" } );
  args::Flag           dump(          parser, "cnf",           "dump intermediate CNF files",                    { "cnf" } );
  args::Flag           all(           parser, "all",           "enumerate all exact ESOPs",                      { "all" } );
  args::Flag           dc(            parser, "dc",            "toggle care-set/dc-set",                         { "dc" } );

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
    print_function = esop::print_esop_as_exprs;
    break;
  case esop_representation_enum::cube_vector:
    print_function = esop::print_esop_as_cubes;
    break;
  default:
    assert( false && "unsupported output representation" );
  }

  std::cout << "# Generated by EASY" << std::endl;
  std::cout << "# command: ";
  for ( auto i = 0; i < argc; ++i )
  {
    std::cout << argv[i] << ' ';
  }
  std::cout << std::endl;

  /* statistics */
  utils::stopwatch<>::duration time{0};

  unsigned unknown_functions = 0;
  unsigned realizable_functions = 0;
  unsigned counter = 0;
  std::vector<unsigned> esop_size;

  std::string line;
  {
    utils::stopwatch<std::chrono::steady_clock> t( time );
    while ( std::getline( std::cin, line ) )
    {
      ++counter;

      /* remove trailing space and skip empty lines */
      utils::trim( line );
      if ( line == "" ) continue;

      std::vector<std::string> tokens;
      boost::split( tokens, line, boost::is_any_of( " " ) );

      auto bits = parse_function( tokens[0u] );
      const auto number_of_variables = int( log2( bits.size() ) );
      if ( tokens.size() == 1u )
      {
        tokens.push_back( std::string( 1 << number_of_variables, '1' ) );
      }
      else if ( tokens.size() > 2u )
      {
        std::cout << "[e] too many tokens" << std::endl;
        continue;
      }

      auto care = parse_function( tokens[1u] );
      if ( dc )
      {
        for ( auto i = 0u; i < care.size(); ++i )
        {
          care[i] = (care[i] == '0' ? '1' : (care[i] == '1' ? '0' : care[i]));
        }
      }

      if ( bits.size() != care.size() )
      {
        std::cout << "[e] bits and care have to be of the same length" << std::endl;
        continue;
      }

      if ( care == std::string( 1 << number_of_variables, '0' ) )
      {
        std::cout << "[e] at least one bit must be within the care set to syntheize a function" << std::endl;
        continue;
      }

      /* echo */
      if ( echo )
      {
        std::cout << "[i] synthesize ESOPs for " << bits << " (" << care << ")" << std::endl;
      }

      if ( reverse )
      {
        std::reverse( bits.begin(), bits.end() );
        std::reverse( care.begin(), care.end() );
      }

      esop::result synthesis_result;

      const auto strat = args::get( strategy );
      if ( strat == 0 )
      {
        esop::simple_synthesizer_params params;
        params.number_of_terms = num_terms ? args::get(num_terms) : 8;
        params.conflict_limit = num_conflicts ? args::get(num_conflicts) : -1;

        esop::simple_synthesizer synthesizer( esop::spec{ bits, care } );
        synthesis_result = synthesizer.synthesize( params );
        if ( synthesis_result.is_unknown() )
        {
          if ( echo )
          {
            std::cout << "[i] skipped function (conflict limit too low)" << std::endl;
          }
          ++unknown_functions;
          continue;
        }
      }
      else if ( strat == 1 )
      {
        /* downward search */
        esop::minimum_synthesizer_params params;
        params.begin = num_terms ? args::get(num_terms) : 8;
        params.conflict_limit = num_conflicts ? args::get(num_conflicts) : -1;
        params.next = [&]( int& i, sat::sat_solver::result sat ){ if ( i <= 1 || sat.is_unsat() ) return false; --i; return true; };

        esop::minimum_synthesizer synthesizer( esop::spec{ bits, care } );
        synthesis_result = synthesizer.synthesize( params );
        if ( synthesis_result.is_unknown() )
        {
          if ( echo )
          {
            std::cout << "[i] skipped function (conflict limit too low)" << std::endl;
          }
          ++unknown_functions;
          continue;
        }
      }
      else if ( strat == 2 )
      {
        /* upward search */
        esop::minimum_synthesizer_params params;
        params.begin = 1;
        params.conflict_limit = num_conflicts ? args::get(num_conflicts) : -1;
        params.next = [&]( int& i, sat::sat_solver::result sat ){ if ( i >= (num_terms ? args::get(num_terms) : 8) || sat.is_sat() ) return false; ++i; return true; };

        esop::minimum_synthesizer synthesizer( esop::spec{ bits, care } );
        synthesis_result = synthesizer.synthesize( params );
        if ( synthesis_result.is_unknown() )
        {
          if ( echo )
          {
            std::cout << "[i] skipped function (conflict limit too low)" << std::endl;
          }
          ++unknown_functions;
          continue;
        }
      }
      else
      {
        std::cerr << "[e] unknown synthesis strategy" << std::endl;
        return -1;
      }

      esop::esops_t esops;
      if ( synthesis_result.is_realizable() )
      {
        ++realizable_functions;
        esops.push_back( synthesis_result.esop );
        esop_size.push_back( synthesis_result.esop.size() );
      }

      /* print result */
      for ( const auto& e : esops )
      {
        std::reverse( bits.begin(), bits.end() );
        std::reverse( care.begin(), care.end() );

        std::cout << bits << ' ' << care << ' ';
        print_function( e, number_of_variables, std::cout );
      }
    }
  }

  /* print statistics */
  unsigned sum_size = 0;
  for ( const auto& size : esop_size )
  {
    sum_size += size;
  }

  std::cout << "# [i] #parsed functions: " << counter << std::endl;
  std::cout << "# [i] #realizable: " << realizable_functions << std::endl;
  std::cout << "# [i] #skipped: " << unknown_functions << std::endl;
  std::cout << "# [i] average ESOP size: " << ( double(sum_size)/realizable_functions ) << std::endl;
  std::cout << fmt::format( "# [i] time: {:5.2f} seconds passed\n", utils::to_seconds( time ) );

  return 0;
}

#else

#warning "missing compile-time dependencies: glucose, kitty and args are required"

#include <iostream>

int main( int argc, char *argv[] )
{
  (void)(argc);
  std::cerr << "[e] missing compile-time dependencies for " << argv[0] << std::endl;
  std::cout << "[e] glucose is required" << std::endl;
  return 1;
}

#endif /* GLUCOSE_EXTENSION */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
