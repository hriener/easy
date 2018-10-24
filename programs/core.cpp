/* easy: C++ ESOP library
 * Copyright (C) 2017-2018  EPFL
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <easy/sat2/sat_solver.hpp>
#include <easy/sat2/core_utils.hpp>
#include <fmt/printf.h>
#include <numeric>
#include <sstream>
#include <fstream>

using namespace easy;

void read_clause_from_file( std::vector<std::vector<int>>& clauses, int& sid, const std::string& filename )
{
  std::ifstream ifs( filename.c_str() );
  std::string line;
  while ( std::getline( ifs, line ) )
  {
    if ( line[0] == 'c' || line[0] == 'p' ) continue;

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

#if 0
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
#endif

struct statistics
{
  std::vector<uint32_t> size_of_core;
  std::vector<uint32_t> size_of_minimized_core;
};

int main(int argc, char **argv)
{
  std::string directory = argc > 1 ? argv[1] : "benchmarks/";

  /* some benchmarks */
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

  statistics stats;
  for ( const auto& filename : benchmarks )
  {    
    sat2::sat_solver_statistics sat_stats;
    sat2::sat_solver_params sat_ps;
    sat2::sat_solver solver( sat_stats, sat_ps );

    std::vector<std::vector<int>> clauses;
    int sid = 0;
    
    read_clause_from_file( clauses, sid, directory + filename );
    if ( clauses.empty() || sid == 0 )
    {
      std::cout << "[w] skipping file " << ( directory + filename ) << std::endl;
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

    assert( solver.solve( s ) == sat2::sat_solver::state::unsat );

    /* extract unsatisfiable core */
    auto const unsat_core = solver.get_core();
    assert( solver.solve( unsat_core ) == sat2::sat_solver::state::unsat );

    /* minimize unsatisfiable core */
    auto minimized_unsat_core = minimize_core_copy( solver, unsat_core, 10000u );
    assert( solver.solve( minimized_unsat_core ) == sat2::sat_solver::state::unsat );
    assert( minimized_unsat_core.size() <= unsat_core.size() );

    /* update statistics */
    stats.size_of_core.emplace_back( unsat_core.size() );
    stats.size_of_minimized_core.emplace_back( minimized_unsat_core.size() );
  }
  
  /* show statistics */
  auto avg = [&]( const auto& vs ){
    double const d = std::accumulate( std::begin( vs ), std::end( vs ), 0.0 );
    return ( d / vs.size() );
  }; /* avg */

  std::cout << fmt::printf( "[i] avg. unsat core size: initial = %8.2f / minimized = %8.2f / reduced = %5.2f%%\n",
                            avg( stats.size_of_core ),
                            avg( stats.size_of_minimized_core ),
                            ( 1.0 - avg( stats.size_of_minimized_core ) / avg( stats.size_of_core ) )*100 );
  
  return 0;
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:

