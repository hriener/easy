/* alice: C++ command shell library
 * Copyright (C) 2017-2018  EPFL
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

#define READLINE_USE_READLINE 1
#include <alice/alice.hpp>
#include <esop/exorlink.hpp>
#include <lorina/pla.hpp>

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

std::uint32_t k_size[] = { /* 0 */0, /* 1 */0, /* 2 */8, /* 3 */54, /* 4 */384, /* 5 */3000, /* 6 */25920 };
std::uint32_t k_incr[] = { /* 0 */0, /* 1 */0, /* 2 */4, /* 3 */ 9, /* 4 */ 16, /* 5 */  25, /* 6 */   36 };

std::uint32_t *cube_groups[] = {
  nullptr,
  nullptr,
  &esop::cube_groups2[0],
  &esop::cube_groups3[0],
  &esop::cube_groups4[0],
  &esop::cube_groups5[0],
  &esop::cube_groups6[0]
};

class pla_storage_reader : public lorina::pla_reader
{
public:
  pla_storage_reader( esop::esop_t& esop, unsigned& num_vars )
    : _esop( esop )
    , _num_vars( num_vars )
  {}

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

  esop::esop_t& _esop;
  unsigned& _num_vars;
}; /* pla_storage_reader */

struct esop_storee
{
  std::string model_name;
  esop::esop_t esop;
  std::size_t num_vars;
}; /* esop_storee */

void write_esop( std::ostream& os, const esop::esop_t& esop, unsigned num_vars )
{
  lorina::pla_writer writer( os );
  writer.on_number_of_inputs( num_vars );
  writer.on_number_of_outputs( 1 );
  writer.on_number_of_terms( esop.size() );
  writer.on_keyword( "type", "esop" );
  for ( const auto& e : esop )
  {
    std::stringstream ss;
    e.print( num_vars, ss );
    writer.on_term( ss.str(), "1" );
  }
  writer.on_end();
}

namespace alice
{

ALICE_ADD_STORE( esop_storee, "esop", "e", "ESOP", "ESOPs" )

ALICE_PRINT_STORE( esop_storee, os, element )
{
  auto i = 0;
  for ( const auto& c : element.esop )
  {
    os << (i++) << ". ";
    c.print( element.num_vars );
    os << '\n';
  }
}

ALICE_DESCRIBE_STORE( esop_storee, element )
{
  return fmt::format( "[i] esop<{}>: vars={} cubes={}\n", element.model_name, element.num_vars, element.esop.size() );
}

ALICE_ADD_FILE_TYPE( pla, "PLA" )

ALICE_READ_FILE( esop_storee, pla, filename, cmd )
{
  lorina::diagnostic_engine diag;
  esop::esop_t esop;
  unsigned num_vars;
  auto parsing_result = lorina::read_pla( filename, pla_storage_reader( esop, num_vars ), &diag );
  assert( parsing_result == lorina::return_code::success );
  return esop_storee{ filename, esop, num_vars };
}

ALICE_WRITE_FILE( esop_storee, pla, element, filename, cmd )
{
  std::ofstream os( filename.c_str(), std::ofstream::out );
  write_esop( os, element.esop, element.num_vars );
  os.close();
}

class exorlink_command : public command
{
public:
  explicit exorlink_command( const environment::ptr& env )
    : command( env, "exorlink operation" )
  {
    opts.add_option( "cube0", cube0, "First cube" );
    opts.add_option( "cube1", cube1, "Second cube" );
    opts.add_flag( "-r,--reverse", change_order, "Reverse variable order (default: 0...n left to right)" );
  }

protected:
  void execute()
  {
    const auto num_vars = cube0.size();
    if ( cube1.size() != num_vars )
    {
      std::cout << "[e] cube must have the same length" << std::endl;
      return;
    }

    if ( !change_order )
    {
      std::reverse( cube0.begin(), cube0.end() );
      std::reverse( cube1.begin(), cube1.end() );
    }

    const auto c0 = kitty::cube( cube0 );
    const auto c1 = kitty::cube( cube1 );

    const auto d = c0.distance( c1 );
    std::cout << "[i] distance = " << d << std::endl;

    if ( d >= 2 && d <= 7 )
    {
      const auto size = k_size[ d ];
      const auto incr = k_incr[ d ];
      for ( auto k = 0; k < size; k += incr )
      {
        const auto cubes = esop::exorlink( cube0, cube1, d, cube_groups[d] + k );
        for ( const auto& c : cubes )
        {
          c.print( num_vars ); std::cout << ' ';
        }
        std::cout << std::endl;
      }
    }
    else
    {
      std::cout << "[e] only distances in the interval [2,...,6] are supported" << std::endl;
    }
  }

private:
  std::string cube0;
  std::string cube1;
  bool change_order= true;
};

ALICE_ADD_COMMAND( exorlink, "Cube" )

}

ALICE_MAIN( easy )
