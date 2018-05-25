#include <catch.hpp>
#include <esop/esop.hpp>
#include <esop/synthesis.hpp>
#include <esop/exact_synthesis.hpp>
#include <esop/exorlink.hpp>
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
 * Synthesize a non-minimum ESOP for a given Boolen function
 */
TEST_CASE( "expand_with_synthesis", "[synthesis]" )
{
  const auto num_vars = 5u;

  /* synthesize all ESOPs with 4 terms equivalent to cube0, cube 1 */
  {
    /* exorlink-4 */
    kitty::cube cube0( "00111" );
    kitty::cube cube1( "01000" );

    /* esop to truth table */
    kitty::dynamic_truth_table tt( num_vars );
    kitty::create_from_cubes( tt, { cube0, cube1 }, true );
    auto func = to_binary( tt );
    std::reverse( func.begin(), func.end() );

    /* specification */
    esop::spec s{ func, std::string( func.size(), '1' ) };

    /* synthesizer */
    esop::minimum_all_synthesizer synth( s );

    /* search downwards from 8, but stop at 4 */
    esop::minimum_all_synthesizer_params params;
    params.begin = 8;
    params.next = [&]( int& i, bool sat ){ if ( i <= 4 || !sat ) return false; --i; return true; };

    const auto result = synth.synthesize( params );
    //CHECK( result.size() == 195 );
    for ( const auto& r : result )
    {
      CHECK( esop::equivalent_esops( {cube0, cube1}, r, num_vars ) );
    }
  }
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

TEST_CASE( "nong_example8", "[synthesis]" )
{
  /**
   * Example 8 from [1].
   *
   * Ning Song and Marek A. Perkowski, Minimization of Exclusive
   * Sum-of-Products Expressions for Multiple-Valued Input,
   * Incompletely Specified Functions, TCAD 15(4), 1996.
   */
  const auto num_vars = 4u;

  const std::string pla =
    ".i 4\n"
    ".o 1\n"
    ".p 4\n"
    "000- 1\n"
    "0-11 1\n"
    "-11- 1\n"
    "1010 1\n"
    ".e\n";

  std::istringstream ss( pla );

  esop::esop_t esop;
  auto parsing_result = lorina::read_pla( ss, pla_storage_reader( esop ) );
  CHECK( parsing_result == lorina::return_code::success );
  CHECK( esop.size() == 4 );

  const auto min = esop::min_pairwise_distance( esop );
  const auto max = esop::max_pairwise_distance( esop );
  CHECK( min == max );
  CHECK( min == 3u );

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
  CHECK( result.size() == 1u );
  for ( const auto& r : result )
  {
    // esop::print_esop_as_cubes( r, num_vars );
    CHECK( r.size() == 3 );
    CHECK( esop::equivalent_esops( r, esop, num_vars ) );
  }

  /**
   * EXORLINK simplification approach:
   *
   * ESOP: 000- XOR 0-11 XOR -11- XOR 1010
   *
   * 1) EXORLINK-3(000-,0-11) = 0111 XOR 00-1 XOR 0000
   *
   *    ESOP: 0111 XOR 00-1 XOR 0000 XOR -11- XOR 1010
   *
   * 2) EXORLINK-2(0000,1010) = -010 XOR 00-0
   *
   *    ESOP: 0111 XOR 00-1 XOR -11- XOR -010 XOR 00-0
   *
   * 3) EXORLINK-1(00-1,00-0) = 00--
   *
   *    ESOP: 0111 XOR 00-- XOR -010 XOR -11-
   *
   * 4) EXORLINK-2(-010,-11-) = --10 XOR -111
   *
   *    ESOP: 0111 XOR 00-- XOR --10 XOR -111
   *
   * 5) EXORLINK-1(0111,-111) = -111
   *
   *    ESOP: 00-- XOR --10 XOR -111
   *
   */
}

TEST_CASE( "exorlink", "[synthesis]" )
{
  const auto num_vars = 4u;

  {
    /* exorlink-2 */
    kitty::cube cube0( "-110" );
    kitty::cube cube1( "0111" );

    for ( auto i = 0; i < 8; i += 4 )
    {
      const auto cubes = esop::exorlink( cube0, cube1, 2, &esop::cube_groups2[i] );
      // cubes[0].print( num_vars ); std::cout << ' ';
      // cubes[1].print( num_vars ); std::cout << std::endl;
      CHECK( esop::equivalent_esops( {cube0, cube1}, {cubes[0], cubes[1]}, num_vars ) );
    }
  }

  {
    /* exorlink-3 */
    kitty::cube cube0( "000-" );
    kitty::cube cube1( "0-11" );

    for ( auto i = 0; i < 54; i += 9 )
    {
      const auto cubes = esop::exorlink( cube0, cube1, 3, &esop::cube_groups3[i] );
      // cubes[0].print( num_vars ); std::cout << ' ';
      // cubes[1].print( num_vars ); std::cout << ' ';
      // cubes[2].print( num_vars ); std::cout << std::endl;
      CHECK( esop::equivalent_esops( {cube0, cube1}, {cubes[0], cubes[1], cubes[2]}, num_vars ) );
    }
  }

  {
    /* exorlink-4 */
    kitty::cube cube0( "0000" );
    kitty::cube cube1( "----" );

    for ( auto i = 0; i < 384; i += 16 )
    {
      const auto cubes = esop::exorlink( cube0, cube1, 4, &esop::cube_groups4[i] );
      // cubes[0].print( num_vars ); std::cout << ' ';
      // cubes[1].print( num_vars ); std::cout << ' ';
      // cubes[2].print( num_vars ); std::cout << ' ';
      // cubes[3].print( num_vars ); std::cout << std::endl;
      CHECK( esop::equivalent_esops( {cube0, cube1}, {cubes[0], cubes[1], cubes[2], cubes[3]}, num_vars ) );
    }
  }

  {
    /* exorlink-4 */
    kitty::cube cube0( "0000" );
    kitty::cube cube1( "----" );

    for ( auto i = 0; i < 384; i += 16 )
    {
      const auto cubes = esop::exorlink4( cube0, cube1, i );
      // cubes[0].print( num_vars ); std::cout << ' ';
      // cubes[1].print( num_vars ); std::cout << ' ';
      // cubes[2].print( num_vars ); std::cout << ' ';
      // cubes[3].print( num_vars ); std::cout << std::endl;
      CHECK( esop::equivalent_esops( {cube0, cube1}, {cubes[0], cubes[1], cubes[2], cubes[3]}, num_vars ) );
    }
  }
}

