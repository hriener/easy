#include <catch.hpp>
#include <easy/esop/esop.hpp>
#include <easy/esop/synthesis.hpp>
#include <easy/esop/exact_synthesis.hpp>
#include <easy/esop/cost.hpp>

using namespace easy;

TEST_CASE( "Compute minimum, maximum, and average pairwise distance", "[esop]" )
{
  const auto num_vars = 5u;
  esop::esop_t esop = esop::esop_t{
    kitty::cube( "0000-" ),
    kitty::cube( "0-110" ),
    kitty::cube( "0-011" ),
    kitty::cube( "-0111" ),
    kitty::cube( "-----" )};

  CHECK( esop::min_pairwise_distance( esop ) == 2 );
  CHECK( esop::max_pairwise_distance( esop ) == 4 );
  CHECK( esop::avg_pairwise_distance( esop ) == 3.5 );
}

TEST_CASE( "Verify ESOP with a truth table", "[esop]" )
{
  const auto num_vars = 5u;
  esop::esop_t esop = esop::esop_t{
    kitty::cube( "0000-" ),
    kitty::cube( "0-110" ),
    kitty::cube( "0-011" ),
    kitty::cube( "-0111" ),
    kitty::cube( "-----" )};

  /* specification */
  esop::spec s{ "01111111111101010111111101010011", "11111111111111111111111111111111" };
  CHECK( esop::verify_esop( esop, s.bits, s.care ) );
}

TEST_CASE( "Check equivalence of two ESOPs", "[esop]" )
{
  const auto num_vars = 5u;
  esop::esop_t esop_a = esop::esop_t{
    kitty::cube( "-1000" ),
    kitty::cube( "11001" ),
    kitty::cube( "00111" ),
    kitty::cube( "1100-" )};

  esop::esop_t esop_b = esop::esop_t{
    kitty::cube( "00000" ),
    kitty::cube( "0-010" ),
    kitty::cube( "0-0-0" ),
    kitty::cube( "00111" )};

  esop::esop_t esop_c = esop::esop_t{
    kitty::cube( "01010" ),
    kitty::cube( "110-0" ),
    kitty::cube( "-10-0" ),
    kitty::cube( "00111" )};

  esop::esop_t esop_min = esop::esop_t{
    kitty::cube( "00111" ),
    kitty::cube( "01000" )};

  CHECK( esop::equivalent_esops( esop_a, esop_b, num_vars ) );
  CHECK( esop::equivalent_esops( esop_a, esop_c, num_vars ) );
  CHECK( esop::equivalent_esops( esop_b, esop_c, num_vars ) );

  CHECK( esop::equivalent_esops( esop_a, esop_min, num_vars ) );
  CHECK( esop::equivalent_esops( esop_b, esop_min, num_vars ) );
  CHECK( esop::equivalent_esops( esop_c, esop_min, num_vars ) );
}

TEST_CASE( "Compute the T_count of an ESOP", "[esop]" )
{
  const auto num_vars = 5u;
  esop::esop_t esop_a = esop::esop_t{
    kitty::cube( "-1000" ),
    kitty::cube( "11001" ),
    kitty::cube( "00111" ),
    kitty::cube( "1100-" )};

  esop::esop_t esop_b = esop::esop_t{
    kitty::cube( "00000" ),
    kitty::cube( "0-010" ),
    kitty::cube( "0-0-0" ),
    kitty::cube( "00111" )};

  esop::esop_t esop_c = esop::esop_t{
    kitty::cube( "01010" ),
    kitty::cube( "110-0" ),
    kitty::cube( "-10-0" ),
    kitty::cube( "00111" )};

  esop::esop_t esop_min = esop::esop_t{
    kitty::cube( "00111" ),
    kitty::cube( "01000" )};

  CHECK( esop::T_count( esop_a, num_vars ) == 160 );
  CHECK( esop::T_count( esop_b, num_vars ) == 128 );
  CHECK( esop::T_count( esop_c, num_vars ) == 128 );
  CHECK( esop::T_count( esop_min, num_vars ) == 64 );
}
