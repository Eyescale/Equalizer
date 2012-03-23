
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

#include <test.h>
#include <lunchbox/bitOperation.h>

int main( int argc, char **argv )
{
    TESTINFO( lunchbox::getIndexOfLastBit( 0u ) == -1,
              lunchbox::getIndexOfLastBit( 0u ));
    TESTINFO( lunchbox::getIndexOfLastBit( 42u ) == 5,
              lunchbox::getIndexOfLastBit( 42u ));
    TESTINFO( lunchbox::getIndexOfLastBit( EQ_BIT12 ) == 11,
              lunchbox::getIndexOfLastBit( EQ_BIT12 ));
    TESTINFO( lunchbox::getIndexOfLastBit( EQ_BIT48 ) == 47,
              lunchbox::getIndexOfLastBit( EQ_BIT48 ));
    return EXIT_SUCCESS;
}
