
/* Copyright (c) 2005-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Cedric Stalder <cedric Stalder@gmail.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include <eq/fabric/api.h>
#include <eq/fabric/types.h>         // typedefs
#include <eq/fabric/object.h>        // DIRTY_CUSTOM enum

namespace eq
{
namespace fabric
{
/** Base data class for a configuration. @sa eq::Config */
template< class S, class C, class O, class L, class CV, class N, class V >
// cppcheck-suppress noConstructor
class Config : public Object
{
public:
    typedef std::vector< O* >  Observers; //!< A vector of observers
    typedef std::vector< L* >  Layouts;   //!< A vector of layouts
    typedef std::vector< CV* > Canvases;  //!< A vector of canvases
    typedef std::vector< N* >  Nodes;     //!< A vector of nodes

    /** @name Data Access */
    //@{
    /** @return the local server proxy. @version 1.0 */
    EQFABRIC_INL lunchbox::RefPtr< S > getServer();

    /** @return the local server proxy. @version 1.0 */
    EQFABRIC_INL lunchbox::RefPtr< const S > getServer() const;

    /** @return the vector of observers, app-node only. @version 1.0 */
    const Observers& getObservers() const { return _observers; }

    /** @return the vector of layouts, app-node only. @version 1.0 */
    const Layouts& getLayouts() const { return _layouts; }

    /** @return the vector of canvases, app-node only. @version 1.0 */
    const Canvases& getCanvases() const { return _canvases; }

    /**
     * @internal
     * @return the timeout in ms or LB_TIMEOUT_INDEFINITE for failures.
     */
    EQFABRIC_INL uint32_t getTimeout() const;

    /**
     * @return the vector of nodes instantiated in this process.
     * @version 1.0
     */
    const Nodes& getNodes() const { return _nodes; }

    EQFABRIC_INL N* findAppNode(); //!< @internal
    EQFABRIC_INL const N* findAppNode() const; //!< @internal

    /**
     * Perform a depth-first traversal of this config.
     *
     * @param visitor the visitor.
     * @return the result of the visitor traversal.
     * @version 1.0
     */
    EQFABRIC_INL VisitorResult accept( V& visitor );

    /** Const-version of accept(). @version 1.0 */
    EQFABRIC_INL VisitorResult accept( V& visitor ) const;

    /** @return the entity of the given identifier, or 0. @version 1.0 */
    template< typename T > EQFABRIC_INL T* find( const uint128_t& id );

    /** @return the entity of the given identifier, or 0. @version 1.0 */
    template< typename T >
    EQFABRIC_INL const T* find( const uint128_t& id ) const;

    /** @return the first entity of the given name, or 0. @version 1.0 */
    template< typename T >
    EQFABRIC_INL T* find( const std::string& name );

    /** @return the first entity of the given name, or 0. @version 1.0 */
    template< typename T >
    EQFABRIC_INL const T* find( const std::string& name ) const;

    /** @internal @return the observer at the given path. */
    O* getObserver( const ObserverPath& path );

    /** @internal @return the layout at the given path. */
    L* getLayout( const LayoutPath& path );

    /** @internal @return the canvas at the given path. */
    CV* getCanvas( const CanvasPath& path );

    /** @internal */
    template< typename T > void find( const uint128_t& id, T** result );

    /** @internal */
    template< typename T > void find( const std::string& name,
                                      const T** result ) const;

    /** @internal Update or init the given canvas in a running config. */
    virtual void updateCanvas( CV* ) { /* NOP */ }

    /** @internal Init the given canvas in a running config. */
    virtual void exitCanvas( CV* ) { /* NOP */ }
    //@}

    /** @name Attributes */
    //@{
    // Note: also update string array initialization in config.ipp

    /** Floating-point attributes */
    enum FAttribute
    {
        FATTR_EYE_BASE, //!< The default interocular distance in meters
        FATTR_VERSION,  //!< The version of the file loaded
        FATTR_LAST,
        FATTR_ALL = FATTR_LAST + 5
    };

    /** Integer attributes. */
    enum IAttribute
    {
        IATTR_ROBUSTNESS, //!< Tolerate resource failures
        IATTR_LAST,
        IATTR_ALL = IATTR_LAST + 5
    };

    /** @internal */
    void setFAttribute( const FAttribute attr, const float value )
    { _fAttributes[attr] = value; }
    /** @internal */
    void setIAttribute( const IAttribute attr, const int32_t value )
    { _iAttributes[attr] = value; }

    /** @return the given floating-point attribute. */
    float getFAttribute( const FAttribute attr ) const
    { return _fAttributes[attr]; }

    /** @return the given integer attribute. */
    int32_t getIAttribute( const IAttribute attr ) const
    { return _iAttributes[attr]; }

    /** @internal */
    static const std::string& getFAttributeString( const FAttribute attr );
    /** @internal */
    static const std::string& getIAttributeString( const IAttribute attr );
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

    /** @internal Restore the last backup. */
    EQFABRIC_INL virtual void restore();
    //@}

    virtual void output( std::ostream& ) const {} //!< @internal
    void create( O** observer ); //!< @internal
    void release( O* observer ); //!< @internal
    void create( L** layout );   //!< @internal
    void release( L* layout );   //!< @internal
    void create( CV** canvas );  //!< @internal
    void release( CV* canvas );  //!< @internal
    void create( N** node );     //!< @internal
    void release( N* node );     //!< @internal

protected:
    /** @internal Construct a new config. */
    EQFABRIC_INL explicit Config( lunchbox::RefPtr< S > parent );

    /** @internal Destruct a config. */
    EQFABRIC_INL virtual ~Config();

    /** @internal */
    EQFABRIC_INL virtual void attach( const uint128_t& id,
                                      const uint32_t instanceID );

    /** @internal */
    EQFABRIC_INL virtual void serialize( co::DataOStream& os,
                                         const uint64_t dirtyBits );
    EQFABRIC_INL virtual void deserialize( co::DataIStream& is,
                                           const uint64_t dirtyBits );
    EQFABRIC_INL virtual void notifyDetach();

    /** @internal Execute the slave remove request. */
    virtual void _removeChild( const uint128_t& )
    { LBUNIMPLEMENTED; }

    template< class, class, class, class, class, class >
    friend class Server;

    void setAppNodeID( const co::NodeID& nodeID ); //!< @internal
    /** @internal */
    const co::NodeID& getAppNodeID() const { return _appNodeID; }

    /** @internal */
    EQFABRIC_INL EventOCommand sendError( co::NodePtr node,
                                          const uint32_t event,
                                          const Error& error );

    virtual void changeLatency( const uint32_t ) { /* NOP */ }//!< @internal
    virtual bool mapViewObjects() const { return false; } //!< @internal
    virtual bool mapNodeObjects() const { return false; } //!< @internal

    /** @internal */
    virtual VisitorResult _acceptCompounds( V& )
    { return TRAVERSE_CONTINUE; }
    /** @internal */
    virtual VisitorResult _acceptCompounds( V& ) const
    { return TRAVERSE_CONTINUE; }
    template< class C2, class V2 >
    friend VisitorResult _acceptImpl( C2*, V2& );

    N* _findNode( const uint128_t& id ); //!< @internal

    /** @internal */
    EQFABRIC_INL virtual uint128_t commit( const uint32_t incarnation =
                                           CO_COMMIT_NEXT );
    //@}

private:
    /** The parent server. */
    lunchbox::RefPtr< S > _server;

    /** Float attributes. */
    float _fAttributes[FATTR_ALL];

    /** Integer attributes. */
    int32_t _iAttributes[IATTR_ALL];

    /** The list of observers. */
    Observers _observers;

    /** The list of layouts. */
    Layouts _layouts;

    /** The list of canvases. */
    Canvases _canvases;

    /** The list of nodes. */
    Nodes _nodes;

    /** The node identifier of the node running the application thread. */
    co::NodeID _appNodeID;

    struct BackupData
    {
        BackupData() : latency( 1 ) {}

        /** The latency between frame start and end frame, in frames. */
        uint32_t latency;
    }
        _data, _backup;

    struct Private;
    Private* _private; // placeholder for binary-compatible changes

    enum DirtyBits
    {
        DIRTY_MEMBER     = Object::DIRTY_CUSTOM << 0,
        DIRTY_LATENCY    = Object::DIRTY_CUSTOM << 1,
        DIRTY_ATTRIBUTES = Object::DIRTY_CUSTOM << 2,
        DIRTY_NODES      = Object::DIRTY_CUSTOM << 3,
        DIRTY_OBSERVERS  = Object::DIRTY_CUSTOM << 4,
        DIRTY_LAYOUTS    = Object::DIRTY_CUSTOM << 5,
        DIRTY_CANVASES   = Object::DIRTY_CUSTOM << 6,
        DIRTY_CONFIG_BITS =
        DIRTY_MEMBER | DIRTY_ATTRIBUTES | DIRTY_OBSERVERS |
        DIRTY_LAYOUTS | DIRTY_CANVASES | DIRTY_NODES | DIRTY_LATENCY
    };

    /** @internal @return the bits to be re-committed by the master. */
    virtual uint64_t getRedistributableBits() const
    { return DIRTY_CONFIG_BITS; }

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
    EQFABRIC_INL bool _removeNode( N* node );

    typedef co::CommandFunc< Config< S, C, O, L, CV, N, V > > CmdFunc;
    bool _cmdNewLayout( co::ICommand& command );
    bool _cmdNewCanvas( co::ICommand& command );
    bool _cmdNewObserver( co::ICommand& command );
    bool _cmdNewEntityReply( co::ICommand& command );
};

template< class S, class C, class O, class L, class CV, class N, class V >
EQFABRIC_INL std::ostream& operator << ( std::ostream& os,
                                         const Config< S, C, O, L, CV, N, V >& config );
}
}
#endif // EQFABRIC_CONFIG_H
