#include <catch.hpp>
#include <easy/sat/sat_solver.hpp>
#include <easy/sat/xor_clauses_to_cnf.hpp>

using namespace easy;

TEST_CASE( "Convert unsatisfiable XOR-constraints to CNF", "[sat]" )
{
  sat::constraints constraints;
  constraints.add_xor_clause( {  1,  2 } );
  constraints.add_xor_clause( { -1,  2 } );

  int sid = 3;
  sat::xor_clauses_to_cnf conv( sid );
  conv.apply( constraints );
  
  sat::sat_solver solver;
  auto sat = solver.solve( constraints );
  CHECK( sat.is_unsat() );
}

TEST_CASE( "Convert satisfiable XOR-constraints to CNF", "[sat]" )
{
  sat::constraints constraints;
  constraints.add_xor_clause( {  1,  2 } );
  constraints.add_xor_clause( { -1, -2 } );

  int sid = 3;
  sat::xor_clauses_to_cnf conv( sid );
  conv.apply( constraints );
  
  sat::sat_solver solver;
  auto sat = solver.solve( constraints );
  CHECK( sat.is_sat() );

  CHECK( static_cast<bool>( sat.model[0u] == l_True ) != static_cast<bool>( sat.model[1u] == l_True ) );
}
