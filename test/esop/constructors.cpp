#include <catch.hpp>

#include <easy/esop/constructors.hpp>
#include <easy/esop/exact_synthesis.hpp>
#include <kitty/constructors.hpp>
#include <kitty/print.hpp>

#include <iostream>
#include <numeric>

using namespace easy;

template<int NumVars>
inline kitty::static_truth_table<NumVars> from_hex( std::string const& hex )
{
  kitty::static_truth_table<NumVars> tt;
  kitty::create_from_hex_string( tt, hex );
  return tt;
}

template<int NumVars>
kitty::static_truth_table<NumVars> from_cubes( esop::esop_t const& cubes )
{
  kitty::static_truth_table<NumVars> tt;
  kitty::create_from_cubes( tt, cubes, true );
  return tt;
}

TEST_CASE( "Create ESOP from small truth table", "[constructors]" )
{
  auto const tt = from_hex<3>( "e8" );

  const auto cubes = esop::esop_from_optimum_pkrm( tt );

  auto tt_copy = tt.construct();
  create_from_cubes( tt_copy, cubes, true );
  CHECK( tt == tt_copy );
}

TEST_CASE( "Create PPRM from random truth table", "[constructors]" )
{
  kitty::static_truth_table<10> tt;

  for ( auto i = 0; i < 100; ++i )
  {
    kitty::create_random( tt );
    auto const cubes = esop::esop_from_pprm( tt );
    auto tt_copy = tt.construct();
    create_from_cubes( tt_copy, cubes, true );
    CHECK( tt == tt_copy );
  }
}

TEST_CASE( "Create PKRM from random truth table", "[constructors]" )
{
  kitty::static_truth_table<10> tt;

  for ( auto i = 0; i < 100; ++i )
  {
    kitty::create_random( tt );
    auto const cubes = esop::esop_from_optimum_pkrm( tt );
    auto tt_copy = tt.construct();
    create_from_cubes( tt_copy, cubes, true );
    CHECK( tt == tt_copy );
  }
}

TEST_CASE( "Create ESOP using Boolean learning from random truth table", "[constructors]" )
{
  kitty::static_truth_table<4> tt;

  for ( auto i = 0; i < 100; ++i )
  {
    kitty::create_random( tt );
    for ( auto const& cubes : esop::exact_esop( tt ) )
    {
      auto tt_copy = tt.construct();
      create_from_cubes( tt_copy, cubes, true );
      CHECK( tt == tt_copy );
    }
  }
}

TEST_CASE( "Create ESOP using Helliwell-SAT from random truth table", "[constructors]" )
{
  using tt_t = kitty::static_truth_table<5>;
  using synthesizer_t = esop::esop_from_tt<tt_t, sat2::maxsat_rc2, esop::helliwell_sat>;
  tt_t tt;

  for ( auto i = 0; i < 100; ++i )
  {
    kitty::create_random( tt );
    esop::helliwell_sat_statistics stats;
    esop::helliwell_sat_params ps;
    synthesizer_t synth( stats, ps );
    auto const cubes = synth.synthesize( tt );
    auto tt_copy = tt.construct();
    create_from_cubes( tt_copy, cubes, true );
    CHECK( tt == tt_copy );
  }
}

TEST_CASE( "Create ESOP using Helliwell-MAXSAT from random truth table", "[constructors]" )
{
  using tt_t = kitty::static_truth_table<4>;
  using synthesizer_t = esop::esop_from_tt<tt_t, sat2::maxsat_rc2, esop::helliwell_maxsat>;
  tt_t tt;

  for ( auto i = 0; i < 100; ++i )
  {
    kitty::create_random( tt );
    esop::helliwell_maxsat_statistics stats;
    esop::helliwell_maxsat_params ps;
    synthesizer_t synth( stats, ps );
    auto const cubes = synth.synthesize( tt );
    auto tt_copy = tt.construct();
    create_from_cubes( tt_copy, cubes, true );
    CHECK( tt == tt_copy );
  }
}

TEST_CASE( "Create PPRM ESOP corner cases", "[constructors]" )
{
  CHECK( from_cubes<3>( esop::esop_from_pprm( from_hex<3>( "00" ) ) ) == from_hex<3>( "00" ) );
  CHECK( from_cubes<3>( esop::esop_from_pprm( from_hex<3>( "fe" ) ) ) == from_hex<3>( "fe" ) );
  CHECK( from_cubes<3>( esop::esop_from_pprm( from_hex<3>( "80" ) ) ) == from_hex<3>( "80" ) );
  CHECK( from_cubes<3>( esop::esop_from_pprm( from_hex<3>( "ff" ) ) ) == from_hex<3>( "ff" ) );
}

TEST_CASE( "Create PKRM ESOP corner cases", "[constructors]" )
{
  CHECK( from_cubes<3>( esop::esop_from_optimum_pkrm( from_hex<3>( "00" ) ) ) == from_hex<3>( "00" ) );
  CHECK( from_cubes<3>( esop::esop_from_optimum_pkrm( from_hex<3>( "fe" ) ) ) == from_hex<3>( "fe" ) );
  CHECK( from_cubes<3>( esop::esop_from_optimum_pkrm( from_hex<3>( "80" ) ) ) == from_hex<3>( "80" ) );
  CHECK( from_cubes<3>( esop::esop_from_optimum_pkrm( from_hex<3>( "ff" ) ) ) == from_hex<3>( "ff" ) );
}

TEST_CASE( "Create ESOP using Helliwell-SAT corner cases", "[constructors]" )
{
  using tt_t = kitty::static_truth_table<3>;
  using synthesizer_t = esop::esop_from_tt<tt_t, sat2::maxsat_rc2, esop::helliwell_sat>;
  esop::helliwell_sat_statistics stats;
  esop::helliwell_sat_params ps;
  CHECK( from_cubes<3>( synthesizer_t( stats, ps ).synthesize( from_hex<3>( "00" ) ) ) == from_hex<3>( "00" ) );
  CHECK( from_cubes<3>( synthesizer_t( stats, ps ).synthesize( from_hex<3>( "80" ) ) ) == from_hex<3>( "80" ) );
  CHECK( from_cubes<3>( synthesizer_t( stats, ps ).synthesize( from_hex<3>( "fe" ) ) ) == from_hex<3>( "fe" ) );
  CHECK( from_cubes<3>( synthesizer_t( stats, ps ).synthesize( from_hex<3>( "ff" ) ) ) == from_hex<3>( "ff" ) );
}

TEST_CASE( "Create ESOP using Helliwell-MAXSAT corner cases", "[constructors]" )
{
  using tt_t = kitty::static_truth_table<3>;
  using synthesizer_t = esop::esop_from_tt<tt_t, sat2::maxsat_rc2, esop::helliwell_maxsat>;
  esop::helliwell_maxsat_statistics stats;
  esop::helliwell_maxsat_params ps;
  CHECK( from_cubes<3>( synthesizer_t( stats, ps ).synthesize( from_hex<3>( "00" ) ) ) == from_hex<3>( "00" ) );
  CHECK( from_cubes<3>( synthesizer_t( stats, ps ).synthesize( from_hex<3>( "80" ) ) ) == from_hex<3>( "80" ) );
  CHECK( from_cubes<3>( synthesizer_t( stats, ps ).synthesize( from_hex<3>( "fe" ) ) ) == from_hex<3>( "fe" ) );
  CHECK( from_cubes<3>( synthesizer_t( stats, ps ).synthesize( from_hex<3>( "ff" ) ) ) == from_hex<3>( "ff" ) );
}

TEST_CASE( "Create PPRM from dynamic truth table", "[constructors]" )
{
  kitty::dynamic_truth_table tt( 8u );

  for ( auto i = 0; i < 50; ++i )
  {
    create_random( tt );
    auto const cubes = esop::esop_from_pprm( tt );

    /* only positive literals */
    for ( const auto& c : cubes )
    {
      CHECK( c._bits == c._mask );
    }

    auto tt_copy = tt.construct();
    create_from_cubes( tt_copy, cubes, true );
    CHECK( tt == tt_copy );
  }
}

TEST_CASE( "Create optimum ESOP from random truth table", "[constructors]" )
{
  static const int size = 4;
  using tt_t = kitty::static_truth_table<size>;
  using synthesizer_t = esop::esop_from_tt<tt_t, sat2::maxsat_rc2, esop::helliwell_maxsat>;

  esop::helliwell_maxsat_statistics stats;
  esop::helliwell_maxsat_params ps;

  std::vector<std::string> tts{"0010011101010011",
      "1100011001111101",
      "0110010101011010",
      "0000011100011001",
      "0010011100111001",
      "0100011101011000",
      "0001011100011011",
      "1000011100011000",
      "1100100110000001",
      "1101000010010001",
      "0001011011100110",
      "1111011101010110",
      "1001111001000010",
      "0101011111011110",
      "0110011011100000",
      "1100011010011100",
      "0100001010011011",
      "0010111111101011",
      "1110000100000001",
      "1110000100100001",
      "1110011001100001",
      "0010111000111111",
      "1000001110011001",
      "1111010111110110",
      "0010000100101000",
      "1000111110111110",
      "1010110010110101",
      "1010110010010101",
      "0000100101110010",
      "0011110010101000",
      "1000111100010101",
      "0010111001000000",
      "1110010101011101",
      "0100011010011011",
      "0011011110101100",
      "0000100000101110",
      "1101011111011000",
      "0111001010100010",
      "0100011111101001",
      "0100101111110011",
      "1101110110110101",
      "1111000001110000",
      "0101000001001110",
      "1010100000101101",
      "1000100100101111",
      "1000001011011001",
      "1010101011101100",
      "1010101011101100",
      "0110110110011011",
      "0110011011001010" };

  std::vector<int> results{4, 4, 3, 3, 4, 4, 3, 4, 4, 3,
                           4, 4, 4, 4, 4, 3, 4, 4, 3, 3,
                           5, 3, 4, 3, 3, 4, 4, 4, 4, 4,
                           4, 3, 4, 4, 4, 3, 3, 3, 4, 4,
                           4, 2, 3, 4, 4, 4, 3, 3, 5, 4};

  std::vector<int> num_cubes( tts.size() );

  auto counter = 0;
  for ( const auto& tt : tts )
  {
    tt_t bits;
    kitty::create_from_binary_string( bits, tt );

    auto const cubes = synthesizer_t( stats, ps ).synthesize( bits );
    num_cubes[counter] = cubes.size();

    // verify correctness
    auto bits_copy = bits.construct();
    create_from_cubes( bits_copy, cubes, true );
    CHECK( bits == bits_copy );

    // verify minimality
    CHECK( cubes.size() == results[counter] );
    counter++;
  }
}
