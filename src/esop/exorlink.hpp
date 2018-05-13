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

#include <esop/esop.hpp>

namespace esop
{

inline unsigned cube_groups2[8] = {/* 0 */2, 0, 1, 2,
                                   /* 4 */0, 2, 2, 1};

inline unsigned cube_groups3[54] = {/*  0 */2, 0, 0, 1, 2, 0, 1, 1, 2,
                                    /*  9 */2, 0, 0, 1, 0, 2, 1, 2, 1,
                                    /* 18 */0, 2, 0, 2, 1, 0, 1, 1, 2,
                                    /* 27 */0, 2, 0, 0, 1, 2, 2, 1, 1,
                                    /* 36 */0, 0, 2, 2, 0, 1, 1, 2, 1,
                                    /* 45 */0, 0, 2, 0, 2, 1, 2, 1, 1};

/*! \brief EXORLINK cube transformation
 *
 * Transform two cubes with distance into a functionally equivalent set of cubes.
 *
 * \param c0 First cube
 * \param c1 Second cube
 * \param distance Distance of ``c0`` and ``c1``. Must be less than 4.
 * \param group A group of cube transformations
 * \return An array of up to 4 new cubes which are functionally equivalent to ``c0`` and ``c1``.
 */
std::array<kitty::cube,4> exorlink( kitty::cube c0, kitty::cube c1, std::uint32_t distance, std::uint32_t *group );

} /* esop */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
