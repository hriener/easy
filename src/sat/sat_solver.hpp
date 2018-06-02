/* ESOP
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

#ifdef GLUCOSE_EXTENSION

#include <core/SolverTypes.h>
#include <core/Solver.h>
#include <cassert>
#include <memory>
#include <vector>

namespace sat
{

struct constraints
{
  using clause_t = std::vector<int>;
  using xor_clause_t = std::pair<clause_t,bool>;

  void add_clause( const clause_t& clause );
  void add_xor_clause( const clause_t& clause, bool value = true);

  std::vector<clause_t> _clauses;
  std::vector<xor_clause_t> _xor_clauses;

  unsigned _num_variables = 0;
}; /* constraints */

struct sat_solver
{
  using assumptions_t = std::vector<int>;
  using state_t = Glucose::lbool;
  using model_t = std::vector<Glucose::lbool>;

  struct result
  {
    result( state_t state = l_Undef )
      : state( state )
    {}

    result( const model_t& m )
      : state( l_True )
      , model( m )
    {}

    inline operator bool() const { return ( state == l_True ); }

    inline bool is_sat()  const  { return ( state == l_True );  }
    inline bool is_unsat() const { return ( state == l_False ); }
    inline bool is_undef() const { return ( state == l_Undef ); }

    state_t state;
    model_t model;
  }; /* result */

  sat_solver();
  result solve( constraints& constraints, const assumptions_t& assumptions = {} );
  void reset();

  void set_conflict_limit( int limit );
  int get_conflicts() const;

  unsigned _num_vars = 0;

  /* -1 indicates no conflict limit */
  int _conflict_limit = -1;

  std::unique_ptr<Glucose::Solver> _solver;
};

} /* sat */

#endif /* GLUCOSE_EXTENSION */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
