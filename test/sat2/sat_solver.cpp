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



void read_clause_from_file( std::vector<std::vector<int>>& clauses, int& sid, const std::string& filename )
{
  std::ifstream ifs( filename.c_str() );

  std::string line;
  while ( std::getline( ifs, line ) )
  {
    if ( line[0] == 'c' ) continue;

    if ( line[0] == 'p' ) continue;

    std::istringstream ss( line );

    std::string token;

    std::vector<int> clause;
    while( std::getline( ss, token, ' ' ) )
    {
      if ( token == "0" || token == "" ) continue;

      int i = std::stoi( token, nullptr);
      if ( abs( i ) > sid )
      {
        sid = abs( i );
      }

      clause.emplace_back( i );
    }

    clauses.emplace_back( clause );
  }
  ifs.close();
}

TEST_CASE( "Test heuristic unsat core simplification", "[sat]" )
{
  std::vector<std::string> benchmarks = {
      "aim/aim-100-1_6-no-1.cnf",
      "aim/aim-100-1_6-no-2.cnf",
      "aim/aim-100-1_6-no-3.cnf",
      "aim/aim-100-1_6-no-4.cnf",
      "aim/aim-100-2_0-no-1.cnf",
      "aim/aim-100-2_0-no-2.cnf",
      "aim/aim-100-2_0-no-3.cnf",
      "aim/aim-100-2_0-no-4.cnf",
      "aim/aim-200-1_6-no-1.cnf",
      "aim/aim-200-1_6-no-2.cnf",
      "aim/aim-200-1_6-no-3.cnf",
      "aim/aim-200-1_6-no-4.cnf",
      "aim/aim-200-2_0-no-1.cnf",
      "aim/aim-200-2_0-no-2.cnf",
      "aim/aim-200-2_0-no-3.cnf",
      "aim/aim-200-2_0-no-4.cnf",
      "aim/aim-50-1_6-no-1.cnf",
      "aim/aim-50-1_6-no-2.cnf",
      "aim/aim-50-1_6-no-3.cnf",
      "aim/aim-50-1_6-no-4.cnf",
      "aim/aim-50-2_0-no-1.cnf",
      "aim/aim-50-2_0-no-2.cnf",
      "aim/aim-50-2_0-no-3.cnf",
      "aim/aim-50-2_0-no-4.cnf",
      "jnh/jnh10.cnf",
      "jnh/jnh11.cnf",
      "jnh/jnh13.cnf",
      "jnh/jnh14.cnf",
      "jnh/jnh15.cnf",
      "jnh/jnh16.cnf",
      "jnh/jnh18.cnf",
      "jnh/jnh19.cnf",
      "jnh/jnh2.cnf",
      "jnh/jnh20.cnf",
      "jnh/jnh202.cnf",
      "jnh/jnh203.cnf",
      "jnh/jnh206.cnf",
      "jnh/jnh208.cnf",
      "jnh/jnh211.cnf",
      "jnh/jnh214.cnf",
      "jnh/jnh215.cnf",
      "jnh/jnh216.cnf",
      "jnh/jnh219.cnf",
      "jnh/jnh3.cnf",
      "jnh/jnh302.cnf",
      "jnh/jnh303.cnf",
      "jnh/jnh304.cnf",
      "jnh/jnh305.cnf",
      "jnh/jnh306.cnf",
      "jnh/jnh307.cnf",
      "jnh/jnh308.cnf",
      "jnh/jnh309.cnf",
      "jnh/jnh310.cnf",
      "jnh/jnh4.cnf",
      "jnh/jnh5.cnf",
      "jnh/jnh6.cnf",
      "jnh/jnh8.cnf",
      "jnh/jnh9.cnf",
      // "pigeon-hole/hole10.cnf",
      "pigeon-hole/hole6.cnf",
      "pigeon-hole/hole7.cnf",
      "pigeon-hole/hole8.cnf",
      "pigeon-hole/hole9.cnf",
      "pret/pret150_25.cnf",
      "pret/pret150_40.cnf",
      "pret/pret150_60.cnf",
      "pret/pret150_75.cnf",
      "pret/pret60_25.cnf",
      "pret/pret60_40.cnf",
      "pret/pret60_60.cnf",
      "pret/pret60_75.cnf",
      "dubois/dubois100.cnf",
      "dubois/dubois20.cnf",
      "dubois/dubois21.cnf",
      "dubois/dubois22.cnf",
      "dubois/dubois23.cnf",
      "dubois/dubois24.cnf",
      "dubois/dubois25.cnf",
      "dubois/dubois26.cnf",
      "dubois/dubois27.cnf",
      "dubois/dubois28.cnf",
      "dubois/dubois29.cnf",
      "dubois/dubois30.cnf",
      "dubois/dubois50.cnf",
      "bf/bf0432-007.cnf",
      "bf/bf1355-075.cnf",
      "bf/bf1355-638.cnf",
      "bf/bf2670-001.cnf",
      "ssa/ssa0432-003.cnf",
      "ssa/ssa2670-130.cnf",
      "ssa/ssa2670-141.cnf",
      "ssa/ssa6288-047.cnf",
      "ssa/ssa7552-038.cnf",
      "ssa/ssa7552-158.cnf",
      "ssa/ssa7552-159.cnf",
      "ssa/ssa7552-160.cnf",
    };

  auto sum0 = 0, sum1 = 0, sum2 = 0;

  for ( const auto& filename : benchmarks )
  {
    sat2::sat_solver_statistics stats;
    sat2::sat_solver_params ps;
    sat2::sat_solver solver( stats, ps );

    std::vector<std::vector<int>> clauses;
    int sid = 0;
    read_clause_from_file( clauses, sid, "benchmarks/" + filename );

    if ( clauses.empty() || sid == 0 )
    {
      std::cout << "[w] skipping file " << filename << std::endl;
      continue;
    }

    std::vector<int> s;
    for ( auto& cl : clauses )
    {
      auto sel = sid++;
      s.emplace_back( sel );
      cl.emplace_back( -sel );
      solver.add_clause( cl );
    }

    CHECK( solver.solve( s ) == sat2::sat_solver::state::unsat );

    /* extract core */
    auto const cs = solver.get_core();
    CHECK( solver.solve( cs ) == sat2::sat_solver::state::unsat );

    auto trimmed_core = trim_core_copy( solver, cs, 10u );
    CHECK( solver.solve( trimmed_core ) == sat2::sat_solver::state::unsat );
    CHECK( trimmed_core.size() <= cs.size() );

    auto minimized_core = minimize_core_copy( solver, cs, 10000u );
    CHECK( solver.solve( minimized_core ) == sat2::sat_solver::state::unsat );
    CHECK( minimized_core.size() <= cs.size() );

    // std::cout << filename << ' ' << cs.size() << ' ' << trimmed_core.size() << ' ' << minimized_core.size() << std::endl;

    sum0 += cs.size();
    sum1 += trimmed_core.size();
    sum2 += minimized_core.size();
  }

  // std::cout << double(sum0)/sum0 << ' ' << double(sum1)/sum0 << ' ' << double(sum2)/sum0 << std::endl;
}
