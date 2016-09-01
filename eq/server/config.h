
/* Copyright (c) 2005-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
 *                          Cedric Stalder <cedric Stalder@gmail.com>
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

#ifndef EQSERVER_CONFIG_H
#define EQSERVER_CONFIG_H

#include <eq/server/api.h>
#include "types.h"
#include "server.h"        // used in inline method
#include "state.h"         // enum
#include "visitorResult.h" // enum

#include <eq/fabric/config.h> // base class
#include <lunchbox/monitor.h> // member

#include <iostream>
#include <vector>

namespace eq
{
namespace server
{
/** The config. */
class Config : public fabric::Config< Server, Config, Observer, Layout,
                                      Canvas, Node, ConfigVisitor >
{
public:
    typedef fabric::Config< Server, Config, Observer, Layout, Canvas, Node,
                            ConfigVisitor > Super;

    /** Construct a new config. */
    EQSERVER_API explicit Config( ServerPtr parent );
    virtual ~Config();

    /**
     * @name Data Access.
     */
    //@{
    Channel* getChannel( const ChannelPath& path );
    Segment* getSegment( const SegmentPath& path );
    View* getView( const ViewPath& path );

    bool isRunning() const { return ( _state == STATE_RUNNING ); }
    bool isUsed() const { return _state != STATE_UNUSED; }
    bool isAutoConfig() const
    { return getName().find( " autoconfig" ) != std::string::npos; }

    co::CommandQueue* getMainThreadQueue()
    { return getServer()->getMainThreadQueue(); }
    co::CommandQueue* getCommandThreadQueue()
    { return getServer()->getCommandThreadQueue(); }

    /**
     * Adds a new compound to this config.
     *
     * @param compound the compound.
     */
    EQSERVER_API void addCompound( Compound* compound );

    /**
     * Removes a compound from this config.
     *
     * @param compound the compound
     * @return <code>true</code> if the compound was removed,
     *         <code>false</code> otherwise.
     */
    EQSERVER_API bool removeCompound( Compound* compound );

    /** @return the vector of compounds. */
    const Compounds& getCompounds() const { return _compounds; }

    /**
     * Find the first channel of a given name.
     *
     * @param name the name of the channel to find
     * @return the first channel with the name, or <code>0</code> if no
     *         channel with the name exists.
     */
    const Channel* findChannel( const std::string& name ) const;

    /**
     * Find the channel for the given view/segment intersection.
     *
     * @param segment the segment.
     * @param view the view.
     * @return the channel for updating the view/segment intersection.
     */
    EQSERVER_API Channel* findChannel( const Segment* segment,
                                       const View* view );

    /** @return the application node, or 0. */
    EQSERVER_API Node* findApplicationNode();
    //@}

    /** @sa fabric::Config::changeLatency() */
    virtual void changeLatency( const uint32_t latency );

    /**
     * Set the network node running the application thread.
     *
     * @param node the application node.
     */
    void setApplicationNetNode( co::NodePtr node );

    /** @return network node running the application thread. */
    co::NodePtr findApplicationNetNode();

    /** @internal set auto-configured server connections */
    void setServerConnections( const co::Connections& connections )
    { _connections = connections; }

    /** @internal @return the auto-configured server connections */
    const co::Connections& getServerConnections() const
    { return _connections; }

    /**
     * Set the name of the render client executable.
     *
     * @param rc the name of the render client executable.
     */
    void setRenderClient( const std::string& rc ){ _renderClient = rc; }

    /** @return the name of the render client executable. */
    const std::string& getRenderClient() const { return _renderClient; }

    /** Set the render client command line options. */
    void setRenderClientArgs( const Strings& args ){ _renderClientArgs = args; }

    /** @return the render client command line options. */
    const Strings& getRenderClientArgs() const { return _renderClientArgs; }

    /** Set the prefixes of the environmental variables to pass on clients. */
    void setRenderClientEnvPrefixes( const Strings& prefixes )
    { _renderClientEnvPrefixes = prefixes; }

    /** @return prefixes of the environmental variables to pass on clients. */
    const Strings& getRenderClientEnvPrefixes() const
    { return _renderClientEnvPrefixes; }

    /**
     * Set the working directory for render client.
     *
     * @param workDir the working directory for the  render client.
     */
    void setWorkDir( const std::string& workDir ){ _workDir = workDir; }

    /** @return the working directory for the  render client. */
    const std::string& getWorkDir() const { return _workDir; }

    /** Notify that a node of this config has finished a frame. */
    void notifyNodeFrameFinished( const uint32_t frameNumber );

    // Used by Server::releaseConfig() to make sure config is exited
    bool exit();

    /** Register the config and all shared object children. */
    void register_();

    /** Deregister all shared objects and the config. */
    void deregister();

    /** Commit the config for the current frame. */
    uint128_t commit();

    virtual void restore();

    EventOCommand sendError( const uint32_t type, const Error& error );

    /** Return the initID, used for late initialization  */
    uint128_t getInitID(){ return _initID; }

    /** Activate the given canvas after it is complete (dest channels). */
    virtual void activateCanvas( Canvas* canvas );

    /** Initialize the given canvas in a running configuration */
    virtual void updateCanvas( Canvas* canvas );

    /** Request a finish of outstanding frames on next frame */
    void postNeedsFinish() { _needsFinish = true; }

    /** @internal @return the last finished frame */
    uint32_t getFinishedFrame() const { return _finishedFrame.get(); }

    /** @internal */
    virtual VisitorResult _acceptCompounds( ConfigVisitor& visitor );
    /** @internal */
    virtual VisitorResult _acceptCompounds( ConfigVisitor& visitor ) const;

    void output( std::ostream& os ) const; //!< @internal
    virtual bool mapViewObjects() const { return true; } //!< @internal
    virtual bool mapNodeObjects() const { return true; } //!< @internal

protected:
    /** @internal */
    virtual void attach( const uint128_t& id, const uint32_t instanceID );

    /** @internal Execute the slave remove request. */
    virtual void removeChild( const uint128_t& id );

private:
    Config( const Config& from );

    /** The initID for late initialization. */
    uint128_t _initID;

    /** The list of compounds. */
    Compounds _compounds;

    /** Auto-configured server connections. */
    co::Connections _connections;

    /** The name of the render client executable. */
    std::string _renderClient;

    /** The render client command line options. */
    Strings _renderClientArgs;

    /** The prefixes of environmental variables to pass on to render clients. */
    Strings _renderClientEnvPrefixes;

    /** The working directory of the render client. */
    std::string _workDir;

    /** The last started frame, or 0. */
    uint32_t _currentFrame;

    /** The eternal frame commit counter, starting at 1 (#66). */
    uint32_t _incarnation;

    /** The last finished frame, or 0. */
    lunchbox::Monitor< uint32_t > _finishedFrame;

    State _state;

    bool _needsFinish; //!< true after runtime changes

    int64_t _lastCheck;

    struct Private;
    Private* _private; // placeholder for binary-compatible changes

    /**
     * @name Operations
     */
    //@{
    /** @return true on success, false on error */
    bool _updateRunning( const bool canFail );

    void _updateCanvases();
    bool _connectNodes();
    bool _connectNode( Node* node );
    bool _syncConnectNode( Node* node, const lunchbox::Clock& clock );
    void _startNodes();
    lunchbox::Request< void > _createConfig( Node* node );
    bool _updateNodes( const bool canFail );
    void _stopNodes();
    template< class T >
    void _deleteEntities( const std::vector< T* >& entities );
    void _syncClock();
    void _verifyFrameFinished( const uint32_t frameNumber );
    bool _init( const uint128_t& initID );

    void _startFrame( const uint128_t& frameID );
    void _flushAllFrames();
    //@}

    virtual Observer* createObserver();
    virtual void releaseObserver( Observer* observer );
    virtual Layout* createLayout();
    virtual void releaseLayout( Layout* layout );
    virtual Canvas* createCanvas();
    virtual void releaseCanvas( Canvas* canvas );

    /** @internal Post deletion for the given child, returns true if found*/
    template< class T > bool _postDelete( const uint128_t& id );

    /** The command functions. */
    bool _cmdInit( co::ICommand& command );
    bool _cmdExit( co::ICommand& command );
    bool _cmdUpdate( co::ICommand& command );
    bool _cmdStartFrame( co::ICommand& command );
    bool _cmdStopFrames( co::ICommand& command );
    bool _cmdFinishAllFrames( co::ICommand& command );
    bool _cmdCreateReply( co::ICommand& command );
    bool _cmdFreezeLoadBalancing( co::ICommand& command );
    bool _cmdCheckFrame( co::ICommand& command );

    LB_TS_VAR( _cmdThread );
    LB_TS_VAR( _mainThread );
};
}
}
#endif // EQSERVER_CONFIG_H
