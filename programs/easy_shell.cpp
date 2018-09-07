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
#include <easy/cli/stores/esop.hpp>
#include <easy/cli/stores/function.hpp>
#include <easy/cli/commands/cover.hpp>
#include <easy/cli/commands/exorlink.hpp>
#include <easy/cli/commands/ec.hpp>
#include <easy/cli/commands/function.hpp>
#include <easy/cli/commands/sort.hpp>
#include <easy/cli/commands/synth.hpp>

#include <easy/io/read_esop.hpp>
#include <easy/io/write_esop.hpp>

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

namespace alice
{

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

/******************************************************************************
 * Function commands                                                          *
 ******************************************************************************/
ALICE_ADD_COMMAND( function, "Function" )
ALICE_ADD_COMMAND( synth,    "Function" )

/******************************************************************************
 * Cube commands                                                              *
 ******************************************************************************/
ALICE_ADD_COMMAND( exorlink, "Cube" )

/******************************************************************************
 * ESOP commands                                                              *
 ******************************************************************************/
ALICE_ADD_COMMAND( cover,    "ESOP" )
ALICE_ADD_COMMAND( ec,       "ESOP" )
ALICE_ADD_COMMAND( sort,     "ESOP" )

} /* namespace alice */

ALICE_MAIN( easy )

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
