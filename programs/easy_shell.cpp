/* easy
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
#include <io/read_esop.hpp>
#include <io/write_esop.hpp>

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

std::uint32_t k_size[] = { /* 0 */0, /* 1 */0, /* 2 */8, /* 3 */54, /* 4 */384, /* 5 */3000, /* 6 */25920 };
std::uint32_t k_incr[] = { /* 0 */0, /* 1 */0, /* 2 */4, /* 3 */ 9, /* 4 */ 16, /* 5 */  25, /* 6 */   36 };

std::uint32_t *cube_groups[] = {
  nullptr
, nullptr
, &esop::cube_groups2[0]
, &esop::cube_groups3[0]
, &esop::cube_groups4[0]
, &esop::cube_groups5[0]
, &esop::cube_groups6[0]
};

struct esop_storee
{
  std::string model_name;
  esop::esop_t esop;
  std::size_t number_of_inputs;
  std::size_t number_of_outputs;
}; /* esop_storee */

namespace alice
{

ALICE_ADD_STORE( esop_storee, "esop", "e", "ESOP", "ESOPs" )

ALICE_PRINT_STORE( esop_storee, os, element )
{
  auto i = 0;
  for ( const auto& c : element.esop )
  {
    os << (i++) << ". ";
    c.print( element.number_of_inputs );
    os << '\n';
  }
}

ALICE_DESCRIBE_STORE( esop_storee, element )
{
  return fmt::format( "[i] esop<{}>", element.model_name );
}

ALICE_PRINT_STORE_STATISTICS( esop_storee, os, element )
{
  os << fmt::format( "[i] esop<{}>: vars={} cubes={}", element.model_name, element.number_of_inputs, element.esop.size() ) << '\n';
}

ALICE_ADD_FILE_TYPE( pla, "PLA" )

ALICE_READ_FILE( esop_storee, pla, filename, cmd )
{
  lorina::diagnostic_engine diag;
  esop::esop_t esop;
  unsigned number_of_inputs;
  unsigned number_of_outputs;
  auto parsing_result = lorina::read_pla( filename, easy::esop_storage_reader( esop, number_of_inputs ), &diag );
  if ( parsing_result != lorina::return_code::success )
  {
    std::cerr << "[e] unable to parse ESOP-PLA file" << std::endl;
    return esop_storee{ "", {}, 0, 0 };
  }
  return esop_storee{ filename, esop, number_of_inputs, number_of_outputs };
}

ALICE_WRITE_FILE( esop_storee, pla, element, filename, cmd )
{
  std::ofstream os( filename.c_str(), std::ofstream::out );
  easy::write_esop( os, element.esop, element.number_of_inputs );
  os.close();
}

class exorlink_command : public command
{
public:
  explicit exorlink_command( const environment::ptr& env )
    : command( env, "exorlink operation" )
  {
    opts.add_option( "cube0", cube0, "First cube" )->required();
    opts.add_option( "cube1", cube1, "Second cube" )->required();
    opts.add_flag( "-r,--reverse", change_order, "Reverse variable order (default: 0...n left to right)" );
  }

protected:
  void execute()
  {
    const auto num_vars = cube0.size();
    if ( cube1.size() != num_vars )
    {
      std::cerr << "[e] cube must have the same length" << std::endl;
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
      std::cerr << "[e] only distances in the interval [2,...,6] are supported" << std::endl;
    }
  }

private:
  std::string cube0;
  std::string cube1;
  bool change_order = true;
};

class ec_command : public command
{
public:
  explicit ec_command( const environment::ptr& env )
    : command( env, "exorlink operation" )
  {
    opts.add_option( "store index", i, "First index in ESOP storage (default: 0)" );
    opts.add_option( "store index", j, "Second index in ESOP storage (default: 1)" );
  }

protected:
  rules validity_rules() const
  {
    rules rules;

    rules.push_back( {[this]() { return i < store<esop_storee>().size(); }, "first index out of bounds"} );
    rules.push_back( {[this]() { return j < store<esop_storee>().size(); }, "second index out of bounds"} );

    return rules;
  }

  void execute()
  {
    const auto& elm1 = store<esop_storee>()[i];
    const auto& elm2 = store<esop_storee>()[j];

    if ( elm1.number_of_inputs != elm2.number_of_inputs )
    {
      std::cout << "[i] ESOPs are NOT equivalent" << std::endl;
      return;
    }

    if ( esop::equivalent_esops( elm1.esop, elm2.esop, elm1.number_of_inputs ) )
    {
      std::cout << "[i] ESOPs are equivalent" << std::endl;
    }
    else
    {
      std::cout << "[i] ESOPs are NOT equivalent" << std::endl;
    }
  }

private:
  int i = 0;
  int j = 1;
};

class sort_command : public command
{
public:
  explicit sort_command( const environment::ptr& env )
    : command( env, "sort current ESOP" )
  {
    opts.add_flag( "-i,--index", index, "Index" );
    opts.add_flag( "-r,--random", random, "Randomize the cubes in the ESOP" );
  }

protected:
  rules validity_rules() const
  {
    rules rules;

    rules.push_back( {[this]() { return index == -1 || index < store<esop_storee>().size(); }, "index out of bounds"} );

    return rules;
  }

  void execute()
  {
    auto& current = index == -1 ? store<esop_storee>().current() : store<esop_storee>()[index];
    if ( random )
    {
      std::random_shuffle( current.esop.begin(), current.esop.end() );
    }
    else
    {
      std::sort( current.esop.begin(), current.esop.end() );
    }
  }

private:
  bool random;
  int index = -1;
};

ALICE_ADD_COMMAND( exorlink, "Cube" )
ALICE_ADD_COMMAND( ec, "ESOP" )
ALICE_ADD_COMMAND( sort, "ESOP" )

} /* namespace alice */

ALICE_MAIN( easy )

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
