#include <catch.hpp>
#include <easy/sat2/maxsat.hpp>

using namespace easy;

TEST_CASE( "MAXSAT solver", "[sat]" )
{
  sat2::maxsat_solver_statistics stats;
  sat2::maxsat_solver_params ps;
  sat2::maxsat_solver solver( stats, ps );
}
