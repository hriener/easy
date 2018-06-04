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

#include <esop/esop.hpp>
#include <json/json.hpp>
#include <esop/exact_synthesis.hpp>

#include <sat/gauss.hpp>
#include <sat/xor_clauses_to_cnf.hpp>
#include <sat/cnf_symmetry_breaking.hpp>

namespace esop
{

struct spec
{
  std::string bits;
  std::string care;
}; /* spec */

enum state_t
{
  unknown      = 0
, realizable   = 10
, unrealizable = 20
}; /* state_t */

struct result
{
  result( state_t state = unknown )
    : state( state )
  {}

  result( const esop_t& esop )
    : state( realizable )
    , esop( esop )
  {}

  inline operator bool() const { return ( state == realizable ); }

  inline bool is_realizable()  const  { return ( state == realizable );  }
  inline bool is_unrealizable() const { return ( state == unrealizable ); }
  inline bool is_unknown() const      { return ( state == unknown ); }

  state_t state;
  esop_t  esop;
}; /* result */

/*! \brief Compute a cover from a truth table.
 *
 * Convert each minterm of the specification into one cube.
 *
 * \param spec Truth-table of a (incompletely-specified) Boolean function
 * \return An ESOP form
 */
esop::esop_t esop_cover( const esop::spec& spec );

struct simple_synthesizer_params
{
  /*! A fixed number of product terms (= k) */
  unsigned number_of_terms;
  int conflict_limit = -1;
}; /* simple_synthesizer_params */

/*! \brief Simple ESOP synthesizer
 *
 * Synthesizes a $k$-ESOP given an incompletely-specified Boolean function as specifiction.
 */
class simple_synthesizer
{
public:
  /*! \brief constructor
   *
   * Creates an ESOP synthesizer from a specification.
   *
   * \param spec Truth-table of a(n) (incompletely-specified) Boolean function
   */
  simple_synthesizer( const spec& spec );

  /*! \brief synthesize
   *
   * SAT-based ESOP synthesis.
   *
   * \params params Parameters
   * \return Synthesis result. If the specification is realizable and
   *         the no resource limit is reached, the result contains an ESOP
   *         that implements the specification.
   */
  result synthesize( const simple_synthesizer_params& params );

  /*! \brief stats
   *
   * \Return A json log with statistics logged during the synthesis
   */
  nlohmann::json stats() const;

private:
  /*! \brief make_esop
   *
   * Extract the ESOP from a satisfying assignments.
   *
   * \param model Satisfying assignment
   * \param num_terms Number of terms
   * \param num_vars Number of variables
   */
  esop_t make_esop( const sat::sat_solver::model_t& model, unsigned num_terms, unsigned num_vars );

private:
  const spec _spec;
  nlohmann::json _stats;
}; /* simple_synthesizer */

/*! \brief minimum_synthesizer_params
 *
 * Parameters for the minimum ESOP synthesizer.
 *
 * The parameters begin and next can be customized for upwards or downwards search.
 *
 * Example: (upwards search)
 *   minimum_synthesizer_params params;
 *   params.begin = 0;
 *   params.next = [&]( int i ){ if ( i >= max_k || sat ) return false; ++i; return true; }
 *
 * Example: (downwards search)
 *   minimum_synthesizer_params params;
 *   params.begin = max_k;
 *   params.next = [&]( int i ){ if ( i <= 0 || !sat ) return false; --i; return true; }
 *
 */
struct minimum_synthesizer_params
{
  /*! Start value for the search */
  int begin;
  /*! A function that evaluates the current value and satisfiability result, decides whether to
      terminate, and updates the value */
  std::function<bool(int&,bool)> next;
}; /* minimum_synthesizer_params */

/*! \brief Minimum ESOP synthesizer
 *
 * Similar to simple_synthesizer, but searches for a minimum ESOP in a
 * specified range.
 */
class minimum_synthesizer
{
public:
  /*! \brief constructor
   *
   * Creates an ESOP synthesizer from a specification.
   *
   * \param spec Truth-table of a(n) (incompletely-specified) Boolean function
   */
  minimum_synthesizer( const spec& spec );

  /*! \brief synthesize
   *
   * SAT-based ESOP synthesis.
   *
   * \params params Parameters
   * \return An ESOP form
   */
  esop_t synthesize( const minimum_synthesizer_params& params );

  /*! \brief stats
   *
   * \Return A json log with statistics logged during the synthesis
   */
  nlohmann::json stats() const;

private:
  /*! \brief make_esop
   *
   * Extract the ESOP from a satisfying assignments.
   *
   * \param model Satisfying assignment
   * \param num_terms Number of terms
   * \param num_vars Number of variables
   */
  esop_t make_esop( const sat::sat_solver::model_t& model, unsigned num_terms, unsigned num_vars );

private:
  const spec _spec;
  nlohmann::json _stats;
}; /* minimum_synthesizer */

/*! \brief minimum_synthesizer_params
 *
 * Parameters for the minimum ESOP synthesizer.
 *
 * The parameters begin and next can be customized for upwards or downwards search.
 *
 * Example: (upwards search)
 *   minimum_synthesizer_params params;
 *   params.begin = 0;
 *   params.next = [&]( int i ){ if ( i >= max_k || sat ) return false; ++i; return true; }
 *
 * Example: (downwards search)
 *   minimum_synthesizer_params params;
 *   params.begin = max_k;
 *   params.next = [&]( int i ){ if ( i <= 0 || !sat ) return false; --i; return true; }
 *
 */
struct minimum_all_synthesizer_params
{
  /*! Start value for the search */
  int begin;
  /*! A function that evaluates the current value and satisfiability result, decides whether to
      terminate, and updates the value */
  std::function<bool(int&,bool)> next;
}; /* minimum_all_synthesizer_params */

/*! \brief Minimum ESOP synthesizer
 *
 * Similar to simple_synthesizer, but searches for a minimum ESOP in a
 * specified range.
 */
class minimum_all_synthesizer
{
public:
  /*! \brief constructor
   *
   * Creates an ESOP synthesizer from a specification.
   *
   * \param spec Truth-table of a(n) (incompletely-specified) Boolean function
   */
  minimum_all_synthesizer( const spec& spec );

  /*! \brief synthesize
   *
   * SAT-based ESOP synthesis.
   *
   * \params params Parameters
   * \return An ESOP form
   */
  esops_t synthesize( const minimum_all_synthesizer_params& params );

  /*! \brief stats
   *
   * \Return A json log with statistics logged during the synthesis
   */
  nlohmann::json stats() const;

private:
  /*! \brief make_esop
   *
   * Extract the ESOP from a satisfying assignments.
   *
   * \param model Satisfying assignment
   * \param num_terms Number of terms
   * \param num_vars Number of variables
   */
  esop_t make_esop( const sat::sat_solver::model_t& model, unsigned num_terms, unsigned num_vars );

private:
  const spec _spec;
  nlohmann::json _stats;
}; /* minimum_all_synthesizer */

} /* esop */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
