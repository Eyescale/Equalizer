
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "canvas.h"

#include "config.h"
#include "layout.h"
#include "segment.h"

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
        , _config( config )
        , _layout( 0 )
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
    for( SegmentVector::const_iterator i = _segments.begin();
         i != _segments.end(); ++i )
    {
        delete *i;
    }

    if( _config )
        _config->removeCanvas( this );

    _config = 0;
    _segments.clear();
    _layout = 0;
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

    _segments.push_back( segment );
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
    if( layout && !layout->getName().empty( ))
        os << "layout   \"" << layout->getName() << "\"" << std::endl;

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
