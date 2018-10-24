#include <catch.hpp>
#include <easy/esop/constructors.hpp>
#include <kitty/constructors.hpp>
#include <iostream>

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

TEST_CASE( "Create ESOP corner cases", "[constructors]" )
{
  CHECK( from_cubes<3>( esop::esop_from_pprm( from_hex<3>( "00" ) ) ) == from_hex<3>( "00" ) );
  CHECK( from_cubes<3>( esop::esop_from_pprm( from_hex<3>( "fe" ) ) ) == from_hex<3>( "fe" ) );
  CHECK( from_cubes<3>( esop::esop_from_pprm( from_hex<3>( "80" ) ) ) == from_hex<3>( "80" ) );
  CHECK( from_cubes<3>( esop::esop_from_pprm( from_hex<3>( "ff" ) ) ) == from_hex<3>( "ff" ) );

  CHECK( from_cubes<3>( esop::esop_from_optimum_pkrm( from_hex<3>( "00" ) ) ) == from_hex<3>( "00" ) );
  CHECK( from_cubes<3>( esop::esop_from_optimum_pkrm( from_hex<3>( "fe" ) ) ) == from_hex<3>( "fe" ) );
  CHECK( from_cubes<3>( esop::esop_from_optimum_pkrm( from_hex<3>( "80" ) ) ) == from_hex<3>( "80" ) );
  CHECK( from_cubes<3>( esop::esop_from_optimum_pkrm( from_hex<3>( "ff" ) ) ) == from_hex<3>( "ff" ) );
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

TEST_CASE( "Create ESOP from helliwell equation", "[constructors]" )
{
  esop::helliwell_statistics stats;
  esop::helliwell_params ps;
  CHECK( from_cubes<3>( esop::esop_from_tt<kitty::static_truth_table<3>, esop::helliwell>( stats, ps ).synthesize( from_hex<3>( "00" ) ) ) == from_hex<3>( "00" ) ); // false
  CHECK( from_cubes<3>( esop::esop_from_tt<kitty::static_truth_table<3>, esop::helliwell>( stats, ps ).synthesize( from_hex<3>( "80" ) ) ) == from_hex<3>( "80" ) ); // and
  CHECK( from_cubes<3>( esop::esop_from_tt<kitty::static_truth_table<3>, esop::helliwell>( stats, ps ).synthesize( from_hex<3>( "fe" ) ) ) == from_hex<3>( "fe" ) ); // or
  CHECK( from_cubes<3>( esop::esop_from_tt<kitty::static_truth_table<3>, esop::helliwell>( stats, ps ).synthesize( from_hex<3>( "ff" ) ) ) == from_hex<3>( "ff" ) ); // true
}
