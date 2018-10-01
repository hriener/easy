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

#include <easy/esop/esop.hpp>
#include <easy/sat/sat_solver.hpp>
#include <easy/sat/xor_clauses_to_cnf.hpp>
#include <easy/sat/cnf_writer.hpp>
#include <kitty/constructors.hpp>

namespace easy::esop
{

namespace detail
{

std::vector<uint32_t> compute_flips( uint32_t n )
{
  auto const size = ( 1u << n );
  auto const total_flips = size - 1;
  std::vector<uint32_t> flip_vec( total_flips );

  auto gray_number = 0u;
  auto temp = 0u;
  for ( auto i = 0u; i < total_flips; ++i )
  {
    gray_number = i ^ ( i >> 1 );
    flip_vec[i] = ffs( temp ^ gray_number ) - 1u;
    temp = gray_number;
  }

  return flip_vec;
}

std::vector<kitty::cube> compute_implicants( const kitty::cube& c, uint32_t num_vars )
{
  const auto flips = compute_flips( num_vars );
  const auto size = 1u << num_vars;

  std::vector<kitty::cube> impls = {c};
  auto copy = c;
  for ( const auto& flip : flips )
  {
    if ( copy.get_mask( flip ) )
    {
      copy.clear_bit( flip );
      copy.clear_mask( flip );
    }
    else
    {
      copy.set_mask( flip );
      if ( c.get_bit( flip ) )
      {
        copy.set_bit( flip );
      }
      else
      {
        copy.clear_bit( flip );
      }
    }

    impls.emplace_back( copy );
  }

  return impls;
}

template<class Solver, typename TT>
class helliwell_esop_synthesizer
{
public:
  explicit helliwell_esop_synthesizer( const TT& tt_bits,  const TT& tt_care )
    : tt_bits( tt_bits )
    , tt_care( tt_care )
  {
    assert( tt_bits.num_vars() == tt_care.num_vars() );
  }

  esop_t operator()( )
  {
    derive_xor_clauses( tt_bits, tt_care );

    auto const max_impl_id = sid;

    /* XOR-clauses to CNF */
    sat::xor_clauses_to_cnf conv( sid );
    conv.apply( constraints );

    /* solve */
    auto const sat = solver.solve( constraints );
    assert( sat );

    if ( sat )
    {
      return esop_from_model( sat.model, max_impl_id );
    }
    else
    {
      return esop_t();
    }
  }

  int lookup_var( const kitty::cube& c ) const
  {
    return cube_to_var.at( c );
  }

  const kitty::cube& lookup_cube( int v ) const
  {
    return var_to_cube.at( v );
  }

protected:
  esop_t esop_from_model( sat::sat_solver::model_t const& model, int max_impl_id )
  {
    esop_t esop;
    for ( auto i = 0; i < max_impl_id-1; ++i )
    {
      if ( model[i] == l_True )
      {
        esop.emplace_back( lookup_cube( i+1 ) );
      }
    }
    return esop;
  }

  void derive_xor_clauses( TT const& tt_bits, TT const& tt_care )
  {
    assert( tt_bits.num_vars() == tt_care.num_vars() );

    /* TODO: create a zero cube */
    kitty::cube minterm;
    for ( auto i = 0; i < tt_bits.num_vars(); ++i )
    {
      minterm.set_mask( i );
    }

    do
    {
      if ( kitty::get_bit( tt_care, minterm._bits ) )
      {
        sat::constraints::clause_t clause;
        for ( const auto& impl : compute_implicants( minterm, tt_bits.num_vars() ) )
        {
          clause.push_back( get_or_create_var( impl ) );
        }
        constraints.add_xor_clause( clause, kitty::get_bit( tt_bits, minterm._bits ) );
      }

      ++minterm._bits;
    } while( minterm._bits < ( 1u << tt_bits.num_vars() ) );
  }

  int get_or_create_var( const kitty::cube& c )
  {
    auto it = cube_to_var.find( c );
    int variable;
    if ( it == cube_to_var.end() )
    {
      variable = sid;
      cube_to_var.emplace( c, variable );
      var_to_cube.emplace( variable, c );
      sid++;
    }
    else
    {
      variable = it->second;
    }
    return variable;
  }

protected:
  const TT& tt_bits;
  const TT& tt_care;

  Solver solver;
  sat::constraints constraints;

  int sid = 1;
  std::unordered_map<kitty::cube, int, kitty::hash<kitty::cube>> cube_to_var;
  std::unordered_map<int, kitty::cube> var_to_cube;
}; /* helliwell_esop_synthesizer */
} // namespace detail

/*! \brief Computes ESOP from from an incompletely-specified Boolean function
 *
 * This algorithm solves the Helliwell decision problem using a SAT-solver.
 *
 * \param tt_bits Truth table of function
 * \param tt_care Truth table of care function
 */
template<typename TT>
inline esop_t esop_from_helliwell( const TT& tt_bits,  const TT& tt_care )
{
  return detail::helliwell_esop_synthesizer<sat::sat_solver,TT>( tt_bits, tt_care )();
}

/*! \brief Computes ESOP from from an completely-specified Boolean function
 *
 * This algorithm solves the Helliwell decision problem using a SAT-solver.
 *
 * \param tt_bits Truth table of function
 */
template<typename TT>
inline esop_t esop_from_helliwell( const TT& tt_bits )
{
  auto const tt_care = kitty::create<TT>( tt_bits.num_vars() );
  return esop_from_helliwell( tt_bits, ~tt_care );
}

} // easy::esop

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
