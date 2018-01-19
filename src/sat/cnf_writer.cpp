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

#include <sat/cnf_writer.hpp>
#include <iostream>

namespace sat
{

cnf_writer::cnf_writer( std::ostream& os )
  : _os( os )
{}

void cnf_writer::apply( constraints& constraints )
{
  _os << "cnf " << constraints._num_variables << " " << ( constraints._clauses.size()+constraints._xor_clauses.size() ) << std::endl;
  for ( const auto& clause : constraints._clauses )
  {
    for ( const auto& l : clause )
    {
      _os << l << ' ';
    }
    _os << '0' << std::endl;
  }

  for ( const auto& clause : constraints._xor_clauses )
  {
    _os << "x";
    for ( auto i = 0u; i < clause.first.size(); ++i )
    {
      if ( !clause.second && i == clause.first.size()-1 )
      {
	_os << "-";
      }
      _os << clause.first[i] << ' ';
    }
      _os << "0" << std::endl;
  }
}

} /* sat */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
