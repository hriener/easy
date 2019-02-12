#include "util/util.h"
#include "cudd/cudd.h"
#include "cudd/cuddInt.h"
#include "cplusplus/cuddObj.hh"
#undef fail

#include <kitty/constructors.hpp>
#include <kitty/static_truth_table.hpp>
#include <kitty/print.hpp>
#include <kitty/esop.hpp>
#include <easy/esop/cost.hpp>
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

void add_neighbors( std::vector<kitty::cube>& cover, uint32_t num_vars, uint32_t limit )
{
  auto count_new_cubes = 0u;
  if ( count_new_cubes >= limit )
    return;

  for ( const auto& c : cover )
  {
    for ( auto i = 0u; i < num_vars; ++i )
    {
      kitty::cube copy( c );
      copy.set_bit( i );
      copy.set_mask( i );
      if ( std::find( cover.begin(), cover.end(), copy ) == cover.end() )
      {
        cover.emplace_back( copy );
        ++count_new_cubes;
        if ( count_new_cubes >= limit )
          return;
      }

      copy.clear_bit( i );
      if ( std::find( cover.begin(), cover.end(), copy ) == cover.end() )
      {
        cover.emplace_back( copy );
        ++count_new_cubes;
        if ( count_new_cubes >= limit )
          return;
      }

      copy.clear_mask( i );
      if ( std::find( cover.begin(), cover.end(), copy ) == cover.end() )
      {
        cover.emplace_back( copy );
        ++count_new_cubes;
        if ( count_new_cubes >= limit )
          return;
      }
    }
  }
}

} /* magic */

class ExtractAssignments
{
public:
  explicit ExtractAssignments( Cudd const& mgr )
    : mgr( mgr )
  {}

  std::vector<std::vector<int>> extract( BDD const& node )
  {
    int *list = ALLOC( int, mgr.getManager()->size );
    if ( list == NULL )
    {
      mgr.getManager()->errorCode = CUDD_MEMORY_OUT;
      assert( false );
      std::cout << "ERROR: extracting assignments out-of-memory" << std::endl;
      return {};
    }

    for ( auto i = 0; i < mgr.getManager()->size; ++i )
      list[i] = 2;
    extract_recur( node.getNode(), list );
    FREE( list );

    return get_assignments();
  }

  std::vector<std::vector<int>> get_assignments() const
  {
    return assignments;
  }

protected:
  void extract_recur( DdNode* node, int *list )
  {
    DdNode *N = Cudd_Regular( node );
    if ( cuddIsConstant( N ) )
    {
      if ( node != Cudd_Not( mgr.getManager()->one ) )
      {
        for ( auto i = 0; i < mgr.getManager()->size; ++i )
        {
          int v = list[i];
          if ( v == 0 )
            current_assignment.emplace_back( -i );
          else if ( v == 1 )
            current_assignment.emplace_back( i );
        }
        if ( current_assignment.size() > 0 )
        {
          assignments.emplace_back( current_assignment );
          current_assignment.clear();
        }
      }
    }
    else
    {
      DdNode *Nv  = cuddT( N );
      DdNode *Nnv = cuddE( N );
      if ( Cudd_IsComplement( node ) )
      {
        Nv  = Cudd_Not( Nv );
        Nnv = Cudd_Not( Nnv );
      }
      uint32_t index = N->index;
      list[index] = 0;
      extract_recur( Nnv, list );
      list[index] = 1;
      extract_recur( Nv, list );
      list[index] = 2;
    }
  }

protected:
  Cudd const& mgr;
  std::vector<int> current_assignment;
  std::vector<std::vector<int>> assignments;
}; /* ExtractAssignments */

void PrintBDDRecur( Cudd const& mgr, DdNode* node, int *list )
{
  DdNode *N = Cudd_Regular( node );
  if ( cuddIsConstant( N ) )
  {
    /* Terminal case: Print one cube based on the current
       recursion path, unless we have reached the backgrounnd value
       (ADDs) or the logical zero (BDDs). */
    if ( node != Cudd_Not( mgr.getManager()->one ) )
    {
      for ( auto i = 0; i < mgr.getManager()->size; ++i )
      {
        int v = list[i];
        if ( v == 0 )
          std::cout << "0";
        else if ( v == 1 )
          std::cout << "1";
        else
          std::cout << "-";
      }
      std::cout << cuddV( node ) << "\n";
    }
  }
  else
  {
    DdNode *Nv  = cuddT( N );
    DdNode *Nnv = cuddE( N );
    if ( Cudd_IsComplement( node ) )
    {
      Nv  = Cudd_Not( Nv );
      Nnv = Cudd_Not( Nnv );
    }
    uint32_t index = N->index;
    list[index] = 0;
    PrintBDDRecur( mgr, Nnv, list );
    list[index] = 1;
    PrintBDDRecur( mgr, Nv, list );
    list[index] = 2;
  }
}

void print_cover( std::vector<kitty::cube> const& cubes, uint32_t num_vars )
{
  std::cout << "{ ";
  for ( const auto& c : cubes )
  {
    c.print( num_vars );
    std::cout << ' ';
  }
  std::cout << "} size = " << cubes.size() << " T-cost = " << easy::esop::T_count( cubes, num_vars ) << std::endl;
}


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

  const uint32_t num_vars = 3u;
  assert( num_vars <= 16 ); /* we support at most 16 variables for now */

  Cudd mgr( 0, 2 );
  // mgr.AutodynEnable();
  mgr.AutodynDisable();
  mgr.EnableGarbageCollection();
  // mgr.DisableGarbageCollection();

  std::vector<BDD> g( pow3[num_vars] );
  for ( uint64_t i = 0ul; i < g.size(); ++i )
    g[i] = mgr.bddVar();

  kitty::static_truth_table<num_vars> tt;
  create_from_expression( tt, "(abc)" );

  auto pkrm_cover = kitty::esop_from_optimum_pkrm( tt );
  auto cover = pkrm_cover;
  std::cout << "COVER: { ";
  for ( const auto& c : cover )
  {
    c.print( num_vars );
    std::cout << ' ';
  }
  std::cout << "} size = " << cover.size() << std::endl;

  // magic::add_neighbors( cover, num_vars, 0 ); // try: 22

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

    // std::cout << "BDD size = " << mgr.ReadNodeCount() << std::endl;
    std::cout << "BDD size = " << helliwell.nodeCount() << std::endl;

    ++minterm._bits;
  }

  if ( helliwell == mgr.bddZero() )
  {
    std::cout << "UNSATISFIABLE" << std::endl;
  }
  else
  {
    std::cout << "SATISFIABLE" << std::endl;
    // helliwell.PrintMinterm();

    uint64_t best_Tcount = std::numeric_limits<uint64_t>::max();

    ExtractAssignments ext( mgr );
    auto const assignments = ext.extract( helliwell );
    for ( const auto& a : assignments )
    {
      std::vector<kitty::cube> optimized_cover;
      for ( const auto& i : a )
      {
        if ( i > 0 )
        {
          optimized_cover.emplace_back( index_to_cube.at( i ) );
        }
      }

      /* verification */
      kitty::static_truth_table<num_vars> tt_opt;
      kitty::create_from_cubes( tt_opt, optimized_cover, true );
      if ( tt_opt == tt )
      {
        std::cout << "SUCCESS" << std::endl;
      }
      else
      {
        print_cover( optimized_cover, num_vars );
        print_cover( pkrm_cover, num_vars );
        std::cout << "FAILED" << std::endl;
        kitty::print_binary( tt ); std::cout << std::endl;
        kitty::print_binary( tt_opt ); std::cout << std::endl;
        continue;
      }

      auto const T_cost = easy::esop::T_count( optimized_cover, num_vars );
      // std::cout << "OPTIMIZED COVER: { ";
      // for ( const auto& c : optimized_cover )
      // {
      //   c.print( num_vars );
      //   std::cout << ' ';
      // }
      // std::cout << "} size = " << optimized_cover.size() << " T-cost = " << T_cost << std::endl;

      if ( best_Tcount > T_cost )
      {
        best_Tcount = T_cost;
      }
    }

    print_cover( pkrm_cover, num_vars );
    print_cover( cover, num_vars );

    std::cout << "T-cost of PKRM cover: " << easy::esop::T_count( pkrm_cover, num_vars ) << std::endl;
    std::cout << "T-cost of opt. cover: " << best_Tcount << std::endl;

#if 0
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
#endif
  }
}

int main()
{
  example1();
  example2();
  example3();

  return 0;
}
