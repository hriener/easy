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

#if defined(KITTY_EXTENSION)

#include <esop/esop.hpp>
#include <kitty/cube.hpp>

/***
 *
 * Combine operations for ESOPs based on
 *
 * Stergios Stergiou, George K. Papakonstantinou: Exact Minimization Of Esop Expressions With Less Than
 * Eight Product Terms. Journal of Circuits, Systems, and Computers 13(1): 1-15, 2004.
 */

namespace esop
{

void simple_combine_inplace( esop_t& expr, uint8_t var_index, uint8_t i );
void simple_combine_inplace( esops_t& esops, uint8_t var_index, uint8_t i );
esop_t simple_combine( const esop_t& expr, uint8_t var_index, uint8_t i );
esops_t simple_combine( const esops_t& esops, uint8_t var_index, uint8_t i );

esop_t complex_combine( esop_t a, esop_t b, uint8_t var_index, uint8_t i, uint8_t j );
esops_t complex_combine( const esops_t& as, const esops_t& bs, uint8_t var_index, uint8_t i, uint8_t j );

} /* esop */

#endif /* KITTY_EXTENSION */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
