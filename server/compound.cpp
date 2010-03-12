
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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
#include "compoundExitVisitor.h"
#include "compoundInitVisitor.h"
#include "compoundListener.h"
#include "compoundUpdateDataVisitor.h"
#include "compoundUpdateInputVisitor.h"
#include "compoundUpdateOutputVisitor.h"
#include "config.h"
#include "equalizers/equalizer.h"
#include "frame.h"
#include "frameData.h"
#include "global.h"
#include "layout.h"
#include "log.h"
#include "segment.h"
#include "swapBarrier.h"
#include "view.h"

#include <eq/client/global.h>
#include <eq/client/packets.h>
#include <eq/client/windowSystem.h>
#include <eq/fabric/paths.h>
#include <eq/net/session.h>
#include <eq/base/base.h>
#include <eq/base/stdExt.h>

#include <algorithm>
#include <math.h>
#include <vector>

#include "compoundActivateVisitor.h"

namespace eq
{
namespace server
{
#define MAKE_ATTR_STRING( attr ) ( std::string("EQ_COMPOUND_") + #attr )
std::string Compound::_iAttributeStrings[IATTR_ALL] = {
    MAKE_ATTR_STRING( IATTR_STEREO_MODE ),
    MAKE_ATTR_STRING( IATTR_STEREO_ANAGLYPH_LEFT_MASK ),
    MAKE_ATTR_STRING( IATTR_STEREO_ANAGLYPH_RIGHT_MASK ),
    MAKE_ATTR_STRING( IATTR_HINT_OFFSET ),
    MAKE_ATTR_STRING( IATTR_FILL1 ),
    MAKE_ATTR_STRING( IATTR_FILL2 )
};

Compound::Compound()
        : _config( 0 )
        , _parent( 0 )
        , _active( false )
        , _usage( 1.0f )
        , _taskID( 0 )
        , _frustum( _data.frustumData )
        , _swapBarrier( 0 )
{
    EQINFO << "New Compound @" << (void*)this << std::endl;
}

// copy constructor
Compound::Compound( const Compound& from, Config* config, Compound* parent )
        : _name( from._name )
        , _config( 0 )
        , _parent( 0 )
        , _active( false )
        , _usage( from._usage )
        , _taskID( from._taskID )
        , _data( from._data )
        , _frustum( from._frustum, _data.frustumData )
        , _swapBarrier( 0 )
{
    EQASSERTINFO( (config && !parent) || (!config && parent),
                  "Either config or parent has to be given" );

    if( config )
        config->addCompound( this );
    else
    {
        EQASSERT( parent );
        parent->addChild( this );
    }

    if( from._data.channel )
    {
        const Channel* oldChannel = from._data.channel;
        const ChannelPath path = oldChannel->getPath();

        _data.channel = getConfig()->getChannel( path );
        EQASSERT( _data.channel );
    }

    for( CompoundVector::const_iterator i = from._children.begin();
         i != from._children.end(); ++i )
    {
        new Compound( **i, 0, this );
    }

    for( EqualizerVector::const_iterator i = from._equalizers.begin();
         i != from._equalizers.end(); ++i )
    {
        addEqualizer( (*i)->clone( ));
    }

    if( from._swapBarrier )
        _swapBarrier = new SwapBarrier( *from._swapBarrier );

    for( FrameVector::const_iterator i = from._inputFrames.begin();
         i != from._inputFrames.end(); ++i )
    {
        addInputFrame( new Frame( **i ));
    }

    for( FrameVector::const_iterator i = from._outputFrames.begin();
         i != from._outputFrames.end(); ++i )
    {
        addOutputFrame( new Frame( **i ));
    }
}

Compound::~Compound()
{
    delete _swapBarrier;
    _swapBarrier = 0;

    for( EqualizerVector::const_iterator i = _equalizers.begin();
         i != _equalizers.end(); ++i )
    {
        Equalizer* equalizer = *i;
        equalizer->attach( 0 );
        delete equalizer;
    }
    _equalizers.clear();

    for( CompoundVector::const_iterator i = _children.begin(); 
         i != _children.end(); ++i )
    {
        Compound* compound = *i;
        delete compound;
    }
    _children.clear();

    if( _config )
        _config->removeCompound( this );

    _config = 0;

    for( FrameVector::const_iterator i = _inputFrames.begin(); 
         i != _inputFrames.end(); ++i )
    {
        Frame* frame = *i;

        frame->_compound = 0;
        delete frame;
    }
    _inputFrames.clear();

    for( FrameVector::const_iterator i = _outputFrames.begin(); 
         i != _outputFrames.end(); ++i )
    {
        Frame* frame = *i;

        frame->_compound = 0;
        delete frame;
    }
    _outputFrames.clear();

    _parent = 0;
}

Compound::InheritData::InheritData()
        : channel( 0 )
        , overdraw( Vector4i::ZERO )
        , buffers( eq::Frame::BUFFER_UNDEFINED )
        , eyes( EYE_UNDEFINED )
        , tasks( fabric::TASK_DEFAULT )
        , period( EQ_UNDEFINED_UINT32 )
        , phase( EQ_UNDEFINED_UINT32 )
        , maxFPS( std::numeric_limits< float >::max( ))
        , active( true )
{
    const Global* global = Global::instance();
    for( int i=0; i<IATTR_ALL; ++i )
        iAttributes[i] =
            global->getCompoundIAttribute( static_cast< IAttribute >( i ));
}

void Compound::addChild( Compound* child )
{
    _children.push_back( child );
    EQASSERT( !child->_parent );
    child->_parent = this;
    _fireChildAdded( child );
}

bool Compound::removeChild( Compound* child )
{
    CompoundVector::iterator i = find( _children.begin(), _children.end(),
                                        child );
    if( i == _children.end( ))
        return false;

    _fireChildRemove( child );
    _children.erase( i );
    child->_parent = 0;
    return true;
}

Compound* Compound::getNext() const
{
    if( !_parent )
        return 0;

    CompoundVector&          siblings = _parent->_children;
    CompoundVector::iterator result   = find( siblings.begin(), siblings.end(), 
                                              this );
    if( result == siblings.end() )
        return 0;
    result++;
    if( result == siblings.end() )
        return 0;

    return *result;
}

Node* Compound::getNode()
{ 
    Channel* channel = getChannel(); 
    return channel ? channel->getNode() : 0; 
}

void Compound::setChannel( Channel* channel )
{ 
    _data.channel = channel;
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

bool Compound::isActive() const 
{
    const Channel* channel = getChannel();
    if( channel )
        return _inherit.active && channel->isActive(); 

    return _inherit.active;
}

//---------------------------------------------------------------------------
// Listener interface
//---------------------------------------------------------------------------
void Compound::addListener( CompoundListener* listener )
{
    _listeners.push_back( listener );
}

void Compound::removeListener(  CompoundListener* listener )
{
    CompoundListeners::iterator i = find( _listeners.begin(), _listeners.end(),
                                          listener );
    if( i != _listeners.end( ))
        _listeners.erase( i );
}

void Compound::fireUpdatePre( const uint32_t frameNumber )
{
    CHECK_THREAD( _serverThread );

    for( CompoundListeners::const_iterator i = _listeners.begin(); 
         i != _listeners.end(); ++i )

        (*i)->notifyUpdatePre( this, frameNumber );
}

void Compound::_fireChildAdded( Compound* child )
{
    CHECK_THREAD( _serverThread );

    for( CompoundListeners::const_iterator i = _listeners.begin(); 
         i != _listeners.end(); ++i )

        (*i)->notifyChildAdded( this, child );
}

void Compound::_fireChildRemove( Compound* child )
{
    CHECK_THREAD( _serverThread );

    for( CompoundListeners::const_iterator i = _listeners.begin(); 
         i != _listeners.end(); ++i )

        (*i)->notifyChildRemove( this, child );
}

//---------------------------------------------------------------------------
// I/O objects access
//---------------------------------------------------------------------------
void Compound::setSwapBarrier( SwapBarrier* barrier )
{
    if( barrier && barrier->getName().empty( ))
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
    frame->_compound = this;
}
void Compound::addOutputFrame( Frame* frame )
{ 
    if( frame->getName().empty() )
        _setDefaultFrameName( frame );
    _outputFrames.push_back( frame ); 
    frame->_compound = this;
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

bool Compound::isDestination() const
{
    if( !getChannel( ))
        return false;
    
    if( !getParent( ))
        return true;

    for( const Compound* compound = getParent(); compound;
         compound = compound->getParent())
    {
        if( compound->getChannel() )
            return false;
    }

    return true;
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

void Compound::updateFrustum()
{
    if( !isDestination( )) // only set view/segment frusta on destination
        return;

    Channel* channel = getChannel();
    if( !channel )
        return;

    const Segment* segment = channel->getSegment();
    const View*    view    = channel->getView();
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
        _updateOverdraw( wall );

        switch( view->getCurrentType( ))
        {
            case Frustum::TYPE_WALL:
                setWall( wall );
                EQLOG( LOG_VIEW ) << "Compound wall: " << wall << std::endl;
                return;

            case Frustum::TYPE_PROJECTION:
            {
                Projection projection( view->getProjection( )); // keep distance
                projection = wall;
                setProjection( projection );
                EQLOG( LOG_VIEW ) << "Compound projection: " << projection
                                  << std::endl;
                return;
            }

            default:
                EQUNIMPLEMENTED;
        }
    }

    if( segment->getCurrentType() != Frustum::TYPE_NONE ) //frustum from segment
    {
        // set compound frustum =
        //         segment frustum X channel/segment coverage
        const Channel* outputChannel = segment->getChannel();
        EQASSERT( outputChannel );

        const Viewport& outputVP  = outputChannel->getViewport();
        const Viewport& channelVP = channel->getViewport();
        const Viewport  coverage  = outputVP.getCoverage( channelVP );

        Wall wall( segment->getWall( ));

        wall.apply( coverage );
        _updateOverdraw( wall );

        switch( segment->getCurrentType( ))
        {
            case Frustum::TYPE_WALL:
            {
                setWall( wall );
                EQLOG( LOG_VIEW ) << "Compound wall: " << wall << std::endl;
                return;
            }

            case Frustum::TYPE_PROJECTION:
            {
                Projection projection( segment->getProjection( ));
                projection = wall;
                setProjection( projection );
                EQLOG( LOG_VIEW ) << "Compound projection: " << projection
                                  << std::endl;
                return;
            }
            default: 
                EQUNIMPLEMENTED;
        }
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

        const CompoundVector& children = current->getChildren();
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

void Compound::activate()
{
    EQASSERT( isDestination( ));

    CompoundActivateVisitor activator( true );
    accept( activator );
}

void Compound::deactivate()
{
    EQASSERT( isDestination( ));

    CompoundActivateVisitor deactivator( false );
    accept( deactivator );
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

//---------------------------------------------------------------------------
// pre-render compound state update
//---------------------------------------------------------------------------
void Compound::update( const uint32_t frameNumber )
{
    CompoundUpdateDataVisitor updateDataVisitor( frameNumber );
    accept( updateDataVisitor );

    CompoundUpdateOutputVisitor updateOutputVisitor( frameNumber );
    accept( updateOutputVisitor );

    const stde::hash_map<std::string, Frame*>& outputFrames =
        updateOutputVisitor.getOutputFrames();
    CompoundUpdateInputVisitor updateInputVisitor( outputFrames );
    accept( updateInputVisitor );

    const BarrierMap& swapBarriers = updateOutputVisitor.getSwapBarriers();

    for( stde::hash_map< std::string, net::Barrier* >::const_iterator i = 
             swapBarriers.begin(); i != swapBarriers.end(); ++i )
    {
        net::Barrier* barrier = i->second;
        if( barrier->getHeight() > 1 )
            barrier->commit();
    }
}

void Compound::updateInheritData( const uint32_t frameNumber )
{
    _data.pixel.validate();
    _data.subpixel.validate();
    _data.zoom.validate();

    if( !_parent )
    {
        _inherit = _data;
        _inherit.zoom = Zoom::NONE; // will be reapplied below

        if( _inherit.eyes == EYE_UNDEFINED )
            _inherit.eyes = EYE_CYCLOP_BIT;

        if( _inherit.period == EQ_UNDEFINED_UINT32 )
            _inherit.period = 1;

        if( _inherit.phase == EQ_UNDEFINED_UINT32 )
            _inherit.phase = 0;

        if( _inherit.channel )
            _updateInheritPVP();

        if( _inherit.buffers == eq::Frame::BUFFER_UNDEFINED )
            _inherit.buffers = eq::Frame::BUFFER_COLOR;

        if( _inherit.iAttributes[IATTR_STEREO_MODE] == UNDEFINED )
            _inherit.iAttributes[IATTR_STEREO_MODE] = QUAD;

        if( _inherit.iAttributes[IATTR_STEREO_ANAGLYPH_LEFT_MASK] == 
            UNDEFINED )
        {
            _inherit.iAttributes[IATTR_STEREO_ANAGLYPH_LEFT_MASK] = 
                COLOR_MASK_RED;
        }
        if( _inherit.iAttributes[IATTR_STEREO_ANAGLYPH_RIGHT_MASK] == 
            UNDEFINED )
        {   
            _inherit.iAttributes[IATTR_STEREO_ANAGLYPH_RIGHT_MASK] =
                COLOR_MASK_GREEN | COLOR_MASK_BLUE;
        }
    }
    else
    {
        _inherit = _parent->_inherit;

        if( !_inherit.channel )
            _inherit.channel = _data.channel;

        if( _data.frustumData.isValid( ))
            _inherit.frustumData = _data.frustumData;

        _inherit.range.apply( _data.range );
        _inherit.pixel.apply( _data.pixel );
        _inherit.subpixel.apply( _data.subpixel );

        if( _data.eyes != EYE_UNDEFINED )
            _inherit.eyes = _data.eyes;
        
        if( _data.period != EQ_UNDEFINED_UINT32 )
            _inherit.period = _data.period;

        if( _data.phase != EQ_UNDEFINED_UINT32 )
            _inherit.phase = _data.phase;

        _inherit.maxFPS = _data.maxFPS;

        if ( _inherit.pvp.isValid( ))
        {
            EQASSERT( _data.vp.isValid( ));
            _inherit.pvp.apply( _data.vp );

            // Compute the inherit viewport to be pixel-correct with the
            // integer-rounded pvp. This is needed to calculate the frustum
            // correctly.
            const Viewport vp = _inherit.pvp.getSubVP( _parent->_inherit.pvp );
            _inherit.vp.apply( vp );
            
            _updateInheritOverdraw();
        }
        else if( _inherit.channel )
        {
            _updateInheritPVP();
            _inherit.vp.apply( _data.vp );
        }

        if( _data.buffers != eq::Frame::BUFFER_UNDEFINED )
            _inherit.buffers = _data.buffers;
        
        if( _data.iAttributes[IATTR_STEREO_MODE] != UNDEFINED )
            _inherit.iAttributes[IATTR_STEREO_MODE] =
                _data.iAttributes[IATTR_STEREO_MODE];

        if( _data.iAttributes[IATTR_STEREO_ANAGLYPH_LEFT_MASK] != UNDEFINED)
            _inherit.iAttributes[IATTR_STEREO_ANAGLYPH_LEFT_MASK] = 
                _data.iAttributes[IATTR_STEREO_ANAGLYPH_LEFT_MASK];

        if( _data.iAttributes[IATTR_STEREO_ANAGLYPH_RIGHT_MASK] !=UNDEFINED)
            _inherit.iAttributes[IATTR_STEREO_ANAGLYPH_RIGHT_MASK] = 
                _data.iAttributes[IATTR_STEREO_ANAGLYPH_RIGHT_MASK];
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
    if( frameNumber != 0 &&
        ( !_inherit.pvp.hasArea() || !_inherit.range.hasData( )) )
    {
        // Channels with no PVP or range do not execute tasks (ignored during
        // init)
        _inherit.tasks = fabric::TASK_NONE;
    }
    else if( _data.tasks == fabric::TASK_DEFAULT )
    {
        if( isLeaf( ))
            _inherit.tasks = fabric::TASK_ALL;
        else
            _inherit.tasks = fabric::TASK_ASSEMBLE | fabric::TASK_READBACK;
    }
    else
        _inherit.tasks = _data.tasks;

    if( isDestination() && getChannel()->getView( ))
        _inherit.tasks |= fabric::TASK_VIEW;
    else
        _inherit.tasks &= ~fabric::TASK_VIEW;        

    // DPlex activation
    _inherit.active = (( frameNumber % _inherit.period ) == _inherit.phase);
}

void Compound::_updateInheritPVP()
{
    const PixelViewport oldPVP( _inherit.pvp );

    const Channel* channel = _inherit.channel;
    const View* view = channel->getView();

    _inherit.pvp = channel->getPixelViewport( );

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
        updateFrustum();
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

std::ostream& operator << (std::ostream& os, const Compound* compound)
{
    if( !compound )
        return os;
    
    os << base::disableFlush << "compound" << std::endl;
    os << "{" << std::endl << base::indent;
      
    const std::string& name = compound->getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << std::endl;

    const Channel* channel = compound->getChannel();
    if( channel )
    {
        Compound* parent = compound->getParent();
        if( !parent || parent->getChannel() != channel )
        {
            const std::string& channelName = channel->getName();
            const Config*      config      = compound->getConfig();
            EQASSERT( config );

            if( !channelName.empty() && 
                config->findChannel( channelName ) == channel )
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

                    const std::string& segmentName = segment->getName();
                    if( !segmentName.empty() && 
                        config->findSegment( segmentName ) == segment )
                    {
                        const Canvas* canvas = segment->getCanvas();
                        const std::string& canvasName = canvas->getName();
                        if( !canvasName.empty() && 
                            config->findCanvas( canvasName ) == canvas )
                        {
                            os << "canvas \"" << canvasName << "\" ";
                        }
                        else
                            os << canvas->getPath() << ' ';

                        os << "segment \"" << segmentName << "\" ";
                    }
                    else
                        os << segment->getPath() << ' ';

                    const std::string& viewName = view->getName();
                    if( !viewName.empty() && 
                        config->findView( viewName ) == view )
                    {
                        const Layout* layout = view->getLayout();
                        const std::string& layoutName = layout->getName();
                        if( !layoutName.empty() && 
                            config->findLayout( layoutName ) == layout )
                        {
                            os << "layout \"" << layoutName << "\" ";
                        }
                        else
                            os << layout->getPath() << ' ';

                        os << "view \"" << viewName << '\"';
                    }
                    else
                        os << view->getPath();

                    os << " )" << std::endl; 
                }
                else
                    os << "channel  ( " << channel->getPath() << " )" << std::endl;
            }
        }
    }

    const uint32_t tasks = compound->getTasks();
    if( tasks != fabric::TASK_DEFAULT )
    {
        os << "task     [";
        if( tasks &  fabric::TASK_CLEAR )    os << " CLEAR";
        if( tasks &  fabric::TASK_CULL )     os << " CULL";
        if( compound->isLeaf() && 
            ( tasks &  fabric::TASK_DRAW ))  os << " DRAW";
        if( tasks &  fabric::TASK_ASSEMBLE ) os << " ASSEMBLE";
        if( tasks &  fabric::TASK_READBACK ) os << " READBACK";
        os << " ]" << std::endl;
    }

    const uint32_t buffers = compound->getBuffers();
    if( buffers != eq::Frame::BUFFER_UNDEFINED )
    {
        os << "buffers  [";
        if( buffers & eq::Frame::BUFFER_COLOR )  os << " COLOR";
        if( buffers & eq::Frame::BUFFER_DEPTH )  os << " DEPTH";
        os << " ]" << std::endl;
    }

    const Viewport& vp = compound->getViewport();
    if( vp.isValid() && vp != Viewport::FULL )
        os << "viewport " << vp << std::endl;
    
    const Range& range = compound->getRange();
    if( range.isValid() && range != Range::ALL )
        os << range << std::endl;

    const Pixel& pixel = compound->getPixel();
    if( pixel.isValid() && pixel != Pixel::ALL )
        os << pixel << std::endl;

    const SubPixel& subpixel = compound->getSubPixel();
    if( subpixel.isValid() && subpixel != SubPixel::ALL )
            os << subpixel << std::endl;

    const Zoom& zoom = compound->getZoom();
    if( zoom.isValid() && zoom != Zoom::NONE )
        os << zoom << std::endl;

    const uint32_t eye = compound->getEyes();
    if( eye )
    {
        os << "eye      [ ";
        if( eye & Compound::EYE_CYCLOP_BIT )
            os << "CYCLOP ";
        if( eye & Compound::EYE_LEFT_BIT )
            os << "LEFT ";
        if( eye & Compound::EYE_RIGHT_BIT )
            os << "RIGHT ";
        os << "]" << std::endl;
    }

    const uint32_t period = compound->getPeriod();
    const uint32_t phase  = compound->getPhase();
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
         i = static_cast< Compound::IAttribute >( static_cast<uint32_t>( i )+1))
    {
        const int value = compound->getIAttribute( i );
        if( value == Global::instance()->getCompoundIAttribute( i ))
            continue;

        if( !attrPrinted )
        {
            os << std::endl << "attributes" << std::endl;
            os << "{" << std::endl << base::indent;
            attrPrinted = true;
        }
        
        os << ( i==Compound::IATTR_STEREO_MODE ?
                    "stereo_mode                " :
                i==Compound::IATTR_STEREO_ANAGLYPH_LEFT_MASK ?
                    "stereo_anaglyph_left_mask  " :
                i==Compound::IATTR_STEREO_ANAGLYPH_RIGHT_MASK ?
                    "stereo_anaglyph_right_mask " : "ERROR" );
        
        switch( i )
        {
            case Compound::IATTR_STEREO_MODE:
                os << static_cast<IAttrValue>( value ) << std::endl;
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
        os << base::exdent << "}" << std::endl << std::endl;

    switch( compound->getFrustumType( ))
    {
        case Frustum::TYPE_WALL:
            os << compound->getWall() << std::endl;
            break;
        case Frustum::TYPE_PROJECTION:
            os << compound->getProjection() << std::endl;
            break;
        default: 
            break;
    }

    const EqualizerVector& equalizers = compound->getEqualizers();
    for( EqualizerVector::const_iterator i = equalizers.begin();
         i != equalizers.end(); ++i )
    {
        os << *i;
    }

    const CompoundVector& children = compound->getChildren();
    if( !children.empty( ))
    {
        os << std::endl;
        for( CompoundVector::const_iterator i = children.begin();
             i != children.end(); ++i )

            os << *i;
    }

    const FrameVector& inputFrames = compound->getInputFrames();
    for( FrameVector::const_iterator i = inputFrames.begin();
         i != inputFrames.end(); ++i )
        
        os << "input" << *i;

    const FrameVector& outputFrames = compound->getOutputFrames();
    for( FrameVector::const_iterator i = outputFrames.begin();
         i != outputFrames.end(); ++i )
        
        os << "output"  << *i;

    os << compound->getSwapBarrier();

    os << base::exdent << "}" << std::endl << base::enableFlush;
    return os;
}

}
}
