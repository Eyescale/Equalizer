
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "channel.h"

#include "commands.h"
#include "object.h"
#include "global.h"
#include "nodeFactory.h"
#include "packets.h"
#include "renderContext.h"

using namespace eq;
using namespace std;

// Move me
namespace eq
{
    float identityMatrix[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
}

Channel::Channel()
        : eqNet::Object( eq::Object::TYPE_CHANNEL, CMD_CHANNEL_ALL ),
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
void Channel::clear( const uint32_t frameID )
{
    applyBuffer();
    applyViewport();
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void Channel::draw( const uint32_t frameID )
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

const Viewport& Channel::getViewport() const
{
    if( _context && (_context->hints & HINT_BUFFER) )
        return _context->vp;

    return _vp;
}

void Channel::applyViewport()
{
    if( !_context || !( _context->hints & HINT_BUFFER ))
        return;

    const PixelViewport& pvp = _context->pvp;
    glViewport( pvp.x, pvp.y, pvp.w, pvp.h );
}

const Frustum& Channel::getFrustum() const
{
    if( _context && ( _context->hints & HINT_FRUSTUM ))
        return _context->frustum;
    return _frustum;
}

void Channel::applyFrustum() const
{
    const Frustum& frustum = getFrustum();
    glFrustum( frustum.left, frustum.right, frustum.top, frustum.bottom,
               frustum.near, frustum.far ); 
    EQVERB << "Applied frustum: " << frustum << endl;
}

const float* Channel::getHeadTransform() const
{
    if( _context && (_context->hints & HINT_FRUSTUM ))
        return _context->headTransform;
    return identityMatrix;
}

void Channel::applyHeadTransform() const
{
    const float* xfm = getHeadTransform();
    glMultMatrixf( xfm );
    EQVERB << "Applied head transform: " << LOG_MATRIX4x4( xfm ) << endl;
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

    _vp = packet->vp;

    ChannelInitReplyPacket reply( packet );
    reply.result = init( packet->initID );
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
    clear( packet->frameID );
    _context = NULL;
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Channel::_reqDraw( eqNet::Node* node,
                                        const eqNet::Packet* pkg )
{
    ChannelClearPacket* packet = (ChannelClearPacket*)pkg;
    EQVERB << "handle channel draw " << packet << endl;

    _context = &packet->context;
    draw( packet->frameID );
    _context = NULL;
    return eqNet::COMMAND_HANDLED;
}
