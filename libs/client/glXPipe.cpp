
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "glXPipe.h"

#include "global.h"
#include "log.h"
#include "pipe.h"

#include <sstream>
#include <string>

namespace eq
{

GLXPipe::GLXPipe( Pipe* parent )
        : SystemPipe( parent )
        , _xDisplay( 0 )
{
}

GLXPipe::~GLXPipe( )
{
}

//---------------------------------------------------------------------------
// GLX init
//---------------------------------------------------------------------------
bool GLXPipe::configInit( )
{
#ifdef GLX
    const std::string displayName  = getXDisplayString();
    const char*       cDisplayName = ( displayName.length() == 0 ? 
                                       0 : displayName.c_str( ));
    Display*          xDisplay     = XOpenDisplay( cDisplayName );
            
    if( !xDisplay )
    {
        setError( ERROR_GLXPIPE_DEVICE_NOTFOUND );
        EQWARN << getError() << ": " << XDisplayName( displayName.c_str( ));
        return false;
    }

    int major, event, error;
    if( !XQueryExtension( xDisplay, "GLX", &major, &event, &error ))
    {
        setError( ERROR_GLXPIPE_GLX_NOTFOUND );
        XCloseDisplay( xDisplay );
        return false;
    }

    setXDisplay( xDisplay );
    EQINFO << "Opened X display " << XDisplayName( displayName.c_str( )) << " @"
           << xDisplay << ", device " << getPipe()->getDevice() << std::endl;
    return true;
#else
    setError( ERROR_GLX_MISSING_SUPPORT );
    return false;
#endif
}


void GLXPipe::configExit()
{
#ifdef GLX
    Display* xDisplay = getXDisplay();
    if( !xDisplay )
        return;

    setXDisplay( 0 );
    XCloseDisplay( xDisplay );
    EQINFO << "Closed X display " << xDisplay << std::endl;
#endif
}


std::string GLXPipe::getXDisplayString()
{
    std::ostringstream  stringStream;
    
    const uint32_t port   = getPipe()->getPort();
    const uint32_t device = getPipe()->getDevice();

    if( port != EQ_UNDEFINED_UINT32 )
    { 
        if( device == EQ_UNDEFINED_UINT32 )
            stringStream << ":" << port;
        else
            stringStream << ":" << port << "." << device;
    }
    else if( device != EQ_UNDEFINED_UINT32 )
        stringStream << ":0." << device;
    else if( !getenv( "DISPLAY" ))
        stringStream <<  ":0";

    return stringStream.str();
}


void GLXPipe::setXDisplay( Display* display )
{
#ifdef GLX
    if( _xDisplay == display )
        return;

    _xDisplay = display; 
    XSetCurrentDisplay( display );

    if( display )
    {
#ifndef NDEBUG
        // somewhat reduntant since it is a global handler
        XSetErrorHandler( eq::GLXPipe::XErrorHandler );
#endif

        std::string displayString = DisplayString( display );
        const size_t colonPos = displayString.find( ':' );
        if( colonPos != std::string::npos )
        {
            const std::string displayNumberString = 
                displayString.substr( colonPos+1 );
            const uint32_t displayNumber = atoi( displayNumberString.c_str( ));
            const uint32_t port          = getPipe()->getPort();
            const uint32_t device        = getPipe()->getDevice();
            
            if( port != EQ_UNDEFINED_UINT32 && displayNumber != port )
                EQWARN << "Display mismatch: provided display connection uses"
                       << " display " << displayNumber
                       << ", but pipe has port " << port << std::endl;

            if( device != EQ_UNDEFINED_UINT32 &&
                DefaultScreen( display ) != (int)device )
            {
                EQWARN << "Screen mismatch: provided display connection uses"
                       << " default screen " << DefaultScreen( display ) 
                       << ", but pipe has screen " << device << std::endl;
            }
            //port = displayNumber;
            //device  = DefaultScreen( display );
        }
    }

    PixelViewport pvp = getPipe()->getPixelViewport();
    if( pvp.isValid( ))
        return;

    if( display )
    {
        pvp.x    = 0;
        pvp.y    = 0;
        pvp.w = DisplayWidth(  display, DefaultScreen( display ));
        pvp.h = DisplayHeight( display, DefaultScreen( display ));
    }
    else
        pvp.invalidate();

    getPipe()->setPixelViewport( pvp );
#endif
}


int GLXPipe::XErrorHandler( Display* display, XErrorEvent* event )
{
#ifdef GLX
    EQWARN << base::disableFlush;
    EQWARN << "X Error occured: " << base::disableHeader << base::indent;

    char buffer[256];
    XGetErrorText( display, event->error_code, buffer, 256);

    EQWARN << buffer << std::endl;
    EQWARN << "Major opcode: " << (int)event->request_code << std::endl;
    EQWARN << "Minor opcode: " << (int)event->minor_code << std::endl;
    EQWARN << "Error code: " << (int)event->error_code << std::endl;
    EQWARN << "Request serial: " << event->serial << std::endl;
    EQWARN << "Current serial: " << NextRequest( display ) - 1 << std::endl;

    switch( event->error_code )
    {
        case BadValue:
            EQWARN << "  Value: " << event->resourceid << std::endl;
            break;

        case BadAtom:
            EQWARN << "  AtomID: " << event->resourceid << std::endl;
            break;

        default:
            EQWARN << "  ResourceID: " << event->resourceid << std::endl;
            break;
    }
    EQWARN << base::enableFlush << base::exdent << base::enableHeader;

#ifndef NDEBUG
    if( getenv( "EQ_ABORT_WAIT" ))
    {
        EQWARN << "Caught X Error, entering infinite loop for debugging" 
               << std::endl;
        while( true ) ;
    }
#endif

#endif // GLX

    return 0;
}

}
