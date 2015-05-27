
/* Copyright (c) 2005-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Maxim Makhinya
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "../gl.h"
#include "../global.h"
#include "../log.h"
#include "../pipe.h"

#include <eq/fabric/gpuInfo.h>
#include <lunchbox/os.h>
#include <boost/lexical_cast.hpp>

using boost::lexical_cast;

namespace eq
{
namespace glx
{
#ifndef NDEBUG
static int XErrorHandler( Display* display, XErrorEvent* event );
#endif

namespace detail
{
class Pipe
{
public:
    Pipe()
        : xDisplay( 0 )
    {
        lunchbox::setZero( &glxewContext, sizeof( GLXEWContext ));
    }

    ~Pipe()
    {
#ifndef NDEBUG
        lunchbox::setZero( &glxewContext, sizeof( GLXEWContext ));
#endif
    }

    /** Window-system specific display information. */
    Display* xDisplay;

    /** Extended GLX function entries. */
    GLXEWContext glxewContext;
};
}

Pipe::Pipe( eq::Pipe* parent )
    : SystemPipe( parent )
    , _impl( new detail::Pipe )
{
}

Pipe::~Pipe( )
{
    delete _impl;
}

Display* Pipe::getXDisplay() const
{
    return _impl->xDisplay;
}

GLXEWContext* Pipe::glxewGetContext()
{
    return &_impl->glxewContext;
}

//---------------------------------------------------------------------------
// GLX init
//---------------------------------------------------------------------------
bool Pipe::configInit()
{
    const std::string displayName = getXDisplayString();
    Display* xDisplay = XOpenDisplay( displayName.c_str( ));

    if( !xDisplay )
    {
        sendError( ERROR_GLXPIPE_DEVICE_NOTFOUND )
            << displayName << lunchbox::sysError();
        return false;
    }

    int major, event, error;
    if( !XQueryExtension( xDisplay, "GLX", &major, &event, &error ))
    {
        sendError( ERROR_GLXPIPE_GLX_NOTFOUND );
        XCloseDisplay( xDisplay );
        return false;
    }

    setXDisplay( xDisplay );
    LBVERB << "Opened X display " << displayName << " @" << xDisplay
           << ", device " << getPipe()->getDevice() << std::endl;

    return _configInitGLXEW();
}

void Pipe::configExit()
{
    Display* xDisplay = getXDisplay();
    if( !xDisplay )
        return;

    setXDisplay( 0 );
    XCloseDisplay( xDisplay );
    LBVERB << "Closed X display " << xDisplay << std::endl;
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

    const std::string name( XDisplayName( stringStream.str().c_str( )));
    return name.empty() ? ":0" : name;
}

void Pipe::setXDisplay( Display* display )
{
    if( _impl->xDisplay == display )
        return;

    _impl->xDisplay = display;
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
            LBWARN << "Display mismatch: provided display connection uses"
                   << " display " << info.port << ", but pipe has port " << port
                   << std::endl;

        if( device != LB_UNDEFINED_UINT32 && info.device != device )
            LBWARN << "Screen mismatch: provided display connection uses "
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
    LBASSERT( _impl->xDisplay );

    //----- Create and make current a temporary GL context to initialize GLXEW
    // visual
    std::vector<int> attributes;
    attributes.push_back( GLX_RGBA );
    attributes.push_back( None );

    const int screen = DefaultScreen( _impl->xDisplay );
    XVisualInfo *visualInfo = glXChooseVisual( _impl->xDisplay, screen,
                                               &attributes.front( ));
    if( !visualInfo )
    {
        sendError( ERROR_SYSTEMPIPE_PIXELFORMAT_NOTFOUND );
        return false;
    }

    //context
    GLXContext context = glXCreateContext( _impl->xDisplay, visualInfo, 0,
                                           True );
    if( !context )
    {
        sendError( ERROR_SYSTEMPIPE_CREATECONTEXT_FAILED );
        return false;
    }

    // window
    const XID parent = RootWindow( _impl->xDisplay, screen );
    XSetWindowAttributes wa;
    wa.colormap = XCreateColormap( _impl->xDisplay, parent, visualInfo->visual,
                                   AllocNone );
    wa.background_pixmap = None;
    wa.border_pixel = 0;
    XID drawable = XCreateWindow( _impl->xDisplay, parent, 0, 0, 16, 16,
                                  0, visualInfo->depth, InputOutput,
                                  visualInfo->visual,
                                  CWBackPixmap | CWBorderPixel | CWColormap,
                                  &wa );
    if( !drawable )
    {
        sendError( ERROR_SYSTEMPIPE_CREATEWINDOW_FAILED );
        return false;
    }

    XFree( visualInfo );
    XSync( _impl->xDisplay, False );

    glXMakeCurrent( _impl->xDisplay, drawable, context );

    const GLenum result = glxewInit();
    bool success = result == GLEW_OK;
    if( success )
    {
        LBVERB << "Pipe GLXEW initialization successful" << std::endl;
        success = configInitGL();

        const char* glVersion = (const char*)glGetString( GL_VERSION );
        if( success && glVersion )
            _maxOpenGLVersion = static_cast<float>( atof( glVersion ));
    }
    else
        sendError( ERROR_GLXPIPE_GLXEWINIT_FAILED )
            << lexical_cast< std::string >( result );

    XSync( _impl->xDisplay, False );
    glXDestroyContext( _impl->xDisplay, context );
    XDestroyWindow( _impl->xDisplay, drawable );

    return success;
}

#ifndef NDEBUG
int XErrorHandler( Display* display, XErrorEvent* event )
{
    LBWARN << lunchbox::disableFlush;
    LBWARN << "X Error occured: " << lunchbox::disableHeader
           << lunchbox::indent;

    char buffer[256];
    XGetErrorText( display, event->error_code, buffer, 256);

    LBWARN << buffer << std::endl;
    LBWARN << "Major opcode: " << (int)event->request_code << std::endl;
    LBWARN << "Minor opcode: " << (int)event->minor_code << std::endl;
    LBWARN << "Error code: " << (int)event->error_code << std::endl;
    LBWARN << "Request serial: " << event->serial << std::endl;
    LBWARN << "Current serial: " << NextRequest( display ) - 1 << std::endl;

    switch( event->error_code )
    {
        case BadValue:
            LBWARN << "  Value: " << event->resourceid << std::endl;
            break;

        case BadAtom:
            LBWARN << "  AtomID: " << event->resourceid << std::endl;
            break;

        default:
            LBWARN << "  ResourceID: " << event->resourceid << std::endl;
            break;
    }
    LBWARN << lunchbox::enableFlush << lunchbox::exdent
           << lunchbox::enableHeader;

#ifndef NDEBUG
    if( getenv( "EQ_ABORT_WAIT" ))
    {
        LBWARN << "Caught X Error, entering infinite loop for debugging"
               << std::endl;
        while( true ) ;
    }
#endif

    return 0;
}
#endif // !NDEBUG

}
}
