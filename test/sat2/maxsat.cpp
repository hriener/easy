#include <catch.hpp>
#include <easy/sat2/maxsat.hpp>

using namespace easy;

TEST_CASE( "MAXSAT solver", "[sat]" )
{
  int sid = 1;

  sat2::maxsat_solver_statistics stats;
  sat2::maxsat_solver_params ps;
  sat2::maxsat_solver solver( stats, ps, sid );

  /* allocate variables */
  std::vector<int> v;
  for ( auto i = 0; i < 3; ++i )
  {
    v.emplace_back( sid++ );
  }

  auto s0 = solver.add_soft_clause( {  v[0], v[1] } );
  auto s1 = solver.add_soft_clause( {  v[1] } );
  auto s2 = solver.add_soft_clause( { -v[1], v[2] } );
  auto s3 = solver.add_soft_clause( { -v[1], -v[2] } );
  auto s4 = solver.add_soft_clause( {  v[1], v[2] } );
  auto s5 = solver.add_soft_clause( { -v[0], v[1], -v[2] } );

  auto result = solver.solve();
  CHECK( result.size() == 5u );
  CHECK( result == std::vector<int>{ s0, s1, s3, s4, s5 } );
}
