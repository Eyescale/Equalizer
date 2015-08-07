
/* Copyright (c) 2005-2013, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder <cedric Stalder@gmail.com>
 *               2011-2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "paths.h"

#include "configVisitor.h"

#include <co/global.h>
#include <co/objectICommand.h>

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
Config< S, C, O, L, CV, N, V >::Config( lunchbox::RefPtr< S > server )
        : Object()
        , _server( server )
{
    server->_addConfig( static_cast< C* >( this ));
    LBLOG( LOG_INIT ) << "New " << lunchbox::className( this ) << std::endl;
}

template< class S, class C, class O, class L, class CV, class N, class V >
Config< S, C, O, L, CV, N, V >::~Config()
{
    LBLOG( LOG_INIT ) << "Delete " << lunchbox::className( this ) << std::endl;
    _appNodeID = 0;

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
void Config< S, C, O, L, CV, N, V >::attach( const uint128_t& id,
                                             const uint32_t instanceID )
{
    Object::attach( id, instanceID );

    co::CommandQueue* queue = _server->getMainThreadQueue();
    LBASSERT( queue );

    registerCommand( CMD_CONFIG_NEW_LAYOUT,
                CmdFunc( this, &Config< S, C, O, L, CV, N, V >::_cmdNewLayout ),
                     queue );
    registerCommand( CMD_CONFIG_NEW_CANVAS,
                CmdFunc( this, &Config< S, C, O, L, CV, N, V >::_cmdNewCanvas ),
                     queue);
    registerCommand( CMD_CONFIG_NEW_OBSERVER,
              CmdFunc( this, &Config< S, C, O, L, CV, N, V >::_cmdNewObserver ),
                     queue);
    registerCommand( CMD_CONFIG_NEW_ENTITY_REPLY,
           CmdFunc( this, &Config< S, C, O, L, CV, N, V >::_cmdNewEntityReply ),
                     0);
}

template< class C, class V >
VisitorResult _acceptImpl( C* config, V& visitor )
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
lunchbox::RefPtr< S > Config< S, C, O, L, CV, N, V >::getServer()
{
    return _server;
}

template< class S, class C, class O, class L, class CV, class N, class V >
lunchbox::RefPtr< const S > Config< S, C, O, L, CV, N, V >::getServer() const
{
    return _server;
}

namespace
{
template< typename T, typename V > class IDFinder : public V
{
public:
    explicit IDFinder( const uint128_t& id ) : _id( id ), _result( 0 ) {}
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
    LBASSERTINFO( _observers.size() > path.observerIndex,
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
uint32_t Config< S, C, O, L, CV, N, V >::getTimeout() const
{
    if( getIAttribute( IATTR_ROBUSTNESS ) == OFF )
        return LB_TIMEOUT_INDEFINITE;
    return co::Global::getIAttribute( co::Global::IATTR_TIMEOUT_DEFAULT );
}

template< class S, class C, class O, class L, class CV, class N, class V >
L* Config< S, C, O, L, CV, N, V >::getLayout( const LayoutPath& path )
{
    LBASSERTINFO( _layouts.size() > path.layoutIndex,
                  _layouts.size() << " <= " << path.layoutIndex );

    if( _layouts.size() <= path.layoutIndex )
        return 0;

    return _layouts[ path.layoutIndex ];
}

template< class S, class C, class O, class L, class CV, class N, class V >
CV* Config< S, C, O, L, CV, N, V >::getCanvas( const CanvasPath& path )
{
    LBASSERTINFO( _canvases.size() > path.canvasIndex,
                  _canvases.size() << " <= " << path.canvasIndex );

    if( _canvases.size() <= path.canvasIndex )
        return 0;

    return _canvases[ path.canvasIndex ];
}

template< class S, class C, class O, class L, class CV, class N, class V >
void Config< S, C, O, L, CV, N, V >::_addObserver( O* observer )
{
    LBASSERT( observer->getConfig() == this );
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

    LBASSERT( observer->getConfig() == this );
    _observers.erase( i );
    setDirty( DIRTY_OBSERVERS );
    if( !isMaster( ))
        postRemove( observer );
    return true;
}

template< class S, class C, class O, class L, class CV, class N, class V >
void Config< S, C, O, L, CV, N, V >::_addLayout( L* layout )
{
    LBASSERT( layout->getConfig() == this );
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

    LBASSERT( layout->getConfig() == this );
    _layouts.erase( i );
    setDirty( DIRTY_LAYOUTS );
    if( !isMaster( ))
        postRemove( layout );
    return true;
}

template< class S, class C, class O, class L, class CV, class N, class V >
void Config< S, C, O, L, CV, N, V >::_addCanvas( CV* canvas )
{
    LBASSERT( canvas->getConfig() == this );
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

    LBASSERT( canvas->getConfig() == this );
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
    setDirty( DIRTY_LATENCY );
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
EventOCommand Config< S, C, O, L, CV, N, V >::sendError( co::NodePtr node,
    const uint32_t event, const Error& error )
{
    LBWARN << "Emit " << error << " at "
           << lunchbox::backtrace( 2 /*cut boring stack frames*/ ) << std::endl;
    EventOCommand cmd( send( node, CMD_CONFIG_EVENT ));
    cmd << event << error;
    return cmd;
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
    setDirty( DIRTY_MEMBER | DIRTY_LATENCY );
}

template< class S, class C, class O, class L, class CV, class N, class V >
void Config< S, C, O, L, CV, N, V >::_addNode( N* node )
{
    LBASSERT( node->getConfig() == this );
    _nodes.push_back( node );
}

template< class S, class C, class O, class L, class CV, class N, class V >
bool Config< S, C, O, L, CV, N, V >::_removeNode( N* node )
{
    typename Nodes::iterator i = std::find( _nodes.begin(),
                                            _nodes.end(), node );
    if( i == _nodes.end( ))
        return false;

    LBASSERT( node->getConfig() == this );
    _nodes.erase( i );
    return true;
}

template< class S, class C, class O, class L, class CV, class N, class V >
N* Config< S, C, O, L, CV, N, V >::findAppNode()
{
    for( typename Nodes::const_iterator i = _nodes.begin();
         i != _nodes.end(); ++i )
    {
        N* node = *i;
        if( node->isApplicationNode( ))
            return node;
    }
    return 0;
}

template< class S, class C, class O, class L, class CV, class N, class V >
const N* Config< S, C, O, L, CV, N, V >::findAppNode() const
{
    for( typename Nodes::const_iterator i = _nodes.begin();
         i != _nodes.end(); ++i )
    {
        const N* node = *i;
        if( node->isApplicationNode( ))
            return node;
    }
    return 0;
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
uint128_t Config< S, C, O, L, CV, N, V >::commit( const uint32_t incarnation )
{
    if( Serializable::isDirty( DIRTY_NODES ))
        commitChildren< N >( _nodes, incarnation );
    if( Serializable::isDirty( DIRTY_OBSERVERS ))
        commitChildren< O, C >( _observers, static_cast< C* >( this ),
                                CMD_CONFIG_NEW_OBSERVER, incarnation );

    // Always traverse layouts and canvases: view/segment objects may be dirty
    commitChildren< L, C >( _layouts, static_cast<C*>( this ),
                            CMD_CONFIG_NEW_LAYOUT, incarnation );
    commitChildren< CV, C >( _canvases, static_cast<C*>( this ),
                            CMD_CONFIG_NEW_CANVAS, incarnation );

    if( Serializable::isDirty( DIRTY_CANVASES ))
        commitChildren< CV, C >( _canvases, static_cast< C* >( this ),
                                 CMD_CONFIG_NEW_CANVAS, incarnation );
    return Object::commit( incarnation );
}

template< class S, class C, class O, class L, class CV, class N, class V >
void Config< S, C, O, L, CV, N, V >::serialize( co::DataOStream& os,
                                                const uint64_t dirtyBits )
{
    Object::serialize( os, dirtyBits );

    if( dirtyBits & Config::DIRTY_MEMBER )
        os << _appNodeID;
    if( dirtyBits & Config::DIRTY_ATTRIBUTES )
        os << co::Array< float >( _fAttributes, C::FATTR_ALL )
           << co::Array< int32_t >( _iAttributes, C::IATTR_ALL );
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
        is >> co::Array< float >( _fAttributes, C::FATTR_ALL )
           >> co::Array< int32_t >( _iAttributes, C::IATTR_ALL );
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
                LBASSERT( _nodes.size() == result.size( ));
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
                LBASSERT( _observers.size() == result.size( ));
            }
            if( dirtyBits & Config::DIRTY_LAYOUTS )
            {
                typename C::Layouts result;
                is.deserializeChildren( this, _layouts, result );
                _layouts.swap( result );
                LBASSERT( _layouts.size() == result.size( ));
            }
            if( dirtyBits & Config::DIRTY_CANVASES )
            {
                typename C::Canvases result;
                is.deserializeChildren( this, _canvases, result );
                _canvases.swap( result );
                LBASSERT( _canvases.size() == result.size( ));
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
        LBASSERT( mapNodeObjects( ));
        N* node = _nodes.back();
        localNode->unmapObject( node );
        _removeNode( node );
        nodeFactory->releaseNode( node );
    }

    while( !_canvases.empty( ))
    {
        LBASSERT( mapViewObjects( ));
        CV* canvas = _canvases.back();
        localNode->unmapObject( canvas );
        _removeCanvas( canvas );
        nodeFactory->releaseCanvas( canvas );
    }

    while( !_layouts.empty( ))
    {
        LBASSERT( mapViewObjects( ));
        L* layout = _layouts.back();
        localNode->unmapObject( layout );
        _removeLayout( layout );
        nodeFactory->releaseLayout( layout );
    }

    while( !_observers.empty( ))
    {
        LBASSERT( mapViewObjects( ));
        O* observer = _observers.back();
        localNode->unmapObject( observer );
        _removeObserver( observer );
        nodeFactory->releaseObserver( observer );
    }
}

//----------------------------------------------------------------------
// ICommand handlers
//----------------------------------------------------------------------
template< class S, class C, class O, class L, class CV, class N, class V >
bool Config< S, C, O, L, CV, N, V >::_cmdNewLayout( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    L* layout = 0;
    create( &layout );
    LBASSERT( layout );

    getLocalNode()->registerObject( layout );
    layout->setAutoObsolete( _data.latency + 1 );
    LBASSERT( layout->isAttached() );

    send( command.getRemoteNode(), CMD_CONFIG_NEW_ENTITY_REPLY )
            << command.read< uint32_t >() << layout->getID();
    return true;
}

template< class S, class C, class O, class L, class CV, class N, class V >
bool Config< S, C, O, L, CV, N, V >::_cmdNewCanvas( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    CV* canvas = 0;
    create( &canvas );
    LBASSERT( canvas );

    getLocalNode()->registerObject( canvas );
    canvas->setAutoObsolete( _data.latency + 1 );
    LBASSERT( canvas->isAttached() );

    send( command.getRemoteNode(), CMD_CONFIG_NEW_ENTITY_REPLY )
            << command.read< uint32_t >() << canvas->getID();
    return true;
}

template< class S, class C, class O, class L, class CV, class N, class V >
bool Config< S, C, O, L, CV, N, V >::_cmdNewObserver( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    O* observer = 0;
    create( &observer );
    LBASSERT( observer );

    getLocalNode()->registerObject( observer );
    observer->setAutoObsolete( _data.latency + 1 );
    LBASSERT( observer->isAttached() );

    send( command.getRemoteNode(), CMD_CONFIG_NEW_ENTITY_REPLY )
            << command.read< uint32_t >() << observer->getID();
    return true;
}

template< class S, class C, class O, class L, class CV, class N, class V >
bool Config< S, C, O, L, CV, N, V >::_cmdNewEntityReply( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    const uint32_t requestID = command.read< uint32_t >();
    const uint128_t& result = command.read< uint128_t >();

    getLocalNode()->serveRequest( requestID, result );

    return true;
}

template< class S, class C, class O, class L, class CV, class N, class V >
std::ostream& operator << ( std::ostream& os,
                            const Config< S, C, O, L, CV, N, V >& config )
{
    os << lunchbox::disableFlush << lunchbox::disableHeader << "config "
       << std::endl;
    os << "{" << std::endl << lunchbox::indent;

    if( !config.getName().empty( ))
        os << "name    \"" << config.getName() << '"' << std::endl;

    if( config.getLatency() != 1 )
        os << "latency " << config.getLatency() << std::endl;
    os << std::endl;

    os << "attributes" << std::endl << "{" << std::endl << lunchbox::indent
       << "robustness "
       << IAttribute( config.getIAttribute( C::IATTR_ROBUSTNESS )) << std::endl
       << "eye_base   " << config.getFAttribute( C::FATTR_EYE_BASE )
       << std::endl
       << lunchbox::exdent << "}" << std::endl;

    const typename C::Nodes& nodes = config.getNodes();
    for( typename C::Nodes::const_iterator i = nodes.begin();
         i != nodes.end(); ++i )
    {
        os << **i;
    }
    os << std::endl;

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

    config.output( os );

    os << lunchbox::exdent << "}" << std::endl << lunchbox::enableHeader
       << lunchbox::enableFlush;

    return os;
}

}
}
