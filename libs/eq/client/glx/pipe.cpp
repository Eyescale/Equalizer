
/* Copyright (c) 2005-2011, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "pipe.h"

#include "../global.h"
#include "../glXTypes.h"
#include "../log.h"
#include "../pipe.h"

#include <eq/fabric/gpuInfo.h>

#include <sstream>
#include <string>

namespace eq
{
namespace glx
{
#ifndef NDEBUG
static int XErrorHandler( Display* display, XErrorEvent* event );
#endif

Pipe::Pipe( eq::Pipe* parent )
        : SystemPipe( parent )
        , _xDisplay( 0 )
        , _glxewContext( new GLXEWContext )
{
}

Pipe::~Pipe( )
{
    delete _glxewContext;
}

//---------------------------------------------------------------------------
// GLX init
//---------------------------------------------------------------------------
bool Pipe::configInit()
{
    const std::string displayName  = getXDisplayString();
    const char*       cDisplayName = ( displayName.empty() ? 
                                       0 : displayName.c_str( ));
    Display*          xDisplay     = XOpenDisplay( cDisplayName );
            
    if( !xDisplay )
    {
        setError( ERROR_GLXPIPE_DEVICE_NOTFOUND );
        EQWARN << getError() << " " << XDisplayName( displayName.c_str( ))
               << ": " << lunchbox::sysError << std::endl;
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

    return _configInitGLXEW();
}

void Pipe::configExit()
{
    Display* xDisplay = getXDisplay();
    if( !xDisplay )
        return;

    setXDisplay( 0 );
    XCloseDisplay( xDisplay );
    EQINFO << "Closed X display " << xDisplay << std::endl;
}


std::string Pipe::getXDisplayString()
{
    std::ostringstream  stringStream;
    
    const uint32_t port   = getPipe()->getPort();
    const uint32_t device = getPipe()->getDevice();

    if( port != LB_UNDEFINED_UINT32 )
    { 
        if( device == LB_UNDEFINED_UINT32 )
            stringStream << ":" << port;
        else
            stringStream << ":" << port << "." << device;
    }
    else if( device != LB_UNDEFINED_UINT32 )
        stringStream << ":0." << device;
    else if( !getenv( "DISPLAY" ))
        stringStream <<  ":0";

    return stringStream.str();
}


void Pipe::setXDisplay( Display* display )
{
    if( _xDisplay == display )
        return;

    _xDisplay = display; 
    XSetCurrentDisplay( display );

    GPUInfo info;
    if( getGPUInfo( display, info ))
    {
#ifndef NDEBUG
        // somewhat reduntant since it is a global handler
        XSetErrorHandler( XErrorHandler );
#endif
        const uint32_t port = getPipe()->getPort();
        const uint32_t device = getPipe()->getDevice();

        if( port != LB_UNDEFINED_UINT32 && info.port != port )
            EQWARN << "Display mismatch: provided display connection uses"
                   << " display " << info.port << ", but pipe has port " << port
                   << std::endl;

        if( device != LB_UNDEFINED_UINT32 && info.device != device )
            EQWARN << "Screen mismatch: provided display connection uses "
                   << "default screen " << info.device
                   << ", but pipe has screen " << device << std::endl;
    }

    const PixelViewport& pvp = getPipe()->getPixelViewport();
    if( !pvp.isValid( ))
        getPipe()->setPixelViewport( info.pvp );
}

bool Pipe::getGPUInfo( Display* display, GPUInfo& info )
{
    if( !display )
        return false;

    std::string displayString = DisplayString( display );
    const size_t colonPos = displayString.find( ':' );
    if( colonPos != std::string::npos )
    {
        const std::string displayNumber = displayString.substr( colonPos+1 );
        info.port = atoi( displayNumber.c_str( ));
        info.device = DefaultScreen( display );
    }

    info.pvp.x = 0;
    info.pvp.y = 0;
    info.pvp.w = DisplayWidth(  display, DefaultScreen( display ));
    info.pvp.h = DisplayHeight( display, DefaultScreen( display ));
    
    return true;
}

bool Pipe::_configInitGLXEW()
{
    EQASSERT( _xDisplay );

    //----- Create and make current a temporary GL context to initialize GLXEW
    // visual
    std::vector<int> attributes;
    attributes.push_back( GLX_RGBA );
    attributes.push_back( None );

    const int    screen  = DefaultScreen( _xDisplay );
    XVisualInfo *visualInfo = glXChooseVisual( _xDisplay, screen,
                                               &attributes.front( ));
    if( !visualInfo )
    {
        setError( ERROR_SYSTEMPIPE_PIXELFORMAT_NOTFOUND );
        return false;
    }

    //context
    GLXContext context = glXCreateContext( _xDisplay, visualInfo, 0, True );
    if( !context )
    {
        setError( ERROR_SYSTEMPIPE_CREATECONTEXT_FAILED );
        return false;
    }

    // window
    const XID parent = RootWindow( _xDisplay, screen );
    XSetWindowAttributes wa;
    wa.colormap = XCreateColormap( _xDisplay, parent, visualInfo->visual,
                                   AllocNone );
    wa.background_pixmap = None;
    wa.border_pixel = 0;
    XID drawable = XCreateWindow( _xDisplay, parent, 0, 0, 16, 16,
                                  0, visualInfo->depth, InputOutput,
                                  visualInfo->visual, 
                                  CWBackPixmap | CWBorderPixel | CWColormap,
                                  &wa );
    if( !drawable )
    {
        setError( ERROR_SYSTEMPIPE_CREATEWINDOW_FAILED );
        return false;
    }   

    XFree( visualInfo );
    XSync( _xDisplay, False );

    glXMakeCurrent( _xDisplay, drawable, context );

    const GLenum result = glxewInit();
    bool success = result == GLEW_OK;
    if( !success )
    {
        setError( ERROR_GLXPIPE_GLXEWINIT_FAILED );
        EQWARN << getError() << ": " << result << std::endl;
    }
    else
    {
        EQINFO << "Pipe GLXEW initialization successful" << std::endl;
        success = configInitGL();
    }

    XSync( _xDisplay, False );
    glXDestroyContext( _xDisplay, context );
    XDestroyWindow( _xDisplay, drawable );

    return success;
}

#ifndef NDEBUG
int XErrorHandler( Display* display, XErrorEvent* event )
{
    EQWARN << lunchbox::disableFlush;
    EQWARN << "X Error occured: " << lunchbox::disableHeader 
           << lunchbox::indent;

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
    EQWARN << lunchbox::enableFlush << lunchbox::exdent 
           << lunchbox::enableHeader;

#ifndef NDEBUG
    if( getenv( "EQ_ABORT_WAIT" ))
    {
        EQWARN << "Caught X Error, entering infinite loop for debugging" 
               << std::endl;
        while( true ) ;
    }
#endif

    return 0;
}
#endif // !NDEBUG

}
}
