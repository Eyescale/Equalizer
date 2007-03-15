
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "channel.h"

#include "commands.h"
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
        : _window(NULL),
          _context( NULL ),
          _frustum( vmml::Frustumf::DEFAULT )
{
    registerCommand( CMD_CHANNEL_CONFIG_INIT,
                   eqNet::CommandFunc<Channel>( this, &Channel::_pushCommand ));
    registerCommand( REQ_CHANNEL_CONFIG_INIT, 
                 eqNet::CommandFunc<Channel>( this, &Channel::_reqConfigInit ));
    registerCommand( CMD_CHANNEL_CONFIG_EXIT, 
                   eqNet::CommandFunc<Channel>( this, &Channel::_pushCommand ));
    registerCommand( REQ_CHANNEL_CONFIG_EXIT, 
                 eqNet::CommandFunc<Channel>( this, &Channel::_reqConfigExit ));
    registerCommand( CMD_CHANNEL_FRAME_START,
                   eqNet::CommandFunc<Channel>( this, &Channel::_pushCommand ));
    registerCommand( REQ_CHANNEL_FRAME_START,
                 eqNet::CommandFunc<Channel>( this, &Channel::_reqFrameStart ));
    registerCommand( CMD_CHANNEL_FRAME_FINISH,
                   eqNet::CommandFunc<Channel>( this, &Channel::_pushCommand ));
    registerCommand( REQ_CHANNEL_FRAME_FINISH,
                eqNet::CommandFunc<Channel>( this, &Channel::_reqFrameFinish ));
    registerCommand( CMD_CHANNEL_FRAME_CLEAR, 
                   eqNet::CommandFunc<Channel>( this, &Channel::_pushCommand ));
    registerCommand( REQ_CHANNEL_FRAME_CLEAR, 
                     eqNet::CommandFunc<Channel>( this, &Channel::_reqFrameClear));
    registerCommand( CMD_CHANNEL_FRAME_DRAW, 
                   eqNet::CommandFunc<Channel>( this, &Channel::_pushCommand ));
    registerCommand( REQ_CHANNEL_FRAME_DRAW, 
                     eqNet::CommandFunc<Channel>( this, &Channel::_reqFrameDraw ));
    registerCommand( CMD_CHANNEL_FRAME_ASSEMBLE, 
                   eqNet::CommandFunc<Channel>( this, &Channel::_pushCommand ));
    registerCommand( REQ_CHANNEL_FRAME_ASSEMBLE, 
                   eqNet::CommandFunc<Channel>( this, &Channel::_reqFrameAssemble ));
    registerCommand( CMD_CHANNEL_FRAME_READBACK, 
                   eqNet::CommandFunc<Channel>( this, &Channel::_pushCommand ));
    registerCommand( REQ_CHANNEL_FRAME_READBACK, 
                   eqNet::CommandFunc<Channel>( this, &Channel::_reqFrameReadback ));
    registerCommand( CMD_CHANNEL_FRAME_TRANSMIT, 
                   eqNet::CommandFunc<Channel>( this, &Channel::_pushCommand ));
    registerCommand( REQ_CHANNEL_FRAME_TRANSMIT, 
                   eqNet::CommandFunc<Channel>( this, &Channel::_reqFrameTransmit ));
}

Channel::~Channel()
{
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
    
    send( getServer(), packet );
}

//---------------------------------------------------------------------------
// operations
//---------------------------------------------------------------------------
void Channel::frameClear( const uint32_t frameID )
{
    applyBuffer();
    applyViewport();

#ifndef NDEBUG
    if( getenv( "EQ_TAINT_CHANNELS" ))
    {
        uint32_t value = reinterpret_cast< size_t >( this ) & 0xffffffffu;
        uint8_t  red   = 0;
        uint8_t  green = 0;
        uint8_t  blue  = 0;

        for( unsigned i=0; i<8; ++i )
        {
            red   |= ( value&1 << (7-i) ); value >>= 1;
            green |= ( value&1 << (7-i) ); value >>= 1;
            blue  |= ( value&1 << (7-i) ); value >>= 1;
        }

        glClearColor( red / 255.0f, green / 255.0f, blue / 255.0f, 1.0f );
    }
#endif // DEBUG

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void Channel::frameDraw( const uint32_t frameID )
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

void Channel::frameAssemble( const uint32_t frameID )
{
    applyBuffer();
    applyViewport();
    setupAssemblyState();

    Pipe*                 pipe    = getPipe();
    const vector<Frame*>& frames  = getInputFrames();
    Monitor<uint32_t>     monitor;

    for( vector<Frame*>::const_iterator i = frames.begin();
         i != frames.end(); ++i )
    {
        Frame* frame = *i;
        frame->addListener( monitor );
    }

    uint32_t       nUsedFrames  = 0;
    vector<Frame*> unusedFrames = frames;

    while( !unusedFrames.empty( ))
    {
        StatEvent event( StatEvent::CHANNEL_WAIT_FRAME, this, 
                         pipe->getFrameTime( ));

        monitor.waitGE( ++nUsedFrames );
        if( getIAttribute( IATTR_HINT_STATISTICS ))
            pipe->addStatEvent( event );

        for( vector<Frame*>::iterator i = unusedFrames.begin();
             i != unusedFrames.end(); ++i )
        {
            Frame* frame = *i;
            if( !frame->isReady( ))
                continue;

            frame->startAssemble();
            unusedFrames.erase( i );
            break;
        }
    }

    for( vector<Frame*>::const_iterator i = frames.begin(); i != frames.end();
         ++i )
    {
        Frame* frame = *i;
        frame->syncAssemble();
        frame->removeListener( monitor );
    }

    resetAssemblyState();
}

void Channel::frameReadback( const uint32_t frameID )
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
    glReadBuffer( getReadBuffer( ));
    glDrawBuffer( getDrawBuffer( ));
    
    const ColorMask& colorMask = getDrawBufferMask();
    glColorMask( colorMask.red, colorMask.green, colorMask.blue, true );
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

const Viewport& Channel::getViewport() const
{
    return _context ? _context->vp : _vp;
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

const uint32_t Channel::getDrawBuffer() const
{
    return _context ? _context->buffer : GL_BACK;
}

const uint32_t Channel::getReadBuffer() const
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
    return _context ? _context->range : Range::FULL;
}

void Channel::applyFrustum() const
{
    const vmml::Frustumf& frustum = getFrustum();
    glFrustum( frustum.left, frustum.right, frustum.bottom, frustum.top,
               frustum.nearPlane, frustum.farPlane ); 
    EQVERB << "Apply " << frustum << endl;
}

const vmml::Matrix4f& Channel::getHeadTransform() const
{
    return _context ? _context->headTransform : vmml::Matrix4f::IDENTITY;
}

void Channel::applyHeadTransform() const
{
    const vmml::Matrix4f& xfm = getHeadTransform();
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

eqNet::CommandResult Channel::_reqConfigInit( eqNet::Command& command )
{
    const ChannelConfigInitPacket* packet = 
        command.getPacket<ChannelConfigInitPacket>();
    EQLOG( LOG_TASKS ) << "TASK configInit " << packet->name <<  " " << packet
                       << endl;

    if( packet->pvp.isValid( ))
        _setPixelViewport( packet->pvp );
    else
        _setViewport( packet->vp );
    _name = packet->name;

    for( uint32_t i=0; i<IATTR_ALL; ++i )
        _iAttributes[i] = packet->iattr[i];

    _error.clear();
    ChannelConfigInitReplyPacket reply( packet );
    reply.result = configInit( packet->initID );
    reply.nearPlane   = _frustum.nearPlane;
    reply.farPlane    = _frustum.farPlane;
    send( command.getNode(), reply, _error );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Channel::_reqConfigExit( eqNet::Command& command )
{
    const ChannelConfigExitPacket* packet =
        command.getPacket<ChannelConfigExitPacket>();
    EQLOG( LOG_TASKS ) << "TASK configExit " << getName() <<  " " << packet 
                       << endl;

    configExit();

    ChannelConfigExitReplyPacket reply( packet );
    send( command.getNode(), reply );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Channel::_reqFrameStart( eqNet::Command& command )
{
    const ChannelFrameStartPacket* packet = 
        command.getPacket<ChannelFrameStartPacket>();
    EQVERB << "handle channel frame start " << packet << endl;

    //_grabFrame( packet->frameNumber ); single-threaded
    frameStart( packet->frameID, packet->frameNumber );

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Channel::_reqFrameFinish( eqNet::Command& command )
{
    const ChannelFrameFinishPacket* packet =
        command.getPacket<ChannelFrameFinishPacket>();
    EQVERB << "handle channel frame sync " << packet << endl;

    frameFinish( packet->frameID, packet->frameNumber );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Channel::_reqFrameClear( eqNet::Command& command )
{
    ChannelFrameClearPacket* packet = 
        command.getPacket<ChannelFrameClearPacket>();
    EQLOG( LOG_TASKS ) << "TASK clear " << getName() <<  " " << packet << endl;

    Pipe* pipe = getPipe();
    StatEvent event( StatEvent::CHANNEL_CLEAR, this, pipe->getFrameTime( ));

    _context = &packet->context;
    frameClear( packet->context.frameID );
    _context = NULL;

    if( getIAttribute( IATTR_HINT_STATISTICS ))
        pipe->addStatEvent( event );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Channel::_reqFrameDraw( eqNet::Command& command )
{
    ChannelFrameDrawPacket* packet = 
        command.getPacket<ChannelFrameDrawPacket>();
    EQLOG( LOG_TASKS ) << "TASK draw " << getName() <<  " " << packet << endl;

    Pipe* pipe = getPipe();
    StatEvent event( StatEvent::CHANNEL_DRAW, this, pipe->getFrameTime( ));

    _context = &packet->context;

    frameDraw( packet->context.frameID );

    if( getIAttribute( IATTR_HINT_STATISTICS ))
        pipe->addStatEvent( event );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Channel::_reqFrameAssemble( eqNet::Command& command )
{
    ChannelFrameAssemblePacket* packet = 
        command.getPacket<ChannelFrameAssemblePacket>();
    EQLOG( LOG_TASKS | LOG_ASSEMBLY ) << "TASK assemble " << getName() <<  " " 
                                       << packet << endl;

    Pipe* pipe = getPipe();
    StatEvent event( StatEvent::CHANNEL_ASSEMBLE, this, pipe->getFrameTime( ));

    _context = &packet->context;

    for( uint32_t i=0; i<packet->nFrames; ++i )
    {
        Frame* frame = pipe->getFrame( packet->frames[i].objectID, 
                                       packet->frames[i].version );
        _inputFrames.push_back( frame );
    }

    frameAssemble( packet->context.frameID );

    _inputFrames.clear();
    _context = NULL;

    if( getIAttribute( IATTR_HINT_STATISTICS ))
    {
        event.endTime = pipe->getFrameTime();
        pipe->addStatEvent( event );
    }
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Channel::_reqFrameReadback( eqNet::Command& command )
{
    ChannelFrameReadbackPacket* packet = 
        command.getPacket<ChannelFrameReadbackPacket>();
    EQLOG( LOG_TASKS | LOG_ASSEMBLY ) << "TASK readback " << getName() <<  " " 
                                       << packet << endl;

    Pipe* pipe = getPipe();
    StatEvent event( StatEvent::CHANNEL_READBACK, this, pipe->getFrameTime( ));

    _context = &packet->context;

    for( uint32_t i=0; i<packet->nFrames; ++i )
    {
        Frame* frame = pipe->getFrame( packet->frames[i].objectID, 
                                       packet->frames[i].version );
        _outputFrames.push_back( frame );
    }

    frameReadback( packet->context.frameID );

    for( vector<Frame*>::const_iterator i = _outputFrames.begin();
         i != _outputFrames.end(); ++i )
    {
        Frame* frame = *i;
        frame->syncReadback();
    }

    _outputFrames.clear();
    _context = NULL;

    if( getIAttribute( IATTR_HINT_STATISTICS ))
        pipe->addStatEvent( event );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Channel::_reqFrameTransmit( eqNet::Command& command )
{
    const ChannelFrameTransmitPacket* packet = 
        command.getPacket<ChannelFrameTransmitPacket>();
    EQLOG( LOG_TASKS | LOG_ASSEMBLY ) << "TASK transmit " << getName() <<  " " 
                                      << packet << endl;

    Pipe* pipe = getPipe();
    StatEvent event( StatEvent::CHANNEL_TRANSMIT, this, pipe->getFrameTime( ));

    eqNet::Session*     session   = getSession();
    RefPtr<eqNet::Node> localNode = session->getLocalNode();
    RefPtr<eqNet::Node> server    = session->getServer();
    Frame*              frame     = pipe->getFrame( packet->frame.objectID, 
                                                    packet->frame.version );
    for( uint32_t i=0; i<packet->nNodes; ++i )
    {
        const eqNet::NodeID& nodeID = packet->nodes[i];
        RefPtr<eqNet::Node>  node   = command.getNode();
        RefPtr<eqNet::Node>  toNode = localNode->connect( nodeID, node );
        EQLOG( LOG_ASSEMBLY ) << "channel \"" << getName() << "\" transmit " 
                              << frame << " to " << nodeID << endl;

        StatEvent nodeEvent( StatEvent::CHANNEL_TRANSMIT_NODE, this,
                             pipe->getFrameTime( ));
        frame->transmit( toNode );
        if( getIAttribute( IATTR_HINT_STATISTICS ))
            pipe->addStatEvent( nodeEvent );
    }

    if( getIAttribute( IATTR_HINT_STATISTICS ))
        pipe->addStatEvent( event );

    return eqNet::COMMAND_HANDLED;
}
