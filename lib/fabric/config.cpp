
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

namespace eq
{
namespace fabric
{

template< class S, class C, class O, class L >
Config< S, C, O, L >::Config( base::RefPtr< S > server )
        : net::Session()
        , _server( server )
{
    server->_addConfig( static_cast< C* >( this ));
}

template< class S, class C, class O, class L >
Config< S, C, O, L >::Config( const Config& from, base::RefPtr< S > server )
        : net::Session()
        , _server( server )
{
    const ObserverVector& observers = from.getObservers();
    for( typename ObserverVector::const_iterator i = observers.begin(); 
         i != observers.end(); ++i )
    {
        new O( **i, static_cast< C* >( this ));
    }
    const LayoutVector& layouts = from.getLayouts();
    for( typename LayoutVector::const_iterator i = layouts.begin(); 
         i != layouts.end(); ++i )
    {
        new L( **i, static_cast< C* >( this ));
    }
    server->_addConfig( static_cast< C* >( this ));
}

template< class S, class C, class O, class L >
Config< S, C, O, L >::~Config()
{
    while( !_observers.empty( ))
    {
        O* observer = _observers.back();;
        _removeObserver( observer );
        delete observer;
    }

    while( !_layouts.empty( ))
    {
        L* layout = _layouts.back();;
        _removeLayout( layout );
        delete layout;
    }

    _server->_removeConfig( static_cast< C* >( this ));
    _server = 0;
}


template< class S, class C, class O, class L >
base::RefPtr< S > Config< S, C, O, L >::getServer()
{
    return _server;
}

template< class S, class C, class O, class L > template< typename T >
void Config< S, C, O, L >::find( const uint32_t id, T** result )
{
    IDFinder< T > finder( id );
    static_cast< C* >( this )->accept( finder );
    *result = finder.getResult();
}

template< class S, class C, class O, class L > template< typename T >
void Config< S, C, O, L >::find( const std::string& name, 
                                 const T** result ) const
{
    NameFinder< T > finder( name );
    static_cast< const C* >( this )->accept( finder );
    *result = finder.getResult();
}

template< class S, class C, class O, class L > template< typename T >
T* Config< S, C, O, L >::find( const uint32_t id )
{
    IDFinder< T > finder( id );
    static_cast< C* >( this )->accept( finder );
    return finder.getResult();
}

template< class S, class C, class O, class L > template< typename T >
const T* Config< S, C, O, L >::find( const uint32_t id ) const
{
    IDFinder< const T > finder( id );
    static_cast< const C* >( this )->accept( finder );
    return finder.getResult();
}

template< class S, class C, class O, class L > template< typename T >
T* Config< S, C, O, L >::find( const std::string& name )
{
    NameFinder< T > finder( name );
    static_cast< C* >( this )->accept( finder );
    return finder.getResult();
}

template< class S, class C, class O, class L > template< typename T >
const T* Config< S, C, O, L >::find( const std::string& name ) const
{
    NameFinder< const T > finder( name );
    static_cast< const C* >( this )->accept( finder );
    return finder.getResult();
}


template< class S, class C, class O, class L >
O* Config< S, C, O, L >::getObserver( const ObserverPath& path )
{
    EQASSERTINFO( _observers.size() > path.observerIndex,
                  _observers.size() << " <= " << path.observerIndex );

    if( _observers.size() <= path.observerIndex )
        return 0;

    return _observers[ path.observerIndex ];
}

template< class S, class C, class O, class L >
L* Config< S, C, O, L >::getLayout( const LayoutPath& path )
{
    EQASSERTINFO( _layouts.size() > path.layoutIndex,
                  _layouts.size() << " <= " << path.layoutIndex );

    if( _layouts.size() <= path.layoutIndex )
        return 0;

    return _layouts[ path.layoutIndex ];
}

template< class S, class C, class O, class L >
void Config< S, C, O, L >::_addObserver( O* observer )
{
    EQASSERT( observer->getConfig() == this );
    _observers.push_back( observer );
}

template< class S, class C, class O, class L >
bool Config< S, C, O, L >::_removeObserver( O* observer )
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

template< class S, class C, class O, class L >
void Config< S, C, O, L >::_addLayout( L* layout )
{
    EQASSERT( layout->getConfig() == this );
    _layouts.push_back( layout );
}

template< class S, class C, class O, class L >
bool Config< S, C, O, L >::_removeLayout( L* layout )
{
    typename LayoutVector::iterator i = std::find( _layouts.begin(),
                                                   _layouts.end(), layout );
    if( i == _layouts.end( ))
        return false;

    EQASSERT( layout->getConfig() == this );
    _layouts.erase( i );
    return true;
}

}
}
