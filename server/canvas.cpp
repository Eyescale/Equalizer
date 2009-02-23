
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "canvas.h"

#include "channel.h"
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
            _layout = 0;
        else
        {
            EQASSERT( _config );
            _layout = _config->findLayout( id );
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
    
    // if segment has no frustum...
    if(( segment->getCurrentType() == TYPE_NONE ))
    {
        switch( getCurrentType( )) // ... and canvas has frustum
        {
            // set segment frustum = canvas frustum X segment viewport
            case Segment::TYPE_WALL:
            {
                eq::Wall wallCanvas( getWall() );
                Viewport viewport = segment->getViewport();
                    
                wallCanvas.apply( viewport );
                segment->setWall( wallCanvas );
                EQLOG( LOG_VIEW ) << "Segment " << segment->getName() 
                                  << segment->getWall() << std::endl;
                break;
            }
            case Segment::TYPE_PROJECTION:
            {
                eq::Projection projection( getProjection( ));
                EQUNIMPLEMENTED;
                //to do compute new projection

                
                segment->setProjection( projection );
                break;
            }
            default: 
                break; 
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
    for( SegmentVector::const_iterator i = _segments.begin(); 
         i != _segments.end(); ++i )
    {
        const Segment* segment = *i;        
        const ChannelVector& destChannels = segment->getDestinationChannels();

        // activate channels used by new layout
        for( ChannelVector::const_iterator j = destChannels.begin();
             j != destChannels.end(); ++j )
        {
            Channel*       channel       = *j;
            const Layout*  channelLayout = channel->getLayout();
            if( channelLayout != layout )
                continue;

            // increase channel, window, pipe, node activation count
            // also sends initialization commands as needed
            channel->activate();
        }
           
        // de-activate channels used by old layout
        for( ChannelVector::const_iterator j = destChannels.begin();
             j != destChannels.end(); ++j )
        {
            Channel*       channel       = *j;
            const Layout*  channelLayout = channel->getLayout();
            if( channelLayout != _layout )
                continue;

            // increase channel, window, pipe, node activation count
            // also sends exit commands as needed
            channel->deactivate();
        }
    }

    _layout = layout;
    setDirty( eq::Canvas::DIRTY_LAYOUT );
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
