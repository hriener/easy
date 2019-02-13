#include "util/util.h"
#include "cudd/cudd.h"
#include "cudd/cuddInt.h"
#include "cplusplus/cuddObj.hh"
#undef fail

#include <kitty/kitty.hpp>
#include <easy/esop/cost.hpp>
#include <easy/io/write_esop.hpp>
#include <fmt/format.h>

#include <iostream>
#include <unordered_map>

template<int num_vars>
std::ostream& operator<<( std::ostream& os, kitty::static_truth_table<num_vars> const& tt )
{
  return ( os << to_binary( tt ) );
}

namespace magic
{

uint64_t pow3[] = {
  /*  0 */ 1,
  /*  1 */ 3,
  /*  2 */ 9,
  /*  3 */ 27,
  /*  4 */ 81,
  /*  5 */ 243,
  /*  6 */ 729,
  /*  7 */ 2187,
  /*  8 */ 6561,
  /*  9 */ 19683,
  /* 10 */ 59049,
  /* 11 */ 177147,
  /* 12 */ 531441,
  /* 13 */ 1594323,
  /* 14 */ 4782969,
  /* 15 */ 14348907,
  /* 16 */ 43046721,
}; /* 3^n */

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

std::vector<kitty::cube> compute_distance_one_cube( const kitty::cube& c, uint32_t num_vars )
{
  std::vector<kitty::cube> cubes;
  for ( auto i = 0u; i < num_vars; ++i )
  {
    kitty::cube copy{c};
    copy.set_mask( i );
    copy.set_bit( i );
    if ( c != copy )
      cubes.emplace_back( copy );

    copy.clear_bit( i );
    if ( c != copy )
      cubes.emplace_back( copy );

    copy.clear_mask( i );
    if ( c != copy )
      cubes.emplace_back( copy );
  }
  return cubes;
}

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
            current_assignment.emplace_back( -(i+1) );
          else if ( v == 1 )
            current_assignment.emplace_back( (i+1) );
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

void add_neighbors( std::vector<kitty::cube>& cover, uint32_t num_vars, uint32_t limit )
{
  auto count_new_cubes = 0u;
  if ( count_new_cubes >= limit )
    return;

  std::vector<kitty::cube> new_cubes{cover};
  for ( const auto& c : cover )
  {
    // c.print( num_vars );
    // std::cout << std::endl;

    for ( auto i = 0u; i < num_vars; ++i )
    {
      kitty::cube copy( c );
      copy.set_bit( i );
      copy.set_mask( i );

      if ( std::find( new_cubes.begin(), new_cubes.end(), copy ) == new_cubes.end() )
      {
        new_cubes.emplace_back( copy );
        ++count_new_cubes;
        if ( count_new_cubes >= limit )
        {
          cover = new_cubes;
          return;
        }
      }

      copy.clear_bit( i );
      if ( std::find( new_cubes.begin(), new_cubes.end(), copy ) == new_cubes.end() )
      {
        new_cubes.emplace_back( copy );
        ++count_new_cubes;
        if ( count_new_cubes >= limit )
        {
          cover = new_cubes;
          return;
        }
      }

      copy.clear_mask( i );
      if ( std::find( new_cubes.begin(), new_cubes.end(), copy ) == new_cubes.end() )
      {
        new_cubes.emplace_back( copy );
        ++count_new_cubes;
        if ( count_new_cubes >= limit )
        {
          cover = new_cubes;
          return;
        }
      }
    }
  }

  cover = new_cubes;
}

} /* magic */

struct vertex
{
  explicit vertex( kitty::cube const& minterm )
    : minterm( minterm )
  {}

  void add_adjacent( uint32_t const& a )
  {
    if ( std::find( adjacent.begin(), adjacent.end(), a ) == adjacent.end() )
    {
      adjacent.emplace_back( a );
    }
  }

  kitty::cube minterm;
  bool covered{false};
  std::vector<uint32_t> adjacent;
}; /* vertex */

class graph
{
public:
  explicit graph()
  {}

  uint64_t num_vertices() const
  {
    return vertices.size();
  }

  uint32_t get_vertex( kitty::cube const& minterm )
  {
    auto const it = cube_to_index.find( minterm );
    if ( it != cube_to_index.end() )
      return it->second;

    auto const index = vertices.size();
    vertices.emplace_back( vertex{minterm} );
    cube_to_index.emplace( minterm, index );
    return index;
  }

  void add_edge( kitty::cube const& a, kitty::cube const& b )
  {
    auto const index_a = cube_to_index.at( a );
    auto const index_b = cube_to_index.at( b );
    vertices.at( index_a ).add_adjacent( index_b );
    vertices.at( index_b ).add_adjacent( index_a );
  }

  std::vector<kitty::cube> get_sorted_cubes() const
  {
    std::vector<vertex> vvv( vertices );
    std::sort( vvv.begin(), vvv.end(),
               [&]( vertex const& a, vertex const& b ){
                 return ( ( a.adjacent.size() < b.adjacent.size() ) ||
                          ( a.adjacent.size() == b.adjacent.size() && a.minterm._value < b.minterm._value ) ); } );
    std::reverse( vvv.begin(), vvv.end() );

    std::vector<kitty::cube> cubes;
    for ( const auto& v : vvv )
    {
      cubes.emplace_back( v.minterm );
    }
    return cubes;
  }

protected:
  std::vector<vertex> vertices;
  std::unordered_map<kitty::cube, uint64_t, kitty::hash<kitty::cube>> cube_to_index;
}; /* graph */

template<typename TT>
class esop_synthesizer
{
public:
  explicit esop_synthesizer( TT const& tt, std::vector<kitty::cube> const& cover )
    : _mgr( magic::pow3[tt.num_vars()], 0 )
    , _tt( tt )
    , _num_vars( tt.num_vars() )
    , _cover( cover )
    , _best_cover( cover )
    , _best_cost( easy::esop::T_count( cover, tt.num_vars() ) )
  {
    /* no reordering */
    _mgr.AutodynDisable();
    /* no garbage collection */
    _mgr.DisableGarbageCollection();

    /* add edges by following distannce-k subcubes */
    std::vector<kitty::cube> cubes{_cover};
    for ( auto i = 0u; i < 5u; ++i )
    {
      std::vector<kitty::cube> new_cubes;
      for ( const auto& c : cubes )
      {
        _dg.get_vertex( c );
        for ( const auto& subcube : magic::compute_distance_one_cube( c, _num_vars ) )
        {
          auto size = _dg.num_vertices();
          _dg.get_vertex( subcube );
          auto size_prime = _dg.num_vertices();
          if ( size_prime > size )
          {
            _dg.add_edge( c, subcube );
            new_cubes.emplace_back( subcube );
          }
        }
      }
      cubes = new_cubes;
    }

    /* create the remaining nodes */
    kitty::cube minterm;
    for ( uint32_t i = 0u; i < _num_vars; ++i )
      minterm.set_mask( i );

    for ( uint64_t i = 0ul; i < ( 1ul << _num_vars ); ++i )
    {
      for ( const auto& subcube : magic::compute_implicants( minterm, _num_vars ) )
      {
        _dg.get_vertex( subcube );
      }
      ++minterm._bits;
    }

    assert( _dg.num_vertices() == magic::pow3[_num_vars] );

    _sorted_cubes = _dg.get_sorted_cubes();    
  }

  std::vector<kitty::cube> run( uint32_t limit = 0 )
  {
    /* prepare variables */
    std::vector<BDD> g( magic::pow3[_num_vars] );
    for ( uint64_t i = 0ul; i < g.size(); ++i )
      g[i] = _mgr.bddVar( i );

    /* add cubes by distance */
    std::vector<kitty::cube> cubes( _sorted_cubes.begin(), _sorted_cubes.begin() + limit );

    /* construct the decision problem */
    kitty::cube minterm;
    for ( uint32_t i = 0u; i < _num_vars; ++i )
      minterm.set_mask( i );

    BDD dp = _mgr.bddOne();
    for ( uint64_t i = 0ul; i < ( 1ul << _num_vars ); ++i )
    {
      auto const m = kitty::get_bit( _tt, i );

      BDD term = m ? _mgr.bddZero() : _mgr.bddOne(); /* == _mgr.bddOne() ^ ( m ? _mgr.bddOne() : _mgr.bddZero() ); */
      for ( const auto& subcube : magic::compute_implicants( minterm, _num_vars ) )
      {
        /* lazy abstraction */
        if ( std::find( cubes.begin(), cubes.end(), subcube ) == cubes.end() )
          continue;

        /* make constraints */
        auto const it = cube_to_index.find( subcube );
        if ( it == cube_to_index.end() )
        {
          auto const id = last_used_index++;
          cube_to_index.emplace( subcube, id );
          index_to_cube.emplace( id, subcube );
        }

        auto const index = cube_to_index.at( subcube );
        term ^= g.at( index );

        if ( dp.nodeCount() > 100000 ) // 5000
        {
          _terminated = true;
          return _best_cover;
        }
      }

      dp *= term;
      ++minterm._bits;
    }

    /* extract 1-paths of the BDD */
    magic::ExtractAssignments aux( _mgr );
    auto const assignments = aux.extract( dp );

    for ( const auto& a : assignments )
    {
      auto const cover_opt = assignment_to_cover( a );
      if ( verify && !verify_esop( cover_opt ) )
        return {}; // error

      auto const T_cost = easy::esop::T_count( cover_opt, _num_vars );
      if ( T_cost < _best_cost )
      {
        _best_cover = cover_opt;
        _best_cost = T_cost;
      }
    }

    return _best_cover;
  }

  bool terminated() const
  {
    return _terminated;
  }

protected:
  std::vector<kitty::cube> assignment_to_cover( std::vector<int> const& assignment ) const
  {
    std::vector<kitty::cube> cover;
    for ( const auto& v : assignment )
    {
      if ( v > 0 )
      {
        cover.emplace_back( index_to_cube.at( v-1 ) );
      }
    }
    return cover;
  }

  bool verify_esop( std::vector<kitty::cube> const& cover ) const
  {
    TT tt;
    kitty::create_from_cubes( tt, cover, true );
    return ( tt == _tt );
  }

public:
  Cudd _mgr;
  TT const& _tt;
  uint32_t _num_vars;
  std::vector<kitty::cube> _cover;
  std::vector<kitty::cube> _sorted_cubes;

  bool _terminated{false};

  std::vector<kitty::cube> _best_cover;
  uint32_t _best_cost;

  uint32_t last_used_index{0u};
  std::unordered_map<kitty::cube, uint32_t, kitty::hash<kitty::cube>> cube_to_index;
  std::unordered_map<uint32_t, kitty::cube> index_to_cube;

  bool verify{true};

  graph _dg;
}; /* exact_synthesizer */

class esop_reader : public lorina::pla_reader
{
public:
  esop_reader( std::vector<kitty::cube>& esop, uint32_t& num_vars )
      : _esop( esop ), _num_vars( num_vars )
  {
  }

  void on_number_of_inputs( std::size_t i ) const override
  {
    _num_vars = i;
  }

  void on_term( const std::string& term, const std::string& out ) const override
  {
    assert( out == "1" );
    _esop.emplace_back( term );
  }

  bool on_keyword( const std::string& keyword, const std::string& value ) const override
  {
    if ( keyword == "type" && value == "esop" )
    {
      return true;
    }
    return false;
  }

  std::vector<kitty::cube>& _esop;
  unsigned& _num_vars;
}; /* esop_reader */

template<typename TT>
std::vector<kitty::cube> esop_from_exorcism( TT const& tt )
{
  /* special case */
  TT const0;
  if ( tt == const0 )
    return {};

  auto const pkrm = kitty::esop_from_optimum_pkrm( tt );
  easy::write_esop( "/tmp/esop.pla", pkrm, tt.num_vars() );  
  system( "abc -c \"&exorcism -q /tmp/esop.pla /tmp/esop_exorcised.pla\" &> /dev/null" );
  
  std::vector<kitty::cube> exorcised_pkrm;
  uint32_t exorcised_num_vars;
  auto result = lorina::read_pla( "/tmp/esop_exorcised.pla", esop_reader( exorcised_pkrm, exorcised_num_vars ) );
  if ( result != lorina::return_code::success || exorcised_num_vars != tt.num_vars() )
  {
    std::cout << "FAILED" << std::endl;
  }
  return exorcised_pkrm;
}

template<typename TT>
std::vector<kitty::cube> esop_from_underapprox( TT const& tt )
{
  auto const exorcised_pkrm = esop_from_exorcism( tt );
  
  std::vector<kitty::cube> cover;
  esop_synthesizer<TT> synth( tt, exorcised_pkrm );
  auto i = 1;
  auto not_improved = 0;
  while ( !synth.terminated() && not_improved < 100 ) // 11
  {
    auto const new_cover = synth.run( i++ );
    if ( easy::esop::T_count( cover, tt.num_vars() ) == easy::esop::T_count( new_cover, tt.num_vars() ) )
      ++not_improved;
    else
      not_improved = 0;
    cover = new_cover;

    std::cout << "cover = " << i << ' ' << cover.size() << ' ' << easy::esop::T_count( cover, tt.num_vars() ) << std::endl;
  }

  return cover;
}

#if 1
int main()
{
  const int num_vars = 9;
  kitty::static_truth_table<num_vars> tt1;
  kitty::create_from_hex_string( tt1, "baafaffefafeaffefa0faceefffeaffefaffaff8fafeafeefa9faceefffeafeebaafaf9efcfeafcef70fa7eeeef7affffa7ffffefafea7eefaefaceefffeafee" );
  // kitty::create_from_expression( tt1, "{abcde}" );

  auto cover = esop_from_underapprox( tt1 );
  kitty::static_truth_table<num_vars> tt2;
  kitty::create_from_cubes( tt2, cover, true );
  if ( tt1 != tt2 )
  {
    std::cout << "ERROR" << std::endl;
  }

  auto exorcised = esop_from_exorcism( tt1 );

  std::cout << fmt::format( "[i] EXACT size / T-count: {:5d} / {:5d}\n"
                            "[i] EXORC size / T-count: {:5d} / {:5d}\n",
                            ( cover.size() ),
                            ( easy::esop::T_count( cover, tt1.num_vars() ) ),
                            ( exorcised.size() ),
                            ( easy::esop::T_count( exorcised, tt1.num_vars() ) ) );
  
  return 0;
}
#endif

#if 0
int main()
{
  int const num_vars = 4u;

  /* truth table type in this example */
  using truth_table = kitty::static_truth_table<num_vars>;

  /* set to store all NPN representatives */
  std::unordered_set<truth_table, kitty::hash<truth_table>> classes;
  std::vector<truth_table> sorted_classes;

  /* initialize truth table (constant 0) */
  truth_table tt;

  do
  {
    /* apply NPN canonization and add resulting representative to set */
    const auto res = kitty::exact_npn_canonization( tt );
    classes.insert( std::get<0>( res ) );

    /* increment truth table */
    kitty::next_inplace( tt );
  } while ( !kitty::is_const0( tt ) );

  std::cout << "[i] enumerated "
            << ( 1 << ( 1 << tt.num_vars() ) ) << " functions into "
            << classes.size() << " classes." << std::endl;

  for ( const auto& c : classes )
    sorted_classes.emplace_back( c );
  std::sort( sorted_classes.begin(), sorted_classes.end() );

  auto total_size = 0u;
  auto total_t_count = 0u;
  auto total_prev_size = 0u;
  auto total_prev_t_count = 0u;
  for ( const auto& c : sorted_classes )
  {
    kitty::print_binary( c );
    std::cout << ' ';

    auto const cover = esop_from_underapprox( c );
    auto const exorcised_cover = esop_from_exorcism( c );

    std::cout << cover.size() << ' ' ;
    std::cout << easy::esop::T_count( cover, num_vars ) << ' ';

    std::cout << exorcised_cover.size() << ' ' ;    
    std::cout << easy::esop::T_count( exorcised_cover, num_vars ) << std::endl;

    total_size += cover.size();
    total_t_count += easy::esop::T_count( cover, num_vars );
    total_prev_size += exorcised_cover.size();
    total_prev_t_count += easy::esop::T_count( exorcised_cover, num_vars );
  }

  std::cout << fmt::format( "[i] EXACT avg size / avg T-count: {:5.2f} / {:5.2f}\n"
                            "[i] BASEL avg size / avg T-count: {:5.2f} / {:5.2f}\n",
                            ( double(total_size)/classes.size() ),
                            ( double(total_t_count)/classes.size() ),
                            ( double(total_prev_size)/classes.size() ),
                            ( double(total_prev_t_count)/classes.size() ) );
  return 0;
}
#endif
