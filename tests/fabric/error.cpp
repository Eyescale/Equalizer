
/* Copyright (c) 2014, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include <lunchbox/test.h>
#include <eq/fabric/error.h>

int main( int, char** )
{
    const eq::fabric::Error noError( eq::fabric::ERROR_NONE );
    const eq::fabric::Error error( eq::fabric::ERROR_NONE + 1 );

    TEST( !noError );
    TEST( error );

    if( noError )
        TEST( false )

    if( !error )
        TEST( false )

    return EXIT_SUCCESS;
}
