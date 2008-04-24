
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "channel.h"

#include "compositor.h"
#include "commands.h"
#include "frame.h"
#include "global.h"
#include "log.h"
#include "nodeFactory.h"
#include "packets.h"
#include "range.h"
#include "renderContext.h"

#include <eq/net/command.h>

using namespace eqBase;
using namespace std;
using eqNet::CommandFunc;

namespace eq
{

#define MAKE_ATTR_STRING( attr ) ( string("EQ_CHANNEL_") + #attr )
std::string eq::Channel::_iAttributeStrings[IATTR_ALL] = {
    MAKE_ATTR_STRING( IATTR_HINT_STATISTICS ),
};

Channel::Channel( Window* parent )
        : _window( parent ),
          _context( NULL ),
          _frustum( vmml::Frustumf::DEFAULT )
{
    eqNet::CommandQueue* queue = parent->getPipeThreadQueue();

    registerCommand( CMD_CHANNEL_CONFIG_INIT, 
                     CommandFunc<Channel>( this, &Channel::_cmdConfigInit ),
                     queue );
    registerCommand( CMD_CHANNEL_CONFIG_EXIT, 
                     CommandFunc<Channel>( this, &Channel::_cmdConfigExit ),
                     queue );
    registerCommand( CMD_CHANNEL_FRAME_START,
                     CommandFunc<Channel>( this, &Channel::_cmdFrameStart ),
                     queue );
    registerCommand( CMD_CHANNEL_FRAME_FINISH,
                     CommandFunc<Channel>( this, &Channel::_cmdFrameFinish ),
                     queue );
    registerCommand( CMD_CHANNEL_FRAME_CLEAR, 
                     CommandFunc<Channel>( this, &Channel::_cmdFrameClear ),
                     queue );
    registerCommand( CMD_CHANNEL_FRAME_DRAW, 
                     CommandFunc<Channel>( this, &Channel::_cmdFrameDraw ),
                     queue );
    registerCommand( CMD_CHANNEL_FRAME_DRAW_FINISH, 
                    CommandFunc<Channel>( this, &Channel::_cmdFrameDrawFinish ),
                     queue );
    registerCommand( CMD_CHANNEL_FRAME_ASSEMBLE, 
                     CommandFunc<Channel>( this, &Channel::_cmdFrameAssemble ),
                     queue );
    registerCommand( CMD_CHANNEL_FRAME_READBACK, 
                     CommandFunc<Channel>( this, &Channel::_cmdFrameReadback ),
                     queue );
    registerCommand( CMD_CHANNEL_FRAME_TRANSMIT, 
                     CommandFunc<Channel>( this, &Channel::_cmdFrameTransmit ),
                     queue );

    parent->_addChannel( this );
    EQINFO << " New eq::Channel @" << (void*)this << endl;
}

Channel::~Channel()
{
    _window->_removeChannel( this );
}

//----------------------------------------------------------------------
// viewport
//----------------------------------------------------------------------
void eq::Channel::_setPixelViewport( const PixelViewport& pvp )
{
    if( _pvp == pvp || !pvp.hasArea( ))
        return;

    _pvp = pvp;
    _vp.invalidate();

    if( !_window )
        return;
    
    const PixelViewport& windowPVP = _window->getPixelViewport();
    if( windowPVP.isValid( ))
        _vp = pvp.getSubVP( windowPVP );

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
        _pvp = windowPVP.getSubPVP( vp );
    }

    EQVERB << "Channel vp set: " << _pvp << ":" << _vp << endl;
}

void eq::Channel::setNearFar( const float nearPlane, const float farPlane )
{
    if( _frustum.nearPlane == nearPlane && _frustum.farPlane == farPlane )
        return;

    _frustum.adjustNear( nearPlane );
    _frustum.farPlane = farPlane;

    if( _context )
    {
        _context->frustum.adjustNear( nearPlane );
        _context->frustum.farPlane = farPlane;
    }

    ChannelSetNearFarPacket packet;
    packet.nearPlane = nearPlane;
    packet.farPlane  = farPlane;
    
    RefPtr< eqNet::Node > node = 
        RefPtr_static_cast< Server, eqNet::Node >( getServer( ));
    send( node, packet );
}

void Channel::addStatEvent( StatEvent& data )
{
    _statEvents.push_back( data );
}

//---------------------------------------------------------------------------
// operations
//---------------------------------------------------------------------------
void Channel::frameClear( const uint32_t frameID )
{
    EQ_GL_CALL( applyBuffer( ));
    EQ_GL_CALL( applyViewport( ));

#ifndef NDEBUG
    if( getenv( "EQ_TAINT_CHANNELS" ))
    {
        const vmml::Vector3ub color = getUniqueColor();
        glClearColor( color.r/255.0f, color.g/255.0f, color.b/255.0f, 1.0f );
    }
#endif // DEBUG

    EQ_GL_CALL( glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ));
}

void Channel::frameDraw( const uint32_t frameID )
{
    EQ_GL_CALL( applyBuffer( ));
    EQ_GL_CALL( applyViewport( ));
    
    EQ_GL_CALL( glMatrixMode( GL_PROJECTION ));
    EQ_GL_CALL( glLoadIdentity( ));
    EQ_GL_CALL( applyFrustum( ));

    EQ_GL_CALL( glMatrixMode( GL_MODELVIEW ));
    EQ_GL_CALL( glLoadIdentity( ));
    EQ_GL_CALL( applyHeadTransform( ));
}

void Channel::frameAssemble( const uint32_t frameID )
{
    EQ_GL_CALL( applyBuffer( ));
    EQ_GL_CALL( applyViewport( ));
    EQ_GL_CALL( setupAssemblyState( ));

    Compositor::assembleFrames( getInputFrames(), this );

    EQ_GL_CALL( resetAssemblyState( ));
}

void Channel::frameReadback( const uint32_t frameID )
{
    EQ_GL_CALL( applyBuffer( ));
    EQ_GL_CALL( applyViewport( ));
    EQ_GL_CALL( setupAssemblyState( ));

    Window::ObjectManager* glObjects = getWindow()->getObjectManager();

    const FrameVector& frames = getOutputFrames();
    for( FrameVector::const_iterator i = frames.begin(); i != frames.end(); ++i)
        (*i)->startReadback( glObjects );

    EQ_GL_CALL( resetAssemblyState( ));
}

void Channel::setupAssemblyState()
{
    EQ_GL_ERROR( "before setupAssemblyState" );
    glPushAttrib( GL_ENABLE_BIT | GL_STENCIL_BUFFER_BIT | GL_VIEWPORT_BIT | 
                  GL_SCISSOR_BIT | GL_LINE_BIT | GL_PIXEL_MODE_BIT );

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

    EQASSERT( _window );    
    const PixelViewport& pvp = _window->getPixelViewport();
    EQASSERT( pvp.isValid( ));

    glViewport( 0, 0, pvp.w, pvp.h );
    glScissor( 0, 0, pvp.w, pvp.h );

    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    glOrtho( 0.0f, pvp.w, 0.0f, pvp.h, -1.0f, 1.0f );

    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();
    EQ_GL_ERROR( "after  setupAssemblyState" );
}

void Channel::resetAssemblyState()
{
    EQ_GL_ERROR( "before resetAssemblyState" );
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();

    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();

    glPopAttrib();
    EQ_GL_ERROR( "after  resetAssemblyState" );
}

void Channel::_setRenderContext( RenderContext& context )
{
    _context = &context;
    _window->addRenderContext( context );
}

const Viewport& Channel::getViewport() const
{
    return _context ? _context->vp : _vp;
}

const PixelViewport& Channel::getPixelViewport() const
{
    return _context ? _context->pvp : _pvp;
}

const vmml::Vector2i& Channel::getPixelOffset() const
{
    return _context ? _context->offset : vmml::Vector2i::ZERO;
}

uint32_t Channel::getDrawBuffer() const
{
    return _context ? _context->buffer : GL_BACK;
}

uint32_t Channel::getReadBuffer() const
{
    return _context ? _context->buffer : GL_BACK;
}

const ColorMask& Channel::getDrawBufferMask() const
{
    return _context ? _context->drawBufferMask : ColorMask::ALL;
}

const vmml::Frustumf& Channel::getFrustum() const
{
    return _context ? _context->frustum : _frustum;
}

const Range& Channel::getRange() const
{
    return _context ? _context->range : Range::ALL;
}

const Pixel& Channel::getPixel() const
{
    return _context ? _context->pixel : Pixel::ALL;
}

Eye Channel::getEye() const
{
    return _context ? _context->eye : EYE_CYCLOP;
}

const vmml::Matrix4f& Channel::getHeadTransform() const
{
    return _context ? _context->headTransform : vmml::Matrix4f::IDENTITY;
}

void Channel::applyBuffer() const
{
    EQ_GL_CALL( glReadBuffer( getReadBuffer( )));
    EQ_GL_CALL( glDrawBuffer( getDrawBuffer( )));

    applyColorMask();
}

void Channel::applyColorMask() const
{
	const ColorMask& colorMask = getDrawBufferMask();
	glColorMask( colorMask.red, colorMask.green, colorMask.blue, true );
}

void Channel::applyViewport() const
{
	const PixelViewport& pvp = getPixelViewport();
	// TODO: OPT return if vp unchanged

	if( !pvp.hasArea( ))
	{ 
		EQERROR << "Can't apply viewport " << pvp << endl;
		return;
	}

	EQ_GL_CALL( glViewport( pvp.x, pvp.y, pvp.w, pvp.h ));
    EQ_GL_CALL( glScissor( pvp.x, pvp.y, pvp.w, pvp.h ));
}

void Channel::applyFrustum() const
{
	const vmml::Frustumf& frustum = getFrustum();
	EQ_GL_CALL( glFrustum( frustum.left, frustum.right,             \
                           frustum.bottom, frustum.top,             \
                           frustum.nearPlane, frustum.farPlane )); 
	EQVERB << "Apply " << frustum << endl;
}

void Channel::applyHeadTransform() const
{
    const vmml::Matrix4f& xfm = getHeadTransform();
    EQ_GL_CALL( glMultMatrixf( xfm.ml ));
    EQVERB << "Apply head transform: " << xfm << endl;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
eqNet::CommandResult Channel::_cmdConfigInit( eqNet::Command& command )
{
    const ChannelConfigInitPacket* packet = 
        command.getPacket<ChannelConfigInitPacket>();
    EQLOG( LOG_TASKS ) << "TASK channel config init " << packet << endl;

    if( packet->pvp.isValid( ))
        _setPixelViewport( packet->pvp );
    else
        _setViewport( packet->vp );

    _name  = packet->name;
    _color = packet->color;
    for( uint32_t i=0; i<IATTR_ALL; ++i )
        _iAttributes[i] = packet->iattr[i];

    _error.clear();
    ChannelConfigInitReplyPacket reply;
    reply.result = configInit( packet->initID );

    reply.nearPlane   = _frustum.nearPlane;
    reply.farPlane    = _frustum.farPlane;

    EQLOG( LOG_TASKS ) << "TASK channel config init reply " << &reply << endl;
    send( command.getNode(), reply, _error );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Channel::_cmdConfigExit( eqNet::Command& command )
{
    const ChannelConfigExitPacket* packet =
        command.getPacket<ChannelConfigExitPacket>();
    EQLOG( LOG_TASKS ) << "TASK configExit " << getName() <<  " " << packet 
                       << endl;

    ChannelConfigExitReplyPacket reply;
    reply.result = configExit();

    send( command.getNode(), reply );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Channel::_cmdFrameStart( eqNet::Command& command )
{
    const ChannelFrameStartPacket* packet = 
        command.getPacket<ChannelFrameStartPacket>();
    EQVERB << "handle channel frame start " << packet << endl;

    //_grabFrame( packet->frameNumber ); single-threaded
    frameStart( packet->frameID, packet->frameNumber );

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Channel::_cmdFrameFinish( eqNet::Command& command )
{
    const ChannelFrameFinishPacket* packet =
        command.getPacket<ChannelFrameFinishPacket>();
    EQVERB << "handle channel frame sync " << packet << endl;

    frameFinish( packet->frameID, packet->frameNumber );

    ChannelFrameFinishReplyPacket reply( packet );
    reply.nStatEvents = _statEvents.size();

    command.getNode()->send( reply, _statEvents );

    _statEvents.clear();
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Channel::_cmdFrameClear( eqNet::Command& command )
{
    ChannelFrameClearPacket* packet = 
        command.getPacket<ChannelFrameClearPacket>();
    EQLOG( LOG_TASKS ) << "TASK clear " << getName() <<  " " << packet << endl;

    ScopedStatistics event( StatEvent::CHANNEL_CLEAR, this );

    _setRenderContext( packet->context );
    frameClear( packet->context.frameID );
    _context = NULL;

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Channel::_cmdFrameDraw( eqNet::Command& command )
{
    ChannelFrameDrawPacket* packet = 
        command.getPacket<ChannelFrameDrawPacket>();
    EQLOG( LOG_TASKS ) << "TASK draw " << getName() <<  " " << packet << endl;

    ScopedStatistics event( StatEvent::CHANNEL_DRAW, this );

    _setRenderContext( packet->context );
    frameDraw( packet->context.frameID );
    _context = NULL;

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Channel::_cmdFrameDrawFinish( eqNet::Command& command )
{
    ChannelFrameDrawFinishPacket* packet = 
        command.getPacket< ChannelFrameDrawFinishPacket >();
    EQLOG( LOG_TASKS ) << "TASK draw finish " << getName() <<  " " << packet
                       << endl;

    ScopedStatistics event( StatEvent::CHANNEL_DRAW_FINISH, this );

    frameDrawFinish( packet->frameID, packet->frameNumber );

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Channel::_cmdFrameAssemble( eqNet::Command& command )
{
    ChannelFrameAssemblePacket* packet = 
        command.getPacket<ChannelFrameAssemblePacket>();
    EQLOG( LOG_TASKS | LOG_ASSEMBLY ) << "TASK assemble " << getName() <<  " " 
                                       << packet << endl;

    ScopedStatistics event( StatEvent::CHANNEL_ASSEMBLE, this );
    _setRenderContext( packet->context );

    for( uint32_t i=0; i<packet->nFrames; ++i )
    {
        Pipe*  pipe  = getPipe();
        Frame* frame = pipe->getFrame( packet->frames[i], _context->eye );
        _inputFrames.push_back( frame );
    }

    frameAssemble( packet->context.frameID );

    _inputFrames.clear();
    _context = NULL;

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Channel::_cmdFrameReadback( eqNet::Command& command )
{
    ChannelFrameReadbackPacket* packet = 
        command.getPacket<ChannelFrameReadbackPacket>();
    EQLOG( LOG_TASKS | LOG_ASSEMBLY ) << "TASK readback " << getName() <<  " " 
                                       << packet << endl;

    ScopedStatistics event( StatEvent::CHANNEL_READBACK, this );
    _setRenderContext( packet->context );

    for( uint32_t i=0; i<packet->nFrames; ++i )
    {
        Pipe*  pipe  = getPipe();
        Frame* frame = pipe->getFrame( packet->frames[i], _context->eye );
        _outputFrames.push_back( frame );
    }

    frameReadback( packet->context.frameID );

    for( FrameVector::const_iterator i = _outputFrames.begin();
         i != _outputFrames.end(); ++i )
    {
        Frame* frame = *i;
        frame->syncReadback();
    }

    _outputFrames.clear();
    _context = NULL;
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Channel::_cmdFrameTransmit( eqNet::Command& command )
{
    const ChannelFrameTransmitPacket* packet = 
        command.getPacket<ChannelFrameTransmitPacket>();
    EQLOG( LOG_TASKS | LOG_ASSEMBLY ) << "TASK transmit " << getName() <<  " " 
                                      << packet << endl;

    ScopedStatistics event( StatEvent::CHANNEL_TRANSMIT, this );

    eqNet::Session*     session   = getSession();
    RefPtr<eqNet::Node> localNode = session->getLocalNode();
    RefPtr<eqNet::Node> server    = session->getServer();
    Pipe*               pipe      = getPipe();
    Frame*              frame     = pipe->getFrame( packet->frame, 
                                                    packet->context.eye );

    for( uint32_t i=0; i<packet->nNodes; ++i )
    {
        eqNet::NodeID nodeID = packet->nodes[i];
        nodeID.convertToHost();

        RefPtr<eqNet::Node> node   = command.getNode();
        RefPtr<eqNet::Node> toNode = localNode->connect( nodeID, node );
        EQLOG( LOG_ASSEMBLY ) << "channel \"" << getName() << "\" transmit " 
                              << frame << " to " << nodeID << endl;

        ScopedStatistics nodeEvent( StatEvent::CHANNEL_TRANSMIT_NODE, this );
        frame->transmit( toNode );
    }

    return eqNet::COMMAND_HANDLED;
}
}
