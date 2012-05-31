
/* Copyright (c) 2012, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "zeroconf.h"

#ifdef CO_USE_SERVUS
#  include <servus/service.h>
#  include <map>
#endif

namespace co
{
static const std::string empty_;

#ifdef CO_USE_SERVUS
namespace detail
{
typedef std::map< std::string, std::string > ValueMap;
typedef std::map< std::string, ValueMap > HostMap;
typedef ValueMap::const_iterator ValueMapCIter;
typedef HostMap::const_iterator HostMapCIter;

class Zeroconf
{
public:
    Zeroconf( servus::Service& service )
            : service_( service )
    {
        service_.getData( hostMap_ );
    }

    void set( const std::string& key, const std::string& value )
    {
        service_.set( key, value );
    }

    Strings getHosts() const
    {
        Strings hosts;
        for( HostMapCIter i = hostMap_.begin(); i != hostMap_.end(); ++i )
            hosts.push_back( i->first );
        return hosts;
    }

    Strings getKeys( const std::string& host ) const
    {
        Strings keys;
        HostMapCIter i = hostMap_.find( host );
        if( i == hostMap_.end( ))
            return keys;

        const ValueMap& values = i->second;
        for( ValueMapCIter j = values.begin(); j != values.end(); ++j )
            keys.push_back( j->first );
        return keys;
    }

    bool containsKey( const std::string& host, const std::string& key ) const
    {
        HostMapCIter i = hostMap_.find( host );
        if( i == hostMap_.end( ))
            return false;

        const ValueMap& values = i->second;
        ValueMapCIter j = values.find( key );
        if( j == values.end( ))
            return false;
        return true;
    }

    const std::string& get( const std::string& host,
                            const std::string& key ) const
    {
        HostMapCIter i = hostMap_.find( host );
        if( i == hostMap_.end( ))
            return empty_;

        const ValueMap& values = i->second;
        ValueMapCIter j = values.find( key );
        if( j == values.end( ))
            return empty_;
        return j->second;
    }

private:
    servus::Service& service_;
    HostMap hostMap_; //!< copy of discovered data
};
}
#endif

#ifdef CO_USE_SERVUS
Zeroconf::Zeroconf( servus::Service& service )
        : _impl( new detail::Zeroconf( service ))
#else
Zeroconf::Zeroconf()
        : _impl( 0 )
#endif
{}

Zeroconf::Zeroconf( const Zeroconf& from )
#ifdef CO_USE_SERVUS
        : _impl( new detail::Zeroconf( *from._impl ))
#else
        : _impl( 0 )
#endif
{
}

Zeroconf::~Zeroconf()
{
#ifdef CO_USE_SERVUS
    delete _impl;
#endif
}

Zeroconf& Zeroconf::operator = ( const Zeroconf& rhs )
{
#ifdef CO_USE_SERVUS
    if( this != &rhs )
    {
        delete _impl;
        _impl = new detail::Zeroconf( *rhs._impl );
    }
#endif
    return *this;
}

void Zeroconf::set( const std::string& key, const std::string& value )
{
#ifdef CO_USE_SERVUS
    _impl->set( key, value );
#endif
}

Strings Zeroconf::getHosts() const
{
#ifdef CO_USE_SERVUS
    return _impl->getHosts();
#endif
    return Strings();
}

Strings Zeroconf::getKeys( const std::string& host ) const
{
#ifdef CO_USE_SERVUS
    return _impl->getKeys( host );
#endif
    return Strings();
}

bool Zeroconf::containsKey( const std::string& host,
                            const std::string& key ) const
{
#ifdef CO_USE_SERVUS
    return _impl->containsKey( host, key );
#endif
    return false;
}

const std::string& Zeroconf::get( const std::string& host,
                                  const std::string& key ) const
{
#ifdef CO_USE_SERVUS
    return _impl->get( host, key );
#endif
    return empty_;
}

}
