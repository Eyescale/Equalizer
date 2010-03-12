
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

template< class S, class C, class O >
Config< S, C, O >::Config( base::RefPtr< S > server )
        : net::Session()
        , _server( server )
{
    server->_addConfig( static_cast< C* >( this ));
}

template< class S, class C, class O >
Config< S, C, O >::Config( const Config& from, base::RefPtr< S > server )
        : net::Session()
        , _server( server )
{
    const ObserverVector& observers = from.getObservers();
    for( typename ObserverVector::const_iterator i = observers.begin(); 
         i != observers.end(); ++i )
    {
        new O( **i, static_cast< C* >( this ));
    }
    server->_addConfig( static_cast< C* >( this ));
}

template< class S, class C, class O >
Config< S, C, O >::~Config()
{
    while( !_observers.empty( ))
    {
        O* observer = _observers.back();;
        _removeObserver( observer );
        delete observer;
    }
    EQASSERT( _observers.empty( ));
    _server->_removeConfig( static_cast< C* >( this ));
    _server = 0;
}


template< class S, class C, class O >
base::RefPtr< S > Config< S, C, O >::getServer()
{
    return _server;
}

template< class S, class C, class O >
O* Config< S, C, O >::findObserver( const uint32_t id )
{
    IDFinder< O > finder( id );
    static_cast< C* >( this )->accept( finder );
    return finder.getResult();
}

template< class S, class C, class O >
const O* Config< S, C, O >::findObserver( const uint32_t id ) const
{
    IDFinder< const O > finder( id );
    static_cast< const C* >( this )->accept( finder );
    return finder.getResult();
}

template< class S, class C, class O >
O* Config< S, C, O >::findObserver( const std::string& name )
{
    ObserverFinder finder( name );
    static_cast< C* >( this )->accept( finder );
    return finder.getResult();
}

template< class S, class C, class O >
const O* Config< S, C, O >::findObserver( const std::string& name ) const
{
    ConstObserverFinder finder( name );
    static_cast< const C* >( this )->accept( finder );
    return finder.getResult();
}


template< class S, class C, class O >
O* Config< S, C, O >::getObserver( const ObserverPath& path )
{
    EQASSERTINFO( _observers.size() > path.observerIndex,
                  _observers.size() << " <= " << path.observerIndex );

    if( _observers.size() <= path.observerIndex )
        return 0;

    return _observers[ path.observerIndex ];
}

template< class S, class C, class O >
void Config< S, C, O >::_addObserver( O* observer )
{
    EQASSERT( observer->getConfig() == this );
    _observers.push_back( observer );
}

template< class S, class C, class O >
void Config< S, C, O >::_removeObserver( O* observer )
{
    typename ObserverVector::iterator i = std::find( _observers.begin(),
                                                     _observers.end(), 
                                                     observer );
    if( i == _observers.end( ))
        return;

    EQASSERT( observer->getConfig() == this );
    _observers.erase( i );
}

}
}
