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

#include <alice/alice.hpp>
#include <esop/exorlink.hpp>

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

namespace alice
{

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

    if ( d == 2 )
    {
      for ( auto k = 0; k < 8; k += 4 )
      {
        const auto cubes = esop::exorlink( cube0, cube1, 2, &esop::cube_groups2[k] );
        for ( const auto& c : cubes )
        {
          c.print( num_vars ); std::cout << ' ';
        }
        std::cout << std::endl;
      }
    }
    else if ( d == 3 )
    {
      for ( auto k = 0; k < 54; k += 9 )
      {
        const auto cubes = esop::exorlink( cube0, cube1, 3, &esop::cube_groups3[k] );
        for ( const auto& c : cubes )
        {
          c.print( num_vars ); std::cout << ' ';
        }
        std::cout << std::endl;
      }
    }
    else if ( d == 4 )
    {    
      for ( auto k = 0; k < 384; k += 16 )
      {
        const auto cubes = esop::exorlink( cube0, cube1, 4, &esop::cube_groups4[k] );
        for ( const auto& c : cubes )
        {
          c.print( num_vars ); std::cout << ' ';
        }
        std::cout << std::endl;
      }
    }
    else if ( d == 5 )
    {
      for ( auto k = 0; k < 3000; k += 25 )
      {
        const auto cubes = esop::exorlink( cube0, cube1, 5, &esop::cube_groups5[k] );
        for ( const auto& c : cubes )
        {
          c.print( num_vars ); std::cout << ' ';
        }
        std::cout << std::endl;
      }
    }
    else if ( d == 6 )
    {
      for ( auto k = 0; k < 25920; k += 36 )
      {
        const auto cubes = esop::exorlink( cube0, cube1, 6, &esop::cube_groups6[k] );
        for ( const auto& c : cubes )
        {
          c.print( num_vars ); std::cout << ' ';
        }
        std::cout << std::endl;
      }
    }
    else
    {
      std::cout << "[e] distance > 6 are not supported" << std::endl;
    }

  }

private:
  std::string cube0;
  std::string cube1;
  bool change_order= true;
};

ALICE_ADD_COMMAND( exorlink, "Exorlink" )

}

ALICE_MAIN( easyshell )
