
/* Copyright (c) 2007-2013, Stefan Eilemann <eile@equalizergraphics.com>
 *               2011-2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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
#include "log.h"
#include "node.h"
#include "observer.h"
#include "pipe.h"
#include "segment.h"
#include "view.h"
#include "window.h"
#include "tileQueue.h"

#include <eq/fabric/commands.h>
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
    const int32_t cyclop = lunchbox::getIndexOfLastBit( EYE_CYCLOP );
    const int32_t left = lunchbox::getIndexOfLastBit( EYE_LEFT );
    const int32_t right = lunchbox::getIndexOfLastBit( EYE_RIGHT );

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

    // OPT: Send render context once before task commands?
    RenderContext context;
    _setupRenderContext( compound, context );
    _updateFrameRate( compound );
    _updateViewStart( compound, context );
    _updateDraw( compound, context );
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

void ChannelUpdateVisitor::_setupRenderContext( const Compound* compound,
                                                RenderContext& context )
{
    const Channel* destChannel = compound->getInheritChannel();
    LBASSERT( destChannel );

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

    const View* view = destChannel->getView();
    LBASSERT( context.view == co::ObjectVersion( view ));

    if( view )
    {
        // compute inherit vp (part of view covered by segment/view channel)
        const Segment* segment = destChannel->getSegment();
        LBASSERT( segment );

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

void ChannelUpdateVisitor::_updateDraw( const Compound* compound,
                                        const RenderContext& context )
{
    if( compound->hasTiles( ))
    {
        _updateDrawTiles( compound, context );
        return;
    }

    if( compound->testInheritTask( fabric::TASK_CLEAR ))
        _sendClear( context );

    if( compound->testInheritTask( fabric::TASK_DRAW ))
    {
        const bool finish = _channel->hasListeners(); // finish for eq stats
        _channel->send( fabric::CMD_CHANNEL_FRAME_DRAW ) << context << finish;
        _updated = true;
        LBLOG( LOG_TASKS ) << "TASK draw " << _channel->getName() <<  " "
                           << finish << std::endl;
    }
}

void ChannelUpdateVisitor::_updateDrawTiles( const Compound* compound,
                                             const RenderContext& context )
{
    Frames frames;
    co::ObjectVersions frameIDs;
    const Frames& outputFrames = compound->getOutputFrames();
    for( FramesCIter i = outputFrames.begin(); i != outputFrames.end(); ++i)
    {
        Frame* frame = *i;

        if( !frame->hasData( _eye )) // TODO: filter: buffers, vp, eye
            continue;

        frames.push_back( frame );
        frameIDs.push_back( co::ObjectVersion( frame ));
    }

    const Channel* destChannel = compound->getInheritChannel();
    const TileQueues& inputQueues = compound->getInputTileQueues();
    for( TileQueuesCIter i = inputQueues.begin(); i != inputQueues.end(); ++i )
    {
        const TileQueue* inputQueue = *i;
        const TileQueue* outputQueue = inputQueue->getOutputQueue( context.eye);
        const uint128_t& id = outputQueue->getQueueMasterID( context.eye );
        LBASSERT( id != 0 );

        const bool isLocal = (_channel == destChannel);
        const uint32_t tasks = compound->getInheritTasks() &
                            ( eq::fabric::TASK_CLEAR | eq::fabric::TASK_DRAW |
                              eq::fabric::TASK_READBACK );

        _channel->send( fabric::CMD_CHANNEL_FRAME_TILES )
                << context << isLocal << id << tasks << frameIDs;
        _updated = true;
        LBLOG( LOG_TASKS ) << "TASK tiles " << _channel->getName() <<  " "
                           << std::endl;
    }
}

void ChannelUpdateVisitor::_updateDrawFinish( const Compound* compound ) const
{
    const Compound* lastDrawCompound = _channel->getLastDrawCompound();
    if( lastDrawCompound && lastDrawCompound != compound )
        return;

    // Only pass if this is the last eye pass of this compound
    if( !compound->isLastInheritEye( _eye ))
        return;

    if( !lastDrawCompound )
        _channel->setLastDrawCompound( compound );

    // Channel::frameDrawFinish
    Node* node = _channel->getNode();

    node->send( fabric::CMD_CHANNEL_FRAME_DRAW_FINISH, _channel->getID( ))
            << _frameID << _frameNumber;
    LBLOG( LOG_TASKS ) << "TASK channel draw finish " << _channel->getName()
                       << " frame " << _frameNumber
                       << " id " << _frameID << std::endl;

    // Window::frameDrawFinish
    Window* window = _channel->getWindow();
    const Channel* lastDrawChannel = window->getLastDrawChannel();

    if( lastDrawChannel && lastDrawChannel != _channel )
        return;

    window->setLastDrawChannel( _channel ); // in case not set

    node->send( fabric::CMD_WINDOW_FRAME_DRAW_FINISH, window->getID( ))
            << _frameID << _frameNumber;
    LBLOG( LOG_TASKS ) << "TASK window draw finish "  << window->getName()
                           <<  " frame " << _frameNumber
                           << " id " << _frameID << std::endl;

    // Pipe::frameDrawFinish
    Pipe* pipe = _channel->getPipe();
    const Window* lastDrawWindow = pipe->getLastDrawWindow();
    if( lastDrawWindow && lastDrawWindow != window )
        return;

    pipe->setLastDrawWindow( window ); // in case not set

    node->send( fabric::CMD_PIPE_FRAME_DRAW_FINISH, pipe->getID( ))
            << _frameID << _frameNumber;
    LBLOG( LOG_TASKS ) << "TASK pipe draw finish " << pipe->getName()
                       << " frame " << _frameNumber
                       << " id " << _frameID << std::endl;

    // Node::frameDrawFinish
    const Pipe* lastDrawPipe = node->getLastDrawPipe();
    if( lastDrawPipe && lastDrawPipe != pipe )
        return;

    node->setLastDrawPipe( pipe ); // in case not set

    node->send( fabric::CMD_NODE_FRAME_DRAW_FINISH, node->getID( ))
            << _frameID << _frameNumber;
    LBLOG( LOG_TASKS ) << "TASK node draw finish " << node->getName() <<  " "
                       << std::endl;
}

void ChannelUpdateVisitor::_sendClear( const RenderContext& context )
{
    _channel->send( fabric::CMD_CHANNEL_FRAME_CLEAR ) << context;
    _updated = true;
    LBLOG( LOG_TASKS ) << "TASK clear " << _channel->getName() <<  " "
                       << std::endl;
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
    const int32_t eye = lunchbox::getIndexOfLastBit( _eye );

    if( compound->getInheritIAttribute(Compound::IATTR_STEREO_MODE) == QUAD )
        return _drawBuffer[ dc.stereo ][ dc.doublebuffered ][ eye ];
    return _drawBuffer[ 0 ][ dc.doublebuffered ][ eye ];
}

fabric::ColorMask
ChannelUpdateVisitor::_getDrawBufferMask(const Compound* compound) const
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
    _updateReadback( compound, context );
    _updateViewFinish( compound, context );
}

void ChannelUpdateVisitor::_updateAssemble( const Compound* compound,
                                            const RenderContext& context )
{
    if( !compound->testInheritTask( fabric::TASK_ASSEMBLE ))
        return;

    const Frames& inputFrames = compound->getInputFrames();
    LBASSERT( !inputFrames.empty( ));

    co::ObjectVersions frames;
    for( Frames::const_iterator iter = inputFrames.begin();
         iter != inputFrames.end(); ++iter )
    {
        Frame* frame = *iter;

        if( !frame->hasData( _eye )) // TODO: filter: buffers, vp, eye
            continue;

        LBLOG( LOG_ASSEMBLY ) << *frame << std::endl;
        frames.push_back( co::ObjectVersion( frame ));
    }

    if( frames.empty( ))
        return;

    // assemble task
    LBLOG( LOG_ASSEMBLY | LOG_TASKS )
        << "TASK assemble " << _channel->getName()
        << " nFrames " << frames.size() << std::endl;
    _channel->send( fabric::CMD_CHANNEL_FRAME_ASSEMBLE )
            << context << frames;
    _updated = true;
}

void ChannelUpdateVisitor::_updateReadback( const Compound* compound,
                                            const RenderContext& context )
{
    if( !compound->testInheritTask( fabric::TASK_READBACK ) ||
        ( compound->hasTiles() && compound->isLeaf( )))
    {
        return;
    }

    const std::vector< Frame* >& outputFrames = compound->getOutputFrames();
    LBASSERT( !outputFrames.empty( ));

    co::ObjectVersions frames;
    for( FramesCIter i = outputFrames.begin(); i != outputFrames.end(); ++i )
    {
        Frame* frame = *i;

        if( !frame->hasData( _eye )) // TODO: filter: buffers, vp, eye
            continue;

        frames.push_back( co::ObjectVersion( frame ));
        LBLOG( LOG_ASSEMBLY ) << *frame << std::endl;
    }

    if( frames.empty() )
        return;

    // readback task
    _channel->send( fabric::CMD_CHANNEL_FRAME_READBACK )
            << context << frames;
    _updated = true;
    LBLOG( LOG_ASSEMBLY | LOG_TASKS )
        << "TASK readback " << _channel->getName()
        << " nFrames " << frames.size() << std::endl;
}

void ChannelUpdateVisitor::_updateViewStart( const Compound* compound,
                                             const RenderContext& context )
{
    LBASSERT( !_skipCompound( compound ));
    if( !compound->testInheritTask( fabric::TASK_VIEW ))
        return;

    // view start task
    LBLOG( LOG_TASKS ) << "TASK view start " << _channel->getName()
                       << std::endl;
    _channel->send( fabric::CMD_CHANNEL_FRAME_VIEW_START ) << context;
}

void ChannelUpdateVisitor::_updateViewFinish( const Compound* compound,
                                              const RenderContext& context )
{
    LBASSERT( !_skipCompound( compound ));
    if( !compound->testInheritTask( fabric::TASK_VIEW ))
        return;

    // view finish task
    LBLOG( LOG_TASKS ) << "TASK view finish " << _channel->getName() <<  " "
                       << std::endl;
    _channel->send( fabric::CMD_CHANNEL_FRAME_VIEW_FINISH ) << context;
}

}
}
