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
TEST_CASE( "new_api", "[synthesis]" )
{
  const auto number_of_variables = 5u;
  const std::string bits = "01111111111101010111111101010011";
  const std::string care = "11111111111111111111111111111111";

  /* try to synthesize with 4 terms which is not enough */
  auto esop = esop::synthesize_esop( bits, care, 4u );
  CHECK( esop.empty() );

  /* try again with 5 terms */
  esop = esop::synthesize_esop( bits, care, 5u );
  CHECK( esop::verify_esop( esop, bits, care ) );
}
