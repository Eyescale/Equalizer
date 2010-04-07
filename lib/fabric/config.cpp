
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

template< class S, class C, class O, class L, class CV >
Config< S, C, O, L, CV >::Config( base::RefPtr< S > server )
        : net::Session()
        , _server( server )
        , _latency( 1 )
#pragma warning( push )
#pragma warning( disable : 4355 )
        , _proxy( new ConfigProxy< S, C, O, L, CV >( *this ))
#pragma warning( pop )
{
    server->_addConfig( static_cast< C* >( this ));
}

template< class S, class C, class O, class L, class CV >
Config< S, C, O, L, CV >::Config( const Config& from, base::RefPtr< S > server )
        : net::Session()
        , _server( server )
        , _latency( from._latency )
#pragma warning( push )
#pragma warning( disable : 4355 )
        , _proxy( new ConfigProxy< S, C, O, L, CV >( *this ))
#pragma warning( pop )
{
    server->_addConfig( static_cast< C* >( this ));
}

template< class S, class C, class O, class L, class CV >
Config< S, C, O, L, CV >::~Config()
{
    while( !_canvases.empty( ))
    {
        CV* canvas = _canvases.back();;
        _removeCanvas( canvas );
        delete canvas;
    }

    while( !_layouts.empty( ))
    {
        L* layout = _layouts.back();;
        _removeLayout( layout );
        delete layout;
    }

    while( !_observers.empty( ))
    {
        O* observer = _observers.back();;
        _removeObserver( observer );
        delete observer;
    }

    _server->_removeConfig( static_cast< C* >( this ));
    _server = 0;

    delete _proxy;
}

template< class S, class C, class O, class L, class CV >
base::RefPtr< S > Config< S, C, O, L, CV >::getServer()
{
    return _server;
}

template< class S, class C, class O, class L, class CV > template< typename T >
void Config< S, C, O, L, CV >::find( const uint32_t id, T** result )
{
    IDFinder< T > finder( id );
    static_cast< C* >( this )->accept( finder );
    *result = finder.getResult();
}

template< class S, class C, class O, class L, class CV > template< typename T >
void Config< S, C, O, L, CV >::find( const std::string& name, 
                                 const T** result ) const
{
    NameFinder< T > finder( name );
    static_cast< const C* >( this )->accept( finder );
    *result = finder.getResult();
}

template< class S, class C, class O, class L, class CV > template< typename T >
T* Config< S, C, O, L, CV >::find( const uint32_t id )
{
    IDFinder< T > finder( id );
    static_cast< C* >( this )->accept( finder );
    return finder.getResult();
}

template< class S, class C, class O, class L, class CV > template< typename T >
const T* Config< S, C, O, L, CV >::find( const uint32_t id ) const
{
    IDFinder< const T > finder( id );
    static_cast< const C* >( this )->accept( finder );
    return finder.getResult();
}

template< class S, class C, class O, class L, class CV > template< typename T >
T* Config< S, C, O, L, CV >::find( const std::string& name )
{
    NameFinder< T > finder( name );
    static_cast< C* >( this )->accept( finder );
    return finder.getResult();
}

template< class S, class C, class O, class L, class CV > template< typename T >
const T* Config< S, C, O, L, CV >::find( const std::string& name ) const
{
    NameFinder< const T > finder( name );
    static_cast< const C* >( this )->accept( finder );
    return finder.getResult();
}


template< class S, class C, class O, class L, class CV >
O* Config< S, C, O, L, CV >::getObserver( const ObserverPath& path )
{
    EQASSERTINFO( _observers.size() > path.observerIndex,
                  _observers.size() << " <= " << path.observerIndex );

    if( _observers.size() <= path.observerIndex )
        return 0;

    return _observers[ path.observerIndex ];
}

template< class S, class C, class O, class L, class CV >
L* Config< S, C, O, L, CV >::getLayout( const LayoutPath& path )
{
    EQASSERTINFO( _layouts.size() > path.layoutIndex,
                  _layouts.size() << " <= " << path.layoutIndex );

    if( _layouts.size() <= path.layoutIndex )
        return 0;

    return _layouts[ path.layoutIndex ];
}

template< class S, class C, class O, class L, class CV >
CV* Config< S, C, O, L, CV >::getCanvas( const CanvasPath& path )
{
    EQASSERTINFO( _canvases.size() > path.canvasIndex,
                  _canvases.size() << " <= " << path.canvasIndex );

    if( _canvases.size() <= path.canvasIndex )
        return 0;

    return _canvases[ path.canvasIndex ];
}

template< class S, class C, class O, class L, class CV >
void Config< S, C, O, L, CV >::_addObserver( O* observer )
{
    EQASSERT( observer->getConfig() == this );
    _observers.push_back( observer );
}

template< class S, class C, class O, class L, class CV >
bool Config< S, C, O, L, CV >::_removeObserver( O* observer )
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

template< class S, class C, class O, class L, class CV >
void Config< S, C, O, L, CV >::_addLayout( L* layout )
{
    EQASSERT( layout->getConfig() == this );
    _layouts.push_back( layout );
}

template< class S, class C, class O, class L, class CV >
bool Config< S, C, O, L, CV >::_removeLayout( L* layout )
{
    typename LayoutVector::iterator i = std::find( _layouts.begin(),
                                                   _layouts.end(), layout );
    if( i == _layouts.end( ))
        return false;

    EQASSERT( layout->getConfig() == this );
    _layouts.erase( i );
    return true;
}

template< class S, class C, class O, class L, class CV >
void Config< S, C, O, L, CV >::_addCanvas( CV* canvas )
{
    EQASSERT( canvas->getConfig() == this );
    _canvases.push_back( canvas );
}

template< class S, class C, class O, class L, class CV >
bool Config< S, C, O, L, CV >::_removeCanvas( CV* canvas )
{
    typename CanvasVector::iterator i = std::find( _canvases.begin(),
                                                   _canvases.end(), canvas );
    if( i == _canvases.end( ))
        return false;

    EQASSERT( canvas->getConfig() == this );
    _canvases.erase( i );
    return true;
}

template< class S, class C, class O, class L, class CV >
void Config< S, C, O, L, CV >::setLatency( const uint32_t latency )
{
    if( _latency == latency )
        return;

    _latency = latency;
    _proxy->setDirty( ConfigProxy< S, C, O, L, CV >::DIRTY_MEMBER );
}

// TODO move visitors for operations on childs here.
template< class S, class C, class O, class L, class CV >
uint32_t Config< S, C, O, L, CV >::register_()
{
    EQASSERT( _proxy->getID() == EQ_ID_INVALID );
    EQCHECK( registerObject( _proxy ));
    return _proxy->getID();
}

template< class S, class C, class O, class L, class CV >
void Config< S, C, O, L, CV >::deregister()
{
    EQASSERT( _proxy->getID() != EQ_ID_INVALID );
    deregisterObject( _proxy );
}

template< class S, class C, class O, class L, class CV >
void Config< S, C, O, L, CV >::map( const uint32_t proxyID )
{
    EQCHECK( mapObject( _proxy, proxyID ));
}

template< class S, class C, class O, class L, class CV >
void Config< S, C, O, L, CV >::unmap()
{
    unmapObject( _proxy );
}

template< class S, class C, class O, class L, class CV >
uint32_t Config< S, C, O, L, CV >::commit()
{
    return _proxy->commit();
}

template< class S, class C, class O, class L, class CV >
void Config< S, C, O, L, CV >::sync( const uint32_t version )
{
    _proxy->sync( version );
}

}
}
