
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch>
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

// Tests the functionality of the random number generator

#include <test.h>
#include <co/base/bitOperation.h>

int main( int argc, char **argv )
{
    TESTINFO( co::base::getIndexOfLastBit( 0 ) == -1,
              co::base::getIndexOfLastBit( 0 ));
    TESTINFO( co::base::getIndexOfLastBit( 42 ) == 5,
              co::base::getIndexOfLastBit( 42 ));
    TESTINFO( co::base::getIndexOfLastBit( EQ_BIT12 ) == 11,
              co::base::getIndexOfLastBit( EQ_BIT12 ));
    return EXIT_SUCCESS;
}
