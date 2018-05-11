#include <catch.hpp>
#include <esop/esop.hpp>
#include <esop/synthesis.hpp>
#include <esop/exact_synthesis.hpp>
#include <kitty/kitty.hpp>
#include <lorina/pla.hpp>

/**
 * Synthesize one ESOP for a given Boolean function
 */
TEST_CASE( "one_esop", "[synthesis]" )
{
  nlohmann::json config;
  config[ "maximum_cubes" ] = 10;
  config[ "one_esop" ] = true;

  // const auto number_of_variables = 5u;
  const std::string bits = "01111111111101010111111101010011";
  const std::string care = "11111111111111111111111111111111";

  const auto esops = esop::exact_synthesis_from_binary_string( bits, care, config );

  /* number of ESOPs */
  CHECK( esops.size() == 1 );

  /* number of terms of first ESOP */
  for ( const auto& e : esops )
  {
    // esop::print_esop_as_exprs( e, number_of_variables );
    CHECK( esop::verify_esop( e, bits, care ) );
  }
}

/**
 * Synthesize all ESOPs for a given Boolean function
 */
TEST_CASE( "all_esop", "[synthesis]" )
{
  nlohmann::json config;
  config[ "maximum_cubes" ] = 10;
  config[ "one_esop" ] = false;

  // const auto number_of_variables = 5u;
  const std::string bits = "01111111111101010111111101010011";
  const std::string care = "11111111111111111111111111111111";

  const auto esops = esop::exact_synthesis_from_binary_string( bits, care, config );

  /* number of ESOPs */
  CHECK( esops.size() == 7 );

  /* number of terms of first ESOP */
  for ( const auto& e : esops )
  {
    // esop::print_esop_as_exprs( e, number_of_variables );
    CHECK( esop::verify_esop( e, bits, care ) );
  }
}

/**
 * Synthesize one ESOP for a given Boolean function (new API)
 */
TEST_CASE( "new_api_synthesize", "[synthesis]" )
{
  /* specification */
  esop::spec s{ "01111111111101010111111101010011", "11111111111111111111111111111111" };

  /* synthesizer */
  esop::simple_synthesizer synth( s );
  esop::simple_synthesizer_params params;

  /* try to synthesize with 4 terms which is *not* enough */
  params.number_of_terms = 4;
  auto esop = synth.synthesize( params );
  CHECK( esop.empty() );

  /* try again with 5 terms */
  params.number_of_terms = 5;
  esop = synth.synthesize( params );

  auto number_of_variables = 5u;
  // esop::print_esop_as_exprs( esop, number_of_variables );
  CHECK( esop::verify_esop( esop, s.bits, s.care ) );
}

/**
 * Synthesize a minimum ESOP for a given Boolen function (new API)
 */
TEST_CASE( "new_api_minimize", "[synthesis]" )
{
  /* specification */
  esop::spec s{ "01111111111101010111111101010011", "11111111111111111111111111111111" };

  /* synthesizer */
  esop::minimum_synthesizer synth( s );
  esop::minimum_synthesizer_params params;

  esop::esop_t esop;

  /* search downwards */
  params.begin = 8;
  params.next = [&]( int& i, bool sat ){ if ( i <= 0 || !sat ) return false; --i; return true; };
  esop = synth.synthesize( params );
  CHECK( !esop.empty() );
  CHECK( esop.size() == 5u );

  /* search upwards */
  params.begin = 1;
  params.next = [&]( int& i, bool sat ){ if ( i >= 8 || sat ) return false; ++i; return true; };
  esop = synth.synthesize( params );
  CHECK( !esop.empty() );
  CHECK( esop.size() == 5u );
}

/**
 * Compute minimum pairwise and maximum pairwise distance of an ESOP
 */
TEST_CASE( "distance", "[synthesis]" )
{
  /* specification */
  esop::spec s{ "01111111111101010111111101010011", "11111111111111111111111111111111" };

  /* synthesizer */
  esop::minimum_synthesizer synth( s );
  esop::minimum_synthesizer_params params;

  esop::esop_t esop;

  /* search downwards */
  params.begin = 8;
  params.next = [&]( int& i, bool sat ){ if ( i <= 0 || !sat ) return false; --i; return true; };
  esop = synth.synthesize( params );
  CHECK( !esop.empty() );
  CHECK( esop.size() == 5u );

  const auto min = esop::min_pairwise_distance( esop );
  CHECK( min == 2 );

  const auto max = esop::max_pairwise_distance( esop );
  CHECK( max == 4 );
}

class pla_storage_reader : public lorina::pla_reader
{
public:
  pla_storage_reader( esop::esop_t& esop )
    : _esop( esop )
  {}

  virtual void on_term( const std::string& term, const std::string& out ) const override
  {
    assert( out == "1" );
    _esop.emplace_back( term );
  }

  esop::esop_t& _esop;
}; /* pla_storage_reader */

/**
 * Resynthesize all minimum ESOPs for a given ESOP.
 */
TEST_CASE( "esop_resynthesis", "[synthesis]" )
{
  const auto num_vars = 4u;

  const std::string pla =
    ".i 4\n"
    ".o 1\n"
    ".p 4\n"
    "011- 1\n"
    "0-11 1\n"
    "-11- 1\n"
    "1010 1\n"
    ".e\n";

  std::istringstream ss( pla );

  esop::esop_t esop;
  auto parsing_result = lorina::read_pla( ss, pla_storage_reader( esop ) );
  CHECK( parsing_result == lorina::return_code::success );
  CHECK( esop.size() == 4 );
  // esop::print_esop_as_exprs( esop, num_vars );

  /* esop to truth table */
  kitty::dynamic_truth_table tt( num_vars );
  kitty::create_from_cubes( tt, esop, true );
  auto func = to_binary( tt );
  std::reverse( func.begin(), func.end() );

  /* specification */
  esop::spec s{ func, std::string( func.size(), '1' ) };

  /* synthesizer */
  esop::minimum_all_synthesizer synth( s );

  /* search upwards */
  esop::minimum_all_synthesizer_params params;
  params.begin = 1;
  params.next = [&]( int& i, bool sat ){ if ( i >= 4 || sat ) return false; ++i; return true; };

  auto result = synth.synthesize( params );
  CHECK( result.size() == 10 );
  for ( const auto& r : result )
  {
    // esop::print_esop_as_exprs( r, num_vars );
    CHECK( r.size() == 3 );
    CHECK( esop::equivalent_esops( r, esop, num_vars ) );
  }
}

