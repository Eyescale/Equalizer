
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

// Tests the functionality of the pixel viewport

#include <test.h>
#include <eq/client/pixelViewport.h>

using namespace eq;

int main( int argc, char **argv )
{
    PixelViewport pvp( 0, 0, 1000, 1000 );
    const Viewport vp( 0.25f, 0.25f, 0.5f, 0.5f );

    TEST( pvp.isValid( ));
    TEST( pvp.hasArea( ));

    pvp *= vp;
    TESTINFO( pvp == PixelViewport( 250, 250, 500, 500 ), pvp );

    pvp.x = -1000;
    pvp.y =  1000;
    pvp.w =  1000;
    pvp.h =  1000;
    
    pvp *= vp;
    TESTINFO( pvp == PixelViewport( -750, 1250, 500, 500 ), pvp );

    const Viewport vp2 = pvp / PixelViewport( -1000, 1000, 1000, 1000 );
    TESTINFO( vp == vp2, vp2 );
}
