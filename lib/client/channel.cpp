
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
    glBegin( GL_TRIANGLE_STRIP );
    glVertex3f( -.25, -.25, -.25 );
    glVertex3f( -.25,  .25, -.25 );
    glVertex3f(  .25, -.25, -.25 );
    glVertex3f(  .25,  .25, -.25 );
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
    EQVERB << "Applied frustum: " << LOG_VECTOR6( frustum ) << endl;
}

void Channel::applyHeadTransform()
{
    if( !_context || !(_context->hints & HINT_FRUSTUM ))
        return;
    
    glMultMatrixf( _context->headTransform );
    EQVERB << "Applied head transform: " 
         << LOG_MATRIX4x4( _context->headTransform ) << endl;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
eqNet::CommandResult Channel::_pushRequest( eqNet::Node* node,
                                            const eqNet::Packet* packet )
{
    Pipe* pipe = getPipe();

    if( pipe )
        return pipe->pushRequest( node, packet );

    return _cmdUnknown( node, packet );
}

eqNet::CommandResult Channel::_reqInit( eqNet::Node* node,
                                        const eqNet::Packet* pkg )
{
    ChannelInitPacket* packet = (ChannelInitPacket*)pkg;
    EQINFO << "handle channel init " << packet << endl;

    ChannelInitReplyPacket reply( packet );
    reply.result = init(); // XXX push to channel thread
    reply._near   = _near;
    reply._far    = _far;
    node->send( reply );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Channel::_reqExit( eqNet::Node* node,
                                        const eqNet::Packet* pkg )
{
    ChannelExitPacket* packet = (ChannelExitPacket*)pkg;
    EQINFO << "handle channel exit " << packet << endl;

    exit();

    ChannelExitReplyPacket reply( packet );
    node->send( reply );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Channel::_reqClear( eqNet::Node* node,
                                         const eqNet::Packet* pkg )
{
    ChannelClearPacket* packet = (ChannelClearPacket*)pkg;
    EQVERB << "handle channel clear " << packet << endl;

    _context = &packet->context;
    clear();
    _context = NULL;
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Channel::_reqDraw( eqNet::Node* node,
                                        const eqNet::Packet* pkg )
{
    ChannelClearPacket* packet = (ChannelClearPacket*)pkg;
    EQVERB << "handle channel draw " << packet << endl;

    _context = &packet->context;
    draw();
    _context = NULL;
    return eqNet::COMMAND_HANDLED;
}
