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
#include <easy/sat2/cardinality.hpp>

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
  enum class state
  {
    fresh = 0,
    success = 1,
    fail = 2,
  }; /* state */

public:
  /* \brief Constructor
   *
   * Constructs a MAXSAT-solver
   *
   * \param stats Statistics
   * \param ps Parameters
   */
  explicit maxsat_solver( maxsat_solver_statistics& stats, maxsat_solver_params& ps, int& sid )
    : _stats( stats )
    , _ps( ps )
    , _sid( sid )
    , _solver( _sat_stats, _sat_params )
  {}

  /* \brief Adds a hard clause to the solver
   *
   * \param clause Clause to be added
   */
  void add_clause( std::vector<int> const& clause )
  {
    _solver.add_clause( clause );
  }

  /* \brief Adds a soft clause to the solver
   *
   * \param clause Clause to be added
   *
   * Returns the added activation variable.
   */
  int add_soft_clause( std::vector<int> const &clause )
  {
    int selector = _sid++;
    _selectors.push_back( -selector );

    std::vector<int> cl( clause );
    cl.emplace_back( -selector );
    _solver.add_clause( cl );
    return selector;
  }

  /* \brief Adds a multiple soft clause to the solver and uses the
   *        same activation variable for all of them
   *
   * \param clause Clause to be added
   *
   * Returns the added activation variable.
   */
  int add_soft_clauses( std::vector<std::vector<int>> const &clauses )
  {
    int selector = _sid++;
    _selectors.push_back( -selector );

    for ( auto cl : clauses )
    {
      cl.emplace_back( -selector );
      _solver.add_clause( cl );
    }
    return selector;
  }

  /*
   * \brief Naive maxsat procedure based on linear search and
   *        at-most-k cardinality constraint.
   *
   * The implementation is based on Z3's MAX-SAT example [1].
   * [1] https://github.com/Z3Prover/z3/blob/master/examples/maxsat/maxsat.c
   *
   * Returns the activation variables of the soft clauses that can be
   * activated.  The return value is empty if the hard clauses cannot
   * be satisfied.
   */
  state solve()
  {
    if ( _solver.solve() == sat2::sat_solver::state::unsat )
    {
      /* it's not possible to satisfy the clauses even when ignoring all soft clauses */
      std::cout << "[i] terminate: it's not possible to satisfy the hard clauses, even when all soft clauses are ignored" << std::endl;
      _state = state::fail;
      return _state;
    }

    if ( _selectors.size() == 0u )
    {
      /* nothing to be done */
      std::cout << "[i] terminate: no soft clauses" << std::endl;
      _state = state::fail;
      return _state;
    }

#if 0
    {
      std::vector<int> assumptions;
      for ( const auto& s : _selectors )
      {
        assumptions.emplace_back( s );
      }

      if ( _solver.solve( assumptions ) != sat2::sat_solver::state::sat )
      {
        std::cout << "[i] terminate: the problem is not sat, even if all soft clauses are disabled" << std::endl;
        _state = state::fail;
        return _state;
      }
    }

    {
      std::vector<int> assumptions;
      for ( const auto& s : _selectors )
      {
        assumptions.emplace_back( -s );
      }

      if ( _solver.solve( assumptions ) != sat2::sat_solver::state::unsat )
      {
        std::cout << "[i] terminate: the problem is not unsat if all soft clauses are enabled" << std::endl;
        _state = state::fail;
        return _state;
      }
    }
#endif

    std::vector<std::vector<int>> clauses;
    auto at_most_k = create_totalizer( clauses, _sid, _selectors, _selectors.size() );
    for ( const auto& c : clauses )
    {
      add_clause( c );
    }

#if 0
    /* disable all _selectors via cardiality assumptions, the problem must be sat */
    {
      std::vector<int> assumptions;
      for ( auto i = 0; i < at_most_k->vars.size(); ++i )
      {
        assumptions.emplace_back( at_most_k->vars[i] );
      }

      if ( _solver.solve( assumptions ) != sat2::sat_solver::state::sat )
      {
        std::cout << "[i] terminate: the problem is not sat, even when all soft clauses are disabled via cardinality assumptions" << std::endl;
        _state = state::fail;
        return _state;
      }
    }
#endif

    /* enforce that at most k soft clauses are satisfied */
    uint32_t k = _selectors.size() - 1u;

    /* perform linear search */
    for ( ;; )
    {
      // std::cout << "[i] try with k = " << k << std::endl;

      /* disable at-most k selectors */
      std::vector<int> assumptions;
      for ( auto i = 0; i < at_most_k->vars.size(); ++i )
      {
        assumptions.emplace_back( i < k ? at_most_k->vars[i] : -at_most_k->vars[i] );
      }

      if ( _solver.solve( assumptions ) == sat2::sat_solver::state::unsat )
      {
        /* unsat */
        _state = state::success;
        return _state;
      }

      auto const m = _solver.get_model();

      _enabled_clauses.clear();
      _disabled_clauses.clear();
      for ( const auto& s : _selectors )
      {
        if ( m[s] )
        {
          _disabled_clauses.push_back( -s );
        }
        else if ( !m[s] )
        {
          _enabled_clauses.push_back( -s );
        }
      }

      k = std::count_if( _selectors.begin(), _selectors.end(),
                         [&m]( auto const& s ){ return m[s]; });
      if ( k == 0 )
      {
        _state = state::fail;
        return _state;
      }
      --k;
    }
  }

  std::vector<int> get_enabled_clauses() const
  {
    return _enabled_clauses;
  }

  std::vector<int> get_disabled_clauses() const
  {
    return _disabled_clauses;
  }

protected:
  state _state = state::fresh;

  maxsat_solver_statistics& _stats;
  maxsat_solver_params const& _ps;
  int& _sid;

  sat_solver_statistics _sat_stats;
  sat_solver_params _sat_params;
  sat_solver _solver;

  std::vector<int> _selectors;

  std::vector<int> _enabled_clauses;
  std::vector<int> _disabled_clauses;
}; /* maxsat_solver */

} /* easy::sat2 */
