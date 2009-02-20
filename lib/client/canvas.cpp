
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "canvas.h"

#include "canvasVisitor.h"
#include "config.h"
#include "global.h"
#include "layout.h"
#include "nodeFactory.h"
#include "segment.h"

namespace eq
{

Canvas::Canvas()
        : _config( 0 )
        , _layout( 0 )
{
}

Canvas::~Canvas()
{
    EQASSERT( !_config );
}

void Canvas::_setLayout( const uint32_t id )
{
    if( id == EQ_ID_INVALID )
        _layout = 0;
    else
    {
        EQASSERT( _config );
        _layout = _config->findLayout( id );
        EQASSERT( _layout );
    }
}

void Canvas::deserialize( net::DataIStream& is, const uint64_t dirtyBits )
{
    Frustum::deserialize( is, dirtyBits );

    uint32_t id;
    if( dirtyBits & DIRTY_LAYOUT )
    {
        is >> id;
        _setLayout( id );
    }

    if( dirtyBits & DIRTY_ALL ) // children are immutable
    {
        EQASSERT( _segments.empty( ));
        EQASSERT( _config );

        NodeFactory* nodeFactory = Global::getNodeFactory();
        for( is >> id; id != EQ_ID_INVALID; is >> id )
        {
            Segment* segment = nodeFactory->createSegment();
            segment->_canvas = this;
            _segments.push_back( segment );

            _config->mapObject( segment, id );
            // RO, don't: segment->becomeMaster();
        }
    }
}

void Canvas::deregister()
{
    EQASSERT( _config );
    EQASSERT( isMaster( ));
    NodeFactory* nodeFactory = Global::getNodeFactory();

    for( SegmentVector::const_iterator i = _segments.begin(); 
         i != _segments.end(); ++i )
    {
        Segment* segment = *i;
        EQASSERT( segment->getID() != EQ_ID_INVALID );
        EQASSERT( !segment->isMaster( ));

        _config->deregisterObject( segment );
        segment->_canvas = 0;
        nodeFactory->releaseSegment( segment );
    }
    
    _segments.clear();
    _config->deregisterObject( this );
}

void Canvas::useLayout( Layout* layout )
{
    _layout = layout;
    setDirty( DIRTY_LAYOUT );
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

}
