#include <catch.hpp>
#include <easy/sat2/maxsat.hpp>

using namespace easy;

TEST_CASE( "MAXSAT solver", "[sat]" )
{
  int sid = 1;

  sat2::maxsat_solver_statistics stats;
  sat2::maxsat_solver_params ps;
  sat2::maxsat_solver<sat2::maxsat_linear> solver( stats, ps, sid );

  /* allocate variables */
  std::vector<int> v;
  for ( auto i = 0; i < 3; ++i )
  {
    v.emplace_back( sid++ );
  }

  /* make constraints */
  auto s0 = solver.add_soft_clause( {  v[0], v[1] } );
  auto s1 = solver.add_soft_clause( {  v[1] } );
  auto s2 = solver.add_soft_clause( { -v[1], v[2] } );
  auto s3 = solver.add_soft_clause( { -v[1], -v[2] } );
  auto s4 = solver.add_soft_clause( {  v[1], v[2] } );
  auto s5 = solver.add_soft_clause( { -v[0], v[1], -v[2] } );

  auto const result = solver.solve();
  CHECK( result == sat2::maxsat_solver<sat2::maxsat_linear>::state::success );

  auto const enabled_clauses = solver.get_enabled_clauses();
  CHECK( enabled_clauses.size() == 5u );

  bool const comp0 = ( enabled_clauses == std::vector<int>{ s0, s1, s3, s4, s5 } );
  bool const comp1 = ( enabled_clauses == std::vector<int>{ s0, s1, s2, s4, s5 } );
  bool const comp2 = comp0 || comp1;
  CHECK( comp2 );

  auto const disabled_clauses = solver.get_disabled_clauses();
  CHECK( disabled_clauses.size() == 1u );
  if ( comp0 )
    CHECK( disabled_clauses == std::vector<int>{ s2 } );
  else if ( comp1 )
    CHECK( disabled_clauses == std::vector<int>{ s3 } );
}

TEST_CASE( "MAXSAT solver 2", "[sat]" )
{
  int sid = 1;

  sat2::maxsat_solver_statistics stats;
  sat2::maxsat_solver_params ps;
  sat2::maxsat_solver<sat2::maxsat_linear> solver( stats, ps, sid );

  /* allocate variables */
  std::vector<int> v;
  for ( auto i = 0; i < 4; ++i )
    v.emplace_back( sid++ );

  /* add hard-clauses */
  solver.add_clause( { -v[0], -v[1] } );
  solver.add_clause( { -v[0],  v[2] } );
  solver.add_clause( { -v[0], -v[2] } );
  solver.add_clause( { -v[1],  v[3] } );
  solver.add_clause( { -v[1], -v[3] } );

  /* add soft-clauses */
  auto const c0 = solver.add_soft_clause( { v[0] } );
  auto const c1 = solver.add_soft_clause( { v[1] } );

  auto const result = solver.solve();
  CHECK( result == sat2::maxsat_solver<sat2::maxsat_linear>::state::success );
}
