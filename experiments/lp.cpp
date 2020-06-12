#include <easy/algorithms/lp.hpp>

#include <kitty/kitty.hpp>
#include <fmt/format.h>

auto constexpr num_vars = 2u;

int main()
{
  using truth_table = kitty::static_truth_table<num_vars>;

  truth_table tt;
  do
  {
    fmt::print( "[i] {}\n", kitty::to_binary( tt ) );

    auto const v = lp_characteristic_vector( tt );
    for ( const auto& vv : v )
    {
      std::cout << uint32_t( vv ) << ' ';
    }
    std::cout << std::endl;

    kitty::next_inplace( tt );
  } while ( !kitty::is_const0( tt ) );

  return 0;
}
