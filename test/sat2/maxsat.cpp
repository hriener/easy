#include <catch.hpp>
#include <easy/sat2/maxsat.hpp>

using namespace easy;

template<typename Algorithm>
void unsat_hard_clauses_test()
{
  int sid = 1;

  using maxsat_solver_t = sat2::maxsat_solver<Algorithm>;
  sat2::maxsat_solver_statistics stats;
  sat2::maxsat_solver_params ps;
  maxsat_solver_t solver( stats, ps, sid );

  /* allocate variables */
  std::vector<int> v;
  for ( auto i = 0; i < 1; ++i )
    v.emplace_back( sid++ );

  /* add hard-clauses */
  solver.add_clause( {  v[0] } );
  solver.add_clause( { -v[0] } );

  /* add soft-clauses */
  solver.add_soft_clause( {  v[0] } );
  solver.add_soft_clause( { -v[0] } );

  /* solve */
  auto const result = solver.solve();
  CHECK( result == maxsat_solver_t::state::fail );
}

template<typename Algorithm>
void no_soft_clauses_test()
{
  int sid = 1;

  using maxsat_solver_t = sat2::maxsat_solver<Algorithm>;
  sat2::maxsat_solver_statistics stats;
  sat2::maxsat_solver_params ps;
  maxsat_solver_t solver( stats, ps, sid );

  /* allocate variables */
  std::vector<int> v;
  for ( auto i = 0; i < 2; ++i )
    v.emplace_back( sid++ );

  /* add hard-clauses */
  solver.add_clause( { v[0] } );
  solver.add_clause( { v[1] } );

  /* solve */
  auto const result = solver.solve();
  CHECK( result == maxsat_solver_t::state::fail );
}

template<typename Algorithm>
void sat_soft_clauses_test()
{
  int sid = 1;

  using maxsat_solver_t = sat2::maxsat_solver<Algorithm>;
  sat2::maxsat_solver_statistics stats;
  sat2::maxsat_solver_params ps;
  maxsat_solver_t solver( stats, ps, sid );

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

  /* solve */
  auto const result = solver.solve();
  CHECK( result == maxsat_solver_t::state::success );

  /* check possible solutions */
  auto const enabled_clauses = solver.get_enabled_clauses();
  auto const disabled_clauses = solver.get_disabled_clauses();
  CHECK( enabled_clauses.size() == 5u );
  CHECK( disabled_clauses.size() == 1u );

  if ( enabled_clauses == std::vector<int>{ s0, s1, s3, s4, s5 } )
  {
    if ( disabled_clauses == std::vector<int>{ s2 } )
      return;
  }
  else if ( enabled_clauses == std::vector<int>{ s0, s1, s2, s4, s5 } )
  {
    if ( disabled_clauses == std::vector<int>{ s3 } )
      return;
  }
  CHECK( false );
}

template<typename Algorithm>
void unsat_soft_clauses_test()
{
  int sid = 1;

  using maxsat_solver_t = sat2::maxsat_solver<Algorithm>;
  sat2::maxsat_solver_statistics stats;
  sat2::maxsat_solver_params ps;
  maxsat_solver_t solver( stats, ps, sid );

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
  auto const s0 = solver.add_soft_clause( { v[0] } );
  auto const s1 = solver.add_soft_clause( { v[1] } );

  /* solve */
  auto const result = solver.solve();
  CHECK( result == maxsat_solver_t::state::success );

  /* check possible solutions */
  auto const enabled_clauses = solver.get_enabled_clauses();
  auto const disabled_clauses = solver.get_disabled_clauses();
  CHECK( enabled_clauses.size() == 0u );
  CHECK( disabled_clauses.size() == 2u );

  CHECK( disabled_clauses == std::vector<int>{ s0, s1 } );
  CHECK( enabled_clauses == std::vector<int>{} );
}

TEST_CASE( "Test unsatisfiable hard-clauses", "[sat]" )
{
  unsat_hard_clauses_test<sat2::maxsat_linear>();
  unsat_hard_clauses_test<sat2::maxsat_uc>();
  unsat_hard_clauses_test<sat2::maxsat_rc2>();
}

TEST_CASE( "Test no soft clauses", "[sat]" )
{
  no_soft_clauses_test<sat2::maxsat_linear>();
  no_soft_clauses_test<sat2::maxsat_uc>();
  no_soft_clauses_test<sat2::maxsat_rc2>();
}

TEST_CASE( "Test satisfiable soft-clauses", "[sat]" )
{
  sat_soft_clauses_test<sat2::maxsat_linear>();
  sat_soft_clauses_test<sat2::maxsat_uc>();
  sat_soft_clauses_test<sat2::maxsat_rc2>();
}

TEST_CASE( "Test unsatisfiable soft-clauses", "[sat]" )
{
  unsat_soft_clauses_test<sat2::maxsat_linear>();
  unsat_soft_clauses_test<sat2::maxsat_uc>();
  unsat_soft_clauses_test<sat2::maxsat_rc2>();
}
