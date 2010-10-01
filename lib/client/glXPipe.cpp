
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

#include "glXEventHandler.h"
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
        , _eventHandler( 0 )
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
    const std::string displayName  = _getXDisplayString();
    const char*       cDisplayName = ( displayName.length() == 0 ? 
                                       0 : displayName.c_str( ));
    Display*          xDisplay     = XOpenDisplay( cDisplayName );
            
    if( !xDisplay )
    {
        std::ostringstream msg;
        msg << "Can't open display: " << XDisplayName( displayName.c_str( ));
        setErrorMessage( msg.str( ));
        return false;
    }

    int major, event, error;
    if( !XQueryExtension( xDisplay, "GLX", &major, &event, &error ))
    {
        std::ostringstream msg;
        msg << "Display " << XDisplayName( displayName.c_str( ))
            << " does not support GLX";
        setErrorMessage( msg.str( ));
        XCloseDisplay( xDisplay );
        return false;
    }

    _setXDisplay( xDisplay );
    EQINFO << "Opened X display " << xDisplay << ", device "
           << _pipe->getDevice() << std::endl;
    return true;
#else
    setErrorMessage( "Client library compiled without GLX support" );
    return false;
#endif
}


void GLXPipe::configExit()
{
#ifdef GLX
    Display* xDisplay = getXDisplay();
    if( !xDisplay )
        return;

    _setXDisplay( 0 );
    XCloseDisplay( xDisplay );
    EQINFO << "Closed X display " << xDisplay << std::endl;
#endif
}


std::string GLXPipe::_getXDisplayString()
{
    std::ostringstream  stringStream;
    
    const uint32_t port   = _pipe->getPort();
    const uint32_t device = _pipe->getDevice();

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


void GLXPipe::_setXDisplay( Display* display )
{
#ifdef GLX
    if( _xDisplay == display )
        return;

    if( _xDisplay )
        exitEventHandler();
    _xDisplay = display; 
    XSetCurrentDisplay( display );

    if( display )
    {
        initEventHandler();
#ifndef NDEBUG
        // somewhat reduntant since it is a global handler
        XSetErrorHandler( eq::GLXPipe::XErrorHandler );
#endif

        std::string       displayString = DisplayString( display );
        const size_t colonPos      = displayString.find( ':' );
        if( colonPos != std::string::npos )
        {
            const std::string displayNumberString = displayString.substr(colonPos+1);
            const uint32_t displayNumber = atoi( displayNumberString.c_str( ));
            const uint32_t port          = _pipe->getPort();
            const uint32_t device        = _pipe->getDevice();
            
            if( port != EQ_UNDEFINED_UINT32 && displayNumber != port )
                EQWARN << "Display mismatch: provided display connection uses"
                   << " display " << displayNumber
                   << ", but pipe has port " << port << std::endl;

            if( device != EQ_UNDEFINED_UINT32 &&
                DefaultScreen( display ) != (int)device )
                
                EQWARN << "Screen mismatch: provided display connection uses"
                       << " default screen " << DefaultScreen( display ) 
                       << ", but pipe has screen " << device << std::endl;
            
            //port = displayNumber;
            //device  = DefaultScreen( display );
        }
    }

    PixelViewport pvp = _pipe->getPixelViewport();
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

    _pipe->setPixelViewport( pvp );
#endif
}


int GLXPipe::XErrorHandler( Display* display, XErrorEvent* event )
{
#ifdef GLX
    EQERROR << base::disableFlush;
    EQERROR << "X Error occured: " << base::disableHeader << base::indent;

    char buffer[256];
    XGetErrorText( display, event->error_code, buffer, 256);

    EQERROR << buffer << std::endl;
    EQERROR << "Major opcode: " << (int)event->request_code << std::endl;
    EQERROR << "Minor opcode: " << (int)event->minor_code << std::endl;
    EQERROR << "Error code: " << (int)event->error_code << std::endl;
    EQERROR << "Request serial: " << event->serial << std::endl;
    EQERROR << "Current serial: " << NextRequest( display ) - 1 << std::endl;

    switch( event->error_code )
    {
        case BadValue:
            EQERROR << "  Value: " << event->resourceid << std::endl;
            break;

        case BadAtom:
            EQERROR << "  AtomID: " << event->resourceid << std::endl;
            break;

        default:
            EQERROR << "  ResourceID: " << event->resourceid << std::endl;
            break;
    }
    EQERROR << base::enableFlush << base::exdent << base::enableHeader;

#ifndef NDEBUG
    if( getenv( "EQ_ABORT_WAIT" ))
    {
        EQERROR << "Caught X Error, entering infinite loop for debugging" 
                << std::endl;
        while( true ) ;
    }
#endif

#endif // GLX

    return 0;
}

void GLXPipe::initEventHandler()
{
    EQASSERT( !_eventHandler );
    _eventHandler = new GLXEventHandler( this );
}

void GLXPipe::exitEventHandler()
{
    delete _eventHandler;
    _eventHandler = 0;
}


}
