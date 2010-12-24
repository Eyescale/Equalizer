
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
#include "layoutPackets.h"
#include "log.h"
#include "nameFinder.h"
#include "observer.h"
#include "paths.h"

#include <co/command.h>
#include <co/dataIStream.h>
#include <co/dataOStream.h>
#include <co/base/stdExt.h>

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
    EQLOG( LOG_INIT ) << "New " << co::base::className( this ) << std::endl;
}

template< class C, class L, class V >
Layout< C, L, V >::~Layout()
{
    EQLOG( LOG_INIT ) << "Delete " << co::base::className( this ) << std::endl;
    while( !_views.empty( ))
    {
        V* view = _views.back();
        EQCHECK( _removeChild( view ));
        release( view );
    }

    _config->_removeLayout( static_cast< L* >( this ));
}

template< class C, class L, class V >
void Layout< C, L, V >::attach( const co::base::UUID& id,
                                const uint32_t instanceID )
{
    Object::attach( id, instanceID );

    co::CommandQueue* queue = _config->getMainThreadQueue();
    EQASSERT( queue );

    registerCommand( fabric::CMD_LAYOUT_NEW_VIEW,
                     CmdFunc( this, &Layout< C, L, V >::_cmdNewView ), queue );
    registerCommand( fabric::CMD_LAYOUT_NEW_VIEW_REPLY, 
                     CmdFunc( this, &Layout< C, L, V >::_cmdNewViewReply ), 0 );
}

template< class C, class L, class V >
uint32_t Layout< C, L, V >::commitNB()
{
    // Always traverse views: view proxy objects may be dirty
    commitChildren< V, LayoutNewViewPacket >( _views );
    return Object::commitNB();
}

template< class C, class L, class V >
void Layout< C, L, V >::serialize( co::DataOStream& os,
                                   const uint64_t dirtyBits )
{
    Object::serialize( os, dirtyBits );

    if( dirtyBits & DIRTY_VIEWS && isMaster( ))
        os.serializeChildren( _views );
}

template< class C, class L, class V >
void Layout< C, L, V >::deserialize( co::DataIStream& is, 
                                     const uint64_t dirtyBits )
{
    Object::deserialize( is, dirtyBits );

    if( dirtyBits & DIRTY_VIEWS )
    {
        if( isMaster( ))
            syncChildren( _views );
        else
        {
            Views result;
            is.deserializeChildren( this, _views, result );
            _views.swap( result );
        }
    }
}

template< class C, class L, class V >
void Layout< C, L, V >::setDirty( const uint64_t dirtyBits )
{
    Object::setDirty( dirtyBits );
    _config->setDirty( C::DIRTY_LAYOUTS );
}

template< class C, class L, class V >
void Layout< C, L, V >::notifyDetach()
{
    Object::notifyDetach();
    releaseChildren< L, V >( _views );
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
void Layout< C, L, V >::_addChild( V* view )
{
    EQASSERT( view );
    EQASSERT( view->getLayout() == this );
    _views.push_back( view );
    setDirty( DIRTY_VIEWS );
}

template< class C, class L, class V >
bool Layout< C, L, V >::_removeChild( V* view )
{
    typename Views::iterator i = stde::find( _views, view );
    if( i == _views.end( ))
        return false;

    EQASSERT( view->getLayout() == this );
    _views.erase( i );
    setDirty( DIRTY_VIEWS );
    if( !isMaster( ))
        postRemove( view );
    return true;
}

template< class C, class L, class V >
template< class O > void Layout< C, L, V >::_removeObserver( const O* observer )
{
    for( typename Views::const_iterator i = _views.begin();
         i != _views.end(); ++i )
    {
        V* view = *i;
        if( view->getObserver() == observer )
        {
            EQINFO << "Removing " << co::base::disableHeader << *observer
                   << " used by " << *view << std::endl << co::base::enableHeader;
            view->setObserver( 0 );
        }
    }
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
template< class C, class L, class V > bool
Layout< C, L, V >::_cmdNewView( co::Command& command )
{
    const LayoutNewViewPacket* packet =
        command.getPacket< LayoutNewViewPacket >();
    
    V* view = 0;
    create( &view );
    EQASSERT( view );

    getLocalNode()->registerObject( view );
    view->setAutoObsolete( _config->getLatency() + 1 );
    EQASSERT( view->isAttached() );

    LayoutNewViewReplyPacket reply( packet );
    reply.viewID = view->getID();
    send( command.getNode(), reply ); 

    return true;
}

template< class C, class L, class V > bool
Layout< C, L, V >::_cmdNewViewReply( co::Command& command )
{
    const LayoutNewViewReplyPacket* packet =
        command.getPacket< LayoutNewViewReplyPacket >();
    getLocalNode()->serveRequest( packet->requestID, packet->viewID );

    return true;
}

template< class C, class L, class V >
std::ostream& operator << ( std::ostream& os, const Layout< C, L, V >& layout )
{
    os << co::base::disableFlush << co::base::disableHeader << "layout"
       << std::endl;
    os << "{" << std::endl << co::base::indent; 

    const std::string& name = layout.getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << std::endl;

    const std::vector< V* >& views = layout.getViews();
    for( typename std::vector< V* >::const_iterator i = views.begin();
         i != views.end(); ++i )
    {
        os << **i;
    }
    os << co::base::exdent << "}" << std::endl << co::base::enableHeader
       << co::base::enableFlush;
    return os;
}

}
}
