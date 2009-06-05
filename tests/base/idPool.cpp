
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
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
#include <eq/base/idPool.h>
#include <eq/base/rng.h>

#include <stdlib.h>
#include <iostream>

using namespace eq::base;
using namespace std;

int main( int argc, char **argv )
{
    IDPool pool( IDPool::MAX_CAPACITY );
    size_t nLoops = 10000;

    while( nLoops-- )
    {
        RNG rng;
        uint32_t range = static_cast<uint32_t>( rng.get<uint32_t>() * .0001f );
        uint32_t id    = pool.genIDs( range );
        
        TESTINFO( id != EQ_ID_INVALID,
                  "Failed to allocate after " << nLoops << " allocations" );
        
        pool.freeIDs( id, range );
    }

    return EXIT_SUCCESS;
}

