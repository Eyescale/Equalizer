
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "window.h"

#include "commands.h"
#include "global.h"
#include "nodeFactory.h"
#include "packets.h"
#include "channel.h"

using namespace eq;
using namespace std;

eq::Window::Window()
        : eqNet::Base( CMD_WINDOW_ALL ),
          _pipe(NULL),
          _xDrawable(0),
          _glXContext(NULL)
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
void eq::Window::_pushRequest( eqNet::Node* node, const eqNet::Packet* packet )
{
    if( _pipe )
        _pipe->pushRequest( node, packet );
    else
        _cmdUnknown( node, packet );
}

void eq::Window::_cmdCreateChannel( eqNet::Node* node, const eqNet::Packet* pkg)
{
    WindowCreateChannelPacket* packet = (WindowCreateChannelPacket*)pkg;
    INFO << "Handle create channel " << packet << endl;

    Channel* channel = Global::getNodeFactory()->createChannel();
    
    getConfig()->addRegisteredObject( packet->channelID, channel );
    _addChannel( channel );
}

void eq::Window::_cmdDestroyChannel(eqNet::Node* node, const eqNet::Packet* pkg)
{
    WindowDestroyChannelPacket* packet = (WindowDestroyChannelPacket*)pkg;
    INFO << "Handle destroy channel " << packet << endl;

    Config*  config  = getConfig();
    Channel* channel = (Channel*)config->getRegisteredObject(packet->channelID);
    if( !channel )
        return;

    _removeChannel( channel );
    config->deregisterObject( channel );
    delete channel;
}

void eq::Window::_reqInit( eqNet::Node* node, const eqNet::Packet* pkg )
{
    WindowInitPacket* packet = (WindowInitPacket*)pkg;
    INFO << "handle window init " << packet << endl;

    WindowInitReplyPacket reply( packet );
    reply.result = init();

    if( !reply.result )
    {
        node->send( reply );
        return;
    }

#ifdef GLX
    if( !_xDrawable || !_glXContext )
    {
        ERROR << "Window::init() did not provide a drawable and context" 
              << endl;

        reply.result = false;
        node->send( reply );
        return;
    }
#endif

    node->send( reply );
}

void eq::Window::_reqExit( eqNet::Node* node, const eqNet::Packet* pkg )
{
    WindowExitPacket* packet = (WindowExitPacket*)pkg;
    INFO << "handle window exit " << packet << endl;

    exit();

    WindowExitReplyPacket reply( packet );
    node->send( reply );
}

//---------------------------------------------------------------------------
// pipe-thread methods
//---------------------------------------------------------------------------
static Bool WaitForNotify(Display *, XEvent *e, char *arg)
{ return (e->type == MapNotify) && (e->xmap.window == (::Window)arg); }

bool eq::Window::init()
{
#ifdef GLX
    Display *display = getXDisplay();
    if( !display ) 
        return false;

    int screen  = DefaultScreen( display );
    XID parent  = RootWindow( display, screen );
    int size[4] = { 0, 0, DisplayWidth( display, screen ), 
                    DisplayHeight( display, screen ) };

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
    *aptr = None;

    XVisualInfo *visInfo = glXChooseVisual( display, screen, attributes );
    if ( !visInfo )
    {
        ERROR << "Could not find a matching visual\n" << endl;
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
        ERROR << "Could not create window\n" << endl;
        return false;
    }

    // map and wait for MapNotify event
    XMapWindow( display, drawable );
    XEvent event;

    XIfEvent( display, &event, WaitForNotify, (XPointer)(drawable) );
    XFlush( display );

    // create context
    GLXContext context = glXCreateContext( display, visInfo, NULL, True );

    glXMakeCurrent( display, drawable, context );
    if ( !drawable )
    {
        ERROR << "Could not create OpenGL context\n" << endl;
        return false;
    }

    setXDrawable( drawable );
    setGLXContext( context );
    return true;
#else
    // TODO
    return false;
#endif
}

void eq::Window::exit()
{
#ifdef GLX
    Display *display = getXDisplay();
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
#else
    // TODO
#endif
}
