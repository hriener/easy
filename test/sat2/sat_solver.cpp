#include <catch.hpp>
#include <easy/sat2/sat_solver.hpp>

using namespace easy;

TEST_CASE( "Test interface of satisyfing model", "[sat]" )
{
  /* empty model */
  sat2::model m0;
  CHECK( m0.size() == 0u );

  /* model from dynamic_bitset<> */
  utils::dynamic_bitset<> bs;
  bs.resize( 3 );
  CHECK( bs.num_bits() == 3u );

  /* set model: 010 */
  bs.set_bit( 0 );
  bs.reset_bit( 1 );
  bs.set_bit( 2 );
  
  sat2::model m1( bs );
  CHECK( m1.size() == bs.num_bits() );

  CHECK(  m1[1] );
  CHECK( !m1[2] );
  CHECK(  m1[3] );

  CHECK( !m1[-1] );
  CHECK(  m1[-2] );
  CHECK( !m1[-3] );
}

TEST_CASE( "Test interface of unsatisfiable core", "[sat]" )
{
  /* empty core */
  sat2::core<> c0;
  CHECK( c0.size() == 0u );

  /* model from std::vector<int>() */
  sat2::core<> c1( {3, 2, 1} );
  CHECK( c1.size() == 3u );
  CHECK( c1[0] == 1 );
  CHECK( c1[1] == 2 );
  CHECK( c1[2] == 3 );
}
