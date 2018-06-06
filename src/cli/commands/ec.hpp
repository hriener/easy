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

namespace alice
{

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
      std::cout << "[i] ESOPs are NOT describes using the same number of inputs" << std::endl;
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
}; /* ec_command */

} // namespace alice

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
