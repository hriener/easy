#include <catch.hpp>

#include <easy/esop_from_rcf.hpp>
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

TEST_CASE( "Create ESOP from dynamic truth table", "[rcf]" )
{
  const int n = 6;
  kitty::dynamic_truth_table tt( n );
  for ( auto r = 0; r < std::min(n,4); ++r )
  {
    for ( auto i = 0u; i < 50u; ++i )
    {
      create_random( tt );
      auto const cubes = esop_from_rcf( tt );
      auto tt_copy = tt.construct();
      create_from_cubes( tt_copy, cubes, true );
      CHECK( tt == tt_copy );
    }
  }
}

TEST_CASE( "Create ESOP from random truth table", "[rcf]" )
{
  constexpr int n = 6;
  kitty::static_truth_table<n> tt;
  for ( auto r = 0; r < std::min(n,4); ++r )
  {
    for ( auto i = 0u; i < 100; ++i )
    {
      kitty::create_random( tt );
      auto const cubes = esop_from_rcf( tt, r );
      auto tt_copy = tt.construct();
      create_from_cubes( tt_copy, cubes, true );
      CHECK( tt == tt_copy );
    }
  }
}

TEST_CASE( "Create ESOP corner cases", "[rcf]" )
{
  constexpr int n = 3;
  for ( auto r = 0; r < std::min(n,3); ++r )
  {
    CHECK( from_cubes<n>( esop_from_rcf( from_hex<n>( "00" ), r ) ) == from_hex<n>( "00" ) );
    CHECK( from_cubes<n>( esop_from_rcf( from_hex<n>( "80" ), r ) ) == from_hex<n>( "80" ) );
    CHECK( from_cubes<n>( esop_from_rcf( from_hex<n>( "fe" ), r ) ) == from_hex<n>( "fe" ) );
    CHECK( from_cubes<n>( esop_from_rcf( from_hex<n>( "ff" ), r ) ) == from_hex<n>( "ff" ) );
  }
}
