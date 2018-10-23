#include <catch.hpp>
#include <easy/sat2/sat_solver.hpp>
#include <easy/sat2/cardinality.hpp>

using namespace easy;

TEST_CASE( "Enumerate cardinality-5 solutions", "[cardinality]" )
{
  sat2::sat_solver_statistics stats;
  sat2::sat_solver_params ps;
  sat2::sat_solver solver( stats, ps );

  auto const n = 5;
  auto const k_max = 5;

  int sid = 1;

  std::vector<int> lits;
  for ( auto i = 0; i < n; ++i )
  {
    lits.emplace_back( sid++ );
  }

  std::vector<std::vector<int>> cls;
  auto tree = sat2::create_totalizer( cls, sid, lits, k_max );
  for ( const auto& c : cls )
  {
    solver.add_clause( c );
  }

  std::vector<int> blocked;
  for ( auto k = 0; k <= k_max; ++k )
  {
    // std::cout << "[i] k = " << k << std::endl;
    std::vector<int> assumptions;
    for ( auto i = 0; i < k_max; ++i )
    {
      if ( i < k )
      {
        assumptions.push_back( tree->vars[i] );
      }
      else
      {
        assumptions.push_back( -tree->vars[i] );
      }
    }

    while ( solver.solve( assumptions ) == sat2::sat_solver::state::sat )
    {
      auto m = solver.get_model();

      /* count number of one-bits */
      auto counter = 0;
      std::vector<int> cl;
      for ( const auto& l : lits )
      {
        if ( m[l] )
          ++counter;

        cl.push_back( m[l] ? -l : l );
      }
      solver.add_clause( cl );

      /* number of one-bits must be equal to k */
      CHECK( counter == k );
    }
  }
}

TEST_CASE( "Increase cardinality n=7, k=3 to cardinality k=6", "[cardinality]" )
{
  sat2::sat_solver_statistics stats;
  sat2::sat_solver_params ps;
  sat2::sat_solver solver( stats, ps );

  /* 7 variables, cardinality constraint with at most 3 */
  auto const n = 7;
  auto k_max = 3;
  auto k = 2; /* k must be smaller than k_max */

  int sid = 1;

  std::vector<int> lits;
  for ( auto i = 0; i < n; ++i )
  {
    lits.emplace_back( sid++ );
  }

  /* create cardinality constraint */
  std::vector<std::vector<int>> cls;
  auto tree = sat2::create_totalizer( cls, sid, lits, k_max );
  for ( const auto& c : cls )
  {
    solver.add_clause( c );
  }

  /* prepare assumptions */
  std::vector<int> assumptions;
  for ( auto i = 0; i < k_max; ++i )
  {
    assumptions.push_back( ( i < k ) ? tree->vars[i] : -tree->vars[i] );
  }

  /* enumerate all solutions and block them */
  auto num_k2_solutions = 0;
  while ( solver.solve( assumptions ) == sat2::sat_solver::state::sat )
  {
    ++num_k2_solutions;

    auto m = solver.get_model();

    std::vector<int> clause;

    auto count = 0;
    for ( const auto& l : lits )
    {
      clause.push_back( m[l] ? -l : l );

      /* count number of ones */
      if ( m[l] )
      {
        ++count;
      }
    }

    /* block */
    solver.add_clause( clause );

    /* in each solution at most k bits are set */
    CHECK( count <= k );
  }

  /* increase the cardinality constraint to k_max = 6 */
  k_max = 6;
  std::vector<std::vector<int>> cls2;
  increase_totalizer( cls2, sid, tree, k_max );
  for ( const auto& c : cls2 )
  {
    solver.add_clause( c );
  }

  /* now we increase k = 5 and prepare new assumptions */
  k = 5;
  assumptions.clear();
  for ( auto i = 0; i < k_max; ++i )
  {
    assumptions.push_back( ( i < k ) ? tree->vars[i] : -tree->vars[i] );
  }

  /* ... and enumerate again */
  auto num_k5_solutions = 0;
  while ( solver.solve( assumptions ) == sat2::sat_solver::state::sat )
  {
    ++num_k5_solutions;

    auto m = solver.get_model();

    std::vector<int> clause;
    auto count = 0;
    for ( const auto& l : lits )
    {
      clause.push_back( m[l] ? -l : l );

      /* count number of ones */
      if ( m[l] )
      {
        ++count;
      }
    }

    /* block */
    solver.add_clause( clause );

    /* in each solution at most k bits are set */
    CHECK( count <= k );
  }

  CHECK( num_k2_solutions == 29 );
  CHECK( num_k5_solutions == 91 );
}
