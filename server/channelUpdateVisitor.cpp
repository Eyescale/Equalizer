
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "channelUpdateVisitor.h"

#include "colorMask.h"
#include "compound.h"
#include "frame.h"
#include "node.h"
#include "pipe.h"
#include "view.h"
#include "window.h"

#include "channel.ipp"

#include <eq/client/log.h>

using namespace std;
using namespace eq::base;

namespace eq
{
namespace server
{
ChannelUpdateVisitor::ChannelUpdateVisitor( Channel* channel, 
                                            const uint32_t frameID,
                                            const uint32_t frameNumber )
        : _channel( channel )
        , _eye( eq::EYE_CYCLOP )
        , _frameID( frameID )
        , _frameNumber( frameNumber )
        , _updated( false )
{}

VisitorResult ChannelUpdateVisitor::visitPre( 
    const Compound* compound )
{
    _updateDrawFinish( compound );

    if( compound->getChannel() != _channel || 
        !compound->testInheritEye( _eye ) )
        
        return TRAVERSE_CONTINUE;

    _updateFrameRate( compound );

    if( compound->testInheritTask( eq::TASK_CLEAR ))
    {
        eq::ChannelFrameClearPacket clearPacket;        
        
        _setupRenderContext( compound, clearPacket.context );
        _channel->send( clearPacket );
        _updated = true;
        EQLOG( eq::LOG_TASKS ) << "TASK clear " << _channel->getName() <<  " "
                               << &clearPacket << endl;
    }
    return TRAVERSE_CONTINUE;
}

VisitorResult ChannelUpdateVisitor::visitLeaf( const Compound* compound )
{
    if( compound->getChannel() != _channel ||
        !compound->testInheritEye( _eye ) )
    {
        _updateDrawFinish( compound );
        return TRAVERSE_CONTINUE;
    }

    _updateFrameRate( compound );

    eq::RenderContext context;
    _setupRenderContext( compound, context );
    // OPT: Send render context once before task packets?

    if( compound->testInheritTask( eq::TASK_CLEAR ))
    {
        eq::ChannelFrameClearPacket clearPacket;        
        clearPacket.context = context;
        _channel->send( clearPacket );
        _updated = true;
        EQLOG( eq::LOG_TASKS ) << "TASK clear " << _channel->getName() <<  " "
                           << &clearPacket << endl;
    }
    if( compound->testInheritTask( eq::TASK_DRAW ))
    {
        eq::ChannelFrameDrawPacket drawPacket;

        drawPacket.context = context;
        _channel->send( drawPacket );
        _updated = true;
        EQLOG( eq::LOG_TASKS ) << "TASK draw " << _channel->getName() <<  " " 
                           << &drawPacket << endl;
    }
    
    _updateDrawFinish( compound );
    _updatePostDraw( compound, context );
    return TRAVERSE_CONTINUE;
}

VisitorResult ChannelUpdateVisitor::visitPost(
    const Compound* compound )
{
    if( compound->getChannel() != _channel || !compound->getInheritTasks() ||
        !compound->testInheritEye( _eye ))

        return TRAVERSE_CONTINUE;

    eq::RenderContext context;
    _setupRenderContext( compound, context );
    _updatePostDraw( compound, context );
    return TRAVERSE_CONTINUE;
}


void ChannelUpdateVisitor::_setupRenderContext( const Compound* compound,
                                                eq::RenderContext& context )
{
    context.frameID        = _frameID;
    context.pvp            = compound->getInheritPixelViewport();
    context.vp             = compound->getInheritViewport();
    context.range          = compound->getInheritRange();
    context.pixel          = compound->getInheritPixel();
    context.zoom           = compound->getInheritZoom();
    context.offset.x       = context.pvp.x;
    context.offset.y       = context.pvp.y;
    context.screenOrigin  = context.offset + compound->getInheritScreenOrigin();
    context.screenSize     = compound->getInheritScreenSize();
    context.eye            = _eye;
    context.buffer         = _getDrawBuffer();
    context.drawBufferMask = _getDrawBufferMask( compound );
    context.viewID         = EQ_ID_INVALID;

    const Channel* destChannel = compound->getInheritChannel();
    EQASSERT( destChannel );

    const View* destView = destChannel->getView();
    if( destView )
        context.viewID = destView->getID();

    if( _channel != destChannel &&
        compound->getIAttribute( Compound::IATTR_HINT_OFFSET ) != eq::ON )
    {
        const eq::PixelViewport& nativePVP = _channel->getPixelViewport();
        context.pvp.x = nativePVP.x;
        context.pvp.y = nativePVP.y;
    }
    // TODO: pvp size overcommit check?

    _computeFrustum( compound, context );
}

void ChannelUpdateVisitor::_updateDrawFinish( const Compound* compound ) const
{
    // Test if this is not the last eye pass of this compound
    if( compound->getInheritEyes() + 1 >
        static_cast< uint32_t >( 1<<( _eye + 1 )) ||
		// or we don't actually draw this eye
		!compound->testInheritEye( _eye ))
    {
        return;
    }
    if( _channel->getLastDrawCompound() != compound )
        return;

    // Channel::frameDrawFinish
    Node* node = _channel->getNode();

    eq::ChannelFrameDrawFinishPacket channelPacket;
    channelPacket.objectID    = _channel->getID();
    channelPacket.frameNumber = _frameNumber;
    channelPacket.frameID     = _frameID;

    node->send( channelPacket );
    EQLOG( eq::LOG_TASKS ) << "TASK channel draw finish " << _channel->getName()
                           <<  " " << &channelPacket <<endl;

    // Window::frameDrawFinish
    const Window* window = _channel->getWindow();
    if( window->getLastDrawChannel() != _channel )
        return;

    eq::WindowFrameDrawFinishPacket windowPacket;
    windowPacket.objectID    = window->getID();
    windowPacket.frameNumber = _frameNumber;
    windowPacket.frameID     = _frameID;

    node->send( windowPacket );
    EQLOG( eq::LOG_TASKS ) << "TASK window draw finish "  << window->getName() 
                           <<  " " << &windowPacket << endl;

    // Pipe::frameDrawFinish
    const Pipe* pipe = _channel->getPipe();
    if( pipe->getLastDrawWindow() != window )
        return;

    eq::PipeFrameDrawFinishPacket pipePacket;
    pipePacket.objectID    = pipe->getID();
    pipePacket.frameNumber = _frameNumber;
    pipePacket.frameID     = _frameID;

    node->send( pipePacket );
    EQLOG( eq::LOG_TASKS ) << "TASK pipe draw finish " 
                           << pipe->getName() <<  " " << &pipePacket << endl;

    // Node::frameDrawFinish
    if( node->getLastDrawPipe() != pipe )
        return;

    eq::NodeFrameDrawFinishPacket nodePacket;
    nodePacket.objectID    = node->getID();
    nodePacket.frameNumber = _frameNumber;
    nodePacket.frameID     = _frameID;

    node->send( nodePacket );
    EQLOG( eq::LOG_TASKS ) << "TASK node draw finish " << node->getName() 
                           <<  " " << &nodePacket << endl;
}

void ChannelUpdateVisitor::_updateFrameRate( const Compound* compound ) const
{
    const float maxFPS = compound->getInheritMaxFPS();
    Window*     window = _channel->getWindow();

    if( maxFPS <  window->getMaxFPS())
        window->setMaxFPS( maxFPS );
}

GLenum ChannelUpdateVisitor::_getDrawBuffer() const
{
    const eq::Window::DrawableConfig& drawableConfig = 
        _channel->getWindow()->getDrawableConfig();
    
    if( !drawableConfig.stereo )
    {    
        if( drawableConfig.doublebuffered )
            return GL_BACK;
        // else singlebuffered
        return GL_FRONT;
    }
    else
    {
        if( drawableConfig.doublebuffered )
        {
            switch( _eye )
            {
                case eq::EYE_LEFT:
                    return GL_BACK_LEFT;
                    break;
                case eq::EYE_RIGHT:
                    return GL_BACK_RIGHT;
                    break;
                default:
                    return GL_BACK;
                    break;
            }
        }
        // else singlebuffered
        switch( _eye )
        {
            case eq::EYE_LEFT:
                return GL_FRONT_LEFT;
                break;
            case eq::EYE_RIGHT:
                return GL_FRONT_RIGHT;
                break;
            default:
                return GL_FRONT;
                break;
        }
    }
}

eq::ColorMask ChannelUpdateVisitor::_getDrawBufferMask(const Compound* compound)
    const
{
    if( compound->getInheritIAttribute( Compound::IATTR_STEREO_MODE ) !=
        eq::ANAGLYPH )
        return eq::ColorMask::ALL;

    switch( _eye )
    {
        case eq::EYE_LEFT:
            return ColorMask( 
                compound->getInheritIAttribute(
                    Compound::IATTR_STEREO_ANAGLYPH_LEFT_MASK ));
        case eq::EYE_RIGHT:
            return ColorMask( 
                compound->getInheritIAttribute( 
                    Compound::IATTR_STEREO_ANAGLYPH_RIGHT_MASK ));
        default:
            return eq::ColorMask::ALL;
    }
}

void ChannelUpdateVisitor::_computeFrustum( const Compound* compound,
                                            eq::RenderContext& context )
{
    const FrustumData& frustumData = compound->getInheritFrustumData();
    const Config*      config      = compound->getConfig();

    // compute eye position in screen space
    const vmml::Vector3f& eyeW = config->getEyePosition( _eye );
    const vmml::Matrix4f& xfm  = frustumData.getTransform();
    const vmml::Vector3f  eye  = xfm * eyeW;

    EQVERB << "Eye position world: " << eyeW << " screen " << eye << endl;

    // compute perspective and orthographic frusta from size and eye position
    _computeFrustumCorners( context.frustum, compound, frustumData, eye, false);
    _computeFrustumCorners( context.ortho,   compound, frustumData, eye, true );

    // compute head transform
    // headTransform = -trans(eye) * view matrix (frustum position)
    vmml::Matrix4f& headTransform = context.headTransform;
    for( int i=0; i<16; i += 4 )
    {
        headTransform.ml[i]   = xfm.ml[i]   - eye[0] * xfm.ml[i+3];
        headTransform.ml[i+1] = xfm.ml[i+1] - eye[1] * xfm.ml[i+3];
        headTransform.ml[i+2] = xfm.ml[i+2] - eye[2] * xfm.ml[i+3];
        headTransform.ml[i+3] = xfm.ml[i+3];
    }
}

void ChannelUpdateVisitor::_computeFrustumCorners( vmml::Frustumf& frustum,
                                                   const Compound* compound,
                                                 const FrustumData& frustumData,
                                                   const vmml::Vector3f& eye,
                                                   const bool ortho )
{
    const Channel* destination = compound->getInheritChannel();
    destination->getNearFar( &frustum.nearPlane, &frustum.farPlane );

    const float ratio    = ortho ? 1.0f : frustum.nearPlane / eye.z;
    const float width_2  = frustumData.getWidth()  * .5f;
    const float height_2 = frustumData.getHeight() * .5f;

    if( eye.z > 0 || ortho )
    {
        frustum.left   =  ( -width_2  - eye.x ) * ratio;
        frustum.right  =  (  width_2  - eye.x ) * ratio;
        frustum.bottom =  ( -height_2 - eye.y ) * ratio;
        frustum.top    =  (  height_2 - eye.y ) * ratio;
    }
    else // eye behind near plane - 'mirror' x
    {
        frustum.left   =  (  width_2  - eye.x ) * ratio;
        frustum.right  =  ( -width_2  - eye.x ) * ratio;
        frustum.bottom =  (  height_2 + eye.y ) * ratio;
        frustum.top    =  ( -height_2 + eye.y ) * ratio;
    }

    // move frustum according to pixel decomposition
    const eq::Pixel& pixel = compound->getInheritPixel();
    if( pixel != eq::Pixel::ALL && pixel.isValid( ))
    {
        const Channel*    inheritChannel = compound->getInheritChannel();
        const eq::PixelViewport& destPVP = inheritChannel->getPixelViewport();
        
        if( pixel.w > 1 )
        {
            const float         frustumWidth = frustum.right - frustum.left;
            const float           pixelWidth = frustumWidth / 
                                               static_cast<float>( destPVP.w );
            const float               jitter = pixelWidth * pixel.x - 
                                               pixelWidth * .5f;

            frustum.left  += jitter;
            frustum.right += jitter;
        }
        if( pixel.h > 1 )
        {
            const float        frustumHeight = frustum.bottom - frustum.top;
            const float          pixelHeight = frustumHeight / 
                                               static_cast<float>( destPVP.h );
            const float               jitter = pixelHeight * pixel.y + 
                                               pixelHeight * .5f;

            frustum.top    -= jitter;
            frustum.bottom -= jitter;
        }
    }

    // adjust to viewport (screen-space decomposition)
    // Note: may need to be computed in pvp space to avoid rounding problems
    const eq::Viewport vp = compound->getInheritViewport();
    if( vp != eq::Viewport::FULL && vp.isValid( ))
    {
        const float frustumWidth = frustum.right - frustum.left;
        frustum.left  += frustumWidth * vp.x;
        frustum.right  = frustum.left + frustumWidth * vp.w;
        
        const float frustumHeight = frustum.top - frustum.bottom;
        frustum.bottom += frustumHeight * vp.y;
        frustum.top     = frustum.bottom + frustumHeight * vp.h;
    }
}

void ChannelUpdateVisitor::_updatePostDraw( const Compound* compound, 
                                            const eq::RenderContext& context )
{
    _updateAssemble( compound, context );
    _updateReadback( compound, context );
}

void ChannelUpdateVisitor::_updateAssemble( const Compound* compound,
                                            const eq::RenderContext& context )
{
    const std::vector< Frame* >& inputFrames = compound->getInputFrames();
    if( !compound->testInheritTask( eq::TASK_ASSEMBLE ))
        return;

    EQASSERT( !inputFrames.empty( ));

    vector<net::ObjectVersion> frameIDs;
    for( vector<Frame*>::const_iterator iter = inputFrames.begin(); 
         iter != inputFrames.end(); ++iter )
    {
        Frame* frame = *iter;

        if( !frame->hasData( _eye )) // TODO: filter: buffers, vp, eye
            continue;

        frameIDs.push_back( net::ObjectVersion( frame ));
    }

    if( frameIDs.empty() )
        return;

    Window* window = _channel->getWindow();
    window->updateFrameFinishNT( _frameNumber );

    // assemble task
    eq::ChannelFrameAssemblePacket packet;
    packet.context   = context;
    packet.nFrames   = frameIDs.size();

    EQLOG( eq::LOG_ASSEMBLY | eq::LOG_TASKS ) 
        << "TASK assemble " << _channel->getName() <<  " " << &packet << endl;
    _channel->send<net::ObjectVersion>( packet, frameIDs );
    _updated = true;
}
    
void ChannelUpdateVisitor::_updateReadback( const Compound* compound,
                                            const eq::RenderContext& context )
{
    const std::vector< Frame* >& outputFrames = compound->getOutputFrames();
    if( !compound->testInheritTask( eq::TASK_READBACK ))
        return;

    EQASSERT( !outputFrames.empty( ));

    FrameVector                frames;
    vector<net::ObjectVersion> frameIDs;
    for( vector<Frame*>::const_iterator i = outputFrames.begin(); 
         i != outputFrames.end(); ++i )
    {
        Frame* frame = *i;

        if( !frame->hasData( _eye )) // TODO: filter: buffers, vp, eye
            continue;

        frames.push_back( frame );
        frameIDs.push_back( net::ObjectVersion( frame ));
    }

    if( frames.empty() )
        return;

    Window* window = _channel->getWindow();
    window->updateFrameFinishNT( _frameNumber );

    // readback task
    eq::ChannelFrameReadbackPacket packet;
    packet.context   = context;
    packet.nFrames   = frames.size();

    _channel->send<net::ObjectVersion>( packet, frameIDs );
    _updated = true;
    EQLOG( eq::LOG_ASSEMBLY | eq::LOG_TASKS ) 
        << "TASK readback " << _channel->getName() <<  " " << &packet << endl;

    // transmit tasks
    Node*                 node         = _channel->getNode();
    net::NodePtr   netNode      = node->getNode();
    const net::NodeID&  outputNodeID = netNode->getNodeID();
    for( vector<Frame*>::const_iterator i = frames.begin(); 
         i != frames.end(); ++i )
    {
        Frame* outputFrame = *i;

        const vector<Frame*>& inputFrames = 
            outputFrame->getInputFrames( context.eye);

        vector<net::NodeID> nodeIDs;
        for( vector<Frame*>::const_iterator j = inputFrames.begin();
             j != inputFrames.end(); ++j )
        {
            const Frame*        inputFrame   = *j;
            const Node*         inputNode    = inputFrame->getNode();
            net::NodePtr inputNetNode = inputNode->getNode();
            net::NodeID       nodeID       = inputNetNode->getNodeID();
            EQASSERT( node );

            if( nodeID == outputNodeID ) // TODO filter: buffers, vp, eye
                continue;

            nodeID.convertToNetwork();
            nodeIDs.push_back( nodeID );
        }

        // sort & filter dupes
        stde::usort( nodeIDs );

        if( nodeIDs.empty( ))
            continue;

        // send
        eq::ChannelFrameTransmitPacket transmitPacket;
        transmitPacket.sessionID = packet.sessionID;
        transmitPacket.objectID  = packet.objectID;
        transmitPacket.context   = context;
        transmitPacket.frame     = net::ObjectVersion( outputFrame );
        transmitPacket.nNodes    = nodeIDs.size();

        EQLOG( eq::LOG_ASSEMBLY | eq::LOG_TASKS )
            << "TASK transmit " << _channel->getName() <<  " " << 
            &transmitPacket << " first " << nodeIDs[0] << endl;

        _channel->send<net::NodeID>( transmitPacket, nodeIDs );
    }        
}

}
}

