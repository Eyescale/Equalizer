
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

#include "configPackets.h"
#include "paths.h"

#include "nameFinder.h"

#include <co/command.h>

#include "layout.ipp" // Layout::_removeObserver template impl

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
};
std::string _iAttributeStrings[] = 
{
    MAKE_ATTR_STRING( IATTR_ROBUSTNESS ),
};
}

template< class S, class C, class O, class L, class CV, class N, class V >
Config< S, C, O, L, CV, N, V >::Config( co::base::RefPtr< S > server )
        : Object()
        , _server( server )
{
    server->_addConfig( static_cast< C* >( this ));
    EQLOG( LOG_INIT ) << "New " << co::base::className( this ) << std::endl;
}

template< class S, class C, class O, class L, class CV, class N, class V >
Config< S, C, O, L, CV, N, V >::~Config()
{
    EQLOG( LOG_INIT ) << "Delete " << co::base::className( this ) << std::endl;
    _appNodeID = co::NodeID::ZERO;

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

}

template< class S, class C, class O, class L, class CV, class N, class V >
void Config< S, C, O, L, CV, N, V >::attach( const co::base::UUID& id, 
                                             const uint32_t instanceID )
{
    Object::attach( id, instanceID );

    co::CommandQueue* queue = _server->getMainThreadQueue();
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
co::base::RefPtr< S > Config< S, C, O, L, CV, N, V >::getServer()
{
    return _server;
}

template< class S, class C, class O, class L, class CV, class N, class V >
co::base::RefPtr< const S > Config< S, C, O, L, CV, N, V >::getServer() const 
{
    return _server;
}

namespace
{
template< typename T, typename V > class IDFinder : public V
{
public:
    IDFinder( const uint128_t& id ) : _id( id ), _result( 0 ) {}
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
    const uint128_t _id;
    T*              _result;
};
}


template< class S, class C, class O, class L, class CV, class N, class V >
template< typename T >
void Config< S, C, O, L, CV, N, V >::find( const uint128_t& id, T** result )
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
T* Config< S, C, O, L, CV, N, V >::find( const uint128_t& id )
{
    IDFinder< T, V > finder( id );
    static_cast< C* >( this )->accept( finder );
    return finder.getResult();
}

template< class S, class C, class O, class L, class CV, class N, class V >
template< typename T >
const T* Config< S, C, O, L, CV, N, V >::find( const uint128_t& id ) const
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
const std::string& Config< S, C, O, L, CV, N, V >::getFAttributeString(
    const FAttribute attr )
{
    return _fAttributeStrings[ attr ];
}

template< class S, class C, class O, class L, class CV, class N, class V >
const std::string& Config< S, C, O, L, CV, N, V >::getIAttributeString(
    const IAttribute attr )
{
    return _iAttributeStrings[ attr ];
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

    // remove from views
    for( typename Layouts::const_iterator j = _layouts.begin();
         j != _layouts.end(); ++j )
    {
        (*j)->_removeObserver( observer );
    }

    EQASSERT( observer->getConfig() == this );
    _observers.erase( i );
    setDirty( DIRTY_OBSERVERS );
    if( !isMaster( ))
        postRemove( observer );
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

    // remove from canvases
    for( typename Canvases::const_iterator j = _canvases.begin();
         j != _canvases.end(); ++j )
    {
        (*j)->removeLayout( layout );
    }

    EQASSERT( layout->getConfig() == this );
    _layouts.erase( i );
    setDirty( DIRTY_LAYOUTS );
    if( !isMaster( ))
        postRemove( layout );
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
void Config< S, C, O, L, CV, N, V >::create( O** observer ) 
{
    *observer = getServer()->getNodeFactory()->createObserver(
        static_cast< C* >( this ));
}
  
template< class S, class C, class O, class L, class CV, class N, class V >
void Config< S, C, O, L, CV, N, V >::release( O* observer ) 
{ 
    getServer()->getNodeFactory()->releaseObserver( observer ); 
}
  
template< class S, class C, class O, class L, class CV, class N, class V >
void Config< S, C, O, L, CV, N, V >::create( L** layout )
{
    *layout = getServer()->getNodeFactory()->createLayout(
        static_cast< C* >( this ));
}
  
template< class S, class C, class O, class L, class CV, class N, class V >
void Config< S, C, O, L, CV, N, V >::release( L* layout )
{ 
    getServer()->getNodeFactory()->releaseLayout( layout ); 
}
  
template< class S, class C, class O, class L, class CV, class N, class V >
void Config< S, C, O, L, CV, N, V >::create( CV** canvas )
{
    *canvas = getServer()->getNodeFactory()->createCanvas(
        static_cast< C* >( this ));
}
  
template< class S, class C, class O, class L, class CV, class N, class V >
void Config< S, C, O, L, CV, N, V >::release( CV* canvas )
{
    getServer()->getNodeFactory()->releaseCanvas( canvas );
}
 
template< class S, class C, class O, class L, class CV, class N, class V >
void Config< S, C, O, L, CV, N, V >::create( N** node )
{
    *node = getServer()->getNodeFactory()->createNode(
        static_cast< C* >( this ));
}

template< class S, class C, class O, class L, class CV, class N, class V >
void Config< S, C, O, L, CV, N, V >::release( N* node )
{ 
     getServer()->getNodeFactory()->releaseNode( node ); 
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
    if( !isMaster( ))
        postRemove( canvas );
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
void Config< S, C, O, L, CV, N, V >::setAppNodeID( const co::NodeID& nodeID )
{
    if( _appNodeID == nodeID )
        return;

    _appNodeID = nodeID;
    setDirty( DIRTY_MEMBER );
}

template< class S, class C, class O, class L, class CV, class N, class V >
void Config< S, C, O, L, CV, N, V >::restore()
{
    Object::restore();
    if( _data.latency != _backup.latency )
    {
        _data = _backup;
        changeLatency( _data.latency );
    }
    else
        _data = _backup;
    setDirty( DIRTY_MEMBER );
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
N* Config< S, C, O, L, CV, N, V >::_findNode( const uint128_t& id )
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


template< class S, class C, class O, class L, class CV, class N, class V >
uint32_t Config< S, C, O, L, CV, N, V >::commitNB()
{
    if( Serializable::isDirty( Config::DIRTY_NODES ))
        commitChildren< N >( _nodes );
    if( Serializable::isDirty( Config::DIRTY_OBSERVERS ))
        commitChildren< O, ConfigNewObserverPacket, C >(
            _observers, static_cast< C* >( this ));
    // Always traverse layouts: view objects may be dirty
    commitChildren< L, ConfigNewLayoutPacket, C >( _layouts,
                                                   static_cast<C*>( this ));
    if( Serializable::isDirty( Config::DIRTY_CANVASES ))
        commitChildren< CV, ConfigNewCanvasPacket, C >(
            _canvases, static_cast< C* >( this ));
    return Object::commitNB();
}

template< class S, class C, class O, class L, class CV, class N, class V >
void Config< S, C, O, L, CV, N, V >::serialize( co::DataOStream& os,
                                                const uint64_t dirtyBits )
{
    Object::serialize( os, dirtyBits );

    if( dirtyBits & Config::DIRTY_MEMBER )
        os << _appNodeID;
    if( dirtyBits & Config::DIRTY_ATTRIBUTES )
    {
        os.write( _fAttributes, C::FATTR_ALL * sizeof( float ));
        os.write( _iAttributes, C::IATTR_ALL * sizeof( int32_t ));
    }
    if( isMaster( ) )
    {
        if( dirtyBits & Config::DIRTY_NODES )
            os.serializeChildren( _nodes );
        if( dirtyBits & Config::DIRTY_OBSERVERS )
            os.serializeChildren( _observers );
        if( dirtyBits & Config::DIRTY_LAYOUTS )
            os.serializeChildren( _layouts );
        if( dirtyBits & Config::DIRTY_CANVASES )
            os.serializeChildren( _canvases );
    }
    if( dirtyBits & Config::DIRTY_LATENCY )
        os << _data.latency;
}

template< class S, class C, class O, class L, class CV, class N, class V >
void Config< S, C, O, L, CV, N, V >::deserialize( co::DataIStream& is, 
                                                  const uint64_t dirtyBits )
{
    Object::deserialize( is, dirtyBits );
    
    if( dirtyBits & Config::DIRTY_MEMBER )
        is >> _appNodeID;
    if( dirtyBits & Config::DIRTY_ATTRIBUTES )
    {
        is.read( _fAttributes, C::FATTR_ALL * sizeof( float ));
        is.read( _iAttributes, C::IATTR_ALL * sizeof( int32_t ));
    }
    if( isMaster( ))
    {
        if( dirtyBits & Config::DIRTY_NODES )
            syncChildren( _nodes );
        if( dirtyBits & Config::DIRTY_OBSERVERS )
            syncChildren( _observers );
        if( dirtyBits & Config::DIRTY_LAYOUTS )
            syncChildren( _layouts );
        if( dirtyBits & Config::DIRTY_CANVASES )
            syncChildren( _canvases );
    }
    else
    {
        if( dirtyBits & Config::DIRTY_NODES )
        {
            if( mapNodeObjects( ))
            {
                typename C::Nodes result;
                is.deserializeChildren( this, _nodes, result );
                _nodes.swap( result );
                EQASSERT( _nodes.size() == result.size( ));
            }
            else // consume unused ObjectVersions
            {
                co::ObjectVersions childIDs;
                is >> childIDs;
            }
        }

        if( mapViewObjects( )) // depends on _config._appNodeID !
        {
            if( dirtyBits & Config::DIRTY_OBSERVERS )
            {
                typename C::Observers result;
                is.deserializeChildren( this, _observers, result );
                _observers.swap( result );
                EQASSERT( _observers.size() == result.size( ));
            }
            if( dirtyBits & Config::DIRTY_LAYOUTS )
            {
                typename C::Layouts result;
                is.deserializeChildren( this, _layouts, result );
                _layouts.swap( result );
                EQASSERT( _layouts.size() == result.size( ));
            }
            if( dirtyBits & Config::DIRTY_CANVASES )
            {
                typename C::Canvases result;
                is.deserializeChildren( this, _canvases, result );
                _canvases.swap( result );
                EQASSERT( _canvases.size() == result.size( ));
            }
        }
        else // consume unused ObjectVersions
        {
            co::ObjectVersions childIDs;
            if( dirtyBits & Config::DIRTY_OBSERVERS )
                is >> childIDs;
            if( dirtyBits & Config::DIRTY_LAYOUTS )
                is >> childIDs;
            if( dirtyBits & Config::DIRTY_CANVASES )
                is >> childIDs;
        }
    }

    if( dirtyBits & Config::DIRTY_LATENCY )
    {
        uint32_t latency = 0;
        is >> latency;
        if( _data.latency != latency )
        {
            _data.latency = latency;
            changeLatency( latency );
        }
    }
}

template< class S, class C, class O, class L, class CV, class N, class V >
void Config< S, C, O, L, CV, N, V >::notifyDetach()
{
    Object::notifyDetach();
    if( isMaster( ))
        return;

    typename S::NodeFactory* nodeFactory = getServer()->getNodeFactory();

    co::LocalNodePtr localNode = getLocalNode();
    while( !_nodes.empty( ))
    {
        EQASSERT( mapNodeObjects( ));
        N* node = _nodes.back();
        localNode->unmapObject( node );
        _removeNode( node );
        nodeFactory->releaseNode( node );
    }

    while( !_canvases.empty( ))
    {
        EQASSERT( mapViewObjects( ));
        CV* canvas = _canvases.back();
        localNode->unmapObject( canvas );
        _removeCanvas( canvas );
        nodeFactory->releaseCanvas( canvas );
    }

    while( !_layouts.empty( ))
    {
        EQASSERT( mapViewObjects( ));
        L* layout = _layouts.back();
        localNode->unmapObject( layout );
        _removeLayout( layout );
        nodeFactory->releaseLayout( layout );
    }

    while( !_observers.empty( ))
    {
        EQASSERT( mapViewObjects( ));
        O* observer = _observers.back();
        localNode->unmapObject( observer );
        _removeObserver( observer );
        nodeFactory->releaseObserver( observer );
    }
}

//----------------------------------------------------------------------
// Command handlers
//----------------------------------------------------------------------
template< class S, class C, class O, class L, class CV, class N, class V >
bool Config< S, C, O, L, CV, N, V >::_cmdNewLayout(
    co::Command& command )
{
    const ConfigNewLayoutPacket* packet =
        command.getPacket< ConfigNewLayoutPacket >();
    
    L* layout = 0;
    create( &layout );
    EQASSERT( layout );

    getLocalNode()->registerObject( layout );
    layout->setAutoObsolete( _data.latency + 1 );
    EQASSERT( layout->isAttached() );

    ConfigNewEntityReplyPacket reply( packet );
    reply.entityID = layout->getID();
    send( command.getNode(), reply ); 
    return true;
}

template< class S, class C, class O, class L, class CV, class N, class V >
bool Config< S, C, O, L, CV, N, V >::_cmdNewCanvas( co::Command& command )
{
    const ConfigNewCanvasPacket* packet =
        command.getPacket< ConfigNewCanvasPacket >();
    
    CV* canvas = 0;
    create( &canvas );
    EQASSERT( canvas );

    getLocalNode()->registerObject( canvas );
    canvas->setAutoObsolete( _data.latency + 1 );
    EQASSERT( canvas->isAttached() );

    ConfigNewEntityReplyPacket reply( packet );
    reply.entityID = canvas->getID();
    send( command.getNode(), reply ); 
    return true;
}

template< class S, class C, class O, class L, class CV, class N, class V >
bool Config< S, C, O, L, CV, N, V >::_cmdNewObserver( co::Command& command )
{
    const ConfigNewObserverPacket* packet =
        command.getPacket< ConfigNewObserverPacket >();
    
    O* observer = 0;
    create( &observer );
    EQASSERT( observer );

    getLocalNode()->registerObject( observer );
    observer->setAutoObsolete( _data.latency + 1 );
    EQASSERT( observer->isAttached() );

    ConfigNewEntityReplyPacket reply( packet );
    reply.entityID = observer->getID();
    send( command.getNode(), reply ); 
    return true;
}

template< class S, class C, class O, class L, class CV, class N, class V >
bool Config< S, C, O, L, CV, N, V >::_cmdNewEntityReply( co::Command& command )
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
    os << co::base::disableFlush << co::base::disableHeader << "config "
       << std::endl;
    os << "{" << std::endl << co::base::indent;

    if( !config.getName().empty( ))
        os << "name    \"" << config.getName() << '"' << std::endl;

    if( config.getLatency() != 1 )
        os << "latency " << config.getLatency() << std::endl;
    os << std::endl;

    os << "attributes" << std::endl << "{" << std::endl << co::base::indent
       << "eye_base     " << config.getFAttribute( C::FATTR_EYE_BASE )
       << std::endl
       << "robustness   " << config.getIAttribute( C::IATTR_ROBUSTNESS )
       << std::endl
       << co::base::exdent << "}" << std::endl;

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

    os << co::base::exdent << "}" << std::endl << co::base::enableHeader
       << co::base::enableFlush;

    return os;
}

}
}
