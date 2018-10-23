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

/*!
  \file maxsat.hpp
  \brief MAXSAT solver

  \author Heinz Riener
*/

#pragma once

#include <easy/sat2/sat_solver.hpp>
#include <easy/sat2/core_utils.hpp>

namespace easy::sat2
{

struct maxsat_solver_statistics
{
}; /* maxsat_solver_statistics */

struct maxsat_solver_params
{
}; /* maxsat_solver_params */

class maxsat_solver
{
public:
  /* \brief Constructor
   *
   * Constructs a MAXSAT-solver
   *
   * \param stats Statistics
   * \param ps Parameters
   */
  explicit maxsat_solver( maxsat_solver_statistics& stats, maxsat_solver_params& ps )
    : _stats( stats )
    , _ps( ps )
  {}

protected:
  maxsat_solver_statistics& _stats;
  maxsat_solver_params const& _ps;
}; /* maxsat_solver */

} /* easy::sat2 */
