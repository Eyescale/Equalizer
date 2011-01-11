
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com>
 * Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com>
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

#include "channel.ipp"

#include <eq/channelPackets.h>
#include <eq/log.h>
#include <eq/nodePackets.h>
#include <eq/pipePackets.h>
#include <eq/windowPackets.h>
#include <eq/fabric/paths.h>

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

using fabric::PASSIVE;

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
    if( compound->getChannel() != _channel ||
        !compound->isInheritActive( _eye ) ||
        compound->getInheritTasks() == fabric::TASK_NONE )
    {
        return true;
    }

    return false;
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
    {
        ChannelFrameClearPacket clearPacket;
        clearPacket.context = context;

        _channel->send( clearPacket );
        _updated = true;
        EQLOG( LOG_TASKS ) << "TASK clear " << _channel->getName() <<  " "
                               << &clearPacket << std::endl;
    }
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
        _channel->send( drawPacket );
        _updated = true;
        EQLOG( LOG_TASKS ) << "TASK draw " << _channel->getName() <<  " " 
                           << &drawPacket << std::endl;
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


void ChannelUpdateVisitor::_setupRenderContext( const Compound* compound,
                                                RenderContext& context )
{
    const Channel* destChannel = compound->getInheritChannel();
    EQASSERT( destChannel );
    const View* view = destChannel->getView();

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
    context.view          = view;
    context.taskID        = compound->getTaskID();

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

    if( _channel != destChannel &&
        compound->getIAttribute( Compound::IATTR_HINT_OFFSET ) != fabric::ON )
    {
        const PixelViewport& nativePVP = _channel->getPixelViewport();
        context.pvp.x = nativePVP.x;
        context.pvp.y = nativePVP.y;
    }
    // TODO: pvp size overcommit check?

    _computeFrustum( compound, context );
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

    if( lastDrawChannel && lastDrawChannel != _channel )
        return;

    WindowFrameDrawFinishPacket windowPacket;
    windowPacket.objectID    = window->getID();
    windowPacket.frameNumber = _frameNumber;
    windowPacket.frameID     = _frameID;

    node->send( windowPacket );
    EQLOG( LOG_TASKS ) << "TASK window draw finish "  << window->getName() 
                           <<  " " << &windowPacket << std::endl;
    if( !lastDrawChannel )
        window->setLastDrawChannel( _channel );

    // Pipe::frameDrawFinish
    Pipe* pipe = _channel->getPipe();
    const Window* lastDrawWindow = pipe->getLastDrawWindow();
    if( lastDrawWindow && lastDrawWindow != window )
        return;

    PipeFrameDrawFinishPacket pipePacket;
    pipePacket.objectID    = pipe->getID();
    pipePacket.frameNumber = _frameNumber;
    pipePacket.frameID     = _frameID;

    node->send( pipePacket );
    EQLOG( LOG_TASKS ) << "TASK pipe draw finish " 
                           << pipe->getName() <<  " " << &pipePacket << std::endl;
    if( !lastDrawWindow )
        pipe->setLastDrawWindow( window );

    // Node::frameDrawFinish
    const Pipe* lastDrawPipe = node->getLastDrawPipe();
    if( lastDrawPipe && lastDrawPipe != pipe )
        return;

    NodeFrameDrawFinishPacket nodePacket;
    nodePacket.objectID    = node->getID();
    nodePacket.frameNumber = _frameNumber;
    nodePacket.frameID     = _frameID;

    node->send( nodePacket );
    EQLOG( LOG_TASKS ) << "TASK node draw finish " << node->getName() 
                           <<  " " << &nodePacket << std::endl;
    if( !lastDrawPipe )
        node->setLastDrawPipe( pipe );
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

    if( compound->getInheritIAttribute(Compound::IATTR_STEREO_MODE) == PASSIVE )
        return _drawBuffer[ 0 ][ dc.doublebuffered ][ eye ];
    return _drawBuffer[ dc.stereo ][ dc.doublebuffered ][ eye ];
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

void ChannelUpdateVisitor::_computeFrustum( const Compound* compound,
                                            RenderContext& context )
{
    // compute eye position in screen space
    const Vector3f  eyeW = _getEyePosition( compound );
    const FrustumData& frustumData = compound->getInheritFrustumData();
    const Matrix4f& xfm  = frustumData.getTransform();
    const Vector3f  eye  = xfm * eyeW;

    EQVERB << "Eye position world: " << eyeW << " screen " << eye << std::endl;

    // compute perspective and orthographic frusta from size and eye position
    _computeFrustumCorners( context.frustum, compound, frustumData, eye, false);
    _computeFrustumCorners( context.ortho,   compound, frustumData, eye, true );

    // compute head transform
    // headTransform = -trans(eye) * view matrix (frustum position)
    Matrix4f& headTransform = context.headTransform;
    for( int i=0; i<16; i += 4 )
    {
        headTransform.array[i]   = xfm.array[i]   - eye[0] * xfm.array[i+3];
        headTransform.array[i+1] = xfm.array[i+1] - eye[1] * xfm.array[i+3];
        headTransform.array[i+2] = xfm.array[i+2] - eye[2] * xfm.array[i+3];
        headTransform.array[i+3] = xfm.array[i+3];
    }

    const bool isHMD = (frustumData.getType() != Wall::TYPE_FIXED);
    if( isHMD )
        headTransform *= _getInverseHeadMatrix( compound );
}

Vector3f ChannelUpdateVisitor::_getEyePosition( const Compound* compound )
    const
{
    const FrustumData& frustumData = compound->getInheritFrustumData();
    const Channel* destChannel = compound->getInheritChannel();
    const View* view = destChannel->getView();
    const Observer* observer = view ? view->getObserver() : 0;

    if( observer && frustumData.getType() == Wall::TYPE_FIXED )
        return observer->getEyePosition( _eye );

    const Config* config = compound->getConfig();
    const float eyeBase_2 = 0.5f * ( observer ? 
      observer->getEyeBase() : config->getFAttribute( Config::FATTR_EYE_BASE ));

    switch( _eye )
    {
        case EYE_LEFT:
            return Vector3f(-eyeBase_2, 0.f, 0.f );

        case EYE_RIGHT:
            return Vector3f( eyeBase_2, 0.f, 0.f );

        default:
            EQUNIMPLEMENTED;
        case EYE_CYCLOP:
            return Vector3f( 0.f, 0.f, 0.f );
    }
}

const Matrix4f& ChannelUpdateVisitor::_getInverseHeadMatrix(
    const Compound* compound ) const
{
    const Channel* destChannel = compound->getInheritChannel();
    const View* view = destChannel->getView();
    const Observer* observer = static_cast< const Observer* >(
        view ? view->getObserver() : 0);

    if( observer )
        return observer->getInverseHeadMatrix();

    return Matrix4f::IDENTITY;
}

void ChannelUpdateVisitor::_computeFrustumCorners( Frustumf& frustum,
                                                   const Compound* compound,
                                                 const FrustumData& frustumData,
                                                   const Vector3f& eye,
                                                   const bool ortho )
{
    const Channel* destination = compound->getInheritChannel();
    frustum = destination->getFrustum();

    const float ratio    = ortho ? 1.0f : frustum.near_plane() / eye.z();
    const float width_2  = frustumData.getWidth()  * .5f;
    const float height_2 = frustumData.getHeight() * .5f;

    if( eye.z() > 0 || ortho )
    {
        frustum.left()   =  ( -width_2  - eye.x() ) * ratio;
        frustum.right()  =  (  width_2  - eye.x() ) * ratio;
        frustum.bottom() =  ( -height_2 - eye.y() ) * ratio;
        frustum.top()    =  (  height_2 - eye.y() ) * ratio;
    }
    else // eye behind near plane - 'mirror' x
    {
        frustum.left()   =  (  width_2  - eye.x() ) * ratio;
        frustum.right()  =  ( -width_2  - eye.x() ) * ratio;
        frustum.bottom() =  (  height_2 + eye.y() ) * ratio;
        frustum.top()    =  ( -height_2 + eye.y() ) * ratio;
    }

    // move frustum according to pixel decomposition
    const Pixel& pixel = compound->getInheritPixel();
    if( pixel != Pixel::ALL && pixel.isValid( ))
    {
        const Channel*    inheritChannel = compound->getInheritChannel();
        const PixelViewport& destPVP = inheritChannel->getPixelViewport();
        
        if( pixel.w > 1 )
        {
            const float         frustumWidth = frustum.right() - frustum.left();
            const float           pixelWidth = frustumWidth / 
                                               static_cast<float>( destPVP.w );
            const float               jitter = pixelWidth * pixel.x - 
                                               pixelWidth * .5f;

            frustum.left()  += jitter;
            frustum.right() += jitter;
        }
        if( pixel.h > 1 )
        {
            const float        frustumHeight = frustum.bottom() - frustum.top();
            const float          pixelHeight = frustumHeight / 
                                               static_cast<float>( destPVP.h );
            const float               jitter = pixelHeight * pixel.y + 
                                               pixelHeight * .5f;

            frustum.top()    -= jitter;
            frustum.bottom() -= jitter;
        }
    }

    // adjust to viewport (screen-space decomposition)
    // Note: vp is computed pixel-correct by Compound::updateInheritData()
    const Viewport vp = compound->getInheritViewport();
    if( vp != Viewport::FULL && vp.isValid( ))
    {
        const float frustumWidth = frustum.right() - frustum.left();
        frustum.left()  += frustumWidth * vp.x;
        frustum.right()  = frustum.left() + frustumWidth * vp.w;
        
        const float frustumHeight = frustum.top() - frustum.bottom();
        frustum.bottom() += frustumHeight * vp.y;
        frustum.top()     = frustum.bottom() + frustumHeight * vp.h;
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
    for( Frames::const_iterator i = outputFrames.begin(); 
         i != outputFrames.end(); ++i )
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
    for( Frames::const_iterator i = frames.begin(); i != frames.end(); ++i )
    {
        Frame* outputFrame = *i;
        const Frames& inputFrames = outputFrame->getInputFrames( context.eye );
        std::set< uint128_t > nodeIDs;

        for( Frames::const_iterator j = inputFrames.begin();
             j != inputFrames.end(); ++j )
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

