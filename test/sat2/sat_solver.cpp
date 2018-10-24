#include <catch.hpp>
#include <easy/sat2/sat_solver.hpp>
#include <easy/sat2/core_utils.hpp>
#include <fmt/printf.h>
#include <fstream>

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

TEST_CASE( "Test SAT-solver state", "[sat]" )
{
  sat2::sat_solver_statistics stats;
  sat2::sat_solver_params ps;
  sat2::sat_solver solver( stats, ps );
  CHECK( solver.get_state() == sat2::sat_solver::state::fresh );
  CHECK( solver.get_num_variables() == 0u );

  solver.add_clause( { 1 } );
  CHECK( solver.solve() == sat2::sat_solver::state::sat );
  solver.add_clause( { -1 } );
  CHECK( solver.get_state() == sat2::sat_solver::state::dirty );
  CHECK( solver.solve() == sat2::sat_solver::state::unsat );
}

TEST_CASE( "Test get model", "[sat]" )
{
  sat2::sat_solver_statistics stats;
  sat2::sat_solver_params ps;
  sat2::sat_solver solver( stats, ps );

  int sid = 1;
  solver.add_clause( { 1 } );

  sat2::sat_solver::state result;
  sat2::model m;

  result = solver.solve();
  CHECK( result == sat2::sat_solver::state::sat );

  m = solver.get_model();
  CHECK( m[1] == true );

  solver.add_clause( { -2 } );
  result = solver.solve();
  CHECK( result == sat2::sat_solver::state::sat );

  m = solver.get_model();
  CHECK( m[2] == false );

  solver.add_clause( { 2 } );
  result = solver.solve();
  CHECK( result == sat2::sat_solver::state::unsat );
}

TEST_CASE( "Test get core", "[sat]" )
{
  sat2::sat_solver_statistics stats;
  sat2::sat_solver_params ps;
  sat2::sat_solver solver( stats, ps );

  int sid = 1;
  solver.add_clause( { -4, 1,  -3 } );
  solver.add_clause( { -5, 2 } );
  solver.add_clause( { -6, -2,  3 } );
  solver.add_clause( { -7, -2, -3 } );
  solver.add_clause( { -8, 2, 3 } );
  solver.add_clause( { -9, -1, 2, -3 } );

  auto result = solver.solve( { 4, 5, 6, 7, 8, 9 } );
  CHECK( result == sat2::sat_solver::state::unsat );

  auto counter = 0;
  sat2::core c = solver.get_core();
  for ( const auto& l : std::vector<int>( c ) )
  {
    switch( counter++ )
    {
    case 0:
      {
        CHECK( l == 5 );
      }
      break;
    case 1:
      {
        CHECK( l == 6 );
      }
      break;
    case 2:
      {
        CHECK( l == 7 );
      }
      break;
    default:
      {
        CHECK( false );
      }
      break;
    }
  }

  result = solver.solve( { 5, 6, 7 } );
  CHECK( result == sat2::sat_solver::state::unsat );

  result = solver.solve( { 4, 8, 9 } );
  CHECK( result == sat2::sat_solver::state::sat );
}

TEST_CASE( "Test SAT-solver with assumptions", "[sat]" )
{
  sat2::sat_solver_statistics stats;
  sat2::sat_solver_params ps;
  sat2::sat_solver solver( stats, ps );

  solver.add_clause( { 1, 2 } );

  CHECK( solver.solve( {} ) == sat2::sat_solver::state::sat );
  CHECK( solver.solve( { -1 } ) == sat2::sat_solver::state::sat );
  CHECK( solver.solve( { -2 } ) == sat2::sat_solver::state::sat );
  CHECK( solver.solve( { -1, -2 } ) == sat2::sat_solver::state::unsat );

  CHECK( solver.solve( { -1 } ) == sat2::sat_solver::state::sat );
  CHECK( solver.solve( { -2 } ) == sat2::sat_solver::state::sat );
  CHECK( solver.solve( {} ) == sat2::sat_solver::state::sat );

  solver.add_clause( { -1 } );
  CHECK( solver.solve( {} ) == sat2::sat_solver::state::sat );
  CHECK( solver.solve( { -2 } ) == sat2::sat_solver::state::unsat );

  solver.add_clause( { -2 } );
  CHECK( solver.solve( {} ) == sat2::sat_solver::state::unsat );
}

TEST_CASE( "Test unsat core extraction", "[sat]" )
{
  sat2::sat_solver_statistics stats;
  sat2::sat_solver_params ps;
  sat2::sat_solver solver( stats, ps );

  /* Example CNF from ''On Computing Minimum Unsatisfiable Cores'',
   * SAT 2004: we use assumption literals 4,...,9 for clause activation */
  solver.add_clause( { -4,  1, 2 } );
  solver.add_clause( { -5,  2 } );
  solver.add_clause( { -6, -2, 3 } );
  solver.add_clause( { -7, -2, -3 } );
  solver.add_clause( { -8,  2, 3 } );
  solver.add_clause( { -9, -1, 2, -3 } );

  /* clause selectors */
  std::vector<int> s = { 4, 5, 6, 7, 8, 9 };

  /* constrained to unsat core */
  CHECK( solver.solve( { s[0], s[1], s[2], s[3], s[4], s[5] } ) == sat2::sat_solver::state::unsat ); /* UC1 */
  CHECK( solver.solve( { s[0], s[1], s[2], s[3], s[4] } ) == sat2::sat_solver::state::unsat ); /* UC2 */
  CHECK( solver.solve( { s[0], s[1], s[2], s[3], s[5] } ) == sat2::sat_solver::state::unsat ); /* UC3 */
  CHECK( solver.solve( { s[0], s[2], s[3], s[4], s[5] } ) == sat2::sat_solver::state::unsat ); /* UC4 */
  CHECK( solver.solve( { s[1], s[2], s[3], s[4], s[5] } ) == sat2::sat_solver::state::unsat ); /* UC5 */
  CHECK( solver.solve( { s[0], s[1], s[2], s[3] } ) == sat2::sat_solver::state::unsat ); /* UC6 */
  CHECK( solver.solve( { s[1], s[2], s[3], s[4] } ) == sat2::sat_solver::state::unsat ); /* UC7 */
  CHECK( solver.solve( { s[1], s[2], s[3], s[5] } ) == sat2::sat_solver::state::unsat ); /* UC8 */
  CHECK( solver.solve( { s[1], s[2], s[3] } ) == sat2::sat_solver::state::unsat ); /* UC9 */

  CHECK( solver.solve( s ) == sat2::sat_solver::state::unsat );

  /* extract core */
  auto const cs = solver.get_core();

  /* verify that it's an unsat core */
  CHECK( solver.solve( cs ) == sat2::sat_solver::state::unsat );
}
