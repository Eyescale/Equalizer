
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "canvas.h"

#include "canvasVisitor.h"
#include "config.h"
#include "global.h"
#include "layout.h"
#include "nodeFactory.h"
#include "segment.h"

#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>

namespace eq
{

Canvas::Canvas()
        : _config( 0 )
        , _activeLayout( 0 )
{
}

Canvas::~Canvas()
{
    EQASSERT( !_config );
}

void Canvas::serialize( net::DataOStream& os, const uint64_t dirtyBits )
{
    Frustum::serialize( os, dirtyBits );

    if( dirtyBits & DIRTY_LAYOUT )
        os << _activeLayout;

    EQASSERT( !(dirtyBits & DIRTY_CHILDREN ));
}

void Canvas::deserialize( net::DataIStream& is, const uint64_t dirtyBits )
{
    Frustum::deserialize( is, dirtyBits );

    if( dirtyBits & DIRTY_LAYOUT )
        is >> _activeLayout;

    if( dirtyBits & DIRTY_CHILDREN )
    {
        EQASSERT( _segments.empty( ));
        EQASSERT( _config );

        NodeFactory* nodeFactory = Global::getNodeFactory();
        uint32_t id;
        for( is >> id; id != EQ_ID_INVALID; is >> id )
        {
            Segment* segment = nodeFactory->createSegment();
            segment->_canvas = this;
            _segments.push_back( segment );

            _config->mapObject( segment, id );
            // RO, don't: segment->becomeMaster();
        }
        for( is >> id; id != EQ_ID_INVALID; is >> id )
        {
            EQASSERT( _config );
            if( id == EQ_ID_NONE )
                _layouts.push_back( 0 );
            else
            {
                Layout* layout = _config->findLayout( id );
                _layouts.push_back( layout );
                EQASSERT( layout );
            }
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

const Layout* Canvas::getActiveLayout() const
{
    EQASSERT( _activeLayout < _layouts.size( ));
    return _layouts[ _activeLayout ];
}

void Canvas::useLayout( const uint32_t index )
{
    if( _activeLayout == index )
        return;

    _activeLayout = index;
    setDirty( DIRTY_LAYOUT );
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

}
