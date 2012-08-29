
/* Copyright (c) 2009-2011, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "commands.h"
#include "elementVisitor.h"
#include "log.h"
#include "nameFinder.h"
#include "observer.h"
#include "paths.h"

#include <co/buffer.h>
#include <co/dataIStream.h>
#include <co/dataOStream.h>
#include <co/objectCommand.h>

#include <lunchbox/stdExt.h>

namespace eq
{
namespace fabric
{
template< class C, class L, class V >
Layout< C, L, V >::Layout( C* config )
        : _config( config )
{
    LBASSERT( config );
    static_cast< L* >( this )->_config->_addLayout( static_cast< L* >( this ));
    LBLOG( LOG_INIT ) << "New " << lunchbox::className( this ) << std::endl;
}

template< class C, class L, class V >
Layout< C, L, V >::~Layout()
{
    LBLOG( LOG_INIT ) << "Delete " << lunchbox::className( this ) << std::endl;
    while( !_views.empty( ))
    {
        V* view = _views.back();
        LBCHECK( _removeChild( view ));
        release( view );
    }

    _config->_removeLayout( static_cast< L* >( this ));
}

template< class C, class L, class V >
void Layout< C, L, V >::attach( const UUID& id,
                                const uint32_t instanceID )
{
    Object::attach( id, instanceID );

    co::CommandQueue* queue = _config->getMainThreadQueue();
    LBASSERT( queue );

    registerCommand( CMD_LAYOUT_NEW_VIEW,
                     CmdFunc( this, &Layout< C, L, V >::_cmdNewView ), queue );
    registerCommand( CMD_LAYOUT_NEW_VIEW_REPLY,
                     CmdFunc( this, &Layout< C, L, V >::_cmdNewViewReply ), 0 );
}

template< class C, class L, class V >
uint128_t Layout< C, L, V >::commit( const uint32_t incarnation )
{
    // Always traverse views: view proxy objects may be dirty
    commitChildren< V >( _views, CMD_LAYOUT_NEW_VIEW, incarnation );
    return Object::commit( incarnation );
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
    LBASSERT( view );
    LBASSERT( view->getLayout() == this );
    _views.push_back( view );
    setDirty( DIRTY_VIEWS );
}

template< class C, class L, class V >
bool Layout< C, L, V >::_removeChild( V* view )
{
    typename Views::iterator i = stde::find( _views, view );
    if( i == _views.end( ))
        return false;

    LBASSERT( view->getLayout() == this );
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
            LBINFO << "Removing " << lunchbox::disableHeader << *observer
                   << " used by " << *view << std::endl
                   << lunchbox::enableHeader;
            view->setObserver( 0 );
        }
    }
}

template< class C, class L, class V >
bool Layout< C, L, V >::isActive() const
{
    const typename C::Canvases& canvases = _config->getCanvases();
    for( typename C::Canvases::const_iterator i = canvases.begin();
         i != canvases.end(); ++i )
    {
        if( (*i)->getActiveLayout() == this )
            return true;
    }
    return false;
}

template< class C, class L, class V >
V* Layout< C, L, V >::getView( const ViewPath& path )
{
    LBASSERTINFO( _views.size() > path.viewIndex,
                  _views.size() << " <= " << path.viewIndex << " " << this );

    if( _views.size() <= path.viewIndex )
        return 0;

    return _views[ path.viewIndex ];
}

template< class C, class L, class V >
LayoutPath Layout< C, L, V >::getPath() const
{
    LBASSERT( _config );
    const std::vector< L* >& layouts = _config->getLayouts();
    typename std::vector< L* >::const_iterator i = std::find( layouts.begin(),
                                                              layouts.end(),
                                                              this );
    LBASSERT( i != layouts.end( ));

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
Layout< C, L, V >::_cmdNewView( co::Command& cmd )
{
    co::ObjectCommand command( cmd.getBuffer( ));

    V* view = 0;
    create( &view );
    LBASSERT( view );

    getLocalNode()->registerObject( view );
    view->setAutoObsolete( _config->getLatency() + 1 );
    LBASSERT( view->isAttached() );

    send( command.getNode(), CMD_LAYOUT_NEW_VIEW_REPLY )
            << command.get< uint32_t >() << view->getID();

    return true;
}

template< class C, class L, class V > bool
Layout< C, L, V >::_cmdNewViewReply( co::Command& cmd )
{
    co::ObjectCommand command( cmd.getBuffer( ));
    const uint32_t requestID = command.get< uint32_t >();
    const UUID result = command.get< UUID >();

    getLocalNode()->serveRequest( requestID, result );

    return true;
}

template< class C, class L, class V >
std::ostream& operator << ( std::ostream& os, const Layout< C, L, V >& layout )
{
    os << lunchbox::disableFlush << lunchbox::disableHeader << "layout"
       << std::endl;
    os << "{" << std::endl << lunchbox::indent;

    const std::string& name = layout.getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << std::endl;

    const std::vector< V* >& views = layout.getViews();
    for( typename std::vector< V* >::const_iterator i = views.begin();
         i != views.end(); ++i )
    {
        os << **i;
    }
    os << lunchbox::exdent << "}" << std::endl << lunchbox::enableHeader
       << lunchbox::enableFlush;
    return os;
}

}
}
