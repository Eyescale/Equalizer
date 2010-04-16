
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
#include "paths.h"

#include "configProxy.ipp"

namespace eq
{
namespace fabric
{

template< class S, class C, class O, class L, class CV, class N >
Config< S, C, O, L, CV, N >::Config( base::RefPtr< S > server )
        : net::Session()
        , _server( server )
#pragma warning( push )
#pragma warning( disable : 4355 )
        , _proxy( new ConfigProxy< S, C, O, L, CV, N >( *this ))
#pragma warning( pop )
{
    server->_addConfig( static_cast< C* >( this ));
}

template< class S, class C, class O, class L, class CV, class N >
Config< S, C, O, L, CV, N >::~Config()
{
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

    _server->_removeConfig( static_cast< C* >( this ));
    _server = 0;

    delete _proxy;
}

template< class S, class C, class O, class L, class CV, class N >
base::RefPtr< S > Config< S, C, O, L, CV, N >::getServer()
{
    return _server;
}

template< class S, class C, class O, class L, class CV, class N >
const base::RefPtr< S > Config< S, C, O, L, CV, N >::getServer() const 
{
    return _server;
}

template< class S, class C, class O, class L, class CV, class N > template< typename T >
void Config< S, C, O, L, CV, N >::find( const uint32_t id, T** result )
{
    IDFinder< T > finder( id );
    static_cast< C* >( this )->accept( finder );
    *result = finder.getResult();
}

template< class S, class C, class O, class L, class CV, class N > template< typename T >
void Config< S, C, O, L, CV, N >::find( const std::string& name, 
                                 const T** result ) const
{
    NameFinder< T > finder( name );
    static_cast< const C* >( this )->accept( finder );
    *result = finder.getResult();
}

template< class S, class C, class O, class L, class CV, class N > template< typename T >
T* Config< S, C, O, L, CV, N >::find( const uint32_t id )
{
    IDFinder< T > finder( id );
    static_cast< C* >( this )->accept( finder );
    return finder.getResult();
}

template< class S, class C, class O, class L, class CV, class N > template< typename T >
const T* Config< S, C, O, L, CV, N >::find( const uint32_t id ) const
{
    IDFinder< const T > finder( id );
    static_cast< const C* >( this )->accept( finder );
    return finder.getResult();
}

template< class S, class C, class O, class L, class CV, class N > template< typename T >
T* Config< S, C, O, L, CV, N >::find( const std::string& name )
{
    NameFinder< T > finder( name );
    static_cast< C* >( this )->accept( finder );
    return finder.getResult();
}

template< class S, class C, class O, class L, class CV, class N > template< typename T >
const T* Config< S, C, O, L, CV, N >::find( const std::string& name ) const
{
    NameFinder< const T > finder( name );
    static_cast< const C* >( this )->accept( finder );
    return finder.getResult();
}


template< class S, class C, class O, class L, class CV, class N >
O* Config< S, C, O, L, CV, N >::getObserver( const ObserverPath& path )
{
    EQASSERTINFO( _observers.size() > path.observerIndex,
                  _observers.size() << " <= " << path.observerIndex );

    if( _observers.size() <= path.observerIndex )
        return 0;

    return _observers[ path.observerIndex ];
}

template< class S, class C, class O, class L, class CV, class N >
void Config< S, C, O, L, CV, N >::setName( const std::string& name )
{
    _proxy->setName( name );
}

template< class S, class C, class O, class L, class CV, class N >
const std::string& Config< S, C, O, L, CV, N >::getName() const
{
    return _proxy->getName();
}

template< class S, class C, class O, class L, class CV, class N >
void Config< S, C, O, L, CV, N >::setErrorMessage( const std::string& message )
{
    _proxy->setErrorMessage( message );
}

template< class S, class C, class O, class L, class CV, class N >
const std::string& Config< S, C, O, L, CV, N >::getErrorMessage() const
{
    return _proxy->getErrorMessage();
}

template< class S, class C, class O, class L, class CV, class N >
L* Config< S, C, O, L, CV, N >::getLayout( const LayoutPath& path )
{
    EQASSERTINFO( _layouts.size() > path.layoutIndex,
                  _layouts.size() << " <= " << path.layoutIndex );

    if( _layouts.size() <= path.layoutIndex )
        return 0;

    return _layouts[ path.layoutIndex ];
}

template< class S, class C, class O, class L, class CV, class N >
CV* Config< S, C, O, L, CV, N >::getCanvas( const CanvasPath& path )
{
    EQASSERTINFO( _canvases.size() > path.canvasIndex,
                  _canvases.size() << " <= " << path.canvasIndex );

    if( _canvases.size() <= path.canvasIndex )
        return 0;

    return _canvases[ path.canvasIndex ];
}

template< class S, class C, class O, class L, class CV, class N >
void Config< S, C, O, L, CV, N >::_addObserver( O* observer )
{
    EQASSERT( observer->getConfig() == this );
    _observers.push_back( observer );
}

template< class S, class C, class O, class L, class CV, class N >
bool Config< S, C, O, L, CV, N >::_removeObserver( O* observer )
{
    typename ObserverVector::iterator i = std::find( _observers.begin(),
                                                     _observers.end(),
                                                     observer );
    if( i == _observers.end( ))
        return false;

    EQASSERT( observer->getConfig() == this );
    _observers.erase( i );
    return true;
}

template< class S, class C, class O, class L, class CV, class N >
void Config< S, C, O, L, CV, N >::_addLayout( L* layout )
{
    EQASSERT( layout->getConfig() == this );
    _layouts.push_back( layout );
}

template< class S, class C, class O, class L, class CV, class N >
bool Config< S, C, O, L, CV, N >::_removeLayout( L* layout )
{
    typename LayoutVector::iterator i = std::find( _layouts.begin(),
                                                   _layouts.end(), layout );
    if( i == _layouts.end( ))
        return false;

    EQASSERT( layout->getConfig() == this );
    _layouts.erase( i );
    return true;
}

template< class S, class C, class O, class L, class CV, class N >
void Config< S, C, O, L, CV, N >::_addCanvas( CV* canvas )
{
    EQASSERT( canvas->getConfig() == this );
    _canvases.push_back( canvas );
}

template< class S, class C, class O, class L, class CV, class N >
bool Config< S, C, O, L, CV, N >::_removeCanvas( CV* canvas )
{
    typename CanvasVector::iterator i = std::find( _canvases.begin(),
                                                   _canvases.end(), canvas );
    if( i == _canvases.end( ))
        return false;

    EQASSERT( canvas->getConfig() == this );
    _canvases.erase( i );
    return true;
}

template< class S, class C, class O, class L, class CV, class N >
void Config< S, C, O, L, CV, N >::setLatency( const uint32_t latency )
{
    if( _data.latency == latency )
        return;

    _data.latency = latency;
    _proxy->setDirty( ConfigProxy< S, C, O, L, CV, N >::DIRTY_MEMBER );
}

template< class S, class C, class O, class L, class CV, class N >
uint32_t Config< S, C, O, L, CV, N >::getProxyID() const
{
    return _proxy->getID();
}

template< class S, class C, class O, class L, class CV, class N >
void Config< S, C, O, L, CV, N >::backup()
{
    _backup = _data;
    _proxy->backup();
}

template< class S, class C, class O, class L, class CV, class N >
void Config< S, C, O, L, CV, N >::restore()
{
    _proxy->restore();
    if( _data.latency != _backup.latency )
    {
        _data = _backup;
        changeLatency( _data.latency );
    }
    else
        _data = _backup;
    _proxy->setDirty( ConfigProxy< S, C, O, L, CV, N >::DIRTY_MEMBER );
}

// TODO move visitors for operations on childs here.
template< class S, class C, class O, class L, class CV, class N >
uint32_t Config< S, C, O, L, CV, N >::register_()
{
    EQASSERT( _proxy->getID() == EQ_ID_INVALID );
    EQCHECK( registerObject( _proxy ));
    return _proxy->getID();
}

template< class S, class C, class O, class L, class CV, class N >
void Config< S, C, O, L, CV, N >::deregister()
{
    EQASSERT( _proxy->getID() != EQ_ID_INVALID );
    deregisterObject( _proxy );
}

template< class S, class C, class O, class L, class CV, class N >
void Config< S, C, O, L, CV, N >::map( const uint32_t proxyID )
{
    EQCHECK( mapObject( _proxy, proxyID ));
}

template< class S, class C, class O, class L, class CV, class N >
void Config< S, C, O, L, CV, N >::unmap()
{
    unmapObject( _proxy );
}

template< class S, class C, class O, class L, class CV, class N >
uint32_t Config< S, C, O, L, CV, N >::commit()
{
    return _proxy->commit();
}

template< class S, class C, class O, class L, class CV, class N >
void Config< S, C, O, L, CV, N >::sync( const uint32_t version )
{
    _proxy->sync( version );
}

template< class S, class C, class O, class L, class CV, class N >
void Config< S, C, O, L, CV, N >::_addNode( N* node )
{
    EQASSERT( node->getConfig() == this );
    _nodes.push_back( node );
}

template< class S, class C, class O, class L, class CV, class N >
bool Config< S, C, O, L, CV, N >::_removeNode( N* node )
{
    typename NodeVector::iterator i = std::find( _nodes.begin(),
                                                 _nodes.end(), node );
    if( i == _nodes.end( ))
        return false;

    EQASSERT( node->getConfig() == this );
    _nodes.erase( i );
    return true;
}

template< class S, class C, class O, class L, class CV, class N >
N* Config< S, C, O, L, CV, N >::_findNode( const uint32_t id )
{
    for( NodeVector::const_iterator i = _nodes.begin(); i != _nodes.end(); 
         ++i )
    {
        N* node = *i;
        if( node->getID() == id )
            return node;
    }
    return 0;
}

}
}
