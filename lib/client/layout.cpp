
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
#include "global.h"
#include "nodeFactory.h"
#include "view.h"
#include <eq/fabric/elementVisitor.h>
#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>

using namespace eq::base;

namespace eq
{

Layout::Layout()
        : _config( 0 )
{
}

Layout::~Layout()
{
    for( ViewVector::const_iterator i = _views.begin(); i != _views.end(); ++i )
    {
        View* view = *i;
        EQCHECK( _removeView( view ));
        delete view;
    }

    _views.clear();
    EQASSERT( !_config );
}

void Layout::deserialize( net::DataIStream& is, const uint64_t dirtyBits )
{
    Object::deserialize( is, dirtyBits );

    if( dirtyBits & DIRTY_VIEWS )
    {
        EQASSERT( _views.empty( ));
        EQASSERT( _config );

        NodeFactory* nodeFactory = Global::getNodeFactory();
        uint32_t id;
        for( is >> id; id != EQ_ID_INVALID; is >> id )
        {
            View* view = nodeFactory->createView( this );
            _config->mapObject( view, id );
        }
    }
}

void Layout::_deregister()
{
    EQASSERT( _config );
    EQASSERT( !isMaster( ));

    NodeFactory* nodeFactory = Global::getNodeFactory();

    while( !_views.empty( ))
    {
        View* view = _views.back();
        EQASSERT( view->getID() != EQ_ID_INVALID );

        _config->unmapObject( view );
        _removeView( view );
        nodeFactory->releaseView( view );
    }

    _config->unmapObject( this );
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
        os << *i;

    os << exdent << "}" << std::endl << enableHeader << enableFlush;
    return os;
}

}
