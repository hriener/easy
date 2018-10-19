#include <catch.hpp>

#include <easy/esop/esop.hpp>
#include <easy/io/read_esop.hpp>

TEST_CASE( "read_esop", "[io]" )
{
  std::string esop_pla_file =
      ".i 4\n"
      ".o 1\n"
      ".p 4\n"
      ".type esop\n"
      "0000 1\n"
      "-1-1 1\n"
      "0-0- 1\n"
      "-011 1\n"
      ".e\n";

  std::istringstream iss( esop_pla_file );

  easy::esop::esop_t esop;
  unsigned num_vars;

  easy::esop_storage_reader reader( esop, num_vars );
  auto result = read_pla( iss, reader );
  CHECK( result == lorina::return_code::success );

  for ( const auto& c : esop )
  {
    c.print( num_vars );
    std::cout << std::endl;
  }
}
