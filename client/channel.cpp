
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "channel.h"

#include "commands.h"
#include "global.h"
#include "nodeFactory.h"
#include "packets.h"
#include "renderContext.h"

using namespace eq;
using namespace std;

Channel::Channel()
        : eqNet::Base( CMD_CHANNEL_ALL ),
          _window(NULL),
          _near(.1),
          _far(100.),
          _context( NULL )
{
    registerCommand( CMD_CHANNEL_INIT, this, reinterpret_cast<CommandFcn>(
                         &eq::Channel::_pushRequest ));
    registerCommand( REQ_CHANNEL_INIT, this, reinterpret_cast<CommandFcn>(
                         &eq::Channel::_reqInit ));
    registerCommand( CMD_CHANNEL_EXIT, this, reinterpret_cast<CommandFcn>( 
                         &eq::Channel::_pushRequest ));
    registerCommand( REQ_CHANNEL_EXIT, this, reinterpret_cast<CommandFcn>( 
                         &eq::Channel::_reqExit ));
    registerCommand( CMD_CHANNEL_CLEAR, this, reinterpret_cast<CommandFcn>( 
                         &eq::Channel::_pushRequest ));
    registerCommand( REQ_CHANNEL_CLEAR, this, reinterpret_cast<CommandFcn>( 
                         &eq::Channel::_reqClear ));
    registerCommand( CMD_CHANNEL_DRAW, this, reinterpret_cast<CommandFcn>( 
                         &eq::Channel::_pushRequest ));
    registerCommand( REQ_CHANNEL_DRAW, this, reinterpret_cast<CommandFcn>( 
                         &eq::Channel::_reqDraw ));
}

Channel::~Channel()
{
}

//---------------------------------------------------------------------------
// operations
//---------------------------------------------------------------------------
void Channel::clear()
{
    applyBuffer();
    applyViewport();
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void Channel::draw()
{
    applyBuffer();
    applyViewport();
    
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    applyFrustum();

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    applyHeadTransform();

    glTranslatef( 0, 0, -2 );
    glColor3f( 1, 1, 0 );
    glBegin( GL_QUADS );
    glVertex3f( -.25, -.25, -.25 );
    glVertex3f( -.25,  .25, -.25 );
    glVertex3f(  .25, -.25, -.25 );
    glVertex3f(  .25, - 25, -.25 );
    glEnd();
    glFinish();
}

void Channel::applyBuffer()
{
    if( !_context || !(_context->hints & HINT_BUFFER ))
        return;

    glDrawBuffer( _context->drawBuffer );
}

void Channel::applyViewport()
{
    if( !_context || !(_context->hints & HINT_BUFFER ))
        return;

    const PixelViewport& pvp = _context->pvp;
    glViewport( pvp.x, pvp.y, pvp.w, pvp.h );
}

void Channel::applyFrustum()
{
    if( !_context || !(_context->hints & HINT_FRUSTUM ))
        return;

    const float* frustum = _context->frustum;
    glFrustum( frustum[0], frustum[1], frustum[2], frustum[3], frustum[4],
               frustum[5] ); 
    INFO << "Applied frustum: " << LOG_VECTOR6( frustum ) << endl;
}

void Channel::applyHeadTransform()
{
    if( !_context || !(_context->hints & HINT_FRUSTUM ))
        return;
    
    glMultMatrixf( _context->headTransform );
    INFO << "Applied head transform: " 
         << LOG_MATRIX4x4( _context->headTransform ) << endl;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
void Channel::_pushRequest( eqNet::Node* node, const eqNet::Packet* packet )
{
    Pipe* pipe = getPipe();

    if( pipe )
        pipe->pushRequest( node, packet );
    else
        _cmdUnknown( node, packet );
}

void Channel::_reqInit( eqNet::Node* node, const eqNet::Packet* pkg )
{
    ChannelInitPacket* packet = (ChannelInitPacket*)pkg;
    INFO << "handle channel init " << packet << endl;

    ChannelInitReplyPacket reply( packet );
    reply.result = init(); // XXX push to channel thread
    reply.near   = _near;
    reply.far    = _far;
    node->send( reply );
}

void Channel::_reqExit( eqNet::Node* node, const eqNet::Packet* pkg )
{
    ChannelExitPacket* packet = (ChannelExitPacket*)pkg;
    INFO << "handle channel exit " << packet << endl;

    exit();

    ChannelExitReplyPacket reply( packet );
    node->send( reply );
}

void Channel::_reqClear( eqNet::Node* node, const eqNet::Packet* pkg )
{
    ChannelClearPacket* packet = (ChannelClearPacket*)pkg;
    INFO << "handle channel clear " << packet << endl;

    _context = &packet->context;
    clear();
    _context = NULL;
}

void Channel::_reqDraw( eqNet::Node* node, const eqNet::Packet* pkg )
{
    ChannelClearPacket* packet = (ChannelClearPacket*)pkg;
    INFO << "handle channel clear " << packet << endl;

    _context = &packet->context;
    draw();
    _context = NULL;
}
