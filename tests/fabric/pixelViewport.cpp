
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <test.h>
#include <eq/types.h>
#include <fabric/pixelViewport.h>

using namespace eq;

int main( int argc, char **argv )
{
    PixelViewport pvp( 0, 0, 1000, 1000 );
    const Viewport vp( 0.25f, 0.25f, 0.5f, 0.5f );

    TEST( pvp.isValid( ));
    TEST( pvp.hasArea( ));

    pvp.apply( vp );
    TESTINFO( pvp == PixelViewport( 250, 250, 500, 500 ), pvp );

    pvp.x = -1000;
    pvp.y =  1000;
    pvp.w =  1000;
    pvp.h =  1000;
    
    pvp.apply( vp );
    TESTINFO( pvp == PixelViewport( -750, 1250, 500, 500 ), pvp );

    const Viewport vp2 = pvp.getSubVP( PixelViewport(-1000, 1000, 1000, 1000));
    TESTINFO( vp == vp2, vp2 );

    return EXIT_SUCCESS;
}
