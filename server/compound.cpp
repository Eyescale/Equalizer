
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "compound.h"

#include "channel.h"
#include "colorMask.h"
#include "compoundExitVisitor.h"
#include "compoundInitVisitor.h"
#include "compoundListener.h"
#include "compoundUpdateDataVisitor.h"
#include "compoundUpdateInputVisitor.h"
#include "compoundUpdateOutputVisitor.h"
#include "config.h"
#include "constCompoundVisitor.h"
#include "frame.h"
#include "frameData.h"
#include "global.h"
#include "loadBalancer.h"
#include "log.h"
#include "paths.h"
#include "segment.h"
#include "swapBarrier.h"
#include "view.h"

#include <eq/base/base.h>
#include <eq/base/stdExt.h>
#include <eq/client/colorMask.h>
#include <eq/client/global.h>
#include <eq/client/packets.h>
#include <eq/client/windowSystem.h>
#include <eq/net/session.h>

#include <algorithm>
#include <math.h>
#include <vector>

using namespace eq::base;
using namespace std;
using namespace stde;

namespace eq
{
namespace server
{
#define MAKE_ATTR_STRING( attr ) ( string("EQ_COMPOUND_") + #attr )
std::string Compound::_iAttributeStrings[IATTR_ALL] = {
    MAKE_ATTR_STRING( IATTR_STEREO_MODE ),
    MAKE_ATTR_STRING( IATTR_STEREO_ANAGLYPH_LEFT_MASK ),
    MAKE_ATTR_STRING( IATTR_STEREO_ANAGLYPH_RIGHT_MASK ),
    MAKE_ATTR_STRING( IATTR_HINT_OFFSET )
};

Compound::Compound()
        : _config( 0 )
        , _parent( 0 )
        , _frustum( _data.frustumData )
        , _loadBalancer( 0 )
        , _swapBarrier( 0 )
{
    EQINFO << "New Compound @" << (void*)this << endl;
}

// copy constructor
Compound::Compound( const Compound& from, Config* config, Compound* parent )
        : _name( from._name )
        , _config( 0 )
        , _parent( 0 )
        , _data( from._data )
        , _frustum( from._frustum, _data.frustumData )
        , _loadBalancer( 0 )
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
        const ChannelPath    path = oldChannel->getPath();

        _data.channel = getConfig()->getChannel( path );
        EQASSERT( _data.channel );
    }

    for( CompoundVector::const_iterator i = from._children.begin();
         i != from._children.end(); ++i )
    {
        new Compound( **i, 0, this );
    }

    if( from._loadBalancer )
        setLoadBalancer( new LoadBalancer( *from._loadBalancer ));

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

    if( _loadBalancer )
    {
        _loadBalancer->attach( 0 );
        delete _loadBalancer;
        _loadBalancer = 0;
    }

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
        , buffers( eq::Frame::BUFFER_UNDEFINED )
        , eyes( EYE_UNDEFINED )
        , tasks( eq::TASK_DEFAULT )
        , period( EQ_UNDEFINED_UINT32 )
        , phase( EQ_UNDEFINED_UINT32 )
        , maxFPS( numeric_limits< float >::max( ))
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

    vector<Compound*>&          siblings = _parent->_children;
    vector<Compound*>::iterator result   = find( siblings.begin(),
                                                 siblings.end(), this);

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

void Compound::setLoadBalancer( LoadBalancer* loadBalancer )
{
    if( _loadBalancer )
        _loadBalancer->attach( 0 );
    
    if( loadBalancer )
        loadBalancer->attach( this );
    
    _loadBalancer = loadBalancer;
}

bool Compound::isActive() const 
{
    if( _data.channel )
        return _inherit.active && _data.channel->isActive(); 

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
        const Compound* root     = getRoot();
        const string&   rootName = root->getName();
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
    if( getChannel() == 0 )
        return false;
    
    if( getParent() == 0 )
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
void Compound::setWall( const eq::Wall& wall )
{
    _frustum.setWall( wall );
    EQVERB << "Wall: " << _data.frustumData << endl;
}

void Compound::setProjection( const eq::Projection& projection )
{
    _frustum.setProjection( projection );
    EQVERB << "Projection: " << _data.frustumData << endl;
}

//---------------------------------------------------------------------------
// accept
//---------------------------------------------------------------------------
namespace
{
template< class C, class V >
VisitorResult _accept( C* compound, V* visitor, const bool activeOnly )
{
    if( compound->isLeaf( )) 
    {
        if ( !activeOnly || compound->isActive( )) 
            return visitor->visitLeaf( compound );
        return TRAVERSE_CONTINUE;
    }

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
            if ( !activeOnly || current->isActive( ))
            {
                switch( visitor->visitLeaf( current ))
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
            else
                current = next;
        } 
        else // node
        {
            if( !activeOnly || current->isActive( ))
            {
                switch( visitor->visitPre( current ))
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
            else
                current = next;
        }

        //---------- up-right traversal
        if( !current && !parent ) return TRAVERSE_CONTINUE;

        while( !current )
        {
            current = parent;
            parent  = current->getParent();
            next    = current->getNext();

            if( !activeOnly || current->isActive( ))
            {
                switch( visitor->visitPost( current ))
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
            }
            
            if ( current == compound ) return result;
            
            current = next;
        }
    }
    return result;
}
}

VisitorResult Compound::accept( ConstCompoundVisitor* visitor,
                                const bool activeOnly ) const
{
    return _accept( this, visitor, activeOnly );
}

VisitorResult Compound::accept( CompoundVisitor* visitor,
                                const bool activeOnly )
{
    return _accept( this, visitor, activeOnly );
}

//---------------------------------------------------------------------------
// Operations
//---------------------------------------------------------------------------

void Compound::init()
{
    CompoundInitVisitor initVisitor;
    accept( &initVisitor, false /* activeOnly */ );
}

void Compound::exit()
{
    CompoundExitVisitor visitor;
    accept( &visitor, false /* activeOnly */ );
}

//---------------------------------------------------------------------------
// pre-render compound state update
//---------------------------------------------------------------------------
void Compound::update( const uint32_t frameNumber )
{
    CompoundUpdateDataVisitor updateDataVisitor( frameNumber );
    accept( &updateDataVisitor, false /*activeOnly*/ );

    CompoundUpdateOutputVisitor updateOutputVisitor( frameNumber );
    accept( &updateOutputVisitor, true /*activeOnly*/ );

    const hash_map<std::string, Frame*>& outputFrames =
        updateOutputVisitor.getOutputFrames();
    CompoundUpdateInputVisitor updateInputVisitor( outputFrames );
    accept( &updateInputVisitor, true /*activeOnly*/ );

    const BarrierMap& swapBarriers = updateOutputVisitor.getSwapBarriers();

    for( hash_map<string, net::Barrier*>::const_iterator i = 
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
    _data.zoom.validate();

    if( !_parent )
    {
        _screens.clear();

        _inherit = _data;
        _inherit.zoom = eq::Zoom::NONE; // will be reapplied below

        if( _inherit.screen.origin.x == eq::AUTO )
            _inherit.screen.origin.x = 0;
        if( _inherit.screen.origin.y == eq::AUTO )
            _inherit.screen.origin.y = 0;

        if( _inherit.eyes == EYE_UNDEFINED )
            _inherit.eyes = EYE_CYCLOP_BIT;

        if( _inherit.period == EQ_UNDEFINED_UINT32 )
            _inherit.period = 1;

        if( _inherit.phase == EQ_UNDEFINED_UINT32 )
            _inherit.phase = 0;

        if( _inherit.channel )
            _inherit.pvp  = _inherit.channel->getPixelViewport();

        if( _inherit.buffers == eq::Frame::BUFFER_UNDEFINED )
            _inherit.buffers = eq::Frame::BUFFER_COLOR;

        if( _inherit.iAttributes[IATTR_STEREO_MODE] == eq::UNDEFINED )
            _inherit.iAttributes[IATTR_STEREO_MODE] = eq::QUAD;

        if( _inherit.iAttributes[IATTR_STEREO_ANAGLYPH_LEFT_MASK] == 
            eq::UNDEFINED )
        {
            _inherit.iAttributes[IATTR_STEREO_ANAGLYPH_LEFT_MASK] = 
                COLOR_MASK_RED;
        }
        if( _inherit.iAttributes[IATTR_STEREO_ANAGLYPH_RIGHT_MASK] == 
            eq::UNDEFINED )
        {   
            _inherit.iAttributes[IATTR_STEREO_ANAGLYPH_RIGHT_MASK] =
                COLOR_MASK_GREEN | COLOR_MASK_BLUE;
        }
    }
    else
    {
        _inherit = _parent->_inherit;

        if( !_inherit.channel )
        {
            _inherit.channel = _data.channel;
            _inherit.screen  = _data.screen;
        }

        if( _data.frustumData.isValid( ))
            _inherit.frustumData = _data.frustumData;

        _inherit.range.apply( _data.range );
        _inherit.pixel.apply( _data.pixel );

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
        }
        else if( _inherit.channel )
        {
            _inherit.pvp = _inherit.channel->getPixelViewport();
            _inherit.vp.apply( _data.vp );

            if( _inherit.pvp.isValid( ))
            {
                // Auto-compute our screen origin offset
                if( _inherit.screen.origin.x == eq::AUTO )
                    _inherit.screen.origin.x = static_cast< int32_t >(
                        _inherit.vp.x * (_inherit.pvp.w / _inherit.vp.w));
                if( _inherit.screen.origin.y == eq::AUTO )
                    _inherit.screen.origin.y = static_cast< int32_t >(
                        _inherit.vp.y * (_inherit.pvp.h / _inherit.vp.h));
            }
        }

        if( _data.buffers != eq::Frame::BUFFER_UNDEFINED )
            _inherit.buffers = _data.buffers;
        
        if( _data.iAttributes[IATTR_STEREO_MODE] != eq::UNDEFINED )
            _inherit.iAttributes[IATTR_STEREO_MODE] =
                _data.iAttributes[IATTR_STEREO_MODE];

        if( _data.iAttributes[IATTR_STEREO_ANAGLYPH_LEFT_MASK] != eq::UNDEFINED)
            _inherit.iAttributes[IATTR_STEREO_ANAGLYPH_LEFT_MASK] = 
                _data.iAttributes[IATTR_STEREO_ANAGLYPH_LEFT_MASK];

        if( _data.iAttributes[IATTR_STEREO_ANAGLYPH_RIGHT_MASK] !=eq::UNDEFINED)
            _inherit.iAttributes[IATTR_STEREO_ANAGLYPH_RIGHT_MASK] = 
                _data.iAttributes[IATTR_STEREO_ANAGLYPH_RIGHT_MASK];
    }

    if( _inherit.pvp.isValid( ))
    {
        const uint32_t screenID = _inherit.screen.id;
        Compound*      root     = getRoot();

        if( root->_screens.find( screenID ) == root->_screens.end( )) // init
        {
            root->_screens[ screenID ].x = 0;
            root->_screens[ screenID ].y = 0;
        }

        vmml::Vector2i& screenSize = root->_screens[ screenID ];
        screenSize.x = EQ_MAX( screenSize.x,
                            _inherit.screen.origin.x + _inherit.pvp.getXEnd( ));
        screenSize.y = EQ_MAX( screenSize.y,
                            _inherit.screen.origin.y + _inherit.pvp.getYEnd( ));

        _inherit.pvp.apply( _data.pixel );

        // Zoom
        const eq::PixelViewport unzoomedPVP = _inherit.pvp;
        _inherit.pvp.apply( _data.zoom );

        // Compute the inherit zoom to be pixel-correct with the integer-rounded
        // pvp.
        const Zoom zoom = _inherit.pvp.getZoom( unzoomedPVP );
        _inherit.zoom *= zoom;
    }

    if( !_inherit.pvp.hasArea( ))
        _inherit.tasks = eq::TASK_NONE;
    else if( _data.tasks == eq::TASK_DEFAULT )
    {
        if( isLeaf( ))
            _inherit.tasks = eq::TASK_ALL;
        else
            _inherit.tasks = eq::TASK_ASSEMBLE | eq::TASK_READBACK;
    }
    else
        _inherit.tasks = _data.tasks;

    _inherit.active = (( frameNumber % _inherit.period ) == _inherit.phase);
}

std::ostream& operator << (std::ostream& os, const Compound* compound)
{
    if( !compound )
        return os;
    
    os << disableFlush << "compound" << endl;
    os << "{" << endl << indent;
      
    const std::string& name = compound->getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << endl;

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
                os << "channel  \"" << channelName << "\"" << endl;
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
                        os << "segment \"" << segmentName << "\" ";
                    }
                    else
                        os << segment->getPath() << ' ';

                    const std::string& viewName = view->getName();
                    if( !viewName.empty() && 
                        config->findView( viewName ) == view )
                    {
                        os << "view \"" << viewName;
                    }
                    else
                        os << view->getPath();

                    os << " )" << endl; 
                }
                else
                    os << "channel  ( " << channel->getPath() << " )" << endl;
            }
        }
    }

    const uint32_t tasks = compound->getTasks();
    if( tasks != eq::TASK_DEFAULT )
    {
        os << "task     [";
        if( tasks &  eq::TASK_CLEAR )    os << " CLEAR";
        if( tasks &  eq::TASK_CULL )     os << " CULL";
        if( compound->isLeaf() && 
            ( tasks &  eq::TASK_DRAW ))  os << " DRAW";
        if( tasks &  eq::TASK_ASSEMBLE ) os << " ASSEMBLE";
        if( tasks &  eq::TASK_READBACK ) os << " READBACK";
        os << " ]" << endl;
    }

    const uint32_t buffers = compound->getBuffers();
    if( buffers != eq::Frame::BUFFER_UNDEFINED )
    {
        os << "buffers  [";
        if( buffers & eq::Frame::BUFFER_COLOR )  os << " COLOR";
        if( buffers & eq::Frame::BUFFER_DEPTH )  os << " DEPTH";
        os << " ]" << endl;
    }

    const eq::Viewport& vp = compound->getViewport();
    if( vp.isValid() && vp != eq::Viewport::FULL )
        os << "viewport " << vp << endl;
    
    const eq::Range& range = compound->getRange();
    if( range.isValid() && range != eq::Range::ALL )
        os << range << endl;

    const eq::Pixel& pixel = compound->getPixel();
    if( pixel.isValid() && pixel != eq::Pixel::ALL )
        os << pixel << endl;

    const eq::Zoom& zoom = compound->getZoom();
    if( zoom.isValid() && zoom != eq::Zoom::NONE )
        os << zoom << endl;

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
        os << "]" << endl;
    }

    const uint32_t period = compound->getPeriod();
    const uint32_t phase  = compound->getPhase();
    if( period != EQ_UNDEFINED_UINT32 )
        os << "period " << period << "  ";

    if( phase != EQ_UNDEFINED_UINT32 )
        os << "phase " << phase;

    if( period != EQ_UNDEFINED_UINT32 || phase != EQ_UNDEFINED_UINT32 )
        os << endl;

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
            os << endl << "attributes" << endl;
            os << "{" << endl << indent;
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
                os << static_cast<eq::IAttrValue>( value ) << endl;
                break;

            case Compound::IATTR_STEREO_ANAGLYPH_LEFT_MASK:
            case Compound::IATTR_STEREO_ANAGLYPH_RIGHT_MASK:
                os << ColorMask( value ) << endl;
                break;

            default:
                EQASSERTINFO( 0, "unimplemented" );
        }
    }
    
    if( attrPrinted )
        os << exdent << "}" << endl << endl;

    switch( compound->getLatestFrustum( ))
    {
        case eq::Frustum::TYPE_WALL:
            os << compound->getWall() << endl;
            break;
        case eq::Frustum::TYPE_PROJECTION:
            os << compound->getProjection() << endl;
            break;
        default: 
            break;
    }

    const uint32_t        screen = compound->getScreen();
    const vmml::Vector2i& origin = compound->getScreenOrigin();
    if( screen != 1 || origin != vmml::Vector2i( eq::AUTO, eq::AUTO ))
    {
        os << "screen   [ " << screen << ' ';
        if( origin.x == eq::AUTO )
            os << "AUTO";
        else
            os << origin.x;

        os << ' ';

        if( origin.y == eq::AUTO )
            os << "AUTO";
        else
            os << origin.y;

        os << " ]" << endl;
    }

    if( compound->getLoadBalancer( ))
        os << compound->getLoadBalancer();

    const CompoundVector& children = compound->getChildren();
    if( !children.empty( ))
    {
        os << endl;
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

    os << exdent << "}" << endl << enableFlush;
    return os;
}

}
}
