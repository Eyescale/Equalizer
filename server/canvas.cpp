
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "canvas.h"

#include "channel.h"
#include "config.h"
#include "layout.h"
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
        : eq::Canvas( from )
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

    if( dirtyBits & DIRTY_LAYOUT )
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

Segment* Canvas::getSegment( const SegmentPath& path )
{
    EQASSERTINFO( _segments.size() >= path.segmentIndex,
                  _segments.size() << " < " << path.segmentIndex );

    if( _segments.size() < path.segmentIndex )
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
    if( segment->getName().empty( ))
    {
        std::stringstream name;
        name << "segment" << _segments.size() + 1;
        segment->setName( name.str( ));
    }
    
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
                EQINFO << "Segment " << segment->getName() << segment->getWall()
                       << std::endl;
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
    accept( &finder );
    return finder.getResult();
}

void Canvas::useLayout( Layout* layout )
{
    _layout = layout;
    setDirty( DIRTY_LAYOUT );

    return; // untested code below:
    // for each segment
    for( SegmentVector::const_iterator i = _segments.begin(); 
         i != _segments.end(); ++i )
    {
        Segment* segment = *i;
        
        Channel* channel = segment->getChannel();
        std::string channelName = channel->getName();
        if ( channelName.find( '.' + layout->getName()) != std::string::npos )
        {
            // increase channel, window, pipe, node activation count
            channel->activate();

            Node* node     = channel->getNode();
            if( node == 0)
            {
                node = new Node();
                node->startConfigInit( _config->getInitID() );
            }


            Pipe* pipe     = channel->getPipe();
            if( pipe == 0 )
            {
                pipe = new Pipe();
                pipe->startConfigInit( _config->getInitID() );
            }


            Window* window = channel->getWindow();
            if( window == 0 )
            {
               window = new Window();
               window->startConfigInit( _config->getInitID() );
            }

        }

           
        if ( channelName.find( '.' + layout->getName()) != std::string::npos )
        {
            // decrease channel, window, pipe, node activation count
            // exit and release entities with 0 activation count
            channel->deactivate();

            Node* node     = channel->getNode();

            if( !node->isActive())
            {
                eq::ConfigExitPacket configExitPacket;
                configExitPacket.requestID  = getID();
                net::NodePtr netNode = node->getNode();
                netNode->send( configExitPacket );
            }

            Pipe* pipe     = channel->getPipe();
            if( !pipe->isActive())
            {
           /*     eq::ConfigExitPacket configExitPacket;
                configExitPacket.requestID  = getID();
                net::NodePtr netNode = pipe->getPipe();
                netNode->send( configExitPacket );*/
            }

            Window* window = channel->getWindow();
            if( !window->isActive())
            {
             /*   eq::ConfigExitPacket configExitPacket;
                configExitPacket.requestID  = getID();
                
                net::NodePtr netNode  = window->getNode();
                netNode->send( configExitPacket );*/
            }   
               
        }   
    }
}

namespace
{
template< class C, class V >
VisitorResult _accept( C* canvas, V* visitor )
{
    VisitorResult result = visitor->visitPre( canvas );
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

    switch( visitor->visitPost( canvas ))
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

VisitorResult Canvas::accept( CanvasVisitor* visitor )
{
    return _accept( this, visitor );
}

VisitorResult Canvas::accept( ConstCanvasVisitor* visitor ) const
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

    os << static_cast< const eq::Canvas& >( *canvas );

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
