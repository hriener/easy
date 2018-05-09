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

#pragma once

#include <kitty/cube.hpp>
#include <vector>

namespace esop
{

using esop_t  = std::vector<kitty::cube>;
using esops_t = std::vector<esop_t>;

/*! \brief Minimum pairwise distance
 *
 * Compute the minimum pairwise distance between ESOP terms.
 *
 * \param esop ESOP
 * \return Minimum pairwise distance
 */
unsigned min_pairwise_distance( const esop_t& esop );

/*! \brief Maximum pairwise distance
 *
 * Compute the maximum pairwise distance between ESOP terms.
 *
 * \param esop ESOP
 * \return maximum pairwise distance
 */
unsigned max_pairwise_distance( const esop_t& esop );

/*! \brief Printer function for ESOP.
 *
 * Print ESOP as an expression.
 *
 * \param esop ESOP
 * \param num_vars Number of variables
 * \param os Output stream
 */
void print_esop_as_exprs( const esop_t& esop, unsigned num_vars, std::ostream& os = std::cout );

/*! \brief Printer function for ESOP.
 *
 * Print ESOP as a list of cubes.
 *
 * \param esop ESOP
 * \param num_vars Number of variables
 * \param os Output stream
 */
void print_esop_as_cubes( const esop_t& esop, unsigned num_vars, std::ostream& os = std::cout );

/*! \brief Verify ESOP.
 *
 * Verify ESOP given an incompletely-specified Boolean function as specification.
 *
 * \param esop ESOP
 * \param bits output as Boolean function
 * \param care care-set as Boolean function
 * \return true if ESOP and bits are equal within the care set, or false otherwise
 */
bool verify_esop( const esop_t& esop, const std::string& bits, const std::string& care );

} /* esop */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
