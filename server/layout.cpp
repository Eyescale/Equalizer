
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

#include "config.h"
#include "layoutVisitor.h"
#include "nameFinder.h"
#include "view.h"

#include <eq/fabric/paths.h>
#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>

using namespace eq::base;

namespace eq
{
namespace server
{

Layout::Layout( Config* parent )
        : _config( parent )
{
    EQASSERT( parent );
    parent->_addLayout( this );
}

Layout::Layout( const Layout& from, Config* parent )
        : Object( from )
        , _config( parent )
{
    EQASSERT( parent );
    for( ViewVector::const_iterator i = from._views.begin();
         i != from._views.end(); ++i )
    {
        new View( **i, this );
    }

    parent->_addLayout( this );
}

Layout::~Layout()
{
    while( !_views.empty( ))
    {
        View* view = _views.back();
        EQASSERT( view->getLayout() == this );
        _removeView( view );
        delete view;
    }
    _config->_removeLayout( this );
}

void Layout::getInstanceData( net::DataOStream& os )
{
    // This function is overwritten from eq::Object, since the class is
    // intended to be subclassed on the client side. When serializing a
    // server::Layout, we only transmit the effective bits, not all since that
    // potentially includes bits from subclassed eq::Layouts.
    const uint64_t dirty = eq::Layout::DIRTY_CUSTOM - 1;
    os << dirty;
    serialize( os, dirty );
}

void Layout::serialize( net::DataOStream& os, const uint64_t dirtyBits )
{
    Object::serialize( os, dirtyBits );

    if( dirtyBits & eq::Layout::DIRTY_VIEWS )
    {
        for( ViewVector::const_iterator i = _views.begin();
             i != _views.end(); ++i )
        {
            View* view = *i;
            EQASSERT( view->getID() != EQ_ID_INVALID );
            os << view->getID();
        }
        os << EQ_ID_INVALID;
    }
}

View* Layout::getView( const ViewPath& path )
{
    EQASSERTINFO( _views.size() > path.viewIndex,
                  _views.size() << " <= " << path.viewIndex << " " << this );

    if( _views.size() <= path.viewIndex )
        return 0;

    return _views[ path.viewIndex ];
}

LayoutPath Layout::getPath() const
{
    EQASSERT( _config );
    
    const LayoutVector&    layouts = _config->getLayouts();
    LayoutVector::const_iterator i = std::find( layouts.begin(), layouts.end(),
                                              this );
    EQASSERT( i != layouts.end( ));

    LayoutPath path;
    path.layoutIndex = std::distance( layouts.begin(), i );
    return path;
}

namespace
{
template< class C >
VisitorResult _accept( C* layout, LayoutVisitor& visitor )
{ 
    VisitorResult result = visitor.visitPre( layout );
    if( result != TRAVERSE_CONTINUE )
        return result;

    const ViewVector& views = layout->getViews();
    for( ViewVector::const_iterator i = views.begin(); i != views.end(); ++i )
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

VisitorResult Layout::accept( LayoutVisitor& visitor )
{
    return _accept( this, visitor );
}
VisitorResult Layout::accept( LayoutVisitor& visitor ) const
{
    return _accept( this, visitor );
}

void Layout::_addView( View* view )
{
    EQASSERT( view );
    EQASSERT( view->getLayout() == this );
    _views.push_back( view );
}

bool Layout::_removeView( View* view )
{
    ViewVector::iterator i = find( _views.begin(), _views.end(), view );
    if( i == _views.end( ))
        return false;

    EQASSERT( view->getLayout() == this );
    _views.erase( i );
    return true;
}

View* Layout::findView( const std::string& name )
{
    ViewFinder finder( name );
    accept( finder );
    return finder.getResult();
}

void Layout::unmap()
{
    net::Session* session = getSession();
    EQASSERT( session );
    for( ViewVector::const_iterator i = _views.begin(); i != _views.end(); ++i )
    {
        View* view = *i;
        EQASSERT( view->getID() != EQ_ID_INVALID );
        
        session->unmapObject( view );
    }

    EQASSERT( getID() != EQ_ID_INVALID );
    session->unmapObject( this );
}

std::ostream& operator << ( std::ostream& os, const Layout* layout )
{
    if( !layout )
        return os;
    
    os << disableFlush << disableHeader << "layout" << std::endl;
    os << "{" << std::endl << indent; 

    const std::string& name = layout->getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << std::endl;

    const ViewVector& views = layout->getViews();
    for( ViewVector::const_iterator i = views.begin(); i != views.end(); ++i )
        os << **i;

    os << exdent << "}" << std::endl << enableHeader << enableFlush;
    return os;
}

}
}
