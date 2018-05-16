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
#include <esop/exorlink.hpp>
#include <kitty/kitty.hpp>
#include <lorina/pla.hpp>
#include <args/args.hxx>
#include <iostream>

class pla_storage_reader : public lorina::pla_reader
{
public:
  pla_storage_reader( esop::esop_t& esop, unsigned& num_vars )
    : _esop( esop )
    , _num_vars( num_vars )
  {}

  void on_number_of_inputs( std::size_t i ) const override
  {
    _num_vars = i;
  }

  void on_term( const std::string& term, const std::string& out ) const override
  {
    assert( out == "1" );
    _esop.emplace_back( term );
  }

  esop::esop_t& _esop;
  unsigned& _num_vars;
}; /* pla_storage_reader */

double avg_norm_minimum_pairwise_distance( const esop::esop_t& esop )
{
  double dist = 0.0;
  for ( auto i = 0u; i < esop.size(); ++i )
  {
    /* compute the minimum distance for term i */
    unsigned min = std::numeric_limits<unsigned>::max();
    for ( auto j = i+1; j < esop.size(); ++j )
    {
      const auto d = esop[i].distance( esop[j] );
      if ( d < min )
      {
        min = d;
      }
    }

    if ( min < std::numeric_limits<unsigned>::max() )
    {
      dist += min;
    }
  }
  return dist / double(esop.size());
}

int main( int argc, char *argv[] )
{
  args::ArgumentParser          parser( "ESOP Maniac", "" );
  args::HelpFlag                help(     parser, "help",     "display this help message", { 'h', "help" } );
  args::Positional<std::string> filename( parser, "filename", "a PLA file" );

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

  std::cout << "[i] filename: " << args::get( filename ) << std::endl;

  lorina::diagnostic_engine diag;

  esop::esop_t esop;
  unsigned num_vars;
  auto parsing_result = lorina::read_pla( args::get( filename ), pla_storage_reader( esop, num_vars ), &diag );
  assert( parsing_result == lorina::return_code::success );

  auto s = [&num_vars]( const kitty::cube& c )
    {
      std::stringstream ss;
      c.print( num_vars, ss );
      return ss.str();
    };

  esop::esop_t esop_opt = esop;
  std::cout << "[i] esop size: " << esop_opt.size() << std::endl;

  auto dist_func = [&]( const esop::esop_t& esop ){ return avg_norm_minimum_pairwise_distance( esop ); };

  const auto initial_score = dist_func( esop_opt );
  std::cout << "[i] initial score: " << initial_score << std::endl;

  auto best_improv = 0.0;
  for ( auto i = 0; i < esop.size(); ++i )
  {
    std::cout << fmt::format("{} / {}\r", i, esop.size());
    std::cout.flush();
    for ( auto j = i+1; j < esop.size(); ++j )
    {
      const auto d = esop[i].distance( esop[j] );

      if ( d <= 1 )
      {
        std::cout << "[i] possible " << d << " optimization ";
        esop[i].print( num_vars ); std::cout << ' ';
        esop[j].print( num_vars ); std::cout << std::endl;
      }

      /* make a copy of the whole esop */
      esop::esop_t esop_opt = esop;

      /* TODO: find a better way to delete */
      /*       we assume no duplicates */
      esop_opt.erase( std::remove( esop_opt.begin(), esop_opt.end(), esop[i] ) );
      esop_opt.erase( std::remove( esop_opt.begin(), esop_opt.end(), esop[j] ) );

      if ( d == 2 )
      {
        for ( auto k = 0; k < 8; k += 4 )
        {
          const auto cubes = esop::exorlink( esop[i], esop[j], 2, &esop::cube_groups2[k] );
          esop_opt.insert( std::end(esop_opt), std::begin(cubes), std::begin(cubes)+2 );

          const auto score = dist_func( esop_opt );
          const auto improv = ( initial_score - score ) / initial_score * 100;
          const auto min = esop::min_pairwise_distance( esop_opt );

          if ( improv > best_improv )
          {
            std::cout << fmt::format( "EXORLINK-2[{}]: {} {} ==> {} {} [score: {}, improv: {}, min: {}]",
                                      k, s(esop[i]), s(esop[j]), s(cubes[0]), s(cubes[1]), score, improv, min )<< std::endl;
            best_improv = improv;
          }

          /* verify */
          // CHECK( esop::equivalent_esops( esop, esop_opt, num_vars ) );

          /* undo */
          esop_opt.pop_back();
          esop_opt.pop_back();
        }
      }
      else if ( d == 3 )
      {
        for ( auto k = 0; k < 54; k += 9 )
        {
          const auto cubes = esop::exorlink( esop[i], esop[j], 3, &esop::cube_groups3[k] );
          esop_opt.insert( std::end(esop_opt), std::begin(cubes), std::begin(cubes)+3 );

          const auto score = dist_func( esop_opt );
          const auto improv = (initial_score - score) / initial_score * 100;
          const auto min = esop::min_pairwise_distance( esop_opt );

          if ( improv > best_improv )
          {
            std::cout << fmt::format( "EXORLINK-3[{}]: {} {} ==> {} {} {} [score: {}, improv: {}, min: {}]",
                                      k, s(esop[i]), s(esop[j]), s(cubes[0]), s(cubes[1]), s(cubes[2]), score, improv, min )<< std::endl;
            best_improv = improv;
          }

          /* verify */
          // CHECK( esop::equivalent_esops( esop, esop_opt, num_vars ) );

          /* undo */
          esop_opt.pop_back();
          esop_opt.pop_back();
          esop_opt.pop_back();
        }
      }
      else if ( d == 4 )
      {
        for ( auto k = 0; k < 384; k += 16 )
        {
          const auto cubes = esop::exorlink( esop[i], esop[j], 4, &esop::cube_groups4[k] );
          esop_opt.insert( std::end(esop_opt), std::begin(cubes), std::end(cubes) );

          const auto score = dist_func( esop_opt );
          const auto improv = (initial_score - score) / initial_score * 100;
          const auto min = esop::min_pairwise_distance( esop_opt );

          if ( improv > best_improv )
          {
            std::cout << fmt::format( "EXORLINK-4[{}]: {} {} ==> {} {} {} {} [score: {}, improv: {}, min: {}]",
                                      k, s(esop[i]), s(esop[j]), s(cubes[0]), s(cubes[1]), s(cubes[2]), s(cubes[3]), score, improv, min )<< std::endl;
            best_improv = improv;
          }

          /* verify */
          // CHECK( esop::equivalent_esops( esop, esop_opt, num_vars ) );

          /* undo */
          esop_opt.pop_back();
          esop_opt.pop_back();
          esop_opt.pop_back();
          esop_opt.pop_back();
        }
      }
    }
  }

  return 0;
}
