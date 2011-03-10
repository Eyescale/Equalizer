
/* Copyright (c) 2009-2011, Stefan Eilemann <eile@equalizergraphics.com>
                      2009, Maxim Makhinya
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
#include "os.h"
#include "pipe.h"

#include <sstream>

namespace eq
{

AGLPipe::AGLPipe( Pipe* parent )
    : SystemPipe( parent )
    , _cgDisplayID( kCGNullDirectDisplay )
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
    CGDirectDisplayID displayID = CGMainDisplayID();
    const uint32_t device = getPipe()->getDevice();

    if( device != EQ_UNDEFINED_UINT32 )
    {
        CGDirectDisplayID displayIDs[device+1];
        CGDisplayCount    nDisplays;

        if( CGGetOnlineDisplayList( device+1, displayIDs, &nDisplays ) !=
            kCGErrorSuccess )
        {
            setError( ERROR_AGLPIPE_DISPLAYS_NOTFOUND );
            return false;
        }

        if( nDisplays <= device )
        {
            setError( ERROR_AGLPIPE_DEVICE_NOTFOUND );
            return false;
        }

        displayID = displayIDs[device];
    }

    _setCGDisplayID( displayID );
    EQINFO << "Using CG displayID " << displayID << std::endl;
    return true;
}


void AGLPipe::_setCGDisplayID( CGDirectDisplayID id )
{
    if( _cgDisplayID == id )
        return;

    _cgDisplayID = id; 
    PixelViewport pvp = getPipe()->getPixelViewport();

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

    getPipe()->setPixelViewport( pvp );
}


void AGLPipe::configExit()
{
    _setCGDisplayID( kCGNullDirectDisplay );
    EQINFO << "Reset CG displayID " << std::endl;
}

} //namespace eq

