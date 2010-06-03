
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
#include "nameFinder.h"
#include "packets.h"
#include "paths.h"

#include <eq/net/command.h>
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
Layout< C, L, V >::~Layout()
{
    while( !_views.empty( ))
    {
        V* view = _views.back();
        EQCHECK( _removeView( view ));
        release( view );
    }

    _config->_removeLayout( static_cast< L* >( this ));
}

template< class C, class L, class V >
void Layout< C, L, V >::attachToSession( const uint32_t id,
                                         const uint32_t instanceID,
                                         net::Session* session )
{
    Object::attachToSession( id, instanceID, session );

    net::CommandQueue* queue = _config->getMainThreadQueue();
    EQASSERT( queue );

    registerCommand( fabric::CMD_LAYOUT_NEW_VIEW,
                     CmdFunc( this, &Layout< C, L, V >::_cmdNewView ), queue );
    registerCommand( fabric::CMD_LAYOUT_NEW_VIEW_REPLY, 
                     CmdFunc( this, &Layout< C, L, V >::_cmdNewViewReply ), 0 );
}

template< class C, class L, class V >
void Layout< C, L, V >::serialize( net::DataOStream& os,
                                   const uint64_t dirtyBits )
{
    Object::serialize( os, dirtyBits );

    if( dirtyBits & DIRTY_VIEWS )
        os.serializeChildren( this, _views );
}

template< class C, class L, class V >
void Layout< C, L, V >::deserialize( net::DataIStream& is, 
                                     const uint64_t dirtyBits )
{
    Object::deserialize( is, dirtyBits );

    if( dirtyBits & DIRTY_VIEWS )
    {
        EQASSERT( _config );

        Views result;
        is.deserializeChildren( this, _views, result );
        _views.swap( result );
    }
}

template< class C, class L, class V >
void Layout< C, L, V >::setDirty( const uint64_t dirtyBits )
{
    Object::setDirty( dirtyBits );
    if( isMaster( ))
        _config->setDirty( C::DIRTY_LAYOUTS );
}

template< class C, class L, class V >
void Layout< C, L, V >::notifyDetach()
{
    while( !_views.empty( ))
    {
        V* view = _views.back();
        if( view->getID() > EQ_ID_MAX )
        {
            EQASSERT( isMaster( ));
            return;
        }

        _config->releaseObject( view );
        if( !isMaster( ))
        {
            _removeView( view );
            _config->getServer()->getNodeFactory()->releaseView( view );
        }
    }
}

template< class C, class L, class V >
void Layout< C, L, V >::create( V** view )
{
    *view = getConfig()->getServer()->getNodeFactory()->createView( 
        static_cast< L* >( this ));
}

template< class C, class L, class V >
void Layout< C, L, V >::release( V* view )
{
    getConfig()->getServer()->getNodeFactory()->releaseView( view );
}

namespace
{
template< class L, class V >
VisitorResult _accept( L* layout, V& visitor )
{
    VisitorResult result = visitor.visitPre( layout );
    if( result != TRAVERSE_CONTINUE )
        return result;

    const typename L::Views& views = layout->getViews();
    for( typename L::Views::const_iterator i = views.begin();
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
    setDirty( DIRTY_VIEWS );
}

template< class C, class L, class V >
bool Layout< C, L, V >::_removeView( V* view )
{
    typename Views::iterator i = stde::find( _views, view );
    if( i == _views.end( ))
        return false;

    EQASSERT( view->getLayout() == this );
    _views.erase( i );
    setDirty( DIRTY_VIEWS );
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
    NameFinder< V, Visitor > finder( name );
    accept( finder );
    return finder.getResult();
}

//----------------------------------------------------------------------
// Command handlers
//----------------------------------------------------------------------
template< class C, class L, class V > net::CommandResult
Layout< C, L, V >::_cmdNewView( net::Command& command )
{
    const LayoutNewViewPacket* packet =
        command.getPacket< LayoutNewViewPacket >();
    
    V* view = 0;
    create( &view );
    EQASSERT( view );

    _config->registerObject( view );
    view->setAutoObsolete( _config->getLatency( ));
    EQASSERT( view->getID() <= EQ_ID_MAX );

    LayoutNewViewReplyPacket reply( packet );
    reply.viewID = view->getID();
    send( command.getNode(), reply ); 

    return net::COMMAND_HANDLED;
}

template< class C, class L, class V > net::CommandResult
Layout< C, L, V >::_cmdNewViewReply( net::Command& command )
{
    const LayoutNewViewReplyPacket* packet =
        command.getPacket< LayoutNewViewReplyPacket >();
    getLocalNode()->serveRequest( packet->requestID, packet->viewID );

    return net::COMMAND_HANDLED;
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
