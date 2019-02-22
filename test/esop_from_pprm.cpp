#include <catch.hpp>

#include <easy/esop_from_pprm.hpp>
#include <kitty/constructors.hpp>

using namespace easy;

template<int NumVars>
inline kitty::static_truth_table<NumVars> from_hex( std::string const& hex )
{
  kitty::static_truth_table<NumVars> tt;
  kitty::create_from_hex_string( tt, hex );
  return tt;
}

template<int NumVars>
kitty::static_truth_table<NumVars> from_cubes( std::vector<kitty::cube> const& cubes )
{
  kitty::static_truth_table<NumVars> tt;
  kitty::create_from_cubes( tt, cubes, true );
  return tt;
}

TEST_CASE( "Create PPRM from dynamic truth table", "[pprm]" )
{
  kitty::dynamic_truth_table tt( 8u );
  for ( auto i = 0u; i < 50u; ++i )
  {
    create_random( tt );
    auto const cubes = esop_from_pprm( tt );

    /* check only positive literals */
    for ( const auto& c : cubes )
    {
      CHECK( c._bits == c._mask );
    }

    auto tt_copy = tt.construct();
    create_from_cubes( tt_copy, cubes, true );
    CHECK( tt == tt_copy );
  }
}

TEST_CASE( "Create PPRM from random truth table", "[pprm]" )
{
  kitty::static_truth_table<10> tt;

  for ( auto i = 0u; i < 100; ++i )
  {
    kitty::create_random( tt );
    auto const cubes = esop_from_pprm( tt );
    auto tt_copy = tt.construct();
    create_from_cubes( tt_copy, cubes, true );
    CHECK( tt == tt_copy );
  }
}

TEST_CASE( "Create PPRM corner cases", "[pprm]" )
{
  CHECK( from_cubes<3>( esop_from_pprm( from_hex<3>( "00" ) ) ) == from_hex<3>( "00" ) );
  CHECK( from_cubes<3>( esop_from_pprm( from_hex<3>( "fe" ) ) ) == from_hex<3>( "fe" ) );
  CHECK( from_cubes<3>( esop_from_pprm( from_hex<3>( "80" ) ) ) == from_hex<3>( "80" ) );
  CHECK( from_cubes<3>( esop_from_pprm( from_hex<3>( "ff" ) ) ) == from_hex<3>( "ff" ) );
}
