
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
#include <eq/fabric/object.h>        // DIRTY_CUSTOM enum

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
    struct CanvasPath;
    struct LayoutPath;
    struct ObserverPath;

    template< class, class, class, class, class, class, class >
    class ConfigProxy;

    /** A configuration is a visualization session driven by an application. */
    template< class S, class C, class O, class L, class CV, class N, class V >
    class Config : public net::Session
    {
    public:
        typedef std::vector< O* >  Observers; //!< A vector of observers
        typedef std::vector< L* >  Layouts;   //!< A vector of layouts
        typedef std::vector< CV* > Canvases;  //!< A vector of canvases
        typedef std::vector< N* >  Nodes;     //!< A vector of nodes

        /** @name Data Access */
        //@{
        /** @return the local server proxy. @version 1.0 */
        EQFABRIC_EXPORT base::RefPtr< S > getServer();

        /** @return the local server proxy. @version 1.0 */
        EQFABRIC_EXPORT base::RefPtr< const S > getServer() const;

        /** @return the vector of observers, app-node only. @version 1.0 */
        const Observers& getObservers() const { return _observers; }

        /** @return the vector of layouts, app-node only. @version 1.0 */
        const Layouts& getLayouts() const { return _layouts; }

        /** @return the vector of canvases, app-node only. @version 1.0 */
        const Canvases& getCanvases() const { return _canvases; }

        /**
         * @return the vector of nodes instantiated in this process.
         * @version 1.0
         */
        const Nodes& getNodes() const { return _nodes; }

        /** 
         * Traverse this config and all children using a config visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         * @version 1.0
         */
        EQFABRIC_EXPORT VisitorResult accept( V& visitor );

        /** Const-version of accept(). @version 1.0 */
        EQFABRIC_EXPORT VisitorResult accept( V& visitor ) const;

        /** @return the entity of the given identifier, or 0. @version 1.0 */
        template< typename T > EQFABRIC_EXPORT T* find( const uint32_t id );

        /** @return the entity of the given identifier, or 0. @version 1.0 */
        template< typename T > 
        EQFABRIC_EXPORT const T* find( const uint32_t id ) const;

        /** @return the first entity of the given name, or 0. @version 1.0 */
        template< typename T >
        EQFABRIC_EXPORT T* find( const std::string& name );

        /** @return the first entity of the given name, or 0. @version 1.0 */
        template< typename T >
        EQFABRIC_EXPORT const T* find( const std::string& name ) const;

        /** @internal @return the observer at the given path. */
        O* getObserver( const ObserverPath& path );

        /** @internal @return the layout at the given path. */
        L* getLayout( const LayoutPath& path );

        /** @internal @return the canvas at the given path. */
        CV* getCanvas( const CanvasPath& path );

        /** @internal */
        template< typename T > void find( const uint32_t id, T** result );

        /** @internal */
        template< typename T > void find( const std::string& name,
                                          const T** result ) const;

        /** @internal Update or init the given canvas in a running config. */
        virtual void updateCanvas( CV* canvas ) { /* NOP */ }

        /** @internal Init the given canvas in a running config. */
        virtual void exitCanvas( CV* canvas ) { /* NOP */ }

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

        /** @name Attributes */
        //@{
        // Note: also update string array initialization in config.ipp
        enum FAttribute
        {
            FATTR_EYE_BASE, //!< The default interocular distance in meters
            FATTR_VERSION,  //!< The version of the file loaded
            FATTR_FILL1,
            FATTR_FILL2,
            FATTR_ALL
        };
        
        /** @internal */
        void setFAttribute( const FAttribute attr, const float value )
            { _fAttributes[attr] = value; }

        /** @return the given floating-point attribute. */
        float getFAttribute( const FAttribute attr ) const
            { return _fAttributes[attr]; }
        /** @internal */
        static const std::string& getFAttributeString( const FAttribute attr );
        //@}
 

        /** @name Operations */
        //@{
        /** 
         * Set the maximum accepted latency for this config.
         * 
         * The latency is defined as the maximum number of frames between the
         * start of a frame and the finish of the last rendering task for that
         * frame. Setting the latency of a running config finishes all pending
         * frames.
         *
         * @param latency the latency.
         * @version 1.0
         */
        virtual void setLatency( const uint32_t latency );

        /** @return the latency of this config. @version 1.0 */
        uint32_t getLatency() const { return _data.latency; }

        EQFABRIC_EXPORT uint32_t getProxyID() const; //!< @internal

        /** @internal Back up app-specific data, excluding child data. */
        EQFABRIC_EXPORT virtual void backup();

        /** @internal Restore the last backup. */
        EQFABRIC_EXPORT virtual void restore();

        /** @internal @sa Serializable::setDirty() */
        void setDirty( const uint64_t bits );

        /** @internal Get the current version. */
        uint32_t getVersion() const;

        /** @internal Commit a new version. */
        EQFABRIC_EXPORT uint32_t commit();

        /** @internal Sync to the given version. */
        void sync( const uint32_t version );
        //@}

        void output( std::ostream& os ) const {} //!< @internal

    protected:
        /** @internal Construct a new config. */
        EQFABRIC_EXPORT Config( base::RefPtr< S > parent );

        /** @internal Destruct a config. */
        EQFABRIC_EXPORT virtual ~Config();

        /** @internal */
        EQFABRIC_EXPORT virtual void notifyMapped( net::NodePtr node );

        /** @internal Execute the slave remove request. */
        EQFABRIC_EXPORT virtual void _removeChild( const uint32_t id )
            { EQUNIMPLEMENTED; }

        uint32_t register_(); //!< @internal
        void deregister(); //!< @internal
        void map( const net::ObjectVersion proxy ); //!< @internal
        virtual void unmap(); //!< @internal
        template< class, class, class, class > friend class Server; // map/unmap

        void setAppNodeID( const net::NodeID& nodeID ); //!< @internal
        /** @internal */
        const net::NodeID& getAppNodeID() const { return _appNodeID; }

        /** @internal */
        virtual void changeLatency( const uint32_t latency ) { /* NOP */ }
        virtual bool mapViewObjects() const { return false; } //!< @internal
        virtual bool mapNodeObjects() const { return false; } //!< @internal

        /** @internal */
        virtual VisitorResult _acceptCompounds( V& visitor )
            { return TRAVERSE_CONTINUE; }
        /** @internal */
        virtual VisitorResult _acceptCompounds( V& visitor ) const
            { return TRAVERSE_CONTINUE; }
        template< class C2, class V2 >
        friend VisitorResult _acceptImpl( C2*, V2& );

        N* _findNode( const uint32_t id ); //!< @internal
        //@}

    private:
        enum DirtyBits
        {
            DIRTY_MEMBER     = Object::DIRTY_CUSTOM << 0,
            DIRTY_ATTRIBUTES = Object::DIRTY_CUSTOM << 1,
            DIRTY_OBSERVERS  = Object::DIRTY_CUSTOM << 2,
            DIRTY_LAYOUTS    = Object::DIRTY_CUSTOM << 3,
            DIRTY_CANVASES   = Object::DIRTY_CUSTOM << 4,
            DIRTY_LATENCY    = Object::DIRTY_CUSTOM << 5,
            DIRTY_NODES      = Object::DIRTY_CUSTOM << 6,
        };

        /** The parent server. */
        base::RefPtr< S > _server;
        
        /** float attributes. */
        float _fAttributes[FATTR_ALL];

        /** The list of observers. */
        Observers _observers;

        /** The list of layouts. */
        Layouts _layouts;

        /** The list of canvases. */
        Canvases _canvases;

        /** The list of nodes. */
        Nodes _nodes;

        /** The node identifier of the node running the application thread. */
        net::NodeID _appNodeID;

        struct BackupData
        {
            BackupData() : latency( 1 ) {}

            /** The latency between frame start and end frame, in frames. */
            uint32_t latency;
        }
            _data, _backup;

        /** Data distribution proxy */
        ConfigProxy< S, C, O, L, CV, N, V >* const _proxy;
        template< class, class, class, class, class, class, class >
        friend class ConfigProxy;
        
        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };

        template< class, class > friend class Observer;
        void _addObserver( O* observer );
        bool _removeObserver( O* observer );
        
        template< class, class, class > friend class Layout;
        void _addLayout( L* layout );
        bool _removeLayout( L* layout );
        
        template< class, class, class, class > friend class Canvas;
        void _addCanvas( CV* canvas );
        bool _removeCanvas( CV* canvas );

        template< class, class, class, class > friend class Node;
        void _addNode( N* node );
        EQFABRIC_EXPORT bool _removeNode( N* node );

        typedef net::CommandFunc< Config< S, C, O, L, CV, N, V > > CmdFunc;
        bool _cmdNewLayout( net::Command& command );
        bool _cmdNewCanvas( net::Command& command );
        bool _cmdNewObserver( net::Command& command );
        bool _cmdNewEntityReply( net::Command& command );
    };

    template< class S, class C, class O, class L, class CV, class N, class V >
    EQFABRIC_EXPORT std::ostream& operator << ( std::ostream& os,
                                 const Config< S, C, O, L, CV, N, V >& config );
}
}
#endif // EQFABRIC_CONFIG_H
