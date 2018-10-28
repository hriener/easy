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
#include <map>

namespace easy::sat2
{

class clause_to_block_vars
{
public:
  void insert( int clause_id , int var )
  {
    auto it = _map.find( clause_id );
    if ( it == _map.end() )
    {
      _map.emplace( clause_id, std::vector<int>{ var } );
    }
    else
    {
      it->second.emplace_back( var );
    }
  }

  std::vector<int> lookup( int clause_id ) const
  {
    auto const it = _map.find( clause_id );
    if ( it != _map.end() )
    {
      return it->second;
    }
    else
    {
      return {};
    }
  }

protected:
  std::unordered_map<int, std::vector<int>> _map;
}; /* clause_to_block_vars */

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
   * \param clause Soft clause to be added
   *
   * Returns the added activation variable.
   */
  int add_soft_clause( std::vector<int> const &clause, int weight = 1 )
  {
    auto id = _soft_clauses.size();
    _soft_clauses.emplace_back( clause );
    _weights.emplace_back( weight );
    return id;
  }

  void add_one_hot_clauses( std::vector<int> lits )
  {
    if ( lits.size() == 1 )
    {
      add_clause( lits );
      return;
    }

    std::vector<std::vector<int>> clauses;
    auto at_most_1 = create_totalizer( clauses, _sid, lits, 2u );
    for ( const auto& c : clauses )
    {
      add_clause( c );
    }

    /* enable at-most 1 lits */
    add_clause( { at_most_1->vars[0u] } );
    for ( auto i = 1; i < at_most_1->vars.size(); ++i )
    {
      add_clause( { -at_most_1->vars[i] } );
    }

    add_clause( lits );
  }

  /*
   * \brief Fu&Malik MAXSAT procedure using UNSAT core extraction and
   * the at-most-k cardinality constraint.
   *
   * The implementation is based on Z3's MAX-SAT example [1].  As
   * seminal reference of the algorithm serves [2].
   *
   * [1] https://github.com/Z3Prover/z3/blob/master/examples/maxsat/maxsat.c
   * [2] Zhaohui Fu, Sharad Malik: On Solving the Partial MAX-SAT
   *    Problem. SAT 2006: 252-265
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

    /* if the number of soft-clauses is empty */
    if ( _soft_clauses.size() == 0u )
    {
      /* nothing to be done */
      std::cout << "[i] terminate: no soft clauses" << std::endl;
      _state = state::fail;
      return _state;
    }

    /* add the soft clauses */
    std::map<int,int> selector_to_clause_id;
    for ( auto i = 0; i < _soft_clauses.size(); ++i )
    {
      int selector = _sid++;
      selector_to_clause_id.emplace( selector, i );
      _selectors.push_back( -selector );

      auto cl = _soft_clauses[i];
      cl.emplace_back( -selector );
      add_clause( cl );
    }

    clause_to_block_vars block_variables;

    auto iteration = 0;
    for ( ;; )
    {
      /* assume all soft-clauses are enabled, which causes the problem to UNSAT */
      std::vector<int> assumptions;

      for ( const auto& s : _selectors )
      {
        assumptions.emplace_back( -s );
      }

      auto const state = _solver.solve( assumptions );
      if ( state == sat2::sat_solver::state::sat )
      {
        auto m = _solver.get_model();

        _disabled_clauses.clear();
        _enabled_clauses.clear();

        for ( auto i = 0; i < _selectors.size(); ++i )
        {
          if ( m[ -_selectors[i] ] )
          {
            /* evaluate block variables of the i-th clause */
            auto is_blocked = false;
            for ( const auto& v : block_variables.lookup( i ) )
            {
              if ( m[ v ] )
              {
                is_blocked = true;
                break;
              }
            }

            if ( !is_blocked )
            {
              _enabled_clauses.push_back( i );
              continue;
            }
          }

          /* otherwise, the clause is enabled */
          _disabled_clauses.push_back( i );
        }

        _state = state::success;
        return _state;
      }
      else
      {
        auto const core = _solver.get_core();

        std::vector<int> block_vars( core.size() );
        for ( auto i = 0; i < core.size(); ++i )
        {
          int sel = _sid++;
          int b = _sid++;

          auto const j = selector_to_clause_id.at( core[i] );
          auto& cl = _soft_clauses[j];
          cl.emplace_back( b );

          _selectors[j] = -sel;
          selector_to_clause_id.emplace( sel, j );

          block_vars[i] = b;
          block_variables.insert( j, b );

          std::vector<int> cl2( cl );
          cl2.emplace_back( -sel );
          add_clause( cl2 );
        }

        /* ensure that at-most-1 block vars is one */
        add_one_hot_clauses( block_vars );
      }

      ++iteration;
    }
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
  state solve2()
  {
    if ( _solver.solve() == sat2::sat_solver::state::unsat )
    {
      /* it's not possible to satisfy the clauses even when ignoring all soft clauses */
      std::cout << "[i] terminate: it's not possible to satisfy the hard clauses, even when all soft clauses are ignored" << std::endl;
      _state = state::fail;
      return _state;
    }

    /* if the number of soft-clauses is empty */
    if ( _soft_clauses.size() == 0u )
    {
      /* nothing to be done */
      std::cout << "[i] terminate: no soft clauses" << std::endl;
      _state = state::fail;
      return _state;
    }

    /* add the soft clauses */
    std::map<int,int> selector_to_clause_id;
    for ( auto i = 0; i < _soft_clauses.size(); ++i )
    {
      int selector = _sid++;
      selector_to_clause_id.emplace( selector, i );
      _selectors.push_back( -selector );

      auto& cl = _soft_clauses[i];
      cl.emplace_back( -selector );
      _solver.add_clause( cl );
    }

    std::vector<std::vector<int>> clauses;
    auto at_most_k = create_totalizer( clauses, _sid, _selectors, _selectors.size() );
    for ( const auto& c : clauses )
    {
      add_clause( c );
    }

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
          _disabled_clauses.push_back( selector_to_clause_id.at( -s ) );
        }
        else if ( !m[s] )
        {
          _enabled_clauses.push_back( selector_to_clause_id.at( -s ) );
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

  std::vector<std::vector<int>> _soft_clauses;
  std::vector<int> _weights;
}; /* maxsat_solver */

} /* easy::sat2 */
