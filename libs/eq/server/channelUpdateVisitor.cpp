
/* Copyright (c) 2007-2011, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com>
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

#include "channelUpdateVisitor.h"

#include "colorMask.h"
#include "compound.h"
#include "frame.h"
#include "node.h"
#include "observer.h"
#include "pipe.h"
#include "segment.h"
#include "view.h"
#include "window.h"
#include "tileQueue.h"

#include "channel.ipp"

#include <eq/client/channelPackets.h>
#include <eq/client/log.h>
#include <eq/client/nodePackets.h>
#include <eq/client/pipePackets.h>
#include <eq/client/windowPackets.h>
#include <eq/fabric/paths.h>

#include <set>

#ifndef GL_BACK_LEFT
#  define GL_FRONT_LEFT 0x0400
#  define GL_FRONT_RIGHT 0x0401
#  define GL_BACK_LEFT 0x0402
#  define GL_BACK_RIGHT 0x0403
#  define GL_FRONT 0x0404
#  define GL_BACK 0x0405
#endif

namespace eq
{
namespace server
{

using fabric::QUAD;

namespace
{
static bool _setDrawBuffers();
static uint32_t _drawBuffer[2][2][NUM_EYES];
static bool _drawBufferInit = _setDrawBuffers();
bool _setDrawBuffers()
{
    const int32_t cyclop = co::base::getIndexOfLastBit( EYE_CYCLOP );
    const int32_t left = co::base::getIndexOfLastBit( EYE_LEFT );
    const int32_t right = co::base::getIndexOfLastBit( EYE_RIGHT );

    // [stereo][doublebuffered][eye]
    _drawBuffer[0][0][ cyclop ] = GL_FRONT;
    _drawBuffer[0][0][ left ] = GL_FRONT;
    _drawBuffer[0][0][ right ] = GL_FRONT;

    _drawBuffer[0][1][ cyclop ] = GL_BACK;
    _drawBuffer[0][1][ left ] = GL_BACK;
    _drawBuffer[0][1][ right ] = GL_BACK;

    _drawBuffer[1][0][ cyclop ] = GL_FRONT;
    _drawBuffer[1][0][ left ] = GL_FRONT_LEFT;
    _drawBuffer[1][0][ right ] = GL_FRONT_RIGHT;

    _drawBuffer[1][1][ cyclop ] = GL_BACK;
    _drawBuffer[1][1][ left ] = GL_BACK_LEFT;
    _drawBuffer[1][1][ right ] = GL_BACK_RIGHT;

    return true;
}
}

ChannelUpdateVisitor::ChannelUpdateVisitor( Channel* channel, 
                                            const uint128_t frameID,
                                            const uint32_t frameNumber )
        : _channel( channel )
        , _eye( EYE_CYCLOP )
        , _frameID( frameID )
        , _frameNumber( frameNumber )
        , _updated( false )
{}

bool ChannelUpdateVisitor::_skipCompound( const Compound* compound )
{
    return ( compound->getChannel() != _channel ||
             !compound->isInheritActive( _eye ) ||
             compound->getInheritTasks() == fabric::TASK_NONE );
}

VisitorResult ChannelUpdateVisitor::visitPre( const Compound* compound )
{
    if( !compound->isInheritActive( _eye ))
        return TRAVERSE_PRUNE;    

    _updateDrawFinish( compound );

    if( _skipCompound( compound ))
        return TRAVERSE_CONTINUE;

    RenderContext context;
    _setupRenderContext( compound, context );

    _updateFrameRate( compound );
    _updateViewStart( compound, context );

    if( compound->testInheritTask( fabric::TASK_CLEAR ))
        _sendClear( context );
    return TRAVERSE_CONTINUE;
}

VisitorResult ChannelUpdateVisitor::visitLeaf( const Compound* compound )
{
    if( !compound->isInheritActive( _eye ))
        return TRAVERSE_CONTINUE;    

    if( _skipCompound( compound ))
    {
        _updateDrawFinish( compound );
        return TRAVERSE_CONTINUE;
    }

    // OPT: Send render context once before task packets?
    RenderContext context;
    _setupRenderContext( compound, context );
    _updateFrameRate( compound );
    _updateViewStart( compound, context );

    // TODO refactor!
    if ( _getTilesEnabled( compound ) )
    {
        Frames frames;
        std::vector< co::ObjectVersion > frameIDs;
        const std::vector< Frame* >& outputFrames = compound->getOutputFrames();
        for( FramesCIter i = outputFrames.begin(); i != outputFrames.end(); ++i )
        {
            Frame* frame = *i;

            if( !frame->hasData( _eye )) // TODO: filter: buffers, vp, eye
                continue;

            frames.push_back( frame );
            frameIDs.push_back( co::ObjectVersion( frame ));
        }

        const TileQueues& inputQueues = compound->getInputTileQueues();
        for( TileQueuesCIter i = inputQueues.begin();
             i != inputQueues.end(); ++i )
        {
            const TileQueue* inputQueue = *i;
            const TileQueue* outputQueue =
                    inputQueue->getOutputQueue( context.eye );

            ChannelFrameTilesPacket tilesPacket;
            tilesPacket.context = context;
            tilesPacket.tasks = compound->getInheritTasks();
            tilesPacket.tasks &= ( eq::fabric::TASK_CLEAR | 
                        eq::fabric::TASK_DRAW | eq::fabric::TASK_READBACK );

            const UUID& id = outputQueue->getQueueMasterID( context.eye );
            EQASSERT( id != co::base::UUID::ZERO );
            tilesPacket.queueVersion.identifier = id;
            tilesPacket.queueVersion.version = co::VERSION_FIRST;
            tilesPacket.nFrames   = uint32_t( frames.size( ));
            _channel->send<co::ObjectVersion>( tilesPacket, frameIDs );

            _updated = true;
            EQLOG( LOG_TASKS ) << "TASK tiles " << _channel->getName()
                               <<  " " << &tilesPacket << std::endl;
        }
    }
    else
    {
        if( compound->testInheritTask( fabric::TASK_CLEAR ))
        {
            ChannelFrameClearPacket clearPacket;        
            clearPacket.context = context;
            _channel->send( clearPacket );
            _updated = true;
            EQLOG( LOG_TASKS ) << "TASK clear " << _channel->getName() <<  " "
                << &clearPacket << std::endl;
        }
        if( compound->testInheritTask( fabric::TASK_DRAW ))
        {
            ChannelFrameDrawPacket drawPacket;

            drawPacket.context = context;
            drawPacket.finish = _channel->hasListeners(); // finish for equalizers
            _channel->send( drawPacket );
            _updated = true;
            EQLOG( LOG_TASKS ) << "TASK draw " << _channel->getName() <<  " " 
                << &drawPacket << std::endl;
        }
    }

    _updateDrawFinish( compound );
    _updatePostDraw( compound, context );
    return TRAVERSE_CONTINUE;
}

VisitorResult ChannelUpdateVisitor::visitPost( const Compound* compound )
{
    if( _skipCompound( compound ))
        return TRAVERSE_CONTINUE;

    RenderContext context;
    _setupRenderContext( compound, context );
    
    
    _updatePostDraw( compound, context );

    return TRAVERSE_CONTINUE;
}

bool ChannelUpdateVisitor::_getTilesEnabled( const Compound* compound )
{
    return !compound->getInputTileQueues().empty();
}


void ChannelUpdateVisitor::_setupRenderContext( const Compound* compound,
                                                RenderContext& context )
{
    const Channel* destChannel = compound->getInheritChannel();
    EQASSERT( destChannel );

    bool tilesEnabled = !compound->getOutputTileQueues().empty() || 
                            !compound->getInputTileQueues().empty();

    context.frameID       = _frameID;
    context.pvp           = compound->getInheritPixelViewport();
    context.overdraw      = compound->getInheritOverdraw();
    context.vp            = compound->getInheritViewport();
    context.range         = compound->getInheritRange();
    context.pixel         = compound->getInheritPixel();
    context.subpixel      = compound->getInheritSubPixel();
    context.zoom          = compound->getInheritZoom();
    context.period        = compound->getInheritPeriod();
    context.phase         = compound->getInheritPhase();
    context.offset.x()    = context.pvp.x;
    context.offset.y()    = context.pvp.y;
    context.eye           = _eye;
    context.buffer        = _getDrawBuffer( compound );
    context.bufferMask    = _getDrawBufferMask( compound );
    context.view          = destChannel->getViewVersion();
    context.taskID        = compound->getTaskID();
    context.tilesEnabled  = tilesEnabled;

    const View* view = destChannel->getView();
    EQASSERT( context.view == view );

    if( view )
    {
        // compute inherit vp (part of view covered by segment/view channel)
        const Segment* segment = destChannel->getSegment();
        EQASSERT( segment );

        const PixelViewport& pvp = destChannel->getPixelViewport();
        if( pvp.hasArea( ))
            context.vp.applyView( segment->getViewport(), view->getViewport(),
                                  pvp, destChannel->getOverdraw( ));
    }

    if( _channel != destChannel )
    {
        const PixelViewport& nativePVP = _channel->getPixelViewport();
        context.pvp.x = nativePVP.x;
        context.pvp.y = nativePVP.y;
    }
    // TODO: pvp size overcommit check?

    compound->computeFrustum( context, _eye );
}

void ChannelUpdateVisitor::_updateDrawFinish( const Compound* compound ) const
{
    const Compound* lastDrawCompound = _channel->getLastDrawCompound();
    if( lastDrawCompound && lastDrawCompound != compound )
        return;

    // Test if this is not the last eye pass of this compound
    if( !compound->isLastInheritEye( _eye ))
        return;

    if( !lastDrawCompound )
        _channel->setLastDrawCompound( compound );

    // Channel::frameDrawFinish
    Node* node = _channel->getNode();

    ChannelFrameDrawFinishPacket channelPacket;
    channelPacket.objectID    = _channel->getID();
    channelPacket.frameNumber = _frameNumber;
    channelPacket.frameID     = _frameID;

    node->send( channelPacket );
    EQLOG( LOG_TASKS ) << "TASK channel draw finish " << _channel->getName()
                       <<  " " << &channelPacket << std::endl;

    // Window::frameDrawFinish
    Window* window = _channel->getWindow();
    const Channel* lastDrawChannel = window->getLastDrawChannel();

    if( lastDrawChannel != _channel )
        return;

    WindowFrameDrawFinishPacket windowPacket;
    windowPacket.objectID    = window->getID();
    windowPacket.frameNumber = _frameNumber;
    windowPacket.frameID     = _frameID;

    node->send( windowPacket );
    EQLOG( LOG_TASKS ) << "TASK window draw finish "  << window->getName() 
                           <<  " " << &windowPacket << std::endl;

    // Pipe::frameDrawFinish
    Pipe* pipe = _channel->getPipe();
    const Window* lastDrawWindow = pipe->getLastDrawWindow();
    if( lastDrawWindow != window )
        return;            

    PipeFrameDrawFinishPacket pipePacket;
    pipePacket.objectID    = pipe->getID();
    pipePacket.frameNumber = _frameNumber;
    pipePacket.frameID     = _frameID;

    node->send( pipePacket );
    EQLOG( LOG_TASKS ) << "TASK pipe draw finish " << pipe->getName() <<  " "
                       << &pipePacket << std::endl;

    // Node::frameDrawFinish
    const Pipe* lastDrawPipe = node->getLastDrawPipe();
    if( lastDrawPipe != pipe )
        return;

    NodeFrameDrawFinishPacket nodePacket;
    nodePacket.objectID    = node->getID();
    nodePacket.frameNumber = _frameNumber;
    nodePacket.frameID     = _frameID;

    node->send( nodePacket );
    EQLOG( LOG_TASKS ) << "TASK node draw finish " << node->getName() <<  " "
                       << &nodePacket << std::endl;
}

void ChannelUpdateVisitor::_sendClear( const RenderContext& context )
{
    ChannelFrameClearPacket clearPacket;
    clearPacket.context = context;
    _channel->send( clearPacket );
    _updated = true;
    EQLOG( LOG_TASKS ) << "TASK clear " << _channel->getName() <<  " "
                       << &clearPacket << std::endl;
}

void ChannelUpdateVisitor::_updateFrameRate( const Compound* compound ) const
{
    const float maxFPS = compound->getInheritMaxFPS();
    Window*     window = _channel->getWindow();

    if( maxFPS < window->getMaxFPS())
        window->setMaxFPS( maxFPS );
}

uint32_t ChannelUpdateVisitor::_getDrawBuffer( const Compound* compound ) const
{
    const DrawableConfig& dc = _channel->getWindow()->getDrawableConfig();
    const int32_t eye = co::base::getIndexOfLastBit( _eye );

    if( compound->getInheritIAttribute(Compound::IATTR_STEREO_MODE) == QUAD )
        return _drawBuffer[ dc.stereo ][ dc.doublebuffered ][ eye ];
    return _drawBuffer[ 0 ][ dc.doublebuffered ][ eye ];
}

eq::ColorMask ChannelUpdateVisitor::_getDrawBufferMask(const Compound* compound)
    const
{
    if( compound->getInheritIAttribute( Compound::IATTR_STEREO_MODE ) !=
        fabric::ANAGLYPH )
    {
        return ColorMask::ALL;
    }

    switch( _eye )
    {
        case EYE_LEFT:
            return ColorMask( 
                compound->getInheritIAttribute(
                    Compound::IATTR_STEREO_ANAGLYPH_LEFT_MASK ));
        case EYE_RIGHT:
            return ColorMask( 
                compound->getInheritIAttribute( 
                    Compound::IATTR_STEREO_ANAGLYPH_RIGHT_MASK ));
        default:
            return ColorMask::ALL;
    }
}

void ChannelUpdateVisitor::_updatePostDraw( const Compound* compound, 
                                            const RenderContext& context )
{
    _updateAssemble( compound, context );
    
    if( !_getTilesEnabled( compound ))
        _updateReadback( compound, context );
    
    _updateViewFinish( compound, context );
}

void ChannelUpdateVisitor::_updateAssemble( const Compound* compound,
                                            const RenderContext& context )
{
    if( !compound->testInheritTask( fabric::TASK_ASSEMBLE ))
        return;

    const Frames& inputFrames = compound->getInputFrames();
    EQASSERT( !inputFrames.empty( ));

    std::vector< co::ObjectVersion > frameIDs;
    for( Frames::const_iterator iter = inputFrames.begin(); 
         iter != inputFrames.end(); ++iter )
    {
        Frame* frame = *iter;

        if( !frame->hasData( _eye )) // TODO: filter: buffers, vp, eye
            continue;

        frameIDs.push_back( co::ObjectVersion( frame ));
    }

    if( frameIDs.empty() )
        return;

    // assemble task
    ChannelFrameAssemblePacket packet;
    packet.context   = context;
    packet.nFrames   = uint32_t( frameIDs.size( ));

    EQLOG( LOG_ASSEMBLY | LOG_TASKS ) 
        << "TASK assemble " << _channel->getName() <<  " " << &packet << std::endl;
    _channel->send<co::ObjectVersion>( packet, frameIDs );
    _updated = true;
}

void ChannelUpdateVisitor::_updateReadback( const Compound* compound,
                                            const RenderContext& context )
{
    if( !compound->testInheritTask( fabric::TASK_READBACK ))
        return;

    const std::vector< Frame* >& outputFrames = compound->getOutputFrames();
    EQASSERT( !outputFrames.empty( ));

    Frames frames;
    std::vector< co::ObjectVersion > frameIDs;
    for( FramesCIter i = outputFrames.begin(); i != outputFrames.end(); ++i )
    {
        Frame* frame = *i;

        if( !frame->hasData( _eye )) // TODO: filter: buffers, vp, eye
            continue;

        frames.push_back( frame );
        frameIDs.push_back( co::ObjectVersion( frame ));
    }

    if( frames.empty() )
        return;

    // readback task
    ChannelFrameReadbackPacket packet;
    packet.context   = context;
    packet.nFrames   = uint32_t( frames.size( ));

    _channel->send<co::ObjectVersion>( packet, frameIDs );
    _updated = true;
    EQLOG( LOG_ASSEMBLY | LOG_TASKS ) 
        << "TASK readback " << _channel->getName() <<  " " << &packet
        << std::endl;

    // transmit tasks
    Node* node = _channel->getNode();
    co::NodePtr netNode = node->getNode();
    const co::NodeID&  outputNodeID = netNode->getNodeID();
    for( FramesCIter i = frames.begin(); i != frames.end(); ++i )
    {
        Frame* outputFrame = *i;
        const Frames& inputFrames = outputFrame->getInputFrames( context.eye );
        std::set< uint128_t > nodeIDs;

        for( FramesCIter j = inputFrames.begin(); j != inputFrames.end(); ++j )
        {
            const Frame* inputFrame   = *j;
            const Node*  inputNode    = inputFrame->getNode();
            co::NodePtr inputNetNode = inputNode->getNode();

            ChannelFrameTransmitPacket transmitPacket;
            transmitPacket.netNodeID = inputNetNode->getNodeID();

            if( transmitPacket.netNodeID == outputNodeID ||
                nodeIDs.find( transmitPacket.netNodeID ) != nodeIDs.end( ))
            {
                continue;  // TODO filter: buffers, vp, eye
            }

            // send
            transmitPacket.context   = context;
            transmitPacket.frameData = outputFrame->getDataVersion( _eye );
            transmitPacket.clientNodeID = inputNode->getID();

            EQLOG( LOG_ASSEMBLY | LOG_TASKS )
                << "TASK transmit " << _channel->getName() <<  " "
                << &transmitPacket << std::endl;

            _channel->send( transmitPacket );
            nodeIDs.insert( transmitPacket.netNodeID );
        }
    }        
}

void ChannelUpdateVisitor::_updateViewStart( const Compound* compound,
                                             const RenderContext& context )
{
    EQASSERT( !_skipCompound( compound ));
    if( !compound->testInheritTask( fabric::TASK_VIEW ))
        return;
    
    // view start task
    ChannelFrameViewStartPacket packet;
    packet.context = context;

    EQLOG( LOG_TASKS ) << "TASK view start " << _channel->getName() <<  " "
                           << &packet << std::endl;
    _channel->send( packet );
}

void ChannelUpdateVisitor::_updateViewFinish( const Compound* compound,
                                              const RenderContext& context )
{
    EQASSERT( !_skipCompound( compound ));
    if( !compound->testInheritTask( fabric::TASK_VIEW ))
        return;
    
    // view finish task
    ChannelFrameViewFinishPacket packet;
    packet.context = context;

    EQLOG( LOG_TASKS ) << "TASK view finish " << _channel->getName() <<  " "
                       << &packet << std::endl;
    _channel->send( packet );
}

}
}

