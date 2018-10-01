#include <catch.hpp>
#include <easy/sat/sat_solver.hpp>
#include <easy/sat/gauss.hpp>
#include <easy/sat/xor_clauses_to_cnf.hpp>
#include <easy/sat/cnf_writer.hpp>

using namespace easy;

TEST_CASE( "Eliminate equal xor-constraints", "[sat]" )
{
  sat::constraints constraints;
  constraints.add_xor_clause( { 1, 2 } );
  constraints.add_xor_clause( { 1, 2 } );
  constraints.add_xor_clause( { 2, 1 } );

  CHECK( constraints._xor_clauses.size() == 3u );

  sat::gauss_elimination gauss;
  auto deduced_unsat = gauss.apply( constraints );
  CHECK( !deduced_unsat );

  CHECK( constraints._xor_clauses.size() == 1u );
}

TEST_CASE( "Eliminate variables using Gauss", "[sat]" )
{
  sat::constraints constraints;
  constraints.add_xor_clause( { 1, 2, 3, 4 } );
  constraints.add_xor_clause( { 1, 2, 4 } );
  constraints.add_xor_clause( { 1, 3, 4 } );

  CHECK( constraints._xor_clauses.size() == 3u );

  sat::gauss_elimination gauss;
  auto deduced_unsat = gauss.apply( constraints );
  CHECK( !deduced_unsat );

  CHECK( constraints._xor_clauses.size() == 3u );

  /* constraint 1 does not change */
  CHECK( constraints._xor_clauses[0u].first == sat::constraints::clause_t{ 1, 2, 3, 4 } );
  CHECK( constraints._xor_clauses[0u].second );

  /* constraint 2 simplifies to x2 = false */
  CHECK( constraints._xor_clauses[1u].first == sat::constraints::clause_t{ 2 } );
  CHECK( !constraints._xor_clauses[1u].second );

  /* constraint 3 simplifies to x3 = false */
  CHECK( constraints._xor_clauses[2u].first == sat::constraints::clause_t{ 3 } );
  CHECK( !constraints._xor_clauses[2u].second );
}

TEST_CASE( "Gauss algorithm deduces UNSAT", "[sat]" )
{
  {
    sat::constraints constraints;
    constraints.add_xor_clause( {  1,  2 } );
    constraints.add_xor_clause( { -1,  2 } );

    CHECK( constraints._xor_clauses.size() == 2u );

    int sid = 3;
    sat::xor_clauses_to_cnf conv( sid );
    conv.apply( constraints );

    CHECK( constraints._xor_clauses.size() == 0u );
    CHECK( constraints._clauses.size() == 10u );

    sat::sat_solver solver;
    auto sat = solver.solve( constraints );
    CHECK( sat.is_unsat() );
  }

  {
    sat::constraints constraints;
    constraints.add_xor_clause( {  1,  2 } );
    constraints.add_xor_clause( { -1,  2 } );

    CHECK( constraints._xor_clauses.size() == 2u );

    sat::gauss_elimination gauss;
    auto unsat_deduced = gauss.apply( constraints );
    CHECK( unsat_deduced );

    int sid = 3;
    sat::xor_clauses_to_cnf conv( sid );
    conv.apply( constraints );

    CHECK( constraints._xor_clauses.size() == 0u );

    sat::sat_solver solver;
    auto sat = solver.solve( constraints );
    CHECK( sat.is_unsat() );
  }
}
