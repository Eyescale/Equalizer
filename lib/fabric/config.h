
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

#include <eq/fabric/types.h>         // typedefs
#include <eq/fabric/visitorResult.h> // enum

#include <eq/net/session.h>          // base class

namespace eq
{
class Config;
class Node;
namespace server
{
    class Node;
    class Config;
}

namespace fabric
{
    template< class, class, class, class > class Canvas;
    template< class, class > class Observer;
    template< class, class, class > class Layout;
    template< class, class, class > class Node;
    struct CanvasPath;
    struct LayoutPath;
    struct ObserverPath;

    template< class, class, class, class, class, class > class ConfigProxy;

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
    template< class S, class C, class O, class L, class CV, class N >
    class Config : public net::Session
    {
    public:
        typedef std::vector< O* >  ObserverVector;
        typedef std::vector< L* >  LayoutVector;
        typedef std::vector< CV* > CanvasVector;
        typedef std::vector< N* >  NodeVector;

        /** Destruct a config. @version 1.0 */
        EQFABRIC_EXPORT virtual ~Config();

        /** @name Data Access */
        //@{
        /** @return the local server proxy. @version 1.0 */
        EQFABRIC_EXPORT base::RefPtr< S > getServer();

        /** @return the local server proxy. @version 1.0 */
        EQFABRIC_EXPORT base::RefPtr< S > getServer() const;

        /** @return the vector of observers, app-node only. @version 1.0 */
        const ObserverVector& getObservers() const { return _observers; }

        /** @return the vector of layouts, app-node only. @version 1.0 */
        const LayoutVector& getLayouts() const { return _layouts; }

        /** @return the vector of canvases, app-node only. @version 1.0 */
        const CanvasVector& getCanvases() const { return _canvases; }

        /** @return the vector of nodes. */
        const NodeVector& getNodes() const { return _nodes; }

        /** @return the entity of the given identifier, or 0. @version 1.0 */
        template< typename T > EQFABRIC_EXPORT T* find( const uint32_t id );

        /** @return the entity of the given identifier, or 0. @version 1.0 */
        template< typename T > EQFABRIC_EXPORT const T* find( const uint32_t id)
            const;

        /** @return the first entity of the given name, or 0. @version 1.0 */
        template< typename T >
        EQFABRIC_EXPORT T* find( const std::string& name );

        /** @return the first entity of the given name, or 0. @version 1.0 */
        template< typename T >
        EQFABRIC_EXPORT const T* find( const std::string& name ) const;

        /** @return the observer at the given path. @internal */
        O* getObserver( const ObserverPath& path );

        /** @return the layout at the given path. @internal */
        L* getLayout( const LayoutPath& path );

        /** @return the canvas at the given path. @internal */
        CV* getCanvas( const CanvasPath& path );

        /** @internal */
        template< typename T > void find( const uint32_t id, T** result );

        /** @internal */
        template< typename T > void find( const std::string& name,
                                          const T** result ) const;

        /** Activate the given canvas after it is complete. @internal */
        virtual void activateCanvas( CV* canvas ) { /* NOP */ }

        /** Set the name of the object. @version 1.0 */
        EQFABRIC_EXPORT void setName( const std::string& name );

        /** @return the name of the object. @version 1.0 */
        EQFABRIC_EXPORT const std::string& getName() const;

        /**
         * Set a message why the last operation failed.
         * 
         * The message will be transmitted to the originator of the request, for
         * example to Config::init when set from within configInit().
         *
         * @param message the error message.
         * @version 1.0
         */
        EQFABRIC_EXPORT void setErrorMessage( const std::string& message );

        /** @return the error message from the last operation. */
        EQFABRIC_EXPORT const std::string& getErrorMessage() const;
        //@}

        /** @name Operations */
        //@{
        /** 
         * Set the maximum accepted latency for this config.
         * 
         * The latency is defined as the maximum number of frames between the
         * start of a frame and the finish of the last rendering task for that
         * frame. Setting the latency of a running config flushes all pending
         * frames.
         *
         * @param latency the latency.
         * @version 1.0
         */
        virtual void setLatency( const uint32_t latency );

        /** @return the latency of this config. @version 1.0 */
        uint32_t getLatency() const { return _data.latency; }

        EQFABRIC_EXPORT uint32_t getProxyID() const; //!< @internal

        /** Back up app-specific data, excluding child data. @internal */
        EQFABRIC_EXPORT virtual void backup();

        /** Restore the last backup. @internal */
        EQFABRIC_EXPORT virtual void restore();
        //@}

    protected:
        /** Construct a new config. @version 1.0 */
        EQFABRIC_EXPORT Config( base::RefPtr< S > parent );

        /** @internal */
        //@{
        virtual O* createObserver() = 0;
        virtual void releaseObserver( O* observer ) = 0;
        virtual L* createLayout() = 0;
        virtual void releaseLayout( L* layout ) = 0;
        virtual CV* createCanvas() = 0;
        virtual void releaseCanvas( CV* canvas ) = 0;

        uint32_t register_();
        void deregister();
        void map( const uint32_t proxyID );
        virtual void unmap();
        uint32_t commit();
        void sync( const uint32_t version );

        virtual void changeLatency( const uint32_t latency ) { /* NOP */ }
        virtual bool distributeChildren() { return false; }
        //@}

    private:
        /** The parent server. */
        base::RefPtr< S > _server;

        /** The list of observers. */
        ObserverVector _observers;

        /** The list of layouts. */
        LayoutVector _layouts;

        /** The list of canvases. */
        CanvasVector _canvases;

        /** The list of nodes. */
        NodeVector _nodes;

        struct BackupData
        {
            BackupData() : latency( 1 ) {}

            /** The latency between frame start and end frame, in frames. */
            uint32_t latency;
        }
            _data, _backup;

        /** Data distribution proxy */
        ConfigProxy< S, C, O, L, CV, N >* const _proxy;
        template< class, class, class, class, class, class >
        friend class ConfigProxy;

        friend class eq::Config; // TODO remove

        template< class, class > friend class Observer;
        void _addObserver( O* observer );
        bool _removeObserver( O* observer );
        
        template< class, class, class > friend class Layout;
        void _addLayout( L* layout );
        bool _removeLayout( L* layout );
        
        template< class, class, class, class > friend class Canvas;
        void _addCanvas( CV* canvas );
        bool _removeCanvas( CV* canvas );

        template< class, class, class > friend class Node;
        void _addNode( N* node );
        EQFABRIC_EXPORT bool _removeNode( N* node );
        N* _findNode( const uint32_t id );
        
        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };
    };
}
}
#endif // EQFABRIC_CONFIG_H
