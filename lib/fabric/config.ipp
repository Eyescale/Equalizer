
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder <cedric Stalder@gmail.com> 
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

#include "config.h"

#include "packets.h"
#include "paths.h"

#include "configProxy.ipp"
#include "nameFinder.h"

#include <eq/net/command.h>

namespace eq
{
namespace fabric
{

#define MAKE_ATTR_STRING( attr ) ( std::string("EQ_CONFIG_") + #attr )

namespace
{
std::string _fAttributeStrings[] = 
{
    MAKE_ATTR_STRING( FATTR_EYE_BASE ),
    MAKE_ATTR_STRING( FATTR_VERSION ),
    MAKE_ATTR_STRING( FATTR_FILL1 ),
    MAKE_ATTR_STRING( FATTR_FILL2 )
};
}

template< class S, class C, class O, class L, class CV, class N, class V >
Config< S, C, O, L, CV, N, V >::Config( base::RefPtr< S > server )
        : net::Session()
        , _server( server )
#pragma warning( push )
#pragma warning( disable : 4355 )
        , _proxy( new ConfigProxy< S, C, O, L, CV, N, V >( *this ))
#pragma warning( pop )
{
    server->_addConfig( static_cast< C* >( this ));
}

template< class S, class C, class O, class L, class CV, class N, class V >
Config< S, C, O, L, CV, N, V >::~Config()
{
    _appNodeID = net::NodeID::ZERO;

    while( !_canvases.empty( ))
    {
        CV* canvas = _canvases.back();
        _removeCanvas( canvas );
        delete canvas;
    }

    while( !_layouts.empty( ))
    {
        L* layout = _layouts.back();
        _removeLayout( layout );
        delete layout;
    }

    while( !_observers.empty( ))
    {
        O* observer = _observers.back();
        _removeObserver( observer );
        delete observer;
    }

    while( !_nodes.empty( ))
    {
        N* node = _nodes.back();
        _removeNode( node );
        delete node;
    }

    _server->_removeConfig( static_cast< C* >( this ));
    _server = 0;

    delete _proxy;
}

template< class S, class C, class O, class L, class CV, class N, class V >
void Config< S, C, O, L, CV, N, V >::notifyMapped( net::NodePtr node )
{
    net::Session::notifyMapped( node );

    net::CommandQueue* queue = _server->getMainThreadQueue();
    EQASSERT( queue );

    registerCommand( fabric::CMD_CONFIG_NEW_LAYOUT, 
                CmdFunc( this, &Config< S, C, O, L, CV, N, V >::_cmdNewLayout ),
                     queue );
    registerCommand( fabric::CMD_CONFIG_NEW_CANVAS, 
                CmdFunc( this, &Config< S, C, O, L, CV, N, V >::_cmdNewCanvas ),
                     queue);
    registerCommand( fabric::CMD_CONFIG_NEW_OBSERVER, 
              CmdFunc( this, &Config< S, C, O, L, CV, N, V >::_cmdNewObserver ),
                     queue);
    registerCommand( fabric::CMD_CONFIG_NEW_ENTITY_REPLY, 
           CmdFunc( this, &Config< S, C, O, L, CV, N, V >::_cmdNewEntityReply ),
                     0);
}

template< class C, class V >
static VisitorResult _acceptImpl( C* config, V& visitor )
{ 
    VisitorResult result = visitor.visitPre( config );
    if( result != TRAVERSE_CONTINUE )
        return result;

    const typename C::Nodes& nodes = config->getNodes();
    for( typename C::Nodes::const_iterator i = nodes.begin();
         i != nodes.end(); ++i )
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

    const typename C::Observers& observers = config->getObservers();
    for( typename C::Observers::const_iterator i = observers.begin(); 
         i != observers.end(); ++i )
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

    const typename C::Layouts& layouts = config->getLayouts();
    for( typename C::Layouts::const_iterator i = layouts.begin(); 
         i != layouts.end(); ++i )
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

    const typename C::Canvases& canvases = config->getCanvases();
    for( typename C::Canvases::const_iterator i = canvases.begin();
         i != canvases.end(); ++i )
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

    switch( config->_acceptCompounds( visitor ))
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

    switch( visitor.visitPost( config ))
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

    return result;
}

template< class S, class C, class O, class L, class CV, class N, class V >
VisitorResult Config< S, C, O, L, CV, N, V >::accept( V& visitor )
{
    return _acceptImpl( static_cast< C* >( this ), visitor );
}

template< class S, class C, class O, class L, class CV, class N, class V >
VisitorResult Config< S, C, O, L, CV, N, V >::accept( V& visitor ) const
{
    return _acceptImpl( static_cast< const C* >( this ), visitor );
}

template< class S, class C, class O, class L, class CV, class N, class V >
base::RefPtr< S > Config< S, C, O, L, CV, N, V >::getServer()
{
    return _server;
}

template< class S, class C, class O, class L, class CV, class N, class V >
base::RefPtr< const S > Config< S, C, O, L, CV, N, V >::getServer() const 
{
    return _server;
}

namespace
{
template< typename T, typename V > class IDFinder : public V
{
public:
    IDFinder( const uint32_t id ) : _id( id ), _result( 0 ) {}
    virtual ~IDFinder(){}

    virtual VisitorResult visitPre( T* node ) { return visit( node ); }
    virtual VisitorResult visit( T* node )
        {
            if( node->getID() == _id )
            {
                _result = node;
                return TRAVERSE_TERMINATE;
            }
            return TRAVERSE_CONTINUE;
        }

    T* getResult() { return _result; }

private:
    const uint32_t _id;
    T*             _result;
};
}


template< class S, class C, class O, class L, class CV, class N, class V >
template< typename T >
void Config< S, C, O, L, CV, N, V >::find( const uint32_t id, T** result )
{
    IDFinder< T, V > finder( id );
    static_cast< C* >( this )->accept( finder );
    *result = finder.getResult();
}

template< class S, class C, class O, class L, class CV, class N, class V >
template< typename T >
void Config< S, C, O, L, CV, N, V >::find( const std::string& name, 
                                           const T** result ) const
{
    NameFinder< T, V > finder( name );
    static_cast< const C* >( this )->accept( finder );
    *result = finder.getResult();
}

template< class S, class C, class O, class L, class CV, class N, class V >
template< typename T >
T* Config< S, C, O, L, CV, N, V >::find( const uint32_t id )
{
    IDFinder< T, V > finder( id );
    static_cast< C* >( this )->accept( finder );
    return finder.getResult();
}

template< class S, class C, class O, class L, class CV, class N, class V >
template< typename T >
const T* Config< S, C, O, L, CV, N, V >::find( const uint32_t id ) const
{
    IDFinder< const T, V > finder( id );
    static_cast< const C* >( this )->accept( finder );
    return finder.getResult();
}

template< class S, class C, class O, class L, class CV, class N, class V >
template< typename T >
T* Config< S, C, O, L, CV, N, V >::find( const std::string& name )
{
    NameFinder< T, V > finder( name );
    static_cast< C* >( this )->accept( finder );
    return finder.getResult();
}

template< class S, class C, class O, class L, class CV, class N, class V >
template< typename T >
const T* Config< S, C, O, L, CV, N, V >::find( const std::string& name ) const
{
    NameFinder< const T, V > finder( name );
    static_cast< const C* >( this )->accept( finder );
    return finder.getResult();
}


template< class S, class C, class O, class L, class CV, class N, class V >
O* Config< S, C, O, L, CV, N, V >::getObserver( const ObserverPath& path )
{
    EQASSERTINFO( _observers.size() > path.observerIndex,
                  _observers.size() << " <= " << path.observerIndex );

    if( _observers.size() <= path.observerIndex )
        return 0;

    return _observers[ path.observerIndex ];
}

template< class S, class C, class O, class L, class CV, class N, class V >
void Config< S, C, O, L, CV, N, V >::setName( const std::string& name )
{
    _proxy->setName( name );
}

template< class S, class C, class O, class L, class CV, class N, class V >
const std::string& Config< S, C, O, L, CV, N, V >::getName() const
{
    return _proxy->getName();
}

template< class S, class C, class O, class L, class CV, class N, class V >
void Config< S, C, O, L, CV, N, V >::setErrorMessage( const std::string& message )
{
    _proxy->setErrorMessage( message );
}

template< class S, class C, class O, class L, class CV, class N, class V >
const std::string& Config< S, C, O, L, CV, N, V >::getErrorMessage() const
{
    return _proxy->getErrorMessage();
}

template< class S, class C, class O, class L, class CV, class N, class V >
const std::string& Config< S, C, O, L, CV, N, V >::getFAttributeString(
    const FAttribute attr )
{
    return _fAttributeStrings[ attr ];
}

template< class S, class C, class O, class L, class CV, class N, class V >
L* Config< S, C, O, L, CV, N, V >::getLayout( const LayoutPath& path )
{
    EQASSERTINFO( _layouts.size() > path.layoutIndex,
                  _layouts.size() << " <= " << path.layoutIndex );

    if( _layouts.size() <= path.layoutIndex )
        return 0;

    return _layouts[ path.layoutIndex ];
}

template< class S, class C, class O, class L, class CV, class N, class V >
CV* Config< S, C, O, L, CV, N, V >::getCanvas( const CanvasPath& path )
{
    EQASSERTINFO( _canvases.size() > path.canvasIndex,
                  _canvases.size() << " <= " << path.canvasIndex );

    if( _canvases.size() <= path.canvasIndex )
        return 0;

    return _canvases[ path.canvasIndex ];
}

template< class S, class C, class O, class L, class CV, class N, class V >
void Config< S, C, O, L, CV, N, V >::_addObserver( O* observer )
{
    EQASSERT( observer->getConfig() == this );
    _observers.push_back( observer );
    setDirty( DIRTY_OBSERVERS );
}

template< class S, class C, class O, class L, class CV, class N, class V >
bool Config< S, C, O, L, CV, N, V >::_removeObserver( O* observer )
{
    typename Observers::iterator i = std::find( _observers.begin(),
                                                _observers.end(), observer );
    if( i == _observers.end( ))
        return false;

    EQASSERT( observer->getConfig() == this );
    _observers.erase( i );
    setDirty( DIRTY_OBSERVERS );
    if( !_proxy->isMaster( ))
        _proxy->postRemove( observer );
    return true;
}

template< class S, class C, class O, class L, class CV, class N, class V >
void Config< S, C, O, L, CV, N, V >::_addLayout( L* layout )
{
    EQASSERT( layout->getConfig() == this );
    _layouts.push_back( layout );
    setDirty( DIRTY_LAYOUTS );
}

template< class S, class C, class O, class L, class CV, class N, class V >
bool Config< S, C, O, L, CV, N, V >::_removeLayout( L* layout )
{
    typename Layouts::iterator i = std::find( _layouts.begin(),
                                                   _layouts.end(), layout );
    if( i == _layouts.end( ))
        return false;

    EQASSERT( layout->getConfig() == this );
    _layouts.erase( i );
    setDirty( DIRTY_LAYOUTS );
    if( !_proxy->isMaster( ))
        _proxy->postRemove( layout );
    return true;
}

template< class S, class C, class O, class L, class CV, class N, class V >
void Config< S, C, O, L, CV, N, V >::_addCanvas( CV* canvas )
{
    EQASSERT( canvas->getConfig() == this );
    _canvases.push_back( canvas );
    setDirty( DIRTY_CANVASES );
}

template< class S, class C, class O, class L, class CV, class N, class V >
bool Config< S, C, O, L, CV, N, V >::_removeCanvas( CV* canvas )
{
    typename Canvases::iterator i = std::find( _canvases.begin(),
                                               _canvases.end(), canvas );
    if( i == _canvases.end( ))
        return false;

    EQASSERT( canvas->getConfig() == this );
    _canvases.erase( i );
    setDirty( DIRTY_CANVASES );
    if( !_proxy->isMaster( ))
        _proxy->postRemove( canvas );
    return true;
}

template< class S, class C, class O, class L, class CV, class N, class V >
void Config< S, C, O, L, CV, N, V >::setLatency( const uint32_t latency )
{
    if( _data.latency == latency )
        return;

    _data.latency = latency;
    setDirty( DIRTY_MEMBER );
}

template< class S, class C, class O, class L, class CV, class N, class V >
void Config< S, C, O, L, CV, N, V >::setAppNodeID( const net::NodeID& nodeID )
{
    if( _appNodeID == nodeID )
        return;

    _appNodeID = nodeID;
    setDirty( DIRTY_MEMBER );
}

template< class S, class C, class O, class L, class CV, class N, class V >
uint32_t Config< S, C, O, L, CV, N, V >::getProxyID() const
{
    EQASSERT( _proxy->isMaster( ));
    return _proxy->getID();
}

template< class S, class C, class O, class L, class CV, class N, class V >
void Config< S, C, O, L, CV, N, V >::backup()
{
    _backup = _data;
    _proxy->backup();
}

template< class S, class C, class O, class L, class CV, class N, class V >
void Config< S, C, O, L, CV, N, V >::restore()
{
    _proxy->restore();
    if( _data.latency != _backup.latency )
    {
        _data = _backup;
        changeLatency( _data.latency );
    }
    else
        _data = _backup;
    setDirty( DIRTY_MEMBER );
}

// TODO move visitors for operations on childs here.
template< class S, class C, class O, class L, class CV, class N, class V >
uint32_t Config< S, C, O, L, CV, N, V >::register_()
{
    EQASSERT( _proxy->getID() == EQ_ID_INVALID );
    EQCHECK( registerObject( _proxy ));
    return _proxy->getID();
}

template< class S, class C, class O, class L, class CV, class N, class V >
void Config< S, C, O, L, CV, N, V >::deregister()
{
    EQASSERT( _proxy->getID() <= EQ_ID_MAX );
    deregisterObject( _proxy );
}

template< class S, class C, class O, class L, class CV, class N, class V >
void Config< S, C, O, L, CV, N, V >::map( const net::ObjectVersion proxy )
{
    EQCHECK( mapObject( _proxy, proxy ));
}

template< class S, class C, class O, class L, class CV, class N, class V >
void Config< S, C, O, L, CV, N, V >::unmap()
{
    unmapObject( _proxy );
}

template< class S, class C, class O, class L, class CV, class N, class V >
void Config< S, C, O, L, CV, N, V >::setDirty( const uint64_t dirtyBits )
{
    return _proxy->setDirty( dirtyBits );
}

template< class S, class C, class O, class L, class CV, class N, class V >
uint32_t Config< S, C, O, L, CV, N, V >::getVersion() const
{
    return _proxy->getVersion();
}

template< class S, class C, class O, class L, class CV, class N, class V >
uint32_t Config< S, C, O, L, CV, N, V >::commit()
{
    return _proxy->commit();
}

template< class S, class C, class O, class L, class CV, class N, class V >
void Config< S, C, O, L, CV, N, V >::sync( const uint32_t version )
{
    _proxy->sync( version );
}

template< class S, class C, class O, class L, class CV, class N, class V >
void Config< S, C, O, L, CV, N, V >::_addNode( N* node )
{
    EQASSERT( node->getConfig() == this );
    _nodes.push_back( node );
}

template< class S, class C, class O, class L, class CV, class N, class V >
bool Config< S, C, O, L, CV, N, V >::_removeNode( N* node )
{
    typename Nodes::iterator i = std::find( _nodes.begin(),
                                            _nodes.end(), node );
    if( i == _nodes.end( ))
        return false;

    EQASSERT( node->getConfig() == this );
    _nodes.erase( i );
    return true;
}

template< class S, class C, class O, class L, class CV, class N, class V >
N* Config< S, C, O, L, CV, N, V >::_findNode( const uint32_t id )
{
    for( typename Nodes::const_iterator i = _nodes.begin(); 
		 i != _nodes.end(); ++i )
    {
        N* node = *i;
        if( node->getID() == id )
            return node;
    }
    return 0;
}

//----------------------------------------------------------------------
// Command handlers
//----------------------------------------------------------------------
template< class S, class C, class O, class L, class CV, class N, class V >
bool Config< S, C, O, L, CV, N, V >::_cmdNewLayout(
    net::Command& command )
{
    const ConfigNewLayoutPacket* packet =
        command.getPacket< ConfigNewLayoutPacket >();
    
    L* layout = 0;
    _proxy->create( &layout );
    EQASSERT( layout );

    registerObject( layout );
    EQASSERT( layout->getID() <= EQ_ID_MAX );

    ConfigNewEntityReplyPacket reply( packet );
    reply.entityID = layout->getID();
    send( command.getNode(), reply ); 

    return true;
}

template< class S, class C, class O, class L, class CV, class N, class V >
bool Config< S, C, O, L, CV, N, V >::_cmdNewCanvas(
    net::Command& command )
{
    const ConfigNewCanvasPacket* packet =
        command.getPacket< ConfigNewCanvasPacket >();
    
    CV* canvas = 0;
    _proxy->create( &canvas );
    EQASSERT( canvas );

    registerObject( canvas );
    EQASSERT( canvas->getID() <= EQ_ID_MAX );

    ConfigNewEntityReplyPacket reply( packet );
    reply.entityID = canvas->getID();
    send( command.getNode(), reply ); 

    return true;
}

template< class S, class C, class O, class L, class CV, class N, class V >
bool Config< S, C, O, L, CV, N, V >::_cmdNewObserver(
    net::Command& command )
{
    const ConfigNewObserverPacket* packet =
        command.getPacket< ConfigNewObserverPacket >();
    
    O* observer = 0;
    _proxy->create( &observer );
    EQASSERT( observer );

    registerObject( observer );
    EQASSERT( observer->getID() <= EQ_ID_MAX );

    ConfigNewEntityReplyPacket reply( packet );
    reply.entityID = observer->getID();
    send( command.getNode(), reply ); 

    return true;
}

template< class S, class C, class O, class L, class CV, class N, class V >
bool Config< S, C, O, L, CV, N, V >::_cmdNewEntityReply(
    net::Command& command )
{
    const ConfigNewEntityReplyPacket* packet =
        command.getPacket< ConfigNewEntityReplyPacket >();
    getLocalNode()->serveRequest( packet->requestID, packet->entityID );

    return true;
}

template< class S, class C, class O, class L, class CV, class N, class V >
std::ostream& operator << ( std::ostream& os,
                            const Config< S, C, O, L, CV, N, V >& config )
{
    os << base::disableFlush << base::disableHeader << "config " << std::endl;
    os << "{" << std::endl << base::indent;

    if( !config.getName().empty( ))
        os << "name    \"" << config.getName() << '"' << std::endl;

    if( config.getLatency() != 1 )
        os << "latency " << config.getLatency() << std::endl;
    os << std::endl;

    os << "attributes" << std::endl << "{" << std::endl << base::indent
       << "eye_base     " << config.getFAttribute( C::FATTR_EYE_BASE )
       << std::endl
       << base::exdent << "}" << std::endl;

    const typename C::Nodes& nodes = config.getNodes();
    for( typename C::Nodes::const_iterator i = nodes.begin();
         i != nodes.end(); ++i )
    {
        os << **i;
    }
    const typename C::Observers& observers = config.getObservers();
    for( typename C::Observers::const_iterator i = observers.begin(); 
         i !=observers.end(); ++i )
    {
        os << **i;
    }
    const typename C::Layouts& layouts = config.getLayouts();
    for( typename C::Layouts::const_iterator i = layouts.begin(); 
         i !=layouts.end(); ++i )
    {
        os << **i;
    }
    const typename C::Canvases& canvases = config.getCanvases();
    for( typename C::Canvases::const_iterator i = canvases.begin(); 
         i != canvases.end(); ++i )
    {
        os << **i;
    }

    static_cast< const C& >( config ).output( os );

    os << base::exdent << "}" << std::endl << base::enableHeader
       << base::enableFlush;

    return os;
}

}
}
