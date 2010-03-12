
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

#ifndef EQFABRIC_CONFIG_H
#define EQFABRIC_CONFIG_H

#include <eq/fabric/object.h>        // base class
#include <eq/fabric/types.h>         // typedefs
#include <eq/fabric/visitorResult.h> // enum

#include <eq/net/session.h>          // base class

namespace eq
{
namespace fabric
{
    template< typename C, typename O > class Observer;
    struct ObserverPath;

    /**
     * A configuration is a visualization session driven by an application.
     *
     * The application Client can choose a configuration from a Server. The
     * Config will be instantiated though the NodeFactory. The Config groups all
     * processes of the application in a single net::Session.
     *
     * A configuration has a number of nodes, which represent the processes
     * involved in it. While the Server drives all nodes, a Config instance in a
     * given process only has its Node instantiated, that is, any given Config
     * has at most one Node.
     *
     * The Config in the application process has access to all Canvas, Segment,
     * Layout, View and Observer instances. Only the active Layout of the each
     * Canvas, the Frustum of each View and the Observer parameters are
     * writable. Views can be subclassed to attach application-specific data.
     *
     * The render client processes have only access to the current View for each
     * of their channels.
     */
    template< class S, class C, class O >
    class Config : public net::Session
    {
    public:
        /** Destruct a config. @version 1.0 */
        EQFABRIC_EXPORT virtual ~Config();

        /** @name Data Access */
        //@{
        /** @return the local server proxy. @version 1.0 */
        EQFABRIC_EXPORT base::RefPtr< S > getServer();

        typedef std::vector< O* > ObserverVector;
        /** @return the vector of observers, app-node only. @version 1.0 */
        const ObserverVector& getObservers() const { return _observers; }

        /** @return the observer of the given identifier, or 0. @version 1.0 */
        O* findObserver( const uint32_t id );

        /** @return the observer of the given identifier, or 0. @version 1.0 */
        const O* findObserver( const uint32_t id ) const;

        /** @return the first observer of the given name, or 0. @version 1.0 */
        O* findObserver( const std::string& name );

        /** @return the first observer of the given name, or 0. @version 1.0 */
        const O* findObserver( const std::string& name ) const;

        /** @return the observer at the given path. */
        O* getObserver( const ObserverPath& path );
        //@}

    protected:
        /** Construct a new config. @version 1.0 */
        EQFABRIC_EXPORT Config( base::RefPtr< S > parent );

        /** Construct a new deep copy of a config. @internal */
        Config( const Config& from, base::RefPtr< S > server );

        friend class fabric::Observer< C, O >;
        void _addObserver( O* observer );
        void _removeObserver( O* observer );

    private:
        /** The parent server. */
        base::RefPtr< S > _server;

        /** The list of observers, app-node only. */
        ObserverVector _observers;

        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };
    };
}
}
#endif // EQFABRIC_CONFIG_H

