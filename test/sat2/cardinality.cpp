#include <catch.hpp>
#include <easy/sat2/sat_solver.hpp>
#include <easy/sat2/cardinality.hpp>

using namespace easy;

TEST_CASE( "Enumerate cardinality-5 solutions", "[cardinality]" )
{
  int sid = 1;

  sat2::sat_solver_statistics stats;
  sat2::sat_solver_params ps;
  sat2::sat_solver solver( stats, ps );

  auto const n = 5;
  auto const k_max = 5;

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

