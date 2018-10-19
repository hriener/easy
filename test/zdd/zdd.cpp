#include <catch.hpp>
#include <easy/zdd/zdd.hpp>

using namespace easy;

TEST_CASE( "Union", "[zdd]" )
{
  auto const num_vars = 1u;
  zdd::Zdd mgr( num_vars, 1 << 10 );

  std::vector<int32_t> x;
  for ( auto i = 0u; i < num_vars; ++i )
    x.emplace_back( mgr.get_ith_var( i ) );

  auto const empty_set = mgr.get_constant( false );

  CHECK( mgr.zdd_union( { x[0], empty_set } ) == x[0] );
  CHECK( mgr.zdd_union( { empty_set, x[0] } ) == x[0] );
  CHECK( mgr.zdd_union( { x[0], x[0] } ) == x[0] );
}

TEST_CASE( "Intersection", "[zdd]" )
{
  auto const num_vars = 1u;
  zdd::Zdd mgr( num_vars, 1 << 10 );

  std::vector<int32_t> x;
  for ( auto i = 0u; i < num_vars; ++i )
    x.emplace_back( mgr.get_ith_var( i ) );

  auto const empty_set = mgr.get_constant( false );

  CHECK( mgr.zdd_intersect( { x[0], empty_set } ) == empty_set );
  CHECK( mgr.zdd_intersect( { empty_set, x[0] } ) == empty_set );
  CHECK( mgr.zdd_intersect( { x[0], x[0] } ) == x[0] );
}

TEST_CASE( "Difference", "[zdd]" )
{
  auto const num_vars = 1u;
  zdd::Zdd mgr( num_vars, 1 << 10 );

  std::vector<int32_t> x;
  for ( auto i = 0u; i < num_vars; ++i )
    x.emplace_back( mgr.get_ith_var( i ) );

  auto const empty_set = mgr.get_constant( false );

  CHECK( mgr.zdd_diff( x[0], x[0] ) == empty_set );
  CHECK( mgr.zdd_diff( x[0], empty_set ) == x[0] );

  CHECK( mgr.zdd_diff( empty_set, x[0] ) == empty_set );
  CHECK( mgr.zdd_diff( empty_set, empty_set ) == empty_set );
}

TEST_CASE( "Symmetric difference", "[zdd]" )
{
  auto const num_vars = 1u;
  zdd::Zdd mgr( num_vars, 1 << 10 );

  std::vector<int32_t> x;
  for ( auto i = 0u; i < num_vars; ++i )
    x.emplace_back( mgr.get_ith_var( i ) );

  auto const empty_set = mgr.get_constant( false );

  CHECK( mgr.zdd_sym_diff( { x[0], empty_set } ) == x[0] );
  CHECK( mgr.zdd_sym_diff( { empty_set, x[0] } ) == x[0] );

  CHECK( mgr.zdd_sym_diff( { x[0], x[0] } ) == empty_set );
}

TEST_CASE( "Cube set #1", "[zdd]" )
{
  auto cube_not = []( int32_t const &a ){ return a ^ 1; };

  auto const num_vars = 3u;
  zdd::Zdd mgr( 2u*num_vars, 1 << 20 );

  auto const a1 = mgr.get_ith_var( 0 );
  auto const a0 = mgr.get_ith_var( 1 );
  auto const b1 = mgr.get_ith_var( 2 );
  auto const b0 = mgr.get_ith_var( 3 );
  auto const c1 = mgr.get_ith_var( 4 );
  auto const c0 = mgr.get_ith_var( 5 );

  CHECK( a1 == cube_not( a0 ) );
  CHECK( b1 == cube_not( b0 ) );
  CHECK( c1 == cube_not( c0 ) );

  CHECK( cube_not( a1 ) == a0 );
  CHECK( cube_not( b1 ) == b0 );
  CHECK( cube_not( c1 ) == c0 );
}

TEST_CASE( "Cube set #2", "[zdd]" )
{
  auto const num_vars = 3u;
  zdd::Zdd mgr( 2u*num_vars, 1 << 20 );

  auto const a = mgr.get_ith_var( 0 );
  auto const b = mgr.get_ith_var( 2 );
  auto const c = mgr.get_ith_var( 4 );

  auto cube_not =  []( int32_t const &a ){ return a ^ 1; };
  auto cube_and = [&mgr]( int32_t const &a, int32_t const &b ){
    return mgr.zdd_dot_product( a, b ); };
  auto cube_or = [&mgr]( int32_t const &a, int32_t const &b ){
    return mgr.zdd_union( a, b); };

  // ab + c'
  auto cube_set = cube_or( cube_and( a, b ), cube_not( c ) );

  CHECK( mgr.zdd_count_nodes( cube_set ) == 3u );
  CHECK( mgr.zdd_count_paths( cube_set ) == 2u );

  // mgr.print_cover( cube_set, num_vars );
  // std::cout << std::endl;
}

TEST_CASE( "Cube set #3", "[zdd]" )
{
  auto const num_vars = 3u;
  zdd::Zdd mgr( 2u*num_vars, 1 << 20 );

  auto const a = mgr.get_ith_var( 0 );
  auto const b = mgr.get_ith_var( 2 );
  auto const c = mgr.get_ith_var( 4 );

  auto cube_not =  []( int32_t const &a ){ return a ^ 1; };
  auto cube_and = [&mgr]( std::vector<int32_t> const& vs ){
    return mgr.zdd_dot_product( vs ); };
  auto cube_or = [&mgr]( std::vector<int32_t> const& vs ){
    return mgr.zdd_union( vs ); };

  // a + b + c
  auto cube_set = cube_or( {a, b, c} );

  CHECK( mgr.zdd_count_nodes( cube_set ) == 3u );
  CHECK( mgr.zdd_count_paths( cube_set ) == 3u );

  // mgr.print_cover( cube_set, num_vars );
  // std::cout << std::endl;
}

TEST_CASE( "Cube set #4", "[zdd]" )
{
  auto const num_vars = 3u;
  zdd::Zdd mgr( 2u*num_vars, 1 << 20 );

  auto const a = mgr.get_ith_var( 0 );
  auto const b = mgr.get_ith_var( 2 );
  auto const c = mgr.get_ith_var( 4 );

  auto cube_not =  []( int32_t const &a ){ return a ^ 1; };
  auto a_onset = mgr.zdd_union( { mgr.zdd_dot_product( { a } ),
                                  mgr.zdd_dot_product( { a, c } ),
                                  mgr.zdd_dot_product( { a, b } ),
                                  mgr.zdd_dot_product( { a, b, c } ) } );

  auto b_onset = mgr.zdd_union( { mgr.zdd_dot_product( { b } ),
                                  mgr.zdd_dot_product( { a, b } ),
                                  mgr.zdd_dot_product( { b, c } ),
                                  mgr.zdd_dot_product( { a, b, c } ) } );

  auto c_onset = mgr.zdd_union( { mgr.zdd_dot_product( { c } ),
                                  mgr.zdd_dot_product( { c, a } ),
                                  mgr.zdd_dot_product( { c, b } ),
                                  mgr.zdd_dot_product( { c, a, b } ) } );

  auto a_set = mgr.zdd_cubeset( a );
  auto b_set = mgr.zdd_cubeset( b );
  auto c_set = mgr.zdd_cubeset( c );

  auto cubes = mgr.zdd_sym_diff( { a_onset, b_onset, c_onset } );
  mgr.print_cover( cubes, num_vars );
  std::cout << std::endl;
}
