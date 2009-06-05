
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com>
                     , Makhinya Maxim
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

#include "aglPipe.h"

#include "global.h"
#include "pipe.h"

#include <sstream>

using namespace std;

namespace eq
{

AGLPipe::AGLPipe( Pipe* parent )
    : OSPipe( parent )
    , _cgDisplayID( 0 )
{
}

AGLPipe::~AGLPipe( )
{
}

//---------------------------------------------------------------------------
// AGL init
//---------------------------------------------------------------------------
bool AGLPipe::configInit()
{
#ifdef AGL
    CGDirectDisplayID displayID = CGMainDisplayID();
    const uint32_t device = _pipe->getDevice();

    if( device != EQ_UNDEFINED_UINT32 )
    {
        CGDirectDisplayID displayIDs[device+1];
        CGDisplayCount    nDisplays;

        if( CGGetOnlineDisplayList( device+1, displayIDs, &nDisplays ) !=
            kCGErrorSuccess )
        {
            ostringstream msg;
            msg << "Can't get display identifier for display " << device;
            setErrorMessage( msg.str( ));
            return false;
        }

        if( nDisplays <= device )
        {
            ostringstream msg;
            msg << "Can't get display identifier for display " << device
                << ", not enough displays in this system";
            setErrorMessage( msg.str( ));
            return false;
        }

        displayID = displayIDs[device];
    }

    _setCGDisplayID( displayID );
    EQINFO << "Using CG displayID " << displayID << endl;
    return true;
#else
    setErrorMessage( "Client library compiled without AGL support" );
    return false;
#endif
}


void AGLPipe::_setCGDisplayID( CGDirectDisplayID id )
{
#ifdef AGL
    if( _cgDisplayID == id )
        return;

    _cgDisplayID = id; 
    PixelViewport pvp = _pipe->getPixelViewport();

    if( pvp.isValid( ))
        return;

    if( id )
    {
        const CGRect displayRect = CGDisplayBounds( id );
        pvp.x = (int32_t)displayRect.origin.x;
        pvp.y = (int32_t)displayRect.origin.y;
        pvp.w = (int32_t)displayRect.size.width;
        pvp.h = (int32_t)displayRect.size.height;
    }
    else
        pvp.invalidate();

    _pipe->setPixelViewport( pvp );
#endif
}


void AGLPipe::configExit()
{
#ifdef AGL
    _setCGDisplayID( 0 );
    EQINFO << "Reset CG displayID " << endl;
#endif
}

} //namespace eq

