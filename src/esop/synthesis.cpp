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

/******************************************************************************
 * minimum_all_synthesizer                                                    *
 ******************************************************************************/

static uint64_t pow3[32] = {
    0x1
  , 0x3
  , 0x9
  , 0x1b
  , 0x51
  , 0xf3
  , 0x2d9
  , 0x88b
  , 0x19a1
  , 0x4ce3
  , 0xe6a9
  , 0x2b3fb
  , 0x81bf1
  , 0x1853d3
  , 0x48fb79
  , 0xdaf26b
  , 0x290d741
  , 0x7b285c3
  , 0x17179149
  , 0x4546b3db
  , 0xcfd41b91
  , 0x26f7c52b3
  , 0x74e74f819
  , 0x15eb5ee84b
  , 0x41c21cb8e1
  , 0xc546562aa3
  , 0x24fd3027fe9
  , 0x6ef79077fbb
  , 0x14ce6b167f31
  , 0x3e6b41437d93
  , 0xbb41c3ca78b9
  , 0x231c54b5f6a2b
}; /* pow3 */

uint64_t cube_weight( const kitty::cube& c, unsigned num_vars )
{
  uint64_t value = 0;
  for ( auto i = 0u; i < num_vars; ++i )
  {
    value += c.get_mask( i ) ? ( c.get_bit( i ) ? pow3[ i ] : 0 ) : 2*pow3[ i ];
  }
  return value;
}

struct cube_weight_compare
{
  cube_weight_compare( unsigned num_vars = 32 );

  bool operator()( const kitty::cube& a, const kitty::cube& b ) const;

  unsigned _num_vars;
}; /* cube_weight_compare */

cube_weight_compare::cube_weight_compare( unsigned num_vars )
  : _num_vars( num_vars )
{}

bool cube_weight_compare::operator()( const kitty::cube& a, const kitty::cube& b ) const
{
  return cube_weight( a, _num_vars ) < cube_weight( b, _num_vars );
}

minimum_all_synthesizer::minimum_all_synthesizer( const spec& spec )
  : _spec( spec )
{}

esops_t minimum_all_synthesizer::synthesize( const minimum_all_synthesizer_params& params )
{
  const int num_vars = log2( _spec.bits.size() );
  assert( _spec.bits.size() == (1ull << num_vars) && "bit-width of bits is not a power of 2" );
  assert( _spec.care.size() == (1ull << num_vars) && "bit-width of care is not a power of 2" );
  assert( num_vars <= 32 && "cube data structure cannot store more than 32 variables" );

  esop::esop_t esop;
  sat::sat_solver::result result;

  std::unique_ptr<sat::constraints> constraints;
  std::unique_ptr<sat::sat_solver> solver;

  auto k = params.begin;
  do
  {
    int sid = 1 + 2*num_vars*k;

    constraints.reset( new sat::constraints );
    solver.reset( new sat::sat_solver );

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

            constraints->add_clause( clause );
          }
          else
          {
            std::vector<int> clause;
            clause.push_back( -z ); // -z_j
            clause.push_back( -( 1 + num_vars*j + l ) ); // -p_j,l

            constraints->add_clause( clause );
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

        constraints->add_clause( clause );
      }

      constraints->add_xor_clause( z_vars, _spec.bits[ minterm._bits ] == '1' );

      ++sample_counter;
      ++minterm._bits;
    } while( minterm._bits < (1 << num_vars) );

    sat::gauss_elimination().apply( *constraints );
    sat::xor_clauses_to_cnf( sid ).apply( *constraints );
    sat::cnf_symmetry_breaking( sid ).apply( *constraints );

    if ( result = solver->solve( *constraints ) )
    {
      esop = make_esop( result.model, k, num_vars );
    }
  } while ( params.next( k, bool(result) ) );

  /* restore the state if the last call has been unsat */
  if ( k < esop.size() )
  {
    k = esop.size();

    int sid = 1 + 2*num_vars*k;

    constraints.reset( new sat::constraints );
    solver.reset( new sat::sat_solver );

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

            constraints->add_clause( clause );
          }
          else
          {
            std::vector<int> clause;
            clause.push_back( -z ); // -z_j
            clause.push_back( -( 1 + num_vars*j + l ) ); // -p_j,l

            constraints->add_clause( clause );
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

        constraints->add_clause( clause );
      }

      constraints->add_xor_clause( z_vars, _spec.bits[ minterm._bits ] == '1' );

      ++sample_counter;
      ++minterm._bits;
    } while( minterm._bits < (1 << num_vars) );

    sat::gauss_elimination().apply( *constraints );
    sat::xor_clauses_to_cnf( sid ).apply( *constraints );
    sat::cnf_symmetry_breaking( sid ).apply( *constraints );
  }

  /* enumerate solutions */
  esop::esops_t esops;
  while ( auto result = solver->solve( *constraints ) )
  {
    std::vector<int> blocking_clause;
    std::vector<unsigned> vs;
    vs.resize( k );
    for ( auto i = 0u; i < k; ++i )
    {
      vs[i] = i;
    }

    esop = make_esop( result.model, k, num_vars );

    std::sort( esop.begin(), esop.end(), cube_weight_compare( num_vars ) );

    /* add one blocking clause for each possible permutation of the cubes */
    do
    {
      std::vector<int> blocking_clause;
      for ( auto j = 0u; j < vs.size(); ++j )
      {
        for ( auto l = 0; l < num_vars; ++l )
        {
          const auto p_value = result.model[ j*num_vars + l ] == l_True;
          const auto q_value = result.model[ num_vars*k + j*num_vars + l ] == l_True;

          /* do not consider all possibilities for canceled cubes */
          if ( p_value && q_value )
          {
            constraints->add_clause( { int(-(1 + vs[j]*num_vars + l)), int(-(1 + num_vars*k + vs[j]*num_vars + l)) } );
            continue;
          }

          blocking_clause.push_back( p_value ? -(1 + vs[j]*num_vars + l) : (1 + vs[j]*num_vars + l) );
          blocking_clause.push_back( q_value ? -(1 + num_vars*k + vs[j]*num_vars + l) : (1 + num_vars*k + vs[j]*num_vars + l) );
        }
      }
      constraints->add_clause( blocking_clause );
    } while ( std::next_permutation( vs.begin(), vs.end() ) );

    if ( esop.size() < k ) continue;
    esops.push_back( esop );
  }

  return esops;
}

nlohmann::json minimum_all_synthesizer::stats() const
{
  return _stats;
}

esop_t minimum_all_synthesizer::make_esop( const sat::sat_solver::model_t& model, unsigned num_terms, unsigned num_vars )
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
