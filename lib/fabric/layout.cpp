
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "layout.h"
#include "elementVisitor.h"
#include "paths.h"

#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>
#include <eq/base/stdExt.h>

namespace eq
{
namespace fabric
{
template< class C, class L, class V >
Layout< C, L, V >::Layout( C* config )
        : _config( config )
{
    EQASSERT( config );
    static_cast< L* >( this )->_config->_addLayout( static_cast< L* >( this ));
}

template< class C, class L, class V >
Layout< C, L, V >::Layout( const Layout& from, C* config )
        : Object( from )
        , _config( config )
{
    EQASSERT( config );
    for( typename ViewVector::const_iterator i = from._views.begin();
         i != from._views.end(); ++i )
    {
        new V( **i, static_cast< L* >( this ));
    }

    config->_addLayout( static_cast< L* >( this ));
}

template< class C, class L, class V >
Layout< C, L, V >::~Layout()
{
    while( !_views.empty( ))
    {
        V* view = _views.back();
        EQCHECK( _removeView( view ));
        delete view;
    }

    _config->_removeLayout( static_cast< L* >( this ));
}

template< class C, class L, class V >
void Layout< C, L, V >::serialize( net::DataOStream& os,
                                   const uint64_t dirtyBits )
{
    Object::serialize( os, dirtyBits );

    if( dirtyBits & DIRTY_VIEWS )
    {
        for( typename ViewVector::const_iterator i = _views.begin();
             i != _views.end(); ++i )
        {
            const V* view = *i;
            EQASSERT( view->getID() != EQ_ID_INVALID );
            os << view->getID();
        }
        os << EQ_ID_INVALID;
    }
}

template< class C, class L, class V >
void Layout< C, L, V >::deserialize( net::DataIStream& is, 
                                     const uint64_t dirtyBits )
{
    Object::deserialize( is, dirtyBits );

    if( dirtyBits & DIRTY_VIEWS )
    {
        EQASSERT( _views.empty( ));
        EQASSERT( _config );

        uint32_t id;
        for( is >> id; id != EQ_ID_INVALID; is >> id )
        {
            V* view = createView();
            _config->mapObject( view, id );
        }
    }
}

namespace
{
template< class L, class V >
VisitorResult _accept( L* layout, V& visitor )
{
    VisitorResult result = visitor.visitPre( layout );
    if( result != TRAVERSE_CONTINUE )
        return result;

    const typename L::ViewVector& views = layout->getViews();
    for( typename L::ViewVector::const_iterator i = views.begin();
         i != views.end(); ++i )
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

    switch( visitor.visitPost( layout ))
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

template< class C, class L, class V >
VisitorResult Layout< C, L, V >::accept( Visitor& visitor )
{
    return _accept( static_cast< L* >( this ), visitor );
}

template< class C, class L, class V >
VisitorResult Layout< C, L, V >::accept( Visitor& visitor ) const
{
    return _accept( static_cast< const L* >( this ), visitor );
}

template< class C, class L, class V >
void Layout< C, L, V >::_addView( V* view )
{
    EQASSERT( view );
    EQASSERT( view->getLayout() == this );
    _views.push_back( view );
}

template< class C, class L, class V >
bool Layout< C, L, V >::_removeView( V* view )
{
    typename ViewVector::iterator i = stde::find( _views, view );
    if( i == _views.end( ))
        return false;

    EQASSERT( view->getLayout() == this );
    _views.erase( i );
    return true;
}

template< class C, class L, class V >
V* Layout< C, L, V >::getView( const ViewPath& path )
{
    EQASSERTINFO( _views.size() > path.viewIndex,
                  _views.size() << " <= " << path.viewIndex << " " << this );

    if( _views.size() <= path.viewIndex )
        return 0;

    return _views[ path.viewIndex ];
}

template< class C, class L, class V >
LayoutPath Layout< C, L, V >::getPath() const
{
    EQASSERT( _config );
    const std::vector< L* >& layouts = _config->getLayouts();
    typename std::vector< L* >::const_iterator i = std::find( layouts.begin(),
                                                              layouts.end(),
                                                              this );
    EQASSERT( i != layouts.end( ));

    LayoutPath path;
    path.layoutIndex = std::distance( layouts.begin(), i );
    return path;
}

template< class C, class L, class V >
V* Layout< C, L, V >::findView( const std::string& name )
{
    NameFinder< V > finder( name );
    accept( finder );
    return finder.getResult();
}

template< class C, class L, class V >
std::ostream& operator << ( std::ostream& os, const Layout< C, L, V >& layout )
{
    os << base::disableFlush << base::disableHeader << "layout" << std::endl;
    os << "{" << std::endl << base::indent; 

    const std::string& name = layout.getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << std::endl;

    const std::vector< V* >& views = layout.getViews();
    for( typename std::vector< V* >::const_iterator i = views.begin();
         i != views.end(); ++i )
    {
        os << **i;
    }
    os << base::exdent << "}" << std::endl << base::enableHeader
       << base::enableFlush;
    return os;
}

}
}
