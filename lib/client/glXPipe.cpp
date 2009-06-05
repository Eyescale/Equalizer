
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "glXPipe.h"

#include "glXEventHandler.h"
#include "global.h"
#include "log.h"
#include "pipe.h"

#include <sstream>

using namespace eq::base;
using namespace std;

namespace eq
{

GLXPipe::GLXPipe( Pipe* parent )
        : OSPipe( parent )
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
        ostringstream msg;
        msg << "Can't open display: " << XDisplayName( displayName.c_str( ));
        setErrorMessage( msg.str( ));
        return false;
    }

    _setXDisplay( xDisplay );
    EQINFO << "Opened X display " << xDisplay << ", device "
           << _pipe->getDevice() << endl;
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
    EQINFO << "Closed X display " << xDisplay << endl;
#endif
}


std::string GLXPipe::_getXDisplayString()
{
    ostringstream  stringStream;
    
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

    if( display )
    {
        initEventHandler();
#ifndef NDEBUG
        // somewhat reduntant since it is a global handler
        XSetErrorHandler( eq::GLXPipe::XErrorHandler );
#endif

        string       displayString = DisplayString( display );
        const size_t colonPos      = displayString.find( ':' );
        if( colonPos != string::npos )
        {
            const string displayNumberString = displayString.substr(colonPos+1);
            const uint32_t displayNumber = atoi( displayNumberString.c_str( ));
            const uint32_t port          = _pipe->getPort();
            const uint32_t device        = _pipe->getDevice();
            
            if( port != EQ_UNDEFINED_UINT32 && displayNumber != port )
                EQWARN << "Display mismatch: provided display connection uses"
                   << " display " << displayNumber
                   << ", but pipe has port " << port << endl;

            if( device != EQ_UNDEFINED_UINT32 &&
                DefaultScreen( display ) != (int)device )
                
                EQWARN << "Screen mismatch: provided display connection uses"
                       << " default screen " << DefaultScreen( display ) 
                       << ", but pipe has screen " << device << endl;
            
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
    EQERROR << disableFlush;
    EQERROR << "X Error occured: " << disableHeader << indent;

    char buffer[256];
    XGetErrorText( display, event->error_code, buffer, 256);

    EQERROR << buffer << endl;
    EQERROR << "Major opcode: " << (int)event->request_code << endl;
    EQERROR << "Minor opcode: " << (int)event->minor_code << endl;
    EQERROR << "Error code: " << (int)event->error_code << endl;
    EQERROR << "Request serial: " << event->serial << endl;
    EQERROR << "Current serial: " << NextRequest( display ) - 1 << endl;

    switch( event->error_code )
    {
        case BadValue:
            EQERROR << "  Value: " << event->resourceid << endl;
            break;

        case BadAtom:
            EQERROR << "  AtomID: " << event->resourceid << endl;
            break;

        default:
            EQERROR << "  ResourceID: " << event->resourceid << endl;
            break;
    }
    EQERROR << enableFlush << exdent << enableHeader;

#ifndef NDEBUG
    if( getenv( "EQ_ABORT_WAIT" ))
    {
        EQERROR << "Caught X Error, entering infinite loop for debugging" 
                << endl;
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
