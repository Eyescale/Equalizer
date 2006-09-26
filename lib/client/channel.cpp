
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "channel.h"

#include "commands.h"
#include "object.h"
#include "frame.h"
#include "global.h"
#include "nodeFactory.h"
#include "packets.h"
#include "range.h"
#include "renderContext.h"

using namespace eq;
using namespace std;

// Move me
namespace eq
{
    static float identityMatrix[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
    static Range fullRange;
}

Channel::Channel()
        : eqNet::Object( eq::Object::TYPE_CHANNEL, CMD_CHANNEL_CUSTOM ),
          _window(NULL),
          _near(.1),
          _far(100.),
          _context( NULL )
{
    registerCommand( CMD_CHANNEL_INIT, this, reinterpret_cast<CommandFcn>(
                         &eq::Channel::_pushCommand ));
    registerCommand( REQ_CHANNEL_INIT, this, reinterpret_cast<CommandFcn>(
                         &eq::Channel::_reqInit ));
    registerCommand( CMD_CHANNEL_EXIT, this, reinterpret_cast<CommandFcn>( 
                         &eq::Channel::_pushCommand ));
    registerCommand( REQ_CHANNEL_EXIT, this, reinterpret_cast<CommandFcn>( 
                         &eq::Channel::_reqExit ));
    registerCommand( CMD_CHANNEL_CLEAR, this, reinterpret_cast<CommandFcn>( 
                         &eq::Channel::_pushCommand ));
    registerCommand( REQ_CHANNEL_CLEAR, this, reinterpret_cast<CommandFcn>( 
                         &eq::Channel::_reqClear ));
    registerCommand( CMD_CHANNEL_DRAW, this, reinterpret_cast<CommandFcn>( 
                         &eq::Channel::_pushCommand ));
    registerCommand( REQ_CHANNEL_DRAW, this, reinterpret_cast<CommandFcn>( 
                         &eq::Channel::_reqDraw ));
    registerCommand( CMD_CHANNEL_READBACK, this, reinterpret_cast<CommandFcn>( 
                         &eq::Channel::_pushCommand ));
    registerCommand( REQ_CHANNEL_READBACK, this, reinterpret_cast<CommandFcn>( 
                         &eq::Channel::_reqReadback ));
}

Channel::~Channel()
{
}

//----------------------------------------------------------------------
// viewport
//----------------------------------------------------------------------
void eq::Channel::_setPixelViewport( const PixelViewport& pvp )
{
    if( !pvp.isValid( ))
        return;

    _pvp = pvp;
    _vp.invalidate();

    if( !_window )
        return;
    
    const PixelViewport& windowPVP = _window->getPixelViewport();
    if( windowPVP.isValid( ))
        _vp = pvp / windowPVP;

    EQVERB << "Channel pvp set: " << _pvp << ":" << _vp << endl;
}

void eq::Channel::_setViewport( const Viewport& vp )
{
    if( !vp.isValid( ))
        return;
    
    _vp = vp;
    _pvp.invalidate();

    if( !_window )
        return;

    PixelViewport windowPVP = _window->getPixelViewport();
    if( windowPVP.isValid( ))
    {
        windowPVP.x = 0;
        windowPVP.y = 0;
        _pvp = windowPVP * vp;
    }

    EQVERB << "Channel vp set: " << _pvp << ":" << _vp << endl;
}

//---------------------------------------------------------------------------
// operations
//---------------------------------------------------------------------------
void Channel::clear( const uint32_t frameID )
{
    applyBuffer();
    applyViewport();
    if( getenv( "EQ_TAINT_CHANNELS" ) != NULL )
    {
        Sgi::hash<const char*> hasher;
        unsigned  seed  = (unsigned)(long long)this + hasher(getName().c_str());
        const int color = rand_r( &seed );

        glClearColor( (color&0xff) / 255., ((color>>8) & 0xff) / 255.,
                      ((color>>16) & 0xff) / 255., 1. );
    }

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

void Channel::readback( const uint32_t frameID )
{
    applyBuffer();
    applyViewport();

    const vector<Frame*>& frames = getOutputFrames();
    for( vector<Frame*>::const_iterator iter = frames.begin();
         iter != frames.end(); ++iter )

        (*iter)->startReadback();
}

void Channel::applyBuffer()
{
    if( !_context )
        return;

    glDrawBuffer( _context->drawBuffer );
}

const PixelViewport& Channel::getPixelViewport() const
{
    return _context ? _context->pvp : _pvp;
}

void Channel::applyViewport()
{
    const PixelViewport& pvp = getPixelViewport();
    // TODO: OPT return if vp unchanged

    if( !pvp.isValid( ))
        return;

    glViewport( pvp.x, pvp.y, pvp.w, pvp.h );
    glScissor( pvp.x, pvp.y, pvp.w, pvp.h );
}

const Frustum& Channel::getFrustum() const
{
    return _context ? _context->frustum : _frustum;
}

const Range& Channel::getRange() const
{
    return _context ? _context->range : fullRange;
}

void Channel::applyFrustum() const
{
    const Frustum& frustum = getFrustum();
    glFrustum( frustum.left, frustum.right, frustum.top, frustum.bottom,
               frustum.near, frustum.far ); 
    EQVERB << "Apply " << frustum << endl;
}

const float* Channel::getHeadTransform() const
{
    return _context ? _context->headTransform : identityMatrix;
}

void Channel::applyHeadTransform() const
{
    const float* xfm = getHeadTransform();
    glMultMatrixf( xfm );
    EQVERB << "Apply head transform: " << LOG_MATRIX4x4( xfm ) << endl;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
eqNet::CommandResult Channel::_pushCommand( eqNet::Node* node,
                                            const eqNet::Packet* packet )
{
    Pipe* pipe = getPipe();

    return ( pipe ? pipe->pushCommand( node, packet ) :
             _cmdUnknown( node, packet ));
}

eqNet::CommandResult Channel::_reqInit( eqNet::Node* node,
                                        const eqNet::Packet* pkg )
{
    ChannelInitPacket* packet = (ChannelInitPacket*)pkg;
    EQINFO << "handle channel init " << packet << endl;

    if( packet->pvp.isValid( ))
        _setPixelViewport( packet->pvp );
    else
        _setViewport( packet->vp );
    _name = packet->name;

    ChannelInitReplyPacket reply( packet );
    reply.result = init( packet->initID );
    reply.near   = _near;
    reply.far    = _far;
    send( node, reply );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Channel::_reqExit( eqNet::Node* node,
                                        const eqNet::Packet* pkg )
{
    ChannelExitPacket* packet = (ChannelExitPacket*)pkg;
    EQINFO << "handle channel exit " << packet << endl;

    exit();

    ChannelExitReplyPacket reply( packet );
    send( node, reply );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Channel::_reqClear( eqNet::Node* node,
                                         const eqNet::Packet* pkg )
{
    ChannelClearPacket* packet = (ChannelClearPacket*)pkg;
    EQVERB << "handle channel clear " << packet << endl;

    _context = &packet->context;
    clear( packet->context.frameID );
    _context = NULL;
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Channel::_reqDraw( eqNet::Node* node,
                                        const eqNet::Packet* pkg )
{
    ChannelDrawPacket* packet = (ChannelDrawPacket*)pkg;
    EQVERB << "handle channel draw " << packet << endl;

    _context = &packet->context;
    draw( packet->context.frameID );
    _context = NULL;
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Channel::_reqReadback( eqNet::Node* node,
                                            const eqNet::Packet* pkg )
{
    ChannelReadbackPacket* packet = (ChannelReadbackPacket*)pkg;
    EQVERB << "handle channel readback " << packet << endl;

    _context = &packet->context;

    eqNet::Session* session = getSession();
    for( uint32_t i=0; i<packet->nFrames; ++i )
    {
        eqNet::Object* object = session->getObject( packet->frames[i].objectID, 
                                                    Object::SHARE_THREAD,
                                                    packet->frames[i].version );
        EQASSERT( dynamic_cast<Frame*>( object ));
        Frame* frame = static_cast<Frame*>( object );
        frame->clear();
        _outputFrames.push_back( frame );
    }

    readback( packet->context.frameID );

    _outputFrames.clear();
    _context = NULL;
    return eqNet::COMMAND_HANDLED;
}
