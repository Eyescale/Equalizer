
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "compound.h"

#include "canvas.h"
#include "channel.h"
#include "colorMask.h"
#include "compoundInitVisitor.h"
#include "compoundListener.h"
#include "compoundUpdateDataVisitor.h"
#include "compoundUpdateInputVisitor.h"
#include "compoundUpdateOutputVisitor.h"
#include "config.h"
#include "equalizers/equalizer.h"
#include "frame.h"
#include "frameData.h"
#include "tileQueue.h"
#include "global.h"
#include "layout.h"
#include "log.h"
#include "segment.h"
#include "view.h"
#include "observer.h"

#include <eq/client/packets.h>
#include <eq/fabric/paths.h>
#include <co/base/os.h>
#include <co/base/stdExt.h>

#include <algorithm>
#include <math.h>
#include <vector>

#include "compoundActivateVisitor.h"
#include "compoundExitVisitor.h"

namespace eq
{

using fabric::EYES_STEREO;
using fabric::UNDEFINED;

namespace server
{
#define MAKE_ATTR_STRING( attr ) ( std::string("EQ_COMPOUND_") + #attr )
std::string Compound::_iAttributeStrings[IATTR_ALL] = {
    MAKE_ATTR_STRING( IATTR_STEREO_MODE ),
    MAKE_ATTR_STRING( IATTR_STEREO_ANAGLYPH_LEFT_MASK ),
    MAKE_ATTR_STRING( IATTR_STEREO_ANAGLYPH_RIGHT_MASK ),
    MAKE_ATTR_STRING( IATTR_FILL1 ),
    MAKE_ATTR_STRING( IATTR_FILL2 )
};

Compound::Compound( Config* parent )
        : _config( parent )
        , _parent( 0 )
        , _usage( 1.0f )
        , _taskID( 0 )
        , _frustum( _data.frustumData )
{
    EQASSERT( parent );
    parent->addCompound( this );
    EQLOG( LOG_INIT ) << "New root compound @" << (void*)this << std::endl;
}

Compound::Compound( Compound* parent )
        : _config( 0 )
        , _parent( parent )
        , _usage( 1.0f )
        , _taskID( 0 )
        , _frustum( _data.frustumData )
{
    EQASSERT( parent );
    parent->_addChild( this );
    EQLOG( LOG_INIT ) << "New compound child @" << (void*)this << std::endl;
}

Compound::~Compound()
{
    _swapBarrier = 0;

    for( Equalizers::const_iterator i = _equalizers.begin();
         i != _equalizers.end(); ++i )
    {
        Equalizer* equalizer = *i;
        equalizer->attach( 0 );
        delete equalizer;
    }
    _equalizers.clear();

    while( !_children.empty( ))
    {
        Compound* compound = _children.back();
        _removeChild( compound );
        delete compound;
    }

    if( _config )
        _config->removeCompound( this );
    else
    {
        EQASSERT( _parent );
        _parent->_removeChild( this );
    }

    for( FramesCIter i = _inputFrames.begin(); i != _inputFrames.end(); ++i )
        delete *i;
    _inputFrames.clear();

    for( FramesCIter i = _outputFrames.begin(); i != _outputFrames.end(); ++i )
        delete *i;
    _outputFrames.clear();

    for( TileQueuesCIter i = _inputTileQueues.begin();
         i != _inputTileQueues.end(); ++i )
    {
        delete *i;
    }
    _inputTileQueues.clear();

    for( TileQueuesCIter i = _outputTileQueues.begin();
         i != _outputTileQueues.end(); ++i )
    {
        delete *i;
    }
    _outputTileQueues.clear();
}

Compound::InheritData::InheritData()
        : channel( 0 )
        , overdraw( Vector4i::ZERO )
        , buffers( eq::Frame::BUFFER_UNDEFINED )
        , eyes( fabric::EYE_UNDEFINED )
        , tasks( fabric::TASK_DEFAULT )
        , period( EQ_UNDEFINED_UINT32 )
        , phase( EQ_UNDEFINED_UINT32 )
        , maxFPS( std::numeric_limits< float >::max( ))
{
    const Global* global = Global::instance();
    for( int i=0; i<IATTR_ALL; ++i )
        iAttributes[i] =
            global->getCompoundIAttribute( static_cast< IAttribute >( i ));
    for( size_t i = 0; i < eq::NUM_EYES; ++i )
        active[ i ] = 0;
}

void Compound::_addChild( Compound* child )
{
    EQASSERT( child->_parent == this );
    _children.push_back( child );
    _fireChildAdded( child );
}

bool Compound::_removeChild( Compound* child )
{
    Compounds::iterator i = stde::find( _children, child );
    if( i == _children.end( ))
        return false;

    _fireChildRemove( child );
    _children.erase( i );
    return true;
}

Compound* Compound::getNext() const
{
    if( !_parent )
        return 0;

    Compounds&          siblings = _parent->_children;
    Compounds::iterator result   = std::find( siblings.begin(), siblings.end(),
                                              this );
    if( result == siblings.end() )
        return 0;
    ++result;
    if( result == siblings.end() )
        return 0;

    return *result;
}

Node* Compound::getNode()
{ 
    Channel* channel = getChannel(); 
    return channel ? channel->getNode() : 0; 
}

ServerPtr Compound::getServer()
{
    return getConfig()->getServer();
}

void Compound::setChannel( Channel* channel )
{ 
    _data.channel = channel;

    // Update swap barrier
    if( !isDestination( ))
        return;

    Segment* segment = channel ? channel->getSegment() : 0;
    if( !segment )
        return;

    SwapBarrierPtr swapBarrier = segment->getSwapBarrier();
    if( swapBarrier )
        setSwapBarrier( swapBarrier );
}

const Channel* Compound::getChannel() const
{
    if( _data.channel )
        return _data.channel;
    if( _parent )
        return _parent->getChannel();
    return 0;
}

Channel* Compound::getChannel()
{
    if( _data.channel )
        return _data.channel;
    if( _parent )
        return _parent->getChannel();
    return 0;
}

Window* Compound::getWindow()
{
    Channel* channel = getChannel();
    if( channel )
        return channel->getWindow();
    return 0;
}

const Window* Compound::getWindow() const
{
    const Channel* channel = getChannel();
    if( channel )
        return channel->getWindow();
    return 0;
}

Pipe* Compound::getPipe()
{
    Channel* channel = getChannel();
    if( channel )
        return channel->getPipe();
    return 0;
}

const Pipe* Compound::getPipe() const
{
    const Channel* channel = getChannel();
    if( channel )
        return channel->getPipe();
    return 0;
}

void Compound::addEqualizer( Equalizer* equalizer )
{
    if( equalizer )
        equalizer->attach( this );
    
    _equalizers.push_back( equalizer );
}

bool Compound::isInheritActive( const Eye eye ) const
{
    const int32_t index = co::base::getIndexOfLastBit( eye );
    EQASSERT( index >= 0 );
    EQASSERT( index < NUM_EYES );
    return _inherit.active[ index ];
}

bool Compound::isLastInheritEye( const Eye eye ) const
{
    int32_t index = co::base::getIndexOfLastBit( eye );
    EQASSERT( index >= 0 );

    while( ++index < NUM_EYES )
        if( _inherit.active[ index ] )
            return false;
    return true;
}

bool Compound::isRunning() const
{
    bool active = false;
    for( size_t i = 0; i < NUM_EYES; ++i )
        active = active || _inherit.active[ i ];

    if( !active )
        return false;

    const Channel* channel = getChannel();
    if( !channel )
        return true;

    if( !channel->isRunning( ))
        return false;

    EQASSERT( _inherit.channel );
    const View* view = _inherit.channel->getView();
    return channel->supportsView( view );
}

//---------------------------------------------------------------------------
// Listener interface
//---------------------------------------------------------------------------
void Compound::addListener( CompoundListener* listener )
{
    EQ_TS_SCOPED( _serverThread );
    _listeners.push_back( listener );
}

void Compound::removeListener(  CompoundListener* listener )
{
    EQ_TS_SCOPED( _serverThread );
    CompoundListeners::iterator i = find( _listeners.begin(), _listeners.end(),
                                          listener );
    if( i != _listeners.end( ))
        _listeners.erase( i );
}

void Compound::fireUpdatePre( const uint32_t frameNumber )
{
    EQ_TS_SCOPED( _serverThread );

    for( CompoundListeners::const_iterator i = _listeners.begin(); 
         i != _listeners.end(); ++i )

        (*i)->notifyUpdatePre( this, frameNumber );
}

void Compound::_fireChildAdded( Compound* child )
{
    EQ_TS_SCOPED( _serverThread );

    for( CompoundListeners::const_iterator i = _listeners.begin(); 
         i != _listeners.end(); ++i )

        (*i)->notifyChildAdded( this, child );
}

void Compound::_fireChildRemove( Compound* child )
{
    EQ_TS_SCOPED( _serverThread );

    for( CompoundListeners::const_iterator i = _listeners.begin(); 
         i != _listeners.end(); ++i )

        (*i)->notifyChildRemove( this, child );
}

//---------------------------------------------------------------------------
// I/O objects access
//---------------------------------------------------------------------------
void Compound::setSwapBarrier( SwapBarrierPtr barrier )
{
    if( barrier.isValid() && barrier->getName().empty( ))
    {
        const Compound* root = getRoot();
        const std::string& rootName = root->getName();
        if( rootName.empty( ))
            barrier->setName( "barrier" );
        else
            barrier->setName( "barrier." + rootName );
    }

    _swapBarrier = barrier; 
}

void Compound::addInputFrame( Frame* frame )
{ 
    EQASSERT( frame );
    if( frame->getName().empty() )
        _setDefaultFrameName( frame );
    _inputFrames.push_back( frame ); 
    frame->setCompound( this );
}

void Compound::addOutputFrame( Frame* frame )
{ 
    if( frame->getName().empty() )
        _setDefaultFrameName( frame );
    _outputFrames.push_back( frame ); 
    frame->setCompound( this );
}

void Compound::addInputTileQueue( TileQueue* tileQueue )
{
    EQASSERT( tileQueue );
    if( tileQueue->getName().empty() )
        _setDefaultTileQueueName( tileQueue );
    _inputTileQueues.push_back( tileQueue ); 
    tileQueue->setCompound( this );
}

void Compound::removeInputTileQueue( TileQueue* tileQueue )
{
    TileQueuesIter i;
    i = find (_inputTileQueues.begin(), _inputTileQueues.end(), tileQueue);
    if ( i != _inputTileQueues.end() )
        _inputTileQueues.erase( i );
}

void Compound::addOutputTileQueue( TileQueue* tileQueue )
{
    if( tileQueue->getName().empty() )
        _setDefaultTileQueueName( tileQueue );
    _outputTileQueues.push_back( tileQueue ); 
    tileQueue->setCompound( this );
}

void Compound::removeOutputTileQueue( TileQueue* tileQueue )
{
    TileQueuesIter i;
    i = find (_outputTileQueues.begin(), _outputTileQueues.end(), tileQueue);
    if ( i != _outputTileQueues.end() )
        _outputTileQueues.erase( i );
}

void Compound::_setDefaultFrameName( Frame* frame )
{
    for( Compound* compound = this; compound; compound = compound->getParent())
    {
        if( !compound->getName().empty( ))
        {
            frame->setName( "frame." + compound->getName( ));
            return;
        }

        const Channel* channel = compound->getChannel();
        if( channel && !channel->getName().empty( ))
        {
            frame->setName( "frame." + channel->getName( ));
            return;
        }
    }
    frame->setName( "frame" );
}

void Compound::_setDefaultTileQueueName( TileQueue* tileQueue )
{
    for( Compound* compound = this; compound; compound = compound->getParent())
    {
        if( !compound->getName().empty( ))
        {
            tileQueue->setName( "queue." + compound->getName( ));
            return;
        }

        const Channel* channel = compound->getChannel();
        if( channel && !channel->getName().empty( ))
        {
            tileQueue->setName( "queue." + channel->getName( ));
            return;
        }
    }
    tileQueue->setName( "queue" );
}

void Compound::adopt( Compound* child )
{
    if( child->_config )
    {
        child->_config->removeCompound( child );
        const_cast< Config*& >( child->_config ) = 0;
    }
    else
    {
        EQASSERT( child->_parent );
        child->_parent->_removeChild( child );
    }

    const_cast< Compound*& >( child->_parent ) = this;
    _addChild( child );
}

bool Compound::isDestination() const
{
    if( !getChannel( ))
        return false;

    for( const Compound* compound = getParent(); compound;
         compound = compound->getParent( ))
    {
        if( compound->getChannel( ))
            return false;
    }

    return true;
}

bool Compound::hasDestinationChannel() const
{
    return getChannel() && getChannel() == getInheritChannel();
}

//---------------------------------------------------------------------------
// frustum operations
//---------------------------------------------------------------------------
void Compound::setWall( const Wall& wall )
{
    _frustum.setWall( wall );
    EQVERB << "Wall: " << _data.frustumData << std::endl;
}

void Compound::setProjection( const Projection& projection )
{
    _frustum.setProjection( projection );
    EQVERB << "Projection: " << _data.frustumData << std::endl;
}

void Compound::updateFrustum( const Vector3f& eye, const float ratio )
{
    if( !isDestination( )) // only set view/segment frusta on destination
        return;

    Channel* channel = getChannel();
    Segment* segment = channel->getSegment();
    const View* view = channel->getView();
    if( !segment || !view )
        return;

    if( view->getCurrentType() != Frustum::TYPE_NONE ) // frustum from view:
    {
        // set compound frustum =
        //         segment frustum X channel/view coverage
        const Viewport& segmentVP = segment->getViewport();
        const Viewport& viewVP    = view->getViewport();
        const Viewport  coverage  = viewVP.getCoverage( segmentVP );

        Wall wall( view->getWall( ));

        wall.apply( coverage );
        wall.moveFocus( eye, ratio );
        _updateOverdraw( wall );

        switch( view->getCurrentType( ))
        {
            case Frustum::TYPE_WALL:
                setWall( wall );
                EQLOG( LOG_VIEW ) << "View wall for " << channel->getName() 
                                  << ": " << wall << std::endl;
                return;

            case Frustum::TYPE_PROJECTION:
            {
                Projection projection( view->getProjection( )); // keep distance
                projection = wall;
                setProjection( projection );
                EQLOG( LOG_VIEW ) << "View projection for " <<channel->getName()
                                  << ": " << projection << std::endl;
                return;
            }

            default:
                EQUNIMPLEMENTED;
        }
    }
    // else frustum from segment

    if( segment->getCurrentType() == Frustum::TYPE_NONE )
    {
        EQASSERT( segment->getCanvas()->getCurrentType() != Frustum::TYPE_NONE);
        segment->notifyFrustumChanged();
    }

    // set compound frustum =
    //         segment frustum X channel/segment coverage
    const Channel* outputChannel = segment->getChannel();
    EQASSERT( outputChannel );

    const Viewport& outputVP  = outputChannel->getViewport();
    const Viewport& channelVP = channel->getViewport();
    const Viewport  coverage  = outputVP.getCoverage( channelVP );

    Wall wall( segment->getWall( ));
    wall.moveFocus( eye, ratio );
    wall.apply( coverage );
    _updateOverdraw( wall );

    switch( segment->getCurrentType( ))
    {
        case Frustum::TYPE_WALL:
        {
            setWall( wall );
            EQLOG( LOG_VIEW ) << "Segment wall for " << channel->getName()
                              << ": " << wall << std::endl;
            return;
        }

        case Frustum::TYPE_PROJECTION:
        {
            Projection projection( segment->getProjection( ));
            projection = wall;
            setProjection( projection );
            EQLOG( LOG_VIEW ) << "Segment projection for " 
                              << channel->getName() << ": " << projection
                              << std::endl;
            return;
        }
        default: 
            EQUNIMPLEMENTED;
    }
}

void Compound::computeFrustum( RenderContext& context, 
                               const fabric::Eye eye ) const
{
    // compute eye position in screen space
    const Vector3f eyeWorld = _getEyePosition( eye );
    const FrustumData& frustumData = getInheritFrustumData();
    const Matrix4f& xfm = frustumData.getTransform();
    const Vector3f eyeWall = xfm * eyeWorld;

    EQVERB << "Eye position world: " << eyeWorld << " wall " << eyeWall
        << std::endl;
    _computePerspective( context, eyeWall );
    _computeOrtho( context, eyeWall );
}

void Compound::computeTileFrustum( Frustumf& frustum, const fabric::Eye eye,
                                   Viewport vp, bool ortho ) const
{
    const Vector3f eyeWorld = _getEyePosition( eye );
    const FrustumData& frustumData = getInheritFrustumData();
    const Matrix4f& xfm = frustumData.getTransform();
    const Vector3f eyeWall = xfm * eyeWorld;

    _computeFrustumCorners( frustum, frustumData, eyeWall, ortho, &vp);
}

namespace
{
    static void _computeHeadTransform( Matrix4f& result, const Matrix4f& xfm,
                                       const Vector3f& eye )
    {
        // headTransform = -trans(eye) * view matrix (frustum position)
        for( int i=0; i<16; i += 4 )
        {
            result.array[i]   = xfm.array[i]   - eye[0] * xfm.array[i+3];
            result.array[i+1] = xfm.array[i+1] - eye[1] * xfm.array[i+3];
            result.array[i+2] = xfm.array[i+2] - eye[2] * xfm.array[i+3];
            result.array[i+3] = xfm.array[i+3];
        }
    }
}

void Compound::_computePerspective( RenderContext& context, 
                                    const Vector3f& eye ) const
{
    const FrustumData& frustumData = getInheritFrustumData();

    _computeFrustumCorners( context.frustum, frustumData, eye, false);
    _computeHeadTransform( context.headTransform, frustumData.getTransform(),
        eye );

    const bool isHMD = (frustumData.getType() != Wall::TYPE_FIXED);
    if( isHMD )
        context.headTransform *= _getInverseHeadMatrix();
}

void Compound::_computeOrtho( RenderContext& context, const Vector3f& eye) const
{
    // Compute corners for cyclop eye without perspective correction:
    const Vector3f cyclopWorld = _getEyePosition( EYE_CYCLOP );
    const FrustumData& frustumData = getInheritFrustumData();
    const Matrix4f& xfm = frustumData.getTransform();
    const Vector3f cyclopWall = xfm * cyclopWorld;

    _computeFrustumCorners( context.ortho, frustumData, cyclopWall, true );
    _computeHeadTransform( context.orthoTransform, xfm, eye );

    // Apply stereo shearing
    context.orthoTransform.array[8] += (cyclopWall[0] - eye[0]) / eye[2];
    context.orthoTransform.array[9] += (cyclopWall[1] - eye[1]) / eye[2];

    const bool isHMD = (frustumData.getType() != Wall::TYPE_FIXED);
    if( isHMD )
        context.orthoTransform *= _getInverseHeadMatrix();
}

Vector3f Compound::_getEyePosition( const fabric::Eye eye ) const
{
    const FrustumData& frustumData = getInheritFrustumData();
    const Channel* destChannel = getInheritChannel();
    const View* view = destChannel->getView();
    const Observer* observer = view ? view->getObserver() : 0;

    if( observer && frustumData.getType() == Wall::TYPE_FIXED )
        return observer->getEyePosition( eye );

    const Config* config = getConfig();
    const float eyeBase_2 = 0.5f * ( observer ? 
        observer->getEyeBase() : 
        config->getFAttribute( Config::FATTR_EYE_BASE ));

    switch( eye )
    {
    case EYE_LEFT:
        return Vector3f(-eyeBase_2, 0.f, 0.f );

    case EYE_RIGHT:
        return Vector3f( eyeBase_2, 0.f, 0.f );

    default:
        EQUNIMPLEMENTED;
    case EYE_CYCLOP:
        return Vector3f::ZERO;
    }
}

const Matrix4f& Compound::_getInverseHeadMatrix() const
{
    const Channel* destChannel = getInheritChannel();
    const View* view = destChannel->getView();
    const Observer* observer = static_cast< const Observer* >(
        view ? view->getObserver() : 0);

    if( observer )
        return observer->getInverseHeadMatrix();

    return Matrix4f::IDENTITY;
}

void Compound::_computeFrustumCorners( Frustumf& frustum,
                                       const FrustumData& frustumData,
                                       const Vector3f& eye,
                                       const bool ortho,
                                       Viewport* invp /* = 0*/) const
{
    const Channel* destination = getInheritChannel();
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
    const Pixel& pixel = getInheritPixel();
    if( pixel != Pixel::ALL && pixel.isValid( ))
    {
        const Channel* inheritChannel = getInheritChannel();
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
            const float frustumHeight = frustum.bottom() - frustum.top();
            const float pixelHeight = frustumHeight / float( destPVP.h );
            const float jitter = pixelHeight * pixel.y + pixelHeight * .5f;

            frustum.top()    -= jitter;
            frustum.bottom() -= jitter;
        }
    }

    // adjust to viewport (screen-space decomposition)
    // Note: vp is computed pixel-correct by Compound::updateInheritData()
    const Viewport vp = invp ? *invp : getInheritViewport();
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

void Compound::_updateOverdraw( Wall& wall )
{
    Channel* channel = getChannel();
    EQASSERT( channel );
    if( !channel )
        return;

    const Segment* segment = channel->getSegment();
    const View*    view    = channel->getView();
    EQASSERT( segment && view );
    if( !segment || !view )
        return;

    const Viewport& segmentVP = segment->getViewport();
    const Viewport& viewVP    = view->getViewport();
    const Vector2i& overdraw  = view->getOverdraw();
    Vector4i channelOverdraw( Vector4i::ZERO );

    // compute overdraw
    if( overdraw.x() && viewVP.x < segmentVP.x )
        channelOverdraw.x() = overdraw.x();

    if( overdraw.x() && viewVP.getXEnd() > segmentVP.getXEnd( ))
        channelOverdraw.z() = overdraw.x();

    if( overdraw.y() && viewVP.y < segmentVP.y )
        channelOverdraw.y() = overdraw.y();

    if( overdraw.y() && viewVP.getYEnd() > segmentVP.getYEnd( ))
        channelOverdraw.w() = overdraw.y();

    // clamp to max channel size
    const Vector2i& maxSize = channel->getMaxSize();
    if( maxSize != Vector2i::ZERO )
    {
        const PixelViewport& channelPVP = channel->getPixelViewport();

        const int32_t xOverdraw = channelOverdraw.x() + channelOverdraw.z();
        const int32_t xSize = xOverdraw + channelPVP.w;
        if( xSize > maxSize.x( ))
        {
            const uint32_t maxOverdraw = maxSize.x() - channelPVP.w;
            const float ratio = static_cast< float >( maxOverdraw ) / 
                                static_cast< float >( xOverdraw );
            channelOverdraw.x() = static_cast< int >(
                channelOverdraw.x() * ratio + .5f );
            channelOverdraw.z() = maxOverdraw - channelOverdraw.x();
        }

        const int32_t yOverdraw = channelOverdraw.y() + channelOverdraw.w();
        const int32_t ySize = yOverdraw + channelPVP.h;
        if( ySize > maxSize.y( ))
        {
            const uint32_t maxOverdraw = maxSize.y() - channelPVP.h;
            const float ratio = static_cast< float >( maxOverdraw ) / 
                                static_cast< float >( yOverdraw );
            channelOverdraw.y() = static_cast< int >(
                channelOverdraw.y() * ratio +.5f );
            channelOverdraw.w() = maxOverdraw - channelOverdraw.y();
        }
    }

    // apply to frustum
    if( channelOverdraw.x() > 0 )
    {
        const PixelViewport& pvp = channel->getPixelViewport();
        const float ratio = static_cast<float>( pvp.w + channelOverdraw.x( )) /
                            static_cast<float>( pvp.w );
        wall.resizeLeft( ratio );
    }

    if( channelOverdraw.z() > 0 )
    {
        const PixelViewport& pvp = channel->getPixelViewport();
        const float ratio = static_cast<float>( pvp.w + channelOverdraw.x( ) + 
                                                channelOverdraw.z( )) /
                            static_cast<float>( pvp.w + channelOverdraw.x( ));
        wall.resizeRight( ratio );
    }

    if( channelOverdraw.y() > 0 )
    {
        const PixelViewport& pvp = channel->getPixelViewport();
        const float ratio = static_cast<float>( pvp.h + channelOverdraw.y( )) /
                            static_cast<float>( pvp.h );
        wall.resizeBottom( ratio );
    }

    if( channelOverdraw.w() > 0 )
    {
        const PixelViewport& pvp = channel->getPixelViewport();
        const float ratio = static_cast<float>( pvp.h + + channelOverdraw.y( ) +
                                                channelOverdraw.w( )) /
                            static_cast<float>( pvp.h + channelOverdraw.y( ));
        wall.resizeTop( ratio );
    }

    channel->setOverdraw( channelOverdraw );
}

//---------------------------------------------------------------------------
// accept
//---------------------------------------------------------------------------
namespace
{
template< class C > 
VisitorResult _accept( C* compound, CompoundVisitor& visitor )
{
    if( compound->isLeaf( )) 
        return visitor.visitLeaf( compound );

    C* current = compound;
    VisitorResult result = TRAVERSE_CONTINUE;

    while( true )
    {
        C* parent = current->getParent();
        C* next   = current->getNext();

        const Compounds& children = current->getChildren();
        C* child  = children.empty() ? 0 : children[0];

        //---------- down-right traversal
        if ( !child ) // leaf
        {
            switch( visitor.visitLeaf( current ))
            {
                case TRAVERSE_TERMINATE:
                    return TRAVERSE_TERMINATE;

                case TRAVERSE_PRUNE:
                    result = TRAVERSE_PRUNE;
                    current = next;
                    break;

                case TRAVERSE_CONTINUE:
                    current = next;
                    break;

                default:
                    EQASSERTINFO( 0, "Unreachable" );
            }
        } 
        else // node
        {
            switch( visitor.visitPre( current ))
            {
                case TRAVERSE_TERMINATE:
                    return TRAVERSE_TERMINATE;
                    
                case TRAVERSE_PRUNE:
                    result = TRAVERSE_PRUNE;
                    current = next;
                    break;

                case TRAVERSE_CONTINUE:
                    current = child;
                    break;

                default:
                    EQASSERTINFO( 0, "Unreachable" );
            }
        }

        //---------- up-right traversal
        if( !current && !parent ) return TRAVERSE_CONTINUE;

        while( !current )
        {
            current = parent;
            parent  = current->getParent();
            next    = current->getNext();

            switch( visitor.visitPost( current ))
            {
                case TRAVERSE_TERMINATE:
                    return TRAVERSE_TERMINATE;

                case TRAVERSE_PRUNE:
                    result = TRAVERSE_PRUNE;
                    break;

                case TRAVERSE_CONTINUE:
                    break;

                default:
                    EQASSERTINFO( 0, "Unreachable" );
            }
            
            if ( current == compound ) 
                return result;
            
            current = next;
        }
    }
    return result;
}
}

VisitorResult Compound::accept( CompoundVisitor& visitor )
{
    return _accept( this, visitor );
}

VisitorResult Compound::accept( CompoundVisitor& visitor ) const
{
    return _accept( this, visitor );
}

//---------------------------------------------------------------------------
// Operations
//---------------------------------------------------------------------------

void Compound::activate( const uint32_t eyes )
{
    for( size_t i = 0; i < NUM_EYES; ++i )
    {
        const fabric::Eye eye = static_cast< Eye >( 1 << i );
        if( !(eyes & eye) )
            continue;

        ++_data.active[ i ];
        if( !getChannel( )) // non-dest root compound
            continue;

        CompoundActivateVisitor channelActivate( true, eye );
        accept( channelActivate );
    }
}

void Compound::deactivate( const uint32_t eyes )
{
    for( size_t i = 0; i < NUM_EYES; ++i )
    {
        const fabric::Eye eye = static_cast< Eye >( 1 << i );
        if( !(eyes & eye) )
            continue;

        EQASSERT( _data.active[ i ] );
        --_data.active[ i ];
        if( !getChannel( )) // non-dest root compound
            continue;

        CompoundActivateVisitor channelDeactivate( false, eye );
        accept( channelDeactivate );
    }
}

void Compound::init()
{
    CompoundInitVisitor initVisitor;
    accept( initVisitor );
}

void Compound::exit()
{
    CompoundExitVisitor visitor;
    accept( visitor );
}

void Compound::register_()
{
    ServerPtr server = getServer();
    const uint32_t latency = getConfig()->getLatency();

    for( Frames::const_iterator i = _outputFrames.begin(); 
         i != _outputFrames.end(); ++i )
    {
        Frame* frame = *i;
        server->registerObject( frame );
        frame->setAutoObsolete( latency );
        EQLOG( eq::LOG_ASSEMBLY ) << "Output frame \"" << frame->getName() 
                                  << "\" id " << frame->getID() << std::endl;
    }

    for( Frames::const_iterator i = _inputFrames.begin(); 
         i != _inputFrames.end(); ++i )
    {
        Frame* frame = *i;
        server->registerObject( frame );
        frame->setAutoObsolete( latency );
        EQLOG( eq::LOG_ASSEMBLY ) << "Input frame \"" << frame->getName() 
                                  << "\" id " << frame->getID() << std::endl;
    }

    for( TileQueuesCIter i = _inputTileQueues.begin();
         i != _inputTileQueues.end(); ++i )
    {
        TileQueue* queue = *i;
        server->registerObject( queue );
        queue->setAutoObsolete( latency );
        EQLOG( eq::LOG_ASSEMBLY ) << "Input queue \"" << queue->getName() 
                                  << "\" id " << queue->getID() << std::endl;
    }

    for( TileQueuesCIter i = _outputTileQueues.begin();
         i != _outputTileQueues.end(); ++i )
    {
        TileQueue* queue = *i;
        server->registerObject( queue );
        queue->setAutoObsolete( latency );
        EQLOG( eq::LOG_ASSEMBLY ) << "Output queue \"" << queue->getName() 
                                  << "\" id " << queue->getID() << std::endl;
    }
}

void Compound::deregister()
{
    ServerPtr server = getServer();

    for( Frames::const_iterator i = _outputFrames.begin(); 
         i != _outputFrames.end(); ++i )
    {
        Frame* frame = *i;
        frame->flush();
        server->deregisterObject( frame );
    }

    for( Frames::const_iterator i = _inputFrames.begin(); 
         i != _inputFrames.end(); ++i )
    {
        Frame* frame = *i;
        server->deregisterObject( frame );
    }

    for( TileQueuesCIter i = _inputTileQueues.begin();
        i != _inputTileQueues.end(); ++i )
    {
        TileQueue* queue = *i;
        server->deregisterObject( queue );
    }

    for( TileQueuesCIter i = _outputTileQueues.begin();
        i != _outputTileQueues.end(); ++i )
    {
        TileQueue* queue = *i;
        queue->flush();
        server->deregisterObject( queue );
    }
}

//---------------------------------------------------------------------------
// pre-render compound state update
//---------------------------------------------------------------------------
void Compound::update( const uint32_t frameNumber )
{
    CompoundUpdateDataVisitor updateDataVisitor( frameNumber );
    accept( updateDataVisitor );

    CompoundUpdateOutputVisitor updateOutputVisitor( frameNumber );
    accept( updateOutputVisitor );

    const FrameMap& outputFrames = updateOutputVisitor.getOutputFrames();
    const TileQueueMap& outputQueues = updateOutputVisitor.getOutputQueues();
    CompoundUpdateInputVisitor updateInputVisitor( outputFrames, outputQueues );
    accept( updateInputVisitor );

    // commit output frames after input frames have been set
    for( FrameMapCIter i = outputFrames.begin(); i != outputFrames.end(); ++i )
    {
        Frame* frame = i->second;
        frame->commit();
    }

    const BarrierMap& swapBarriers = updateOutputVisitor.getSwapBarriers();
    for( BarrierMapCIter i = swapBarriers.begin(); i != swapBarriers.end(); ++i)
    {
        co::Barrier* barrier = i->second;
        if( barrier->isAttached( ))
        {
            if( barrier->getHeight() > 1 )
                barrier->commit();
        }
        else
        {
            getServer()->registerObject( barrier );
            barrier->setAutoObsolete( getConfig()->getLatency() + 1 );
        }
    }
}

void Compound::updateInheritData( const uint32_t frameNumber )
{
    _data.pixel.validate();
    _data.subpixel.validate();
    _data.zoom.validate();
    const PixelViewport oldPVP( _inherit.pvp );

    if( isRoot( ))
        _updateInheritRoot( oldPVP );
    else
        _updateInheritNode( oldPVP );

    if( _inherit.channel )
    {
        _updateInheritStereo();
        _updateInheritActive( frameNumber );
    }

    if( _inherit.pvp.isValid( ))
    {
        _inherit.pvp.apply( _data.pixel );

        // Zoom
        const PixelViewport unzoomedPVP( _inherit.pvp );
        _inherit.pvp.apply( _data.zoom );

        // Compute the inherit zoom to be pixel-correct with the integer-rounded
        // pvp.
        const Zoom zoom = _inherit.pvp.getZoom( unzoomedPVP );
        _inherit.zoom *= zoom;
    }

    // Tasks
    updateInheritTasks();

    const View* view = _inherit.channel ? _inherit.channel->getView() : 0;
    const Channel* channel = getChannel();
    if( channel && !channel->supportsView( view ))
        _inherit.tasks = fabric::TASK_NONE;

    if( !_inherit.pvp.hasArea() || !_inherit.range.hasData( ))
        // Channels with no PVP or range do not execute tasks
        _inherit.tasks = fabric::TASK_NONE;
}

void Compound::_updateInheritRoot( const PixelViewport& oldPVP )
{
    EQASSERT( !_parent );
    _inherit = _data;
    _inherit.zoom = Zoom::NONE; // will be reapplied below
    _updateInheritPVP( oldPVP );

    if( _inherit.eyes == fabric::EYE_UNDEFINED )
        _inherit.eyes = fabric::EYES_ALL;
    else if( _inherit.channel )
    {
        const View* view = _inherit.channel->getView();
        if( !view )
            _inherit.eyes = EYE_CYCLOP;
    }

    if( _inherit.period == EQ_UNDEFINED_UINT32 )
        _inherit.period = 1;

    if( _inherit.phase == EQ_UNDEFINED_UINT32 )
        _inherit.phase = 0;

    if( _inherit.buffers == eq::Frame::BUFFER_UNDEFINED )
        _inherit.buffers = eq::Frame::BUFFER_COLOR;

    if( _inherit.iAttributes[IATTR_STEREO_MODE] == UNDEFINED )
        _inherit.iAttributes[IATTR_STEREO_MODE] = fabric::AUTO;

    if( _inherit.iAttributes[IATTR_STEREO_ANAGLYPH_LEFT_MASK] == UNDEFINED )
        _inherit.iAttributes[IATTR_STEREO_ANAGLYPH_LEFT_MASK] = COLOR_MASK_RED;

    if( _inherit.iAttributes[IATTR_STEREO_ANAGLYPH_RIGHT_MASK] == UNDEFINED )
        _inherit.iAttributes[IATTR_STEREO_ANAGLYPH_RIGHT_MASK] =
            COLOR_MASK_GREEN | COLOR_MASK_BLUE;
}

void Compound::_updateInheritNode( const PixelViewport& oldPVP )
{
    EQASSERT( _parent );
    _inherit = _parent->_inherit;

    if( !_inherit.channel )
    {
        _updateInheritPVP( oldPVP );
        _inherit.vp.apply( _data.vp );
    }
    else if( _inherit.pvp.isValid( ))
    {
        EQASSERT( _data.vp.isValid( ));
        _inherit.pvp.apply( _data.vp );

        // Compute the inherit viewport to be pixel-correct with the integer-
        // rounded pvp. This is needed to calculate the frustum correctly.
        const Viewport vp = _inherit.pvp.getSubVP( _parent->_inherit.pvp );
        _inherit.vp.apply( vp );
            
        _updateInheritOverdraw();
    }
    else
    {
        EQASSERT( !_inherit.channel->isRunning( ));
    }

    if( _data.frustumData.isValid( ))
        _inherit.frustumData = _data.frustumData;

    _inherit.range.apply( _data.range );
    _inherit.pixel.apply( _data.pixel );
    _inherit.subpixel.apply( _data.subpixel );

    if( _data.eyes != fabric::EYE_UNDEFINED )
        _inherit.eyes = _data.eyes;
    else if( _inherit.channel )
    {
        const View* view = _inherit.channel->getView();
        if( !view )
            _inherit.eyes = EYE_CYCLOP;
    }
        
    if( _data.period != EQ_UNDEFINED_UINT32 )
        _inherit.period = _data.period;

    if( _data.phase != EQ_UNDEFINED_UINT32 )
        _inherit.phase = _data.phase;

    _inherit.maxFPS = _data.maxFPS;

    if( _data.buffers != eq::Frame::BUFFER_UNDEFINED )
        _inherit.buffers = _data.buffers;
        
    if( _data.iAttributes[IATTR_STEREO_MODE] != UNDEFINED )
        _inherit.iAttributes[IATTR_STEREO_MODE] =
            _data.iAttributes[IATTR_STEREO_MODE];

    if( _data.iAttributes[IATTR_STEREO_ANAGLYPH_LEFT_MASK] != UNDEFINED )
        _inherit.iAttributes[IATTR_STEREO_ANAGLYPH_LEFT_MASK] = 
            _data.iAttributes[IATTR_STEREO_ANAGLYPH_LEFT_MASK];

    if( _data.iAttributes[IATTR_STEREO_ANAGLYPH_RIGHT_MASK] != UNDEFINED )
        _inherit.iAttributes[IATTR_STEREO_ANAGLYPH_RIGHT_MASK] = 
            _data.iAttributes[IATTR_STEREO_ANAGLYPH_RIGHT_MASK];
}

void Compound::_updateInheritPVP( const PixelViewport& oldPVP )
{
    Channel* channel = _data.channel;
    if( !channel )
        return;

    _inherit.channel = channel;
    _inherit.pvp = channel->getPixelViewport( );

    View* view = channel->getView();
    if( !view || !_inherit.pvp.isValid( ))
    {
        EQASSERT( channel->getOverdraw() == Vector4i::ZERO );
        return;
    }
    EQASSERT( channel == getChannel( ));

    // enlarge pvp by overdraw
    const Vector4i& overdraw = channel->getOverdraw();
    _inherit.pvp.w += overdraw.x() + overdraw.z();
    _inherit.pvp.h += overdraw.y() + overdraw.w();

    if( oldPVP != _inherit.pvp ) // channel PVP changed
    {
        view->updateFrusta();
        EQASSERT( overdraw == channel->getOverdraw( ));
    }

    _inherit.overdraw = overdraw;
}

void Compound::_updateInheritOverdraw()
{
    const PixelViewport& pvp = _inherit.pvp;
    const PixelViewport& parentPVP = _parent->_inherit.pvp;

    _inherit.overdraw.x() -= pvp.x - parentPVP.x;
    _inherit.overdraw.y() -= pvp.y - parentPVP.y;
    _inherit.overdraw.z() -= parentPVP.getXEnd() - pvp.getXEnd();
    _inherit.overdraw.w() -= parentPVP.getYEnd() - pvp.getYEnd();

    _inherit.overdraw.x() = EQ_MAX( _inherit.overdraw.x(), 0 );
    _inherit.overdraw.y() = EQ_MAX( _inherit.overdraw.y(), 0 );
    _inherit.overdraw.z() = EQ_MAX( _inherit.overdraw.z(), 0 );
    _inherit.overdraw.w() = EQ_MAX( _inherit.overdraw.w(), 0 );

    _inherit.overdraw.x() = EQ_MIN( _inherit.overdraw.x(), pvp.w );
    _inherit.overdraw.y() = EQ_MIN( _inherit.overdraw.y(), pvp.h );
    _inherit.overdraw.z() = EQ_MIN( _inherit.overdraw.z(), pvp.w );
    _inherit.overdraw.w() = EQ_MIN( _inherit.overdraw.w(), pvp.h );

    EQASSERTINFO( pvp.w >= _inherit.overdraw.x() + _inherit.overdraw.z(), 
                  pvp.w << " < " << 
                  _inherit.overdraw.x() + _inherit.overdraw.z( ));
    EQASSERTINFO( pvp.h >= _inherit.overdraw.y() + _inherit.overdraw.w(), 
                  pvp.h << " < " <<
                  _inherit.overdraw.y() + _inherit.overdraw.w( ));
}

void Compound::updateInheritTasks()
{
    if( _data.tasks == fabric::TASK_DEFAULT )
    {
        if( isLeaf( ))
        {
            _inherit.tasks = fabric::TASK_ALL;
            // check if a parent compound has cleared us
            for( Compound* compound = getParent(); compound;
                 compound = compound->getParent( ))
            {
                Channel* channel = compound->getChannel();
                if( channel == getChannel( ))
                    _inherit.tasks &= ~fabric::TASK_CLEAR; // done already
            }
        }
        else
            _inherit.tasks = fabric::TASK_CLEAR | fabric::TASK_ASSEMBLE |
                             fabric::TASK_READBACK;
    }
    else
        _inherit.tasks = _data.tasks;

    const Channel* channel = getChannel();
    if( isDestination() && channel->getView( ))
        _inherit.tasks |= fabric::TASK_VIEW;
    else
        _inherit.tasks &= ~fabric::TASK_VIEW;
}

void Compound::_updateInheritStereo()
{
    if( _inherit.iAttributes[IATTR_STEREO_MODE] != fabric::AUTO )
        return;

    const Segment* segment = _inherit.channel->getSegment();
    const uint32_t eyes = segment ? segment->getEyes() : _inherit.eyes;
    const bool stereoEyes = ( eyes & EYES_STEREO ) == EYES_STEREO;
    if( !stereoEyes )
    {
        _inherit.iAttributes[IATTR_STEREO_MODE] = fabric::PASSIVE;
        return;
    }

    const Window* window = _inherit.channel->getWindow();
    const bool stereoWindow = window->getDrawableConfig().stereo;
    const bool usesFBO =  window && 
        (( window->getIAttribute(Window::IATTR_HINT_DRAWABLE) == fabric::FBO) ||
         _inherit.channel->getDrawable() != Channel::FB_WINDOW );

    if( stereoWindow && !usesFBO )
        _inherit.iAttributes[IATTR_STEREO_MODE] = fabric::QUAD;
    else
        _inherit.iAttributes[IATTR_STEREO_MODE] = fabric::ANAGLYPH;
}

void Compound::_updateInheritActive( const uint32_t frameNumber )
{
    const bool phaseActive = ((frameNumber%_inherit.period) == _inherit.phase );
    const bool channelActive = _inherit.channel->isRunning(); // runtime failure

    for( size_t i = 0; i < fabric::NUM_EYES; ++i )
    {
        const uint32_t eye = 1 << i;
        const bool destActive = isDestination() ? _data.active[i] :
                                                  _inherit.active[i];
        const bool eyeActive = _inherit.eyes & eye;

        if( destActive && eyeActive && phaseActive && channelActive )
            _inherit.active[i] = 1;
        else
            _inherit.active[i] = 0; // deactivate
    }
}

std::ostream& operator << (std::ostream& os, const Compound& compound)
{
    os << co::base::disableFlush << "compound" << std::endl;
    os << "{" << std::endl << co::base::indent;
      
    const std::string& name = compound.getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << std::endl;

    const Channel* channel = compound.getChannel();
    if( channel )
    {
        Compound* parent = compound.getParent();
        if( !parent || parent->getChannel() != channel )
        {
            const std::string& channelName = channel->getName();
            const Config*      config      = compound.getConfig();
            EQASSERT( config );

            if( !channelName.empty() && 
                config->find< Channel >( channelName ) == channel )
            {
                os << "channel  \"" << channelName << "\"" << std::endl;
            }
            else
            {
                const Segment* segment = channel->getSegment();
                const View*    view    = channel->getView();

                if( view && segment )
                {
                    os << "channel  ( ";

                    const Canvas* canvas = segment->getCanvas();
                    const std::string& canvasName = canvas->getName();
                    if( !canvasName.empty() && 
                        config->find< Canvas >( canvasName ) == canvas )
                    {
                        os << "canvas \"" << canvasName << "\"  ";
                    }
                    else
                        os << canvas->getPath() << "  ";

                    const std::string& segmentName = segment->getName();
                    if( !segmentName.empty() && 
                        canvas->findSegment( segmentName ) == segment )
                    {
                        os << "segment \"" << segmentName << "\"   ";
                    }
                    else
                        os << "segment " << segment->getPath().segmentIndex
                           << "   ";

                    const Layout* layout = view->getLayout();
                    const std::string& layoutName = layout->getName();
                    if( !layoutName.empty() && 
                        config->find< Layout >( layoutName ) == layout )
                    {
                        os << "layout \"" << layoutName << "\"  ";
                    }
                    else
                        os << layout->getPath() << "  ";

                    const std::string& viewName = view->getName();
                    if( !viewName.empty() && 
                        config->find< View >( viewName ) == view )
                    {
                        os << "view \"" << viewName << '\"';
                    }
                    else
                        os << "view " << view->getPath().viewIndex;

                    os << " )" << std::endl; 
                }
                else
                    os << "channel  ( " << channel->getPath() << " )"
                       << std::endl;
            }
        }
    }

    const uint32_t tasks = compound.getTasks();
    if( tasks != fabric::TASK_DEFAULT )
    {
        os << "task     [";
        if( tasks &  fabric::TASK_CLEAR )    os << " CLEAR";
        if( tasks &  fabric::TASK_CULL )     os << " CULL";
        if( compound.isLeaf() && 
            ( tasks &  fabric::TASK_DRAW ))  os << " DRAW";
        if( tasks &  fabric::TASK_ASSEMBLE ) os << " ASSEMBLE";
        if( tasks &  fabric::TASK_READBACK ) os << " READBACK";
        os << " ]" << std::endl;
    }

    const uint32_t buffers = compound.getBuffers();
    if( buffers != eq::Frame::BUFFER_UNDEFINED )
    {
        os << "buffers  [";
        if( buffers & eq::Frame::BUFFER_COLOR )  os << " COLOR";
        if( buffers & eq::Frame::BUFFER_DEPTH )  os << " DEPTH";
        os << " ]" << std::endl;
    }

    const Viewport& vp = compound.getViewport();
    if( vp.isValid() && vp != Viewport::FULL )
        os << "viewport " << vp << std::endl;
    
    const Range& range = compound.getRange();
    if( range.isValid() && range != Range::ALL )
        os << range << std::endl;

    const Pixel& pixel = compound.getPixel();
    if( pixel.isValid() && pixel != Pixel::ALL )
        os << pixel << std::endl;

    const SubPixel& subpixel = compound.getSubPixel();
    if( subpixel.isValid() && subpixel != SubPixel::ALL )
            os << subpixel << std::endl;

    const Zoom& zoom = compound.getZoom();
    if( zoom.isValid() && zoom != Zoom::NONE )
        os << zoom << std::endl;

    const uint32_t eye = compound.getEyes();
    if( eye )
    {
        os << "eye      [ ";
        if( eye & fabric::EYE_CYCLOP )
            os << "CYCLOP ";
        if( eye & fabric::EYE_LEFT )
            os << "LEFT ";
        if( eye & fabric::EYE_RIGHT )
            os << "RIGHT ";
        os << "]" << std::endl;
    }

    const uint32_t period = compound.getPeriod();
    const uint32_t phase  = compound.getPhase();
    if( period != EQ_UNDEFINED_UINT32 )
        os << "period " << period << "  ";

    if( phase != EQ_UNDEFINED_UINT32 )
        os << "phase " << phase;

    if( period != EQ_UNDEFINED_UINT32 || phase != EQ_UNDEFINED_UINT32 )
        os << std::endl;

    // attributes
    bool attrPrinted = false;
    
    for( Compound::IAttribute i = static_cast< Compound::IAttribute >( 0 );
         i<Compound::IATTR_ALL;
         i = static_cast< Compound::IAttribute >( uint32_t( i ) + 1 ))
    {
        const int value = compound.getIAttribute( i );
        if( value == Global::instance()->getCompoundIAttribute( i ))
            continue;

        if( !attrPrinted )
        {
            os << std::endl << "attributes" << std::endl;
            os << "{" << std::endl << co::base::indent;
            attrPrinted = true;
        }
        
        os << ( i==Compound::IATTR_STEREO_MODE ?
                    "stereo_mode                " :
                i==Compound::IATTR_STEREO_ANAGLYPH_LEFT_MASK ?
                    "stereo_anaglyph_left_mask  " :
                i==Compound::IATTR_STEREO_ANAGLYPH_RIGHT_MASK ?
                    "stereo_anaglyph_right_mask " : "ERROR " );
        
        switch( i )
        {
            case Compound::IATTR_STEREO_MODE:
                os << static_cast< fabric::IAttribute >( value ) << std::endl;
                break;

            case Compound::IATTR_STEREO_ANAGLYPH_LEFT_MASK:
            case Compound::IATTR_STEREO_ANAGLYPH_RIGHT_MASK:
                os << ColorMask( value ) << std::endl;
                break;

            default:
                EQASSERTINFO( 0, "unimplemented" );
        }
    }
    
    if( attrPrinted )
        os << co::base::exdent << "}" << std::endl << std::endl;

    switch( compound.getFrustumType( ))
    {
        case Frustum::TYPE_WALL:
            os << compound.getWall() << std::endl;
            break;
        case Frustum::TYPE_PROJECTION:
            os << compound.getProjection() << std::endl;
            break;
        default: 
            break;
    }

    const Equalizers& equalizers = compound.getEqualizers();
    for( EqualizersCIter i = equalizers.begin(); i != equalizers.end(); ++i )
        os << *i;

    const TileQueues& outputQueues = compound.getOutputTileQueues();
    for( TileQueuesCIter i = outputQueues.begin(); i != outputQueues.end(); ++i)
        os << "output" <<  *i;

    const TileQueues& inputQueues = compound.getInputTileQueues();
    for( TileQueuesCIter i = inputQueues.begin(); i != inputQueues.end(); ++i )
        os << "input" << *i;

    if( compound.getSwapBarrier( ))
        os << *compound.getSwapBarrier();

    const Compounds& children = compound.getChildren();
    if( !children.empty( ))
    {
        os << std::endl;
        for( CompoundsCIter i = children.begin(); i != children.end(); ++i )
            os << **i;
    }

    const Frames& inputFrames = compound.getInputFrames();
    for( FramesCIter i = inputFrames.begin(); i != inputFrames.end(); ++i )
        os << "input" << *i;

    const Frames& outputFrames = compound.getOutputFrames();
    for( FramesCIter i = outputFrames.begin(); i != outputFrames.end(); ++i )
        os << "output"  << *i;

    os << co::base::exdent << "}" << std::endl << co::base::enableFlush;
    return os;
}

}
}
