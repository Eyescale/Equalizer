
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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

#include "canvas.h"

#include "channel.h"
#include "compound.h"
#include "config.h"
#include "layout.h"
#include "log.h"
#include "nameFinder.h"
#include "node.h"
#include "paths.h"
#include "pipe.h"
#include "segment.h"
#include "window.h"


using namespace eq::base;

namespace eq
{
namespace server
{

Canvas::Canvas()
        : _config( 0 )
        , _layout( 0 )
{}

Canvas::Canvas( const Canvas& from, Config* config )
        : eq::Frustum( from )
        , _config( 0 )
        , _layout( 0 )
{
    EQASSERT( config );

    for( SegmentVector::const_iterator i = from._segments.begin();
         i != from._segments.end(); ++i )
    {
        addSegment( new Segment( **i, config ));
    }

    if( from._layout )
    {
        const LayoutPath path( from._layout->getPath( ));
        useLayout( config->getLayout( path ));
        EQASSERT( _layout );
    }

    config->addCanvas( this );
}

Canvas::~Canvas()
{
    for( SegmentVector::const_iterator i = _segments.begin();
         i != _segments.end(); ++i )
    {
        Segment* segment = *i;
        segment->_canvas = 0;
        delete segment;
    }
    _segments.clear();

    if( _config )
        _config->removeCanvas( this );

    _config = 0;
    _layout = 0;
}

void Canvas::serialize( net::DataOStream& os, const uint64_t dirtyBits )
{
    Frustum::serialize( os, dirtyBits );

    if( dirtyBits & eq::Canvas::DIRTY_LAYOUT )
    {
        if( _layout )
        {
            EQASSERT( _layout->getID() != EQ_ID_INVALID );
            os << _layout->getID();
        }
        else
            os << EQ_ID_INVALID;
    }

    if( dirtyBits & DIRTY_ALL ) // children are immutable
    {
        for( SegmentVector::const_iterator i = _segments.begin(); 
             i != _segments.end(); ++i )
        {
            Segment* segment = *i;
            EQASSERT( segment->getID() != EQ_ID_INVALID );
            os << segment->getID();
        }
        os << EQ_ID_INVALID;
    }
}

void Canvas::deserialize( net::DataIStream& is, const uint64_t dirtyBits )
{
    Frustum::deserialize( is, dirtyBits );

    if( dirtyBits & eq::Canvas::DIRTY_LAYOUT )
    {
        uint32_t id;
        is >> id;
        if( id == EQ_ID_INVALID )
            useLayout( 0 );
        else
        {
            EQASSERT( _config );
            useLayout( _config->findLayout( id ));
            EQASSERTINFO( _layout, id );
        }
    }
}

Segment* Canvas::getSegment( const SegmentPath& path )
{
    EQASSERTINFO( _segments.size() > path.segmentIndex,
                  _segments.size() << " <= " << path.segmentIndex );

    if( _segments.size() <= path.segmentIndex )
        return 0;

    return _segments[ path.segmentIndex ];
}

CanvasPath Canvas::getPath() const
{
    EQASSERT( _config );

    const CanvasVector& canvases = _config->getCanvases();
    CanvasVector::const_iterator i = std::find( canvases.begin(),
                                                 canvases.end(), this );
    EQASSERT( i != canvases.end( ));

    CanvasPath path;
    path.canvasIndex = std::distance( canvases.begin(), i );
    return path;
}

void Canvas::addSegment( Segment* segment )
{
    EQASSERT( segment );
    EQASSERT( std::find( _segments.begin(), _segments.end(), segment ) == 
              _segments.end( ));
    
    // if segment has no frustum...
    if( segment->getCurrentType() == TYPE_NONE )
    {
        if( getCurrentType() != TYPE_NONE ) // ... and canvas has frustum
        {
            eq::Wall wall( getWall( ));
            const Viewport& viewport( segment->getViewport( ));
                    
            wall.apply( viewport );
            switch( getCurrentType( ))
            {
                case Frustum::TYPE_WALL:
                    segment->setWall( wall );
                    EQLOG( LOG_VIEW ) << "Segment " << segment->getName() 
                                      << segment->getWall() << std::endl;
                    break;
                case Frustum::TYPE_PROJECTION:
                {
                    Projection projection( getProjection( )); // keep distance
                    projection = wall;
                    segment->setProjection( projection );
                    break;
                }
                default: 
                    EQUNIMPLEMENTED;
                    break; 
            }
        }
    }

    segment->_canvas = this;
    _segments.push_back( segment );
}

Segment* Canvas::findSegment( const std::string& name )
{
    SegmentFinder finder( name );
    accept( finder );
    return finder.getResult();
}

void Canvas::useLayout( Layout* layout )
{
    if( _config && _config->isRunning( ))
        _switchLayout( _layout, layout );

    _layout = layout;
    setDirty( eq::Canvas::DIRTY_LAYOUT );
}

void Canvas::init()
{
    _switchLayout( 0, _layout );
}

void Canvas::exit()
{
    _switchLayout( _layout, 0 );
}

namespace
{
class ActivateVisitor : public ConfigVisitor
{
public:
    ActivateVisitor( const ChannelVector& channels ) : _channels( channels ) {}
    virtual ~ActivateVisitor() {}

    virtual VisitorResult visit( Compound* compound )
        {
            Channel* channel = compound->getChannel();
            if( !channel )
                return TRAVERSE_CONTINUE;
            
            for( ChannelVector::iterator i = _channels.begin();
                 i != _channels.end(); ++i )
            {
                Channel* destChannel = *i;
                if( destChannel != channel ) 
                    continue;

                compound->activate();
                _channels.erase( i );
                break;
            }

            return TRAVERSE_PRUNE;
        }

    virtual VisitorResult visitPost( Config* config )
        { 
            if( !_channels.empty( ))
                EQWARN << _channels.size() << " unused destination channels"
                       << std::endl;
            return TRAVERSE_CONTINUE;
        }

private:
    ChannelVector _channels;
};

class DeactivateVisitor : public ConfigVisitor
{
public:
    DeactivateVisitor( ChannelVector& channels )
            : _channels( channels ) {}
    virtual ~DeactivateVisitor() {}

    virtual VisitorResult visit( Compound* compound )
        {
            Channel* channel = compound->getChannel();
            if( !channel )
                return TRAVERSE_CONTINUE;
            
            for( ChannelVector::iterator i = _channels.begin();
                 i != _channels.end(); ++i )
            {
                Channel* destChannel = *i;
                if( destChannel != channel ) 
                    continue;

                compound->deactivate();
                _channels.erase( i );
                break;
            }

            return TRAVERSE_PRUNE;
        }

    virtual VisitorResult visitPost( Config* config )
        { 
            if( !_channels.empty( ))
                EQWARN << _channels.size() << " unused destination channels"
                       << std::endl;
            return TRAVERSE_CONTINUE;
        }

private:
    ChannelVector& _channels;
};
}

void Canvas::_switchLayout( const Layout* oldLayout, const Layout* newLayout )
{
    EQASSERT( _config );

    for( SegmentVector::const_iterator i = _segments.begin();
         i != _segments.end(); ++i )
    {
        const Segment* segment = *i;        
        const ChannelVector& destChannels = segment->getDestinationChannels();

        if( newLayout )
        {
            // activate channels used by new layout
            ChannelVector usedChannels;
            for( ChannelVector::const_iterator j = destChannels.begin();
                 j != destChannels.end(); ++j )
            {
                Channel*       channel       = *j;
                const Layout*  channelLayout = channel->getLayout();
                if( channelLayout == newLayout )
                    usedChannels.push_back( channel );
            }
            
            ActivateVisitor activator( usedChannels );
            _config->accept( activator );
        }

        if( oldLayout )
        {
            // de-activate channels used by old layout
            ChannelVector usedChannels;

            for( ChannelVector::const_iterator j = destChannels.begin();
                 j != destChannels.end(); ++j )
            {
                Channel*       channel       = *j;
                const Layout*  channelLayout = channel->getLayout();
                if( channelLayout == oldLayout )
                    usedChannels.push_back( channel );
            }
            DeactivateVisitor deactivator( usedChannels );
            _config->accept( deactivator );
        }
    }
}

namespace
{
template< class C, class V >
VisitorResult _accept( C* canvas, V& visitor )
{
    VisitorResult result = visitor.visitPre( canvas );
    if( result != TRAVERSE_CONTINUE )
        return result;

    const SegmentVector& segments = canvas->getSegments();
    for( SegmentVector::const_iterator i = segments.begin(); 
         i != segments.end(); ++i )
    {
        switch( (*i)->accept( visitor ))
        {
            case TRAVERSE_TERMINATE:
                return TRAVERSE_TERMINATE;

            case TRAVERSE_PRUNE:
                result = TRAVERSE_PRUNE;
                break;
                
            case TRAVERSE_CONTINUE:
            default:
                break;
        }
    }

    switch( visitor.visitPost( canvas ))
    {
        case TRAVERSE_TERMINATE:
            return TRAVERSE_TERMINATE;

        case TRAVERSE_PRUNE:
            return TRAVERSE_PRUNE;
                
        case TRAVERSE_CONTINUE:
        default:
            break;
    }

    return result;
}
}

VisitorResult Canvas::accept( CanvasVisitor& visitor )
{
    return _accept( this, visitor );
}

VisitorResult Canvas::accept( ConstCanvasVisitor& visitor ) const
{
    return _accept( this, visitor );
}

void Canvas::unmap()
{
    net::Session* session = getSession();
    EQASSERT( session );
    for( SegmentVector::const_iterator i = _segments.begin(); 
         i != _segments.end(); ++i )
    {
        Segment* segment = *i;
        EQASSERT( segment->getID() != EQ_ID_INVALID );
        
        session->unmapObject( segment );
    }

    EQASSERT( getID() != EQ_ID_INVALID );
    session->unmapObject( this );
}

std::ostream& operator << ( std::ostream& os, const Canvas* canvas )
{
    if( !canvas )
        return os;
    
    os << disableFlush << disableHeader << "canvas" << std::endl;
    os << "{" << std::endl << indent; 

    const std::string& name = canvas->getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << std::endl;

    const Layout* layout = canvas->getLayout();
    if( layout )
    {
        const Config*      config     = layout->getConfig();
        const std::string& layoutName = layout->getName();
        if( layoutName.empty() || config->findLayout( layoutName ) != layout )
            os << layout->getPath() << std::endl;
        else
            os << "layout   \"" << layout->getName() << "\"" << std::endl;
    }

    os << static_cast< const eq::Frustum& >( *canvas );

    const SegmentVector& segments = canvas->getSegments();
    for( SegmentVector::const_iterator i = segments.begin(); 
         i != segments.end(); ++i )
    {
        os << *i;
    }
    os << exdent << "}" << std::endl << enableHeader << enableFlush;
    return os;
}

}
}
