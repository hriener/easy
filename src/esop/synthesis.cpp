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

#include <esop/synthesis.hpp>

namespace esop
{

/******************************************************************************
 * simple_synthesizer                                                         *
 ******************************************************************************/

simple_synthesizer::simple_synthesizer( const spec& spec )
  : _spec( spec )
{}

esop_t simple_synthesizer::synthesize( const simple_synthesizer_params& params )
{
  const int num_vars = log2( _spec.bits.size() );
  assert( _spec.bits.size() == (1ull << num_vars) && "bit-width of bits is not a power of 2" );
  assert( _spec.care.size() == (1ull << num_vars) && "bit-width of care is not a power of 2" );
  assert( num_vars <= 32 && "cube data structure cannot store more than 32 variables" );

  const auto num_terms = params.number_of_terms;
  assert( num_terms >= 1 );

  int sid = 1 + 2*num_vars*num_terms;

  sat::constraints constraints;
  sat::sat_solver solver;

  /* add constraints */
  kitty::cube minterm = kitty::cube::neg_cube( num_vars );

  auto sample_counter = 0u;
  do
  {
    /* skip don't cares */
    if ( ( _spec.bits[ minterm._bits ] != '0' && _spec.bits[ minterm._bits ] != '1' ) || _spec.care[ minterm._bits ] != '1' )
    {
      ++minterm._bits;
      continue;
    }

    std::vector<int> z_vars( num_terms, 0u );
    for ( auto j = 0; j < num_terms; ++j )
    {
      assert( sid == 1 + 2*num_vars*num_terms + sample_counter*num_terms + j );
      z_vars[ j ] = sid++;
    }

    for ( auto j = 0u; j < num_terms; ++j )
    {
      const int z = z_vars[ j ];

      // positive
      for ( auto l = 0; l < num_vars; ++l )
      {
        if ( minterm.get_bit( l ) )
        {
          std::vector<int> clause;
          clause.push_back( -z ); // - z_j
          clause.push_back( -( 1 + num_vars*num_terms + num_vars*j + l ) ); // -q_j,l

          constraints.add_clause( clause );
        }
        else
        {
          std::vector<int> clause;
          clause.push_back( -z ); // -z_j
          clause.push_back( -( 1 + num_vars*j + l ) ); // -p_j,l

          constraints.add_clause( clause );
        }
      }
    }

    for ( auto j = 0u; j < num_terms; ++j )
    {
      const int z = z_vars[ j ];

      // negative
      std::vector<int> clause = { z };
      for ( auto l = 0; l < num_vars; ++l )
      {
        if ( minterm.get_bit( l ) )
        {
          clause.push_back( 1 + num_vars*num_terms + num_vars*j + l ); // q_j,l
        }
        else
        {
          clause.push_back( 1 + num_vars*j + l ); // p_j,l
        }
      }

      constraints.add_clause( clause );
    }

    constraints.add_xor_clause( z_vars, _spec.bits[ minterm._bits ] == '1' );

    ++sample_counter;
    ++minterm._bits;
  } while( minterm._bits < (1 << num_vars) );

  sat::gauss_elimination().apply( constraints );
  sat::xor_clauses_to_cnf( sid ).apply( constraints );
  sat::cnf_symmetry_breaking( sid ).apply( constraints );

  if ( auto result = solver.solve( constraints ) )
  {
    return make_esop( result.model, num_terms, num_vars );
  }
  else
  {
    return {};
  }
}

nlohmann::json simple_synthesizer::stats() const
{
  return _stats;
}

esop_t simple_synthesizer::make_esop( const sat::sat_solver::model_t& model, unsigned num_terms, unsigned num_vars )
{
  esop_t esop;
  for ( auto j = 0u; j < num_terms; ++j )
  {
    kitty::cube c;
    bool cancel_cube = false;
    for ( auto l = 0; l < num_vars; ++l )
    {
      const auto p_value = model[ j*num_vars + l ] == l_True;
      const auto q_value = model[ num_vars*num_terms + j*num_vars + l ] == l_True;

      if ( p_value && q_value )
      {
        cancel_cube = true;
        break;
      }
      else if ( p_value )
      {
        c.add_literal( l, true );
      }
      else if ( q_value )
      {
        c.add_literal( l, false );
      }
    }

    if ( cancel_cube )
    {
      continue;
    }

    esop.push_back( c );
  }
  return esop;
}

/******************************************************************************
 * minimum_synthesizer                                                        *
 ******************************************************************************/

minimum_synthesizer::minimum_synthesizer( const spec& spec )
  : _spec( spec )
{}

esop_t minimum_synthesizer::synthesize( const minimum_synthesizer_params& params )
{
  const int num_vars = log2( _spec.bits.size() );
  assert( _spec.bits.size() == (1ull << num_vars) && "bit-width of bits is not a power of 2" );
  assert( _spec.care.size() == (1ull << num_vars) && "bit-width of care is not a power of 2" );
  assert( num_vars <= 32 && "cube data structure cannot store more than 32 variables" );

  esop_t esop;
  sat::sat_solver::result result;

  auto k = params.begin;
  do
  {
    int sid = 1 + 2*num_vars*k;

    sat::constraints constraints;
    sat::sat_solver solver;

    /* add constraints */
    kitty::cube minterm = kitty::cube::neg_cube( num_vars );

    auto sample_counter = 0u;
    do
    {
      /* skip don't cares */
      if ( ( _spec.bits[ minterm._bits ] != '0' && _spec.bits[ minterm._bits ] != '1' ) || _spec.care[ minterm._bits ] != '1' )
      {
        ++minterm._bits;
        continue;
      }

      std::vector<int> z_vars( k, 0u );
      for ( auto j = 0; j < k; ++j )
      {
        assert( sid == 1 + 2*num_vars*k + sample_counter*k + j );
        z_vars[ j ] = sid++;
      }

      for ( auto j = 0u; j < k; ++j )
      {
        const int z = z_vars[ j ];

        // positive
        for ( auto l = 0; l < num_vars; ++l )
        {
          if ( minterm.get_bit( l ) )
          {
            std::vector<int> clause;
            clause.push_back( -z ); // - z_j
            clause.push_back( -( 1 + num_vars*k + num_vars*j + l ) ); // -q_j,l

            constraints.add_clause( clause );
          }
          else
          {
            std::vector<int> clause;
            clause.push_back( -z ); // -z_j
            clause.push_back( -( 1 + num_vars*j + l ) ); // -p_j,l

            constraints.add_clause( clause );
          }
        }
      }

      for ( auto j = 0u; j < k; ++j )
      {
        const int z = z_vars[ j ];

        // negative
        std::vector<int> clause = { z };
        for ( auto l = 0; l < num_vars; ++l )
        {
          if ( minterm.get_bit( l ) )
          {
            clause.push_back( 1 + num_vars*k + num_vars*j + l ); // q_j,l
          }
          else
          {
            clause.push_back( 1 + num_vars*j + l ); // p_j,l
          }
        }

        constraints.add_clause( clause );
      }

      constraints.add_xor_clause( z_vars, _spec.bits[ minterm._bits ] == '1' );

      ++sample_counter;
      ++minterm._bits;
    } while( minterm._bits < (1 << num_vars) );

    sat::gauss_elimination().apply( constraints );
    sat::xor_clauses_to_cnf( sid ).apply( constraints );
    sat::cnf_symmetry_breaking( sid ).apply( constraints );

    if ( result = solver.solve( constraints ) )
    {
      esop = make_esop( result.model, k, num_vars );
    }
  } while ( params.next( k, bool(result) ) );
  return esop;
}

nlohmann::json minimum_synthesizer::stats() const
{
  return _stats;
}

esop_t minimum_synthesizer::make_esop( const sat::sat_solver::model_t& model, unsigned num_terms, unsigned num_vars )
{
  esop_t esop;
  for ( auto j = 0u; j < num_terms; ++j )
  {
    kitty::cube c;
    bool cancel_cube = false;
    for ( auto l = 0; l < num_vars; ++l )
    {
      const auto p_value = model[ j*num_vars + l ] == l_True;
      const auto q_value = model[ num_vars*num_terms + j*num_vars + l ] == l_True;

      if ( p_value && q_value )
      {
        cancel_cube = true;
        break;
      }
      else if ( p_value )
      {
        c.add_literal( l, true );
      }
      else if ( q_value )
      {
        c.add_literal( l, false );
      }
    }

    if ( cancel_cube )
    {
      continue;
    }

    esop.push_back( c );
  }
  return esop;
}

} /* esop */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
