#include <catch.hpp>
#include <easy/sat/constraints.hpp>

using namespace easy;

TEST_CASE( "Simple WCNF example one", "[sat]" )
{
  auto const top_weight = 2;

  sat::constraints constraints( top_weight );
  constraints.add_weighted_clause( { -1,  2 }, 2 ); // hard
  constraints.add_weighted_clause( {  1, -2 }, 1 ); // soft

  CHECK( constraints.num_variables() == 2u );
  CHECK( constraints.num_clauses() == 2u );

  uint32_t index{0};
  constraints.foreach_weighted_clause( [&]( const sat::constraints::clause_t& cl, uint32_t w ){
      switch( index )
      {
      case 0u:
        CHECK( cl == sat::constraints::clause_t{-1, 2} );
        CHECK( w == 2 );
        break;
      case 1:
        CHECK( cl == sat::constraints::clause_t{1, -2} );
        CHECK( w == 1 );
        break;
      default:
        CHECK( false );
        break;
      }
      ++index;
    });
}

TEST_CASE( "Simple WCNF example two", "[sat]" )
{
  auto const top_weight = 2;

  sat::constraints constraints( top_weight );
  constraints.add_weighted_clause( { -1,  2 }, 2 ); // hard
  constraints.add_weighted_clause( { -2,  3 }, 2 ); // hard
  constraints.add_weighted_clause( { -3 }, 1 );     // soft

  CHECK( constraints.num_variables() == 3u );
  CHECK( constraints.num_clauses() == 3u );

  uint32_t index{0};
  constraints.foreach_weighted_clause( [&]( const sat::constraints::clause_t& cl, uint32_t w ){
      switch( index )
      {
      case 0u:
        CHECK( cl == sat::constraints::clause_t{-1, 2} );
        CHECK( w == 2 );
        break;
      case 1:
        CHECK( cl == sat::constraints::clause_t{-2, 3} );
        CHECK( w == 2 );
        break;
      case 2:
        CHECK( cl == sat::constraints::clause_t{-3} );
        CHECK( w == 1 );
        break;
      default:
        CHECK( false );
        break;
      }
      ++index;
    });
}
