#include <catch.hpp>
#include <easy/sat/sat_solver.hpp>

using namespace easy;

TEST_CASE( "satisfiable", "[sat]" )
{
  sat::constraints constraints;
  constraints.add_clause( { 1, 2 } );
  
  sat::sat_solver solver;
  auto sat = solver.solve( constraints );
  CHECK( sat );
  CHECK( sat.is_sat() );
}

TEST_CASE( "unsatisfiable", "[sat]" )
{
  sat::constraints constraints;
  constraints.add_clause( { 1 } );
  constraints.add_clause( { -1 } );
  
  sat::sat_solver solver;
  auto sat = solver.solve( constraints );
  CHECK( !sat );
  CHECK( sat.is_unsat() );
}

TEST_CASE( "conflict_limit", "[sat]" )
{
  sat::constraints constraints;
  constraints.add_clause( {  1, -2 } );
  constraints.add_clause( { -1,  2 } );
  constraints.add_clause( { -1, -2 } );
  constraints.add_clause( {  1,  2 } );
  
  sat::sat_solver solver;
  solver.set_conflict_limit( 1 );
  auto sat = solver.solve( constraints );

  CHECK( solver.get_conflicts() > 1 );
  CHECK( sat.is_undef() );

  /* casting to bool leads to UNSAT */
  CHECK( !sat );

  /* the result, however, is neither true nor false */
  CHECK( !sat.is_sat() );
  CHECK( !sat.is_unsat() );
}

