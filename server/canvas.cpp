
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "canvas.h"

#include "config.h"
#include "layout.h"
#include "segment.h"

namespace eq
{
namespace server
{

Canvas::Canvas( const Canvas& from, Config* config )
        : _name( from._name )
{
    EQASSERT( config );
    
    for( SegmentVector::const_iterator i = from._segments.begin();
         i != from._segments.end(); ++i )
    {
        _segments.push_back( new Segment( **i, config ));
    }

    if( from._layout )
    {
        const std::string& name = from._layout->getName();
        useLayout( config->findLayout( name ));

        EQASSERT( !name.empty( ));
        EQASSERT( _layout );
    }

    config->addCanvas( this );
}

Canvas::~Canvas()
{
//    if( _config )
//        _config->removeCanvas( this );
    
    for( SegmentVector::const_iterator i = _segments.begin();
         i != _segments.end(); ++i )
    {
        delete *i;
    }

    _segments.clear();
    _layout = 0;
}

void Canvas::setWall( const eq::Wall& wall )
{
    //eq::Canvas::setWall( wall );
    //_updateCanvas();
}
        
void Canvas::setProjection( const eq::Projection& projection )
{
    //eq::Canvas::setProjection( projection );
    //_updateCanvas();
}

void Canvas::useLayout( Layout* layout )
{
    _layout = layout;
}

VisitorResult Canvas::accept( CanvasVisitor* visitor )
{ 
    VisitorResult result = visitor->visitPre( this );
    if( result != TRAVERSE_CONTINUE )
        return result;

    for( SegmentVector::const_iterator i = _segments.begin(); 
         i != _segments.end(); ++i )
    {
        Segment* segment = *i;
        switch( segment->accept( visitor ))
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

    switch( visitor->visitPost( this ))
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
}
