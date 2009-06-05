
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "test.h"

#include <eq/net/nodeID.h>

#include <stdlib.h>

using namespace eq::net;
using namespace std;

int main( int argc, char **argv )
{
    NodeID id1( true );
    NodeID id2( true );

    TEST( id1 );
    TEST( id1 != id2 );

    id1 = id2;
    TEST( id1 == id2 );
    
    NodeID* id3 = new NodeID( id1 );
    NodeID* id4 = new NodeID( true );

    TEST( id1 == *id3 );
    TEST( *id4 != *id3 );
    
    *id4 = *id3;
    TEST( *id4 == *id3 );
    
    delete id3;
    delete id4;

    NodeID id5, id6;
    TEST( !id5 );
    TEST( id5 == id6 );
    
    return EXIT_SUCCESS;
}

