
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch> 
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
#include "elementVisitor.h"
#include "nameFinder.h"
#include "paths.h"
#include "segment.h"

#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>

namespace eq
{
namespace fabric
{

template< class CFG, class C, class S, class L >
Canvas< CFG, C, S, L >::Canvas( CFG* config )
        : _config( config )
{
    EQASSERT( config );
    config->_addCanvas( static_cast< C* >( this ));
}

template< class CFG, class C, class S, class L >
Canvas< CFG, C, S, L >::~Canvas()
{
    while( !_segments.empty( ))
    {
        S* segment = _segments.back();
        _removeSegment( segment );
        delete segment;
    }

    _data.activeLayout = 0;
    _layouts.clear();
    _config->_removeCanvas( static_cast< C* >( this ));
}

template< class CFG, class C, class S, class L >
void Canvas< CFG, C, S, L >::restore()
{
    Object::restore();
    activateLayout( 0 );
    _data.activeLayout = 0;
    setDirty( DIRTY_LAYOUT );
}

template< class CFG, class C, class S, class L >
void Canvas< CFG, C, S, L >::serialize( net::DataOStream& os,
                                        const uint64_t dirtyBits )
{
    Frustum::serialize( os, dirtyBits );

    if( dirtyBits & DIRTY_LAYOUT )
        os << _data.activeLayout;

    if( dirtyBits & DIRTY_CHILDREN )
    {
        for( typename SegmentVector::const_iterator i = _segments.begin(); 
             i != _segments.end(); ++i )
        {
            S* segment = *i;
            EQASSERT( segment->getID() != EQ_ID_INVALID );
            EQASSERT( segment->isMaster( ));
            os << segment->getID();
        }
        os << EQ_ID_INVALID;

        for( typename LayoutVector::const_iterator i = _layouts.begin(); 
             i != _layouts.end(); ++i )
        {
            L* layout = *i;
            if( layout )
            {
                EQASSERT( layout->getID() != EQ_ID_INVALID );
                EQASSERT( layout->isMaster( ));
                os << layout->getID();
            }
            else
                os << EQ_ID_NONE;
        }
        os << EQ_ID_INVALID;
    }
}

template< class CFG, class C, class S, class L >
void Canvas< CFG, C, S, L >::deserialize( net::DataIStream& is,
                                          const uint64_t dirtyBits )
{
    Frustum::deserialize( is, dirtyBits );

    if( dirtyBits & DIRTY_LAYOUT )
    {
        uint32_t index;
        is >> index;
        activateLayout( index );
        _data.activeLayout = index;
    }

    if( dirtyBits & DIRTY_CHILDREN )
    {
        EQASSERT( _segments.empty( ));
        EQASSERT( _layouts.empty( ));
        EQASSERT( _config );

        uint32_t id;
        for( is >> id; id != EQ_ID_INVALID; is >> id )
        {
            S* segment = _config->getServer()->getNodeFactory()->createSegment(
                static_cast< C* >( this ));
            _config->mapObject( segment, id );
        }
        for( is >> id; id != EQ_ID_INVALID; is >> id )
        {
            EQASSERT( _config );
            if( id == EQ_ID_NONE )
                _layouts.push_back( 0 );
            else
            {
                L* layout = 0;
                _config->find( id, &layout );
                EQASSERT( layout );
                _layouts.push_back( layout );
            }
        }
    }
}

template< class CFG, class C, class S, class L >
void Canvas< CFG, C, S, L >::_addSegment( S* segment )
{
    EQASSERT( segment );
    EQASSERT( segment->getCanvas() == this );
    _segments.push_back( segment );
}

template< class CFG, class C, class S, class L >
bool Canvas< CFG, C, S, L >::_removeSegment( S* segment )
{
    typename SegmentVector::iterator i = stde::find( _segments, segment );
    if( i == _segments.end( ))
        return false;

    EQASSERT( segment->getCanvas() == this );
    _segments.erase( i );
    return true;
}

template< class CFG, class C, class S, class L >
S* Canvas< CFG, C, S, L >::findSegment( const std::string& name )
{
    NameFinder< S, Visitor > finder( name );
    accept( finder );
    return finder.getResult();
}

template< class CFG, class C, class S, class L >
void Canvas< CFG, C, S, L >::addLayout( L* layout )
{
    EQASSERT( stde::find( _layouts, layout ) == _layouts.end( ));

    // dest channel creation is done be Config::addCanvas
    _layouts.push_back( layout );
}


template< class CFG, class C, class S, class L >
const L* Canvas< CFG, C, S, L >::getActiveLayout() const
{
    EQASSERT( _data.activeLayout < _layouts.size( ));
    return _layouts[ _data.activeLayout ];
}

template< class CFG, class C, class S, class L >
void Canvas< CFG, C, S, L >::useLayout( const uint32_t index )
{
    EQASSERT( index < _layouts.size( ));
    if( _data.activeLayout == index )
        return;

    _data.activeLayout = index;
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

    const typename C::SegmentVector& segments = canvas->getSegments();
    for( typename C::SegmentVector::const_iterator i = segments.begin(); 
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

template< class CFG, class C, class S, class L >
VisitorResult Canvas< CFG, C, S, L >::accept( Visitor& visitor )
{
    return _accept( static_cast< C* >( this ), visitor );
}

template< class CFG, class C, class S, class L >
VisitorResult Canvas< CFG, C, S, L >::accept( Visitor& visitor ) const
{
    return _accept( static_cast< const C* >( this ), visitor );
}

template< class CFG, class C, class S, class L >
std::ostream& operator << ( std::ostream& os, 
                            const Canvas< CFG, C, S, L >& canvas )
{
    os << base::disableFlush << base::disableHeader << "canvas" << std::endl;
    os << "{" << std::endl << base::indent; 

    const std::string& name = canvas.getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << std::endl;

    const std::vector< L* >& layouts = canvas.getLayouts();
    for( typename std::vector< L* >::const_iterator i = layouts.begin(); 
         i != layouts.end(); ++i )
    {
        L* layout = *i;
        if( layout )
        {
            const CFG* config = layout->getConfig();
            const std::string& layoutName = layout->getName();
            const L* foundLayout = 0;
            config->find( layoutName, &foundLayout );
            if( foundLayout == layout )
                os << "layout   \"" << layoutName << "\"" << std::endl;
            else
                os << layout->getPath() << std::endl;
        }
        else
            os << "layout   OFF" << std::endl;
    }

    os << static_cast< const eq::Frustum& >( canvas );

    const std::vector< S* >& segments = canvas.getSegments();
    for( typename std::vector< S* >::const_iterator i = segments.begin(); 
         i != segments.end(); ++i )
    {
        os << **i;
    }
    os << base::exdent << "}" << std::endl << base::enableHeader
       << base::enableFlush;
    return os;
}

}
}
