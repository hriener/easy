/* ESOP
 * Copyright (C) 2018  EPFL
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

#include <esop/exorlink.hpp>
#include <kitty/print.hpp>

#include <benchmark/benchmark.h>

#include <chrono>
#include <random>

using namespace esop;

void BM_exorlink4( benchmark::State& state )
{
  /* exorlink-4 */
  std::default_random_engine gen( std::chrono::system_clock::now().time_since_epoch().count() );
  std::uniform_int_distribution<uint64_t> dist( 0ul, 15ul );

  while ( state.KeepRunning() )
  {
    kitty::cube cube0;
    cube0._value = 0xfffffffff - dist( gen );

    kitty::cube cube1( "----" );
    for ( auto i = 0; i < 384; i += 16 )
    {
      esop::exorlink( cube0, cube1, 4, &esop::cube_groups4[i] );
    }
  }
}

BENCHMARK( BM_exorlink4 )->Arg( 9 )->Arg( 11 )->Arg( 13 )->Arg( 15 )->Arg( 17 )->Arg( 19 )->Arg( 21 )->Arg( 23 )->Arg( 25 )->Arg( 27 );

BENCHMARK_MAIN()
