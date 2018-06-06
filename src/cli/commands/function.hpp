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

#include <alice/alice.hpp>

namespace alice
{

class function_command : public command
{
public:
  explicit function_command( const environment::ptr& env )
    : command( env, "loads an incompletely-specified Boolean function and adds it to the store" )
  {
    opts.add_option( "string", f, "Incompletely-specified Boolean function (MSB ... LSB)" );
  }

protected:
  void execute()
  {
    const unsigned number_of_variables = std::ceil( std::log2( f.size() ) );
    std::reverse( f.begin(), f.end() );

    kitty::dynamic_truth_table bits( number_of_variables );
    kitty::dynamic_truth_table care( number_of_variables );

    unsigned i;
    for ( i = 0; i < f.size(); ++i )
    {
      if ( f[i] == '1' )
      {
        kitty::set_bit( bits, i );
      }
      else if ( f[i] == '-' )
      {
        kitty::set_bit( care, i );
      }
    }

    env->store<function_storee>().extend() = function_storee{bits, care, number_of_variables};
  }

private:
  std::string f = "";
};
 
} // namespace alice

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
