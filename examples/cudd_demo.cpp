#include "util/util.h"
#include "cudd/cudd.h"
#include "cplusplus/cuddObj.hh"
#undef fail

#include <kitty/constructors.hpp>
#include <kitty/static_truth_table.hpp>
#include <kitty/print.hpp>
#include <kitty/esop.hpp>
#include <fmt/format.h>

#include <iostream>
#include <unordered_map>

template<int num_vars>
std::ostream& operator<<( std::ostream& os, kitty::static_truth_table<num_vars> const& tt )
{
  return ( os << to_binary( tt ) );
}

void example1()
{
  std::cout << "=========================[    Example #1    ]=========================" << std::endl;
  DdManager *gbm = Cudd_Init( 0, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0 );

  DdNode *x0 = Cudd_bddNewVar( gbm );
  DdNode *x1 = Cudd_bddNewVar( gbm );
  DdNode *x2 = Cudd_bddNewVar( gbm );

  auto b0 = Cudd_bddXor( gbm, x0, x1 );
  auto b1 = Cudd_bddXor( gbm, b0, x2 );
  Cudd_Ref( b1 );

  Cudd_PrintDebug( gbm, b1, 2, 4 );

  Cudd_Quit(gbm);
}

void example2()
{
  std::cout << "=========================[    Example #2    ]=========================" << std::endl;
  Cudd mgr( 0, 2 );
  mgr.makeVerbose();

  BDD x0 = mgr.bddVar();
  BDD x1 = mgr.bddVar();
  BDD x2 = mgr.bddVar();

  auto b0 = x0 ^ x1;
  auto b1 = b0 ^ x2;

  b1.print( 2, 4 );
}

namespace magic
{

std::vector<uint32_t> compute_flips( uint32_t n )
{
  auto const size = ( 1u << n );
  auto const total_flips = size - 1;
  std::vector<uint32_t> flip_vec( total_flips );

  auto gray_number = 0u;
  auto temp = 0u;
  for ( auto i = 1u; i <= total_flips; ++i )
  {
    gray_number = i ^ ( i >> 1 );
    flip_vec[total_flips-i] = ffs( temp ^ gray_number ) - 1u;
    temp = gray_number;
  }

  return flip_vec;
}

std::vector<kitty::cube> compute_implicants( const kitty::cube& c, uint32_t num_vars )
{
  const auto flips = compute_flips( num_vars );

  std::vector<kitty::cube> impls = { c };
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

} /* magic */

void example3()
{
  std::cout << "=========================[    Example #3    ]=========================" << std::endl;

  uint64_t pow3[] =
    {
      1,
      3,
      9,
      27,
      81,
      243,
      729,
      2187,
      6561,
      19683,
      59049,
      177147,
      531441,
      1594323,
      4782969,
      14348907,
      43046721,
    }; // 3^n

  const uint32_t num_vars = 5u;
  assert( num_vars <= 16 ); /* we support at most 16 variables for now */

  Cudd mgr( 0, 2 );
  // mgr.AutodynEnable();
  mgr.AutodynDisable();
  mgr.EnableGarbageCollection();
  // mgr.DisableGarbageCollection();

  std::cout << pow3[5] << std::endl;
  std::vector<BDD> g( pow3[num_vars] );
  for ( uint64_t i = 0ul; i < g.size(); ++i )
    g[i] = mgr.bddVar();

  kitty::static_truth_table<num_vars> tt;
  create_from_expression( tt, "<<abc>de>" );

  const auto cover = kitty::esop_from_optimum_pkrm( tt );
  std::cout << "COVER: { ";
  for ( const auto& c : cover )
  {
    c.print( num_vars );
    std::cout << ' ';
  }
  std::cout << "} size = " << cover.size() << std::endl;

  std::cout << "tt = " << tt << std::endl;

  kitty::cube minterm;
  for ( uint32_t i = 0u; i < num_vars; ++i )
    minterm.set_mask( i );

  std::unordered_map<uint32_t, kitty::cube> index_to_cube;
  std::unordered_map<kitty::cube, uint32_t, kitty::hash<kitty::cube>> cube_to_index;
  auto last_used_index = 0;

  BDD helliwell = mgr.bddOne();
  for ( uint64_t i = 0ul; i < ( 1ul << num_vars ); ++i )
  {
    auto const m = kitty::get_bit( tt, i );

    minterm.print( num_vars );
    std::cout << fmt::format( "{:8d}. {:1d}\n", i, m );

    BDD term = mgr.bddOne() ^ ( m ? mgr.bddOne() : mgr.bddZero() );
    for ( const auto& subcube : magic::compute_implicants( minterm, num_vars ) )
    {
      // subcube.print( num_vars ); std::cout << ' ';

      if ( std::find( cover.begin(), cover.end(), subcube ) == cover.end() )
        continue;

      auto it = cube_to_index.find( subcube );
      if ( it == cube_to_index.end() )
      {
        auto const id = last_used_index++;
        cube_to_index.emplace( subcube, id );
        index_to_cube.emplace( id, subcube );
      }

      // std::cout << "[i] index access: " << cube_to_index.at( subcube ) << std::endl;
      BDD const& gv = g.at( cube_to_index.at( subcube ) );
      term ^= gv;
    }
    // std::cout << std::endl;

    helliwell *= term;

    std::cout << "BDD size = " << mgr.ReadNodeCount() << std::endl;

    ++minterm._bits;
  }

  if ( helliwell == mgr.bddZero() )
  {
    std::cout << "UNSATISFIABLE" << std::endl;
  }
  else
  {
    std::cout << "SATISFIABLE" << std::endl;
    uint32_t size = mgr.ReadSize();
    char *buffer = new char[size];
    helliwell.PickOneCube( buffer );

    std::vector<kitty::cube> optimized_cover;
    for ( uint32_t i = 0u; i < size; ++i )
    {
      if ( buffer[i] == 2 )
        continue;

      if ( buffer[i] )
      {
        optimized_cover.emplace_back( index_to_cube.at( i ) );
      }
    }
    delete[] buffer;

    std::cout << "OPTIMIZED COVER: { ";
    for ( const auto& c : optimized_cover )
    {
      c.print( num_vars );
      std::cout << ' ';
    }
    std::cout << "} size = " << optimized_cover.size() << std::endl;
  }
}

int main()
{
  example1();
  example2();
  example3();

  return 0;
}
