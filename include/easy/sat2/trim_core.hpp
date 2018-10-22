/* easy: C++ ESOP library
 * Copyright (C) 2018  EPFL
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

#pragma once

#include <easy/sat2/sat_solver.hpp>

namespace easy::sat2
{

/* \brief Trim unsatisfiable core
 *
 * Given a SAT-solver `solver` and a unsatisfiable core `cs`, such
 * that the solver UNSAT under the given assumptions.  The algorithm
 * heuristically tries to trim `cs`.  The core is trimmed at
 * most `num_tries`-times.
 *
 * \param solver SAT-solver
 * \param cs An unsatisfiable core
 * \param num_tries Maximal number of tries to trim core
 *
 * Returns the trimmed core.
 */
  core<> trim_core_copy( sat_solver& solver, core<> const& cs, uint32_t num_tries = 8u )
{
  auto current = cs;

  int32_t counter = 0;
  while ( counter++ < num_tries && solver.solve( current ) == sat_solver::state::unsat )
  {
    auto const new_core = solver.get_core();
    if ( new_core.size() == current.size() )
    {
      break;
    }
    else
    {
      current = new_core;
    }
  }

  return current;
}

/* \brief Trim unsatisfiable core
 *
 * Given a SAT-solver `solver` and a unsatisfiable core `cs`, such
 * that the solver UNSAT under the given assumptions.  The algorithm
 * heuristically tries to trim `cs` inplace.  The core is trimmed at
 * most `num_tries`-times.
 *
 * \param solver SAT-solver
 * \param cs An unsatisfiable core
 * \param num_tries Maximal number of tries to trim core
 */
void trim_core( sat_solver& solver, core<>& cs, uint32_t num_tries = 8u )
{
  auto const new_cs = trim_core_copy( solver, cs, num_tries );
  cs = new_cs;
}

} /* easy::sat2 */
