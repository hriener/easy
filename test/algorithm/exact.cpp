#include <catch.hpp>
#include <easy/esop/esop.hpp>
#include <easy/sat/sat_solver.hpp>
#include <easy/sat/cnf_writer.hpp>
#include <easy/sat/xor_clauses_to_cnf.hpp>
#include <easy/sat/gauss.hpp>
#include <kitty/kitty.hpp>
#include <lorina/pla.hpp>
#include <json/json.hpp>

std::vector<unsigned> compute_flips( unsigned n )
{
  const auto total_flips = ( 1u << n ) - 1;
  std::vector<unsigned> flip_array( total_flips );

  auto graynumber = 0u;
  auto temp = 0u;
  for ( auto i = 1u; i <= total_flips; ++i )
  {
    graynumber = i ^ ( i >> 1 );
    flip_array[total_flips - i] = ffs( temp ^ graynumber ) - 1u;
    temp = graynumber;
  }

  return flip_array;
}

std::vector<kitty::cube> comupte_prime_implicants( const kitty::cube& c, unsigned num_vars )
{
  const auto flips = compute_flips( num_vars );

  std::vector<kitty::cube> group = { c };
  auto copy = c;
  for ( auto i = 0u; i < flips.size(); ++i )
  {
    if ( copy.get_mask( flips[i] ) )
    {
      copy.clear_bit( flips[i] );
      copy.clear_mask( flips[i] );
    }
    else
    {
      copy.set_mask( flips[i] );
      if ( c.get_bit( flips[i] ) )
      {
        copy.set_bit( flips[i] );
      }
      else
      {
        copy.clear_bit( flips[i] );
      }
    }
    group.push_back( copy );
  }

  return group;
}

std::vector<kitty::cube> compute_minterms( const kitty::cube& c, unsigned num_vars )
{
  if ( c.num_literals() == num_vars )
  {
    return { c };
  }
  
  std::vector<kitty::cube> result;
  std::vector<kitty::cube> cubes = { c };
  for ( auto i = 0; i < num_vars; ++i )
  {
    for ( auto& c : cubes )
    {
      if ( !c.get_mask( i ) )
      {
        c.set_mask( i );
        c.clear_bit( i );

        if ( c.num_literals() == num_vars )
          result.emplace_back( c );

        kitty::cube copy( c );
        copy.set_bit( i );

        if ( copy.num_literals() == num_vars )
          result.emplace_back( copy );
        else
          cubes.emplace_back( copy );
      }
    }
  }

  std::sort( result.begin(), result.end() );
  result.erase( std::unique( result.begin(), result.end() ), result.end() );

  return result;
}

std::vector<kitty::cube> compute_neighbors( const kitty::cube& c, unsigned num_vars )
{
  kitty::cube copy( c );
  
  std::vector<kitty::cube> cubes;
  for ( auto i = 0; i < num_vars; ++i )
  {
    if ( copy.get_mask( i ) )
    {
      copy.flip_bit( i );
      cubes.emplace_back( copy );
      copy.flip_bit( i );
    }
  }
  return cubes;
}

template<typename TT>
bool verify_esop( easy::esop::esop_t const& esop, TT const& bits, TT const& care )
{
  assert( bits.num_vars() == care.num_vars() );

  auto const size = ( 1 << bits.num_vars() ); 
  
  kitty::dynamic_truth_table tt( bits.num_vars() );
  kitty::create_from_cubes( tt, esop, true );
  
  for ( auto i = 0u; i < size; ++i )
  {
    if ( get_bit( care, i ) && get_bit( bits, i ) != get_bit( tt, i ) )
    {
      return false;
    }
  }
  return true;
}

/**
 * Implementation of exact synthesis using the Helliwell equation
 */
template<typename TT>
class exact_synthesis_helliwell_impl
{
public:
  explicit exact_synthesis_helliwell_impl( const TT& bits, const TT& care )
    : _bits( bits )
    , _care( care )
  {
    assert( bits.num_vars() == care.num_vars() );
  }

  void compute_statistics( easy::sat::constraints const& constraints )
  {
    auto lits = 0;
    for ( const auto& c : constraints._xor_clauses )
    {
      for ( const auto& l : c.first )
      {
        (void)l;
        ++lits;
      }
    }

    std::cout << "clauses: " << constraints._xor_clauses.size() << std::endl;
    std::cout << "literals: " << lits << std::endl;
  }
  
  void synthesize()
  {
    auto const num_vars = _bits.num_vars();
    assert( num_vars <= 32 );

    /* prepare cube with the right mask size */
    kitty::cube minterm;
    for ( auto i = 0; i < num_vars; ++i )
      minterm.set_mask( i );

    std::unordered_map<kitty::cube, int32_t, kitty::hash<kitty::cube>> g;
    std::unordered_map<int32_t, kitty::cube> g_rev;

    easy::sat::constraints constraints;
    int32_t sid = 1;
    do
    {
      auto const b = get_bit( _bits, minterm._bits );
      if ( b != 0 && b != 1 )
        continue;

      std::cout << "===========================================================================" << std::endl;
      std::vector<int32_t> clause;
      for ( const auto& t : comupte_prime_implicants( minterm, num_vars ) )
      { 
        auto it = g.find( t );
        if ( it != g.end() )
        {
          clause.emplace_back( it->second );
          continue;
        }
        else
        {
          int32_t id = sid;
          g.emplace( t, id );
          g_rev.emplace( id, t );
            
          clause.push_back( id );
          ++sid;

          std::cout << id << ' '; t.print(); std::cout << std::endl;
        }
      }

      constraints.add_xor_clause( clause, b );
      
      ++minterm._bits;      
    } while ( minterm._bits < ( 1 << num_vars ) );

    /* save the current sid */
    auto max_pi_id = sid;

    easy::sat::cnf_writer writer;
    writer.apply( constraints );

    easy::sat::xor_clauses_to_cnf cnf( sid );
    cnf.apply( constraints );

    // std::cout << "===========================================================================" << std::endl;
    // writer.apply( constraints );
    
    easy::sat::sat_solver solver;
    auto sat = solver.solve( constraints );
    CHECK( sat.is_sat() );

    // std::cout << "===========================================================================" << std::endl;

    /*** ESOP ***/
    easy::esop::esop_t esop;
    for ( auto i = 1; i < max_pi_id-1; ++i )
    {
      std::cout << i << ' '; g_rev.at( i ).print( num_vars ); std::cout << ' ';
      if ( sat.model.at( i-1 ) == l_True )
      {
        std::cout << "TRUE" << std::endl;
        esop.emplace_back( g_rev.at( i ) );
      }
      else if ( sat.model.at( i-1 ) == l_False )
      {
        std::cout << "FALSE" << std::endl;
      }
    }

    CHECK( verify_esop( esop, _bits, _care ) );

    /* TODO: restrict solution and try again */
    
    std::cout << "===========================================================================" << std::endl;
    for ( const auto& cube : esop )
    {
      std::cout << "PI: "; cube.print( num_vars ); std::cout << std::endl;
      auto const minterms = compute_minterms( cube, num_vars );
      for ( const auto& m : minterms )
      {
        auto const neighbors = compute_neighbors( m, num_vars );

        auto count_equal = 0;
        for ( const auto& n : neighbors )
        {
          m.print( num_vars ); std::cout << " -> ";
          n.print( num_vars ); std::cout << std::endl;

          if ( kitty::get_bit( _bits, m._bits ) == kitty::get_bit( _bits, n._bits ) )
          {
            ++count_equal;
          }
        }


        std::cout << "P("; m.print( num_vars ); std::cout << "): ";
        std::cout << ( double( count_equal ) / neighbors.size() ) << std::endl;
      }
    }

    std::cout << "DONE" << std::endl;
  }

private:
  const TT& _bits;
  const TT& _care;
  const nlohmann::json parameter;
  nlohmann::json statistics;
}; // exact_synthesis_helliwell_impl

template<typename TT>
easy::esop::esops_t exact_synthesis_helliwell( const TT& bits, const TT& care )
{
  std::cout << "[i] exact synthesis using helliwell" << std::endl;

  exact_synthesis_helliwell_impl<TT> impl( bits, care );
  impl.synthesize();
  
  return {};
}

TEST_CASE( "Exact synthesis using Helliwell", "[exact]" )
{
  kitty::static_truth_table<3> tt_bits;
  kitty::static_truth_table<3> tt_care;

  create_from_binary_string( tt_bits, "01101101" );
  create_from_binary_string( tt_care, "11111111" );

  exact_synthesis_helliwell( tt_bits, tt_care );
}

TEST_CASE( "Exact synthesis with pre-ordering", "[exact]" )
{
  nlohmann::json config;
  config[ "maximum_cubes" ] = 10;
  config[ "one_esop" ] = true;

  // const auto number_of_variables = 5u;
  const std::string bits = "01111111111101010111111101010011";
  const std::string care = "11111111111111111111111111111111";

  /* yet not implemented */
}

TEST_CASE( "Minterms from implicant", "[exact]" )
{
  kitty::cube c( "11--" );
  c.print(); std::cout << std::endl;

  auto const minterms = compute_minterms( c, 4u );
  CHECK( minterms.size() == 4u );
}
