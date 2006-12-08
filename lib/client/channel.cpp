
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "channel.h"

#include "commands.h"
#include "object.h"
#include "frame.h"
#include "global.h"
#include "log.h"
#include "nodeFactory.h"
#include "packets.h"
#include "range.h"
#include "renderContext.h"

#include <eq/net/command.h>

using namespace eq;
using namespace eqBase;
using namespace std;

#define MAKE_ATTR_STRING( attr ) ( string("EQ_CHANNEL_") + #attr )
std::string eq::Channel::_iAttributeStrings[IATTR_ALL] = {
    MAKE_ATTR_STRING( IATTR_HINT_STATISTICS ),
};

Channel::Channel()
        : eqNet::Object( eq::Object::TYPE_CHANNEL ),
          _window(NULL),
          _context( NULL ),
          _frustum( vmml::Frustumf::DEFAULT )
{
    registerCommand( CMD_CHANNEL_INIT,
                   eqNet::CommandFunc<Channel>( this, &Channel::_pushCommand ));
    registerCommand( REQ_CHANNEL_INIT, 
                     eqNet::CommandFunc<Channel>( this, &Channel::_reqInit ));
    registerCommand( CMD_CHANNEL_EXIT, 
                   eqNet::CommandFunc<Channel>( this, &Channel::_pushCommand ));
    registerCommand( REQ_CHANNEL_EXIT, 
                     eqNet::CommandFunc<Channel>( this, &Channel::_reqExit ));
    registerCommand( CMD_CHANNEL_CLEAR, 
                   eqNet::CommandFunc<Channel>( this, &Channel::_pushCommand ));
    registerCommand( REQ_CHANNEL_CLEAR, 
                     eqNet::CommandFunc<Channel>( this, &Channel::_reqClear));
    registerCommand( CMD_CHANNEL_DRAW, 
                   eqNet::CommandFunc<Channel>( this, &Channel::_pushCommand ));
    registerCommand( REQ_CHANNEL_DRAW, 
                     eqNet::CommandFunc<Channel>( this, &Channel::_reqDraw ));
    registerCommand( CMD_CHANNEL_ASSEMBLE, 
                   eqNet::CommandFunc<Channel>( this, &Channel::_pushCommand ));
    registerCommand( REQ_CHANNEL_ASSEMBLE, 
                   eqNet::CommandFunc<Channel>( this, &Channel::_reqAssemble ));
    registerCommand( CMD_CHANNEL_READBACK, 
                   eqNet::CommandFunc<Channel>( this, &Channel::_pushCommand ));
    registerCommand( REQ_CHANNEL_READBACK, 
                   eqNet::CommandFunc<Channel>( this, &Channel::_reqReadback ));
    registerCommand( CMD_CHANNEL_TRANSMIT, 
                   eqNet::CommandFunc<Channel>( this, &Channel::_pushCommand ));
    registerCommand( REQ_CHANNEL_TRANSMIT, 
                   eqNet::CommandFunc<Channel>( this, &Channel::_reqTransmit ));
}

Channel::~Channel()
{
}

//----------------------------------------------------------------------
// viewport
//----------------------------------------------------------------------
void eq::Channel::_setPixelViewport( const PixelViewport& pvp )
{
    if( !pvp.hasArea( ))
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
    if( !vp.hasArea( ))
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

void eq::Channel::setNearFar( const float near, const float far )
{
    if( _frustum.near == near && _frustum.far == far )
        return;

    _frustum.adjustNear( near );
    _frustum.far = far;

    if( _context )
    {
        _context->frustum.adjustNear( near );
        _context->frustum.far = far;
    }

    ChannelSetNearFarPacket packet;
    packet.near = near;
    packet.far  = far;
    
    send( getServer(), packet );
}

//---------------------------------------------------------------------------
// operations
//---------------------------------------------------------------------------
void Channel::clear( const uint32_t frameID )
{
    applyBuffer();
    applyViewport();
    if( getenv( "EQ_TAINT_CHANNELS" ))
    {
        stde::hash<const char*> hasher;
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

void Channel::assemble( const uint32_t frameID )
{
    applyBuffer();
    applyViewport();
    setupAssemblyState();

    Pipe* pipe = getPipe();
    const vector<Frame*>& frames = getInputFrames();
    for( vector<Frame*>::const_iterator i = frames.begin();
         i != frames.end(); ++i )
    {
        Frame* frame = *i;
        
        StatEvent event( StatEvent::CHANNEL_WAIT_FRAME, this, 
                         pipe->getFrameTime( ));

        frame->waitReady();

        if( getIAttribute( IATTR_HINT_STATISTICS ))
        {
            event.endTime = pipe->getFrameTime();
            pipe->addStatEvent( event );
        }

        frame->startAssemble();
    }
    for( vector<Frame*>::const_iterator i = frames.begin();
         i != frames.end(); ++i )
    {
        Frame* frame = *i;
        
        frame->syncAssemble();
    }
    resetAssemblyState();
}

void Channel::readback( const uint32_t frameID )
{
    applyBuffer();
    applyViewport();
    setupAssemblyState();

    const vector<Frame*>& frames = getOutputFrames();
    for( vector<Frame*>::const_iterator iter = frames.begin();
         iter != frames.end(); ++iter )

        (*iter)->startReadback();

    resetAssemblyState();
}

void Channel::applyBuffer()
{
    if( !_context )
        return;

    glReadBuffer( _context->buffer );
    glDrawBuffer( _context->buffer );
}

void Channel::setupAssemblyState()
{
    glPushAttrib( GL_ENABLE_BIT | GL_STENCIL_BUFFER_BIT );

    glDisable( GL_DEPTH_TEST );
    glDisable( GL_BLEND );
    glDisable( GL_ALPHA_TEST );
    glDisable( GL_STENCIL_TEST );
    glDisable( GL_TEXTURE_1D );
    glDisable( GL_TEXTURE_2D );
    glDisable( GL_TEXTURE_3D );
    glDisable( GL_FOG );
    glDisable( GL_CLIP_PLANE0 );
    glDisable( GL_CLIP_PLANE1 );
    glDisable( GL_CLIP_PLANE2 );
    glDisable( GL_CLIP_PLANE3 );
    glDisable( GL_CLIP_PLANE4 );
    glDisable( GL_CLIP_PLANE5 );

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    const PixelViewport& pvp = getPixelViewport();
    glOrtho( 0.0f, pvp.w, 0.0f, pvp.h, -1.0f, 1.0f );
   
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();
}

void Channel::resetAssemblyState()
{
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();

    glPopAttrib();
}

const PixelViewport& Channel::getPixelViewport() const
{
    return _context ? _context->pvp : _pvp;
}

void Channel::applyViewport()
{
    const PixelViewport& pvp = getPixelViewport();
    // TODO: OPT return if vp unchanged

    if( !pvp.hasArea( ))
    { 
        EQERROR << "Can't apply viewport " << pvp << endl;
        return;
    }

    glViewport( pvp.x, pvp.y, pvp.w, pvp.h );
    glScissor( pvp.x, pvp.y, pvp.w, pvp.h );
}

const vmml::Frustumf& Channel::getFrustum() const
{
    return _context ? _context->frustum : _frustum;
}

const Range& Channel::getRange() const
{
    return _context ? _context->range : Range::FULL;
}

void Channel::applyFrustum() const
{
    const vmml::Frustumf& frustum = getFrustum();
    glFrustum( frustum.left, frustum.right, frustum.top, frustum.bottom,
               frustum.near, frustum.far ); 
    EQVERB << "Apply " << frustum << endl;
}

const vmml::Matrix4f& Channel::getHeadTransform() const
{
    return _context ? _context->headTransform : Matrix4f::IDENTITY;
}

void Channel::applyHeadTransform() const
{
    const Matrix4f& xfm = getHeadTransform();
    glMultMatrixf( xfm.ml );
    EQVERB << "Apply head transform: " << xfm << endl;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
eqNet::CommandResult Channel::_pushCommand( eqNet::Command& command )
{
    Pipe*        pipe = getPipe();
    return ( pipe ? pipe->pushCommand( command ) : _cmdUnknown( command ));
}

eqNet::CommandResult Channel::_reqInit( eqNet::Command& command )
{
    const ChannelInitPacket* packet = command.getPacket<ChannelInitPacket>();
    EQLOG( LOG_TASKS ) << "TASK init " << packet->name <<  " " << packet <<endl;

    if( packet->pvp.isValid( ))
        _setPixelViewport( packet->pvp );
    else
        _setViewport( packet->vp );
    _name = packet->name;

    for( uint32_t i=0; i<IATTR_ALL; ++i )
        _iAttributes[i] = packet->iattr[i];

    ChannelInitReplyPacket reply( packet );
    reply.result = init( packet->initID );
    reply.near   = _frustum.near;
    reply.far    = _frustum.far;
    send( command.getNode(), reply );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Channel::_reqExit( eqNet::Command& command )
{
    const ChannelExitPacket* packet = command.getPacket<ChannelExitPacket>();
    EQLOG( LOG_TASKS ) << "TASK exit " << getName() <<  " " << packet << endl;

    exit();

    ChannelExitReplyPacket reply( packet );
    send( command.getNode(), reply );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Channel::_reqClear( eqNet::Command& command )
{
    ChannelClearPacket* packet = command.getPacket<ChannelClearPacket>();
    EQLOG( LOG_TASKS ) << "TASK clear " << getName() <<  " " << packet << endl;

    Pipe* pipe = getPipe();
    StatEvent event( StatEvent::CHANNEL_CLEAR, this, pipe->getFrameTime( ));

    _context = &packet->context;
    clear( packet->context.frameID );
    _context = NULL;

    if( getIAttribute( IATTR_HINT_STATISTICS ))
    {
        event.endTime = pipe->getFrameTime();
        pipe->addStatEvent( event );
    }
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Channel::_reqDraw( eqNet::Command& command )
{
    ChannelDrawPacket* packet = command.getPacket<ChannelDrawPacket>();
    EQLOG( LOG_TASKS ) << "TASK draw " << getName() <<  " " << packet << endl;

    Pipe* pipe = getPipe();
    StatEvent event( StatEvent::CHANNEL_DRAW, this, pipe->getFrameTime( ));

    _context = &packet->context;
    draw( packet->context.frameID );
    _context = NULL;

    if( getIAttribute( IATTR_HINT_STATISTICS ))
    {
        event.endTime = pipe->getFrameTime();
        pipe->addStatEvent( event );
    }
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Channel::_reqAssemble( eqNet::Command& command )
{
    ChannelAssemblePacket* packet = command.getPacket<ChannelAssemblePacket>();
    EQLOG( LOG_TASKS | LOG_ASSEMBLY ) << "TASK assemble " << getName() <<  " " 
                                       << packet << endl;

    Pipe* pipe = getPipe();
    StatEvent event( StatEvent::CHANNEL_ASSEMBLE, this, pipe->getFrameTime( ));

    _context = &packet->context;

    eqNet::Session* session = getSession();
    for( uint32_t i=0; i<packet->nFrames; ++i )
    {
        eqNet::Object* object = session->getObject( packet->frames[i].objectID, 
                                                    Object::SHARE_THREAD,
                                                    packet->frames[i].version );
        EQASSERT( dynamic_cast<Frame*>( object ));
        Frame* frame = static_cast<Frame*>( object );
        _inputFrames.push_back( frame );
    }

    assemble( packet->context.frameID );

    _inputFrames.clear();
    _context = NULL;

    if( getIAttribute( IATTR_HINT_STATISTICS ))
    {
        event.endTime = pipe->getFrameTime();
        pipe->addStatEvent( event );
    }
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Channel::_reqReadback( eqNet::Command& command )
{
    ChannelReadbackPacket* packet = command.getPacket<ChannelReadbackPacket>();
    EQLOG( LOG_TASKS | LOG_ASSEMBLY ) << "TASK readback " << getName() <<  " " 
                                       << packet << endl;

    Pipe* pipe = getPipe();
    StatEvent event( StatEvent::CHANNEL_READBACK, this, pipe->getFrameTime( ));

    _context = &packet->context;

    eqNet::Session* session = getSession();
    for( uint32_t i=0; i<packet->nFrames; ++i )
    {
        eqNet::Object* object = session->getObject( packet->frames[i].objectID, 
                                                    Object::SHARE_THREAD,
                                                    packet->frames[i].version );
        EQASSERT( dynamic_cast<Frame*>( object ));
        Frame* frame = static_cast<Frame*>( object );
        _outputFrames.push_back( frame );
    }

    readback( packet->context.frameID );

    for( vector<Frame*>::const_iterator i = _outputFrames.begin();
         i != _outputFrames.end(); ++i )
    {
        Frame* frame = *i;
        frame->syncReadback();
    }

    _outputFrames.clear();
    _context = NULL;

    if( getIAttribute( IATTR_HINT_STATISTICS ))
    {
        event.endTime = pipe->getFrameTime();
        pipe->addStatEvent( event );
    }
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Channel::_reqTransmit( eqNet::Command& command )
{
    const ChannelTransmitPacket* packet = 
        command.getPacket<ChannelTransmitPacket>();
    EQLOG( LOG_TASKS | LOG_ASSEMBLY ) << "TASK transmit " << getName() <<  " " 
                                      << packet << endl;

    Pipe* pipe = getPipe();
    StatEvent event( StatEvent::CHANNEL_TRANSMIT, this, pipe->getFrameTime( ));

    eqNet::Session*     session   = getSession();
    RefPtr<eqNet::Node> localNode = session->getLocalNode();
    RefPtr<eqNet::Node> server    = session->getServer();
    eqNet::Object*      object    = session->getObject( packet->frame.objectID,
                                                        Object::SHARE_THREAD,
                                                        packet->frame.version );
    EQASSERT( dynamic_cast<Frame*>( object ));
    Frame* frame = static_cast<Frame*>( object );

    for( uint32_t i=0; i<packet->nNodes; ++i )
    {
        const eqNet::NodeID& nodeID = packet->nodes[i];
        RefPtr<eqNet::Node>  node   = command.getNode();
        RefPtr<eqNet::Node>  toNode = localNode->connect( nodeID, node );
        EQLOG( LOG_ASSEMBLY ) << "channel \"" << getName() << "\" transmit " 
                              << frame << " to " << nodeID << endl;
        frame->transmit( toNode );
    }

    if( getIAttribute( IATTR_HINT_STATISTICS ))
    {
        event.endTime = pipe->getFrameTime();
        pipe->addStatEvent( event );
    }
    return eqNet::COMMAND_HANDLED;
}
