
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "window.h"

#include "commands.h"
#include "global.h"
#include "nodeFactory.h"
#include "packets.h"
#include "channel.h"

#include <eq/net/barrier.h>

using namespace eq;
using namespace std;

eq::Window::Window()
        : eqNet::Base( CMD_WINDOW_ALL ),
#ifdef GLX
          _xDrawable(0),
          _glXContext(NULL),
#endif
#ifdef CGL
          _cglContext( NULL ),
#endif
          _pipe(NULL)
{
    registerCommand( CMD_WINDOW_CREATE_CHANNEL, this,
                     reinterpret_cast<CommandFcn>(
                         &eq::Window::_cmdCreateChannel ));
    registerCommand( CMD_WINDOW_DESTROY_CHANNEL, this,
                     reinterpret_cast<CommandFcn>(
                         &eq::Window::_cmdDestroyChannel ));
    registerCommand( CMD_WINDOW_INIT, this, reinterpret_cast<CommandFcn>(
                         &eq::Window::_pushRequest ));
    registerCommand( REQ_WINDOW_INIT, this, reinterpret_cast<CommandFcn>(
                         &eq::Window::_reqInit ));
    registerCommand( CMD_WINDOW_EXIT, this, reinterpret_cast<CommandFcn>( 
                         &eq::Window::_pushRequest ));
    registerCommand( REQ_WINDOW_EXIT, this, reinterpret_cast<CommandFcn>( 
                         &eq::Window::_reqExit ));
    registerCommand( CMD_WINDOW_SWAP, this, reinterpret_cast<CommandFcn>(
                         &eq::Window::_pushRequest));
    registerCommand( REQ_WINDOW_SWAP, this, reinterpret_cast<CommandFcn>(
                         &eq::Window::_reqSwap));
    registerCommand( CMD_WINDOW_SWAP_WITH_BARRIER, this,
                     reinterpret_cast<CommandFcn>(&eq::Window::_pushRequest));
    registerCommand( REQ_WINDOW_SWAP_WITH_BARRIER, this,
                     reinterpret_cast<CommandFcn>(
                         &eq::Window::_reqSwapWithBarrier));
}

eq::Window::~Window()
{
}

void eq::Window::_addChannel( Channel* channel )
{
    _channels.push_back( channel );
    channel->_window = this;
}

void eq::Window::_removeChannel( Channel* channel )
{
    vector<Channel*>::iterator iter = find( _channels.begin(), _channels.end(), 
                                            channel );
    if( iter == _channels.end( ))
        return;
    
    _channels.erase( iter );
    channel->_window = NULL;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
eqNet::CommandResult eq::Window::_pushRequest( eqNet::Node* node,
                                               const eqNet::Packet* packet )
{
    if( _pipe )
        return _pipe->pushRequest( node, packet );

    return _cmdUnknown( node, packet );
}

eqNet::CommandResult eq::Window::_cmdCreateChannel( eqNet::Node* node,
                                                    const eqNet::Packet* pkg )
{
    WindowCreateChannelPacket* packet = (WindowCreateChannelPacket*)pkg;
    EQINFO << "Handle create channel " << packet << endl;

    Channel* channel = Global::getNodeFactory()->createChannel();
    
    getConfig()->addRegisteredObject( packet->channelID, channel );
    _addChannel( channel );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult eq::Window::_cmdDestroyChannel(eqNet::Node* node, 
                                                    const eqNet::Packet* pkg)
{
    WindowDestroyChannelPacket* packet = (WindowDestroyChannelPacket*)pkg;
    EQINFO << "Handle destroy channel " << packet << endl;

    Config*  config  = getConfig();
    Channel* channel = (Channel*)config->getObject(packet->channelID);
    if( !channel )
        return eqNet::COMMAND_HANDLED;

    _removeChannel( channel );
    config->deregisterObject( channel );
    delete channel;
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult eq::Window::_reqInit( eqNet::Node* node,
                                           const eqNet::Packet* pkg )
{
    WindowInitPacket* packet = (WindowInitPacket*)pkg;
    EQINFO << "handle window init " << packet << endl;

    WindowInitReplyPacket reply( packet );
    reply.result = init();

    if( !reply.result )
    {
        node->send( reply );
        return eqNet::COMMAND_HANDLED;
    }

    switch( _pipe->getWindowSystem( ))
    {
#ifdef GLX
        case WINDOW_SYSTEM_GLX:
            if( !_xDrawable || !_glXContext )
            {
                EQERROR << "init() did not provide a drawable and context" 
                        << endl;
                reply.result = false;
                node->send( reply );
                return eqNet::COMMAND_HANDLED;
            }
            break;
#endif
#ifdef CGL
        case WINDOW_SYSTEM_CGL:
            if( !_cglContext )
            {
                EQERROR << "init() did not provide an OpenGL context" << endl;
                reply.result = false;
                node->send( reply );
                return eqNet::COMMAND_HANDLED;
            }
            // TODO: pvp
            break;
#endif

        default: EQUNIMPLEMENTED;
    }

    reply.pvp = _pvp;
    node->send( reply );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult eq::Window::_reqExit( eqNet::Node* node,
                                           const eqNet::Packet* pkg )
{
    WindowExitPacket* packet = (WindowExitPacket*)pkg;
    EQINFO << "handle window exit " << packet << endl;

    exit();

    WindowExitReplyPacket reply( packet );
    node->send( reply );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult eq::Window::_reqSwap(eqNet::Node* node, 
                                      const eqNet::Packet* pkg)
{
    WindowSwapPacket* packet = (WindowSwapPacket*)pkg;
    EQINFO << "handle swap " << packet << endl;

    swap();
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult eq::Window::_reqSwapWithBarrier(eqNet::Node* node,
                                                 const eqNet::Packet* pkg)
{
    WindowSwapWithBarrierPacket* packet = (WindowSwapWithBarrierPacket*)pkg;
    EQINFO << "handle swap with barrier " << packet << endl;

    eqNet::Mobject* mobject = _session->getMobject( packet->barrierID );
    EQASSERT( dynamic_cast<eqNet::Barrier*>(mobject) );

    eqNet::Barrier* barrier = (eqNet::Barrier*)mobject;
    finish();
    barrier->enter();
    swap();
    return eqNet::COMMAND_HANDLED;
}

//---------------------------------------------------------------------------
// pipe-thread methods
//---------------------------------------------------------------------------

//----------------------------------------------------------------------
// init
//----------------------------------------------------------------------
bool eq::Window::init()
{
    const WindowSystem windowSystem = _pipe->getWindowSystem();
    switch( windowSystem )
    {
        case WINDOW_SYSTEM_GLX:
            return initGLX();

        case WINDOW_SYSTEM_CGL:
            return initCGL();

        default:
            EQERROR << "Unknown windowing system: " << windowSystem << endl;
            return false;
    }
    glClearColor( .7, .5, .5, 1. );
}

#ifdef GLX
static Bool WaitForNotify(Display *, XEvent *e, char *arg)
{ return (e->type == MapNotify) && (e->xmap.window == (::Window)arg); }
#endif

bool eq::Window::initGLX()
{
#ifdef GLX
    Display *display = _pipe->getXDisplay();
    if( !display ) 
        return false;

    int screen  = DefaultScreen( display );
    XID parent  = RootWindow( display, screen );
    int size[4] = { 0, 0, DisplayWidth( display, screen )/2, 
                    DisplayHeight( display, screen )/2 };

    int attributes[100], *aptr=attributes;    
    *aptr++ = GLX_RGBA;
    *aptr++ = 1;
    *aptr++ = GLX_RED_SIZE;
    *aptr++ = 8;
    //*aptr++ = GLX_ALPHA_SIZE;
    //*aptr++ = 1;
    *aptr++ = GLX_DEPTH_SIZE;
    *aptr++ = 1;
    *aptr++ = GLX_STENCIL_SIZE;
    *aptr++ = 8;
    *aptr++ = GLX_DOUBLEBUFFER;
    *aptr = None;

    XVisualInfo *visInfo = glXChooseVisual( display, screen, attributes );
    if ( !visInfo )
    {
        EQERROR << "Could not find a matching visual\n" << endl;
        return false;
    }

    XSetWindowAttributes wa;
    wa.colormap          = XCreateColormap( display, parent, visInfo->visual,
                                            AllocNone );
    wa.background_pixmap = None;
    wa.border_pixel      = 0;
    wa.event_mask        = StructureNotifyMask | VisibilityChangeMask;
    wa.override_redirect = True;

    XID drawable = XCreateWindow( display, parent, size[0], size[1], size[2],
                                  size[3], 0, visInfo->depth, InputOutput,
                                  visInfo->visual, CWBackPixmap|CWBorderPixel|
                                  CWEventMask|CWColormap|CWOverrideRedirect,
                                  &wa );
    
    if ( !drawable )
    {
        EQERROR << "Could not create window\n" << endl;
        return false;
    }

    // map and wait for MapNotify event
    XMapWindow( display, drawable );
    XEvent event;

    XIfEvent( display, &event, WaitForNotify, (XPointer)(drawable) );
    XFlush( display );

    // create context
    GLXContext context = glXCreateContext( display, visInfo, NULL, True );
    if ( !context )
    {
        EQERROR << "Could not create OpenGL context\n" << endl;
        return false;
    }

    glXMakeCurrent( display, drawable, context );
    glClear( GL_COLOR_BUFFER_BIT );
    glXSwapBuffers( display, drawable );
    glClear( GL_COLOR_BUFFER_BIT );

    setXDrawable( drawable );
    setGLXContext( context );
    return true;
#else
    return false;
#endif
}

bool eq::Window::initCGL()
{
#ifdef CGL
    CGDirectDisplayID displayID = _pipe->getCGLDisplayID();

    CGLPixelFormatAttribute attribs[] = { 
        kCGLPFADisplayMask,
        (CGLPixelFormatAttribute)CGDisplayIDToOpenGLDisplayMask( displayID ),
        kCGLPFAFullScreen, 
        kCGLPFADoubleBuffer, 
        kCGLPFADepthSize, (CGLPixelFormatAttribute)16, 
        (CGLPixelFormatAttribute)0 };

    CGLPixelFormatObj pixelFormat = NULL;
    long numPixelFormats = 0;
    CGLChoosePixelFormat( attribs, &pixelFormat, &numPixelFormats );

    if( !pixelFormat )
    {
        EQERROR << "Could not find a matching pixel format\n" << endl;
        return false;
    }

    CGLContextObj context = 0;
    CGLCreateContext( pixelFormat, NULL, &context );
    CGLDestroyPixelFormat ( pixelFormat );

    if( !context ) 
    {
        EQERROR << "Could not create OpenGL context\n" << endl;
        return false;
    }

    // CGRect displayRect = CGDisplayBounds( displayID );
    // glViewport( 0, 0, displayRect.size.width, displayRect.size.height );

    CGLSetCurrentContext( context );
    CGLSetFullScreen( context );
    glClear( GL_COLOR_BUFFER_BIT );
    CGLFlushDrawable( context );
    glClear( GL_COLOR_BUFFER_BIT );

    setCGLContext( context );
    return true;
#else
    return false;
#endif
}

//----------------------------------------------------------------------
// exit
//----------------------------------------------------------------------
void eq::Window::exit()
{
    const WindowSystem windowSystem = _pipe->getWindowSystem();
    switch( windowSystem )
    {
        case WINDOW_SYSTEM_GLX:
            exitGLX();
            break;

        case WINDOW_SYSTEM_CGL:
            exitCGL();
            break;

        default:
            EQWARN << "Unknown windowing system: " << windowSystem << endl;
            return;
    }
}

void eq::Window::exitGLX()
{
#ifdef GLX
    Display *display = _pipe->getXDisplay();
    if( !display ) 
        return;

    GLXContext context = getGLXContext();
    if( context )
        glXDestroyContext( display, context );
    setGLXContext( NULL );

    XID drawable = getXDrawable();
    if( drawable )
        XDestroyWindow( display, drawable );
    setXDrawable( 0 );
#endif
}

void eq::Window::exitCGL()
{
#ifdef CGL
    CGLContextObj context = getCGLContext();
    if( !context )
        return;

    setCGLContext( NULL );

    CGLSetCurrentContext( NULL );
    CGLClearDrawable( context );
    CGLDestroyContext ( context );       
#endif
}


#ifdef GLX
void eq::Window::setXDrawable( XID drawable )
{
    _xDrawable = drawable;

    if( !drawable )
    {
        _pvp.reset();
        return;
    }

    // query pixel viewport of window
    Display          *display = _pipe->getXDisplay();
    EQASSERT( display );

    XWindowAttributes wa;
    XGetWindowAttributes( display, drawable, &wa );
    
    // Window position is relative to parent: translate to absolute coordinates
    ::Window root, parent, *children;
    unsigned nChildren;
    
    XQueryTree( display, drawable, &root, &parent, &children, &nChildren );
    if( children != NULL ) XFree( children );

    int x,y;
    ::Window childReturn;
    XTranslateCoordinates( display, parent, root, wa.x, wa.y, &x, &y,
        &childReturn );

    _pvp.x = x;
    _pvp.y = y;
    _pvp.w = wa.width;
    _pvp.h = wa.height;
}
#endif // GLX

#ifdef CGL
void eq::Window::setCGLContext( CGLContextObj context )
{
    _cglContext = context;
    // TODO: pvp
}
#endif // CGL

void eq::Window::swap()
{
    switch( _pipe->getWindowSystem( ))
    {
#ifdef GLX
        case WINDOW_SYSTEM_GLX:
            glXSwapBuffers( _pipe->getXDisplay(), _xDrawable );
            break;
#endif
#ifdef CGL
        case WINDOW_SYSTEM_CGL:
            CGLFlushDrawable( _cglContext );
            break;
#endif
        default: EQUNIMPLEMENTED;
    }
    EQINFO << "----- SWAP -----" << endl;
}
