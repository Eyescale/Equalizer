
/* Copyright (c) 2015, Stefan.Eilemann@epfl.ch
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

// Tests the functionality of the pixel viewport

#include <lunchbox/test.h>
#include <eq/fabric/range.h>
#include <eq/types.h>

int main( int, char** )
{
    eq::Range range;
    TEST( range == eq::Range::ALL );

    range.merge( eq::Range( .5f, .6f ));
    TEST( range == eq::Range::ALL );

    range = eq::Range( .5f, .6f );
    TEST( range != eq::Range::ALL );
    TEST( range == eq::Range( .5f, .6f ));

    range.merge( eq::Range( 0.1f, 0.2f ));
    TEST( range == eq::Range( .1f, .6f ));

    range.merge( eq::Range( 0.3f, 0.4f ));
    TEST( range == eq::Range( .1f, .6f ));

    range.merge( eq::Range( ));
    TEST( range == eq::Range::ALL );

    return EXIT_SUCCESS;
}
