
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

#ifndef CO_ZEROCONF_H
#define CO_ZEROCONF_H

#include <co/api.h>
#include <co/types.h>

namespace co
{
namespace detail { class Zeroconf; }
    /**
     * A zeroconf communicator.
     *
     * When Collage is compiled with Servus support (CO_USE_SERVUS), it uses the
     * ZeroConf service "_collage._tcp" to announce the presence of a listening
     * LocalNode using the zeroconf protocol, unless the LocalNode has no
     * connections. This class may be used to add additional key/value pairs to
     * this service to announce application-specific data, and to retrieve a
     * snapshot of the discovered nodes.
     * Internal keys start with 'co_', this
     * prefix should not be used by applications. Please refer to the
     * documentation of servus::Service::set() for details.
     *
     * When Collage is compiled without Servus support, this class implements
     * dummy functionality.
     */
    class Zeroconf
    {
    public:
        /** Create a copy of a zeroconf communicator. */
        CO_API Zeroconf( const Zeroconf& from );

        /** Destruct this zeroconf communicator. */
        CO_API ~Zeroconf();

        /** Assign the data from another zeroconf communicator. */
        CO_API Zeroconf& operator = ( const Zeroconf& rhs );

        /**
         * Set a key/value pair to be announced.
         *
         * Keys should be at most eight characters, and values are truncated to
         * 255 characters. The total length of all keys and values cannot exceed
         * 65535 characters. Setting a value on an announced service causes an
         * update which needs some time to propagate after this function
         * returns.
         *
         */
        CO_API void set( const std::string& key, const std::string& value );

        /** @return all instances found at the time of creation. */
        CO_API Strings getInstances() const;

        /** @return all keys discovered on the given instance. */
        CO_API Strings getKeys( const std::string& instance ) const;

        /** @return true if the given key was discovered. */
        CO_API bool containsKey( const std::string& instance,
                                 const std::string& key ) const;

        /** @return the value of the given key on the given instance. */
        CO_API const std::string& get( const std::string& instance,
                                       const std::string& key ) const;
    private:
        Zeroconf();
        Zeroconf( servus::Service& service );
        friend class LocalNode;

        detail::Zeroconf* _impl;
    };
}
#endif // CO_ZEROCONF_H
