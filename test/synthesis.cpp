#include <catch.hpp>
#include <esop/exact_synthesis.hpp>

/**
 * Synthesize one ESOP for a given Boolean function
 */
TEST_CASE( "one_esop", "[synthesis]" )
{
  nlohmann::json config;
  config[ "maximum_cubes" ] = 10;
  config[ "one_esop" ] = true;

  const std::string bits = "01111111111101010111111101010011";
  const std::string care = "11111111111111111111111111111111";

  const auto esops = esop::exact_synthesis_from_binary_string( bits, care, config );

  /* number of ESOPs */
  CHECK( esops.size() == 1 );

  /* number of terms of first ESOP */
  for ( const auto& e : esops )
  {
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

  const std::string bits = "01111111111101010111111101010011";
  const std::string care = "11111111111111111111111111111111";

  const auto esops = esop::exact_synthesis_from_binary_string( bits, care, config );

  /* number of ESOPs */
  CHECK( esops.size() == 7 );

  /* number of terms of first ESOP */
  for ( const auto& e : esops )
  {
    CHECK( esop::verify_esop( e, bits, care ) );
  }
}
