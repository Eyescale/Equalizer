
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQSERVER_CHANNEL_H
#define EQSERVER_CHANNEL_H

#ifdef EQSERVER_EXPORTS
   // We need to instantiate a Monitor< State > when compiling the library,
   // but we don't want to have <pthread.h> for a normal build, hence this hack
#  include <pthread.h>
#endif
#include <eq/base/monitor.h>

#include <eq/fabric/channel.h>

#include "base.h"
#include "types.h"

#include <eq/fabric/pixelViewport.h> // member
#include <eq/fabric/viewport.h>      // member
#include <eq/net/object.h>
#include <eq/net/packets.h>

#include <iostream>
#include <vector>

namespace eq
{
    struct Statistic;

namespace server
{
    class ChannelListener;
    class Window;

    class Channel : public fabric::Channel< Window, Channel >
    {
    public:
        enum State
        {
            STATE_STOPPED = 0,  // next: INITIALIZING
            STATE_INITIALIZING, // next: INIT_FAILED or INIT_SUCCESS
            STATE_INIT_SUCCESS, // next: RUNNING
            STATE_INIT_FAILED,  // next: EXITING
            STATE_RUNNING,      // next: EXITING
            STATE_EXITING,      // next: EXIT_FAILED or EXIT_SUCCESS
            STATE_EXIT_SUCCESS, // next: STOPPED
            STATE_EXIT_FAILED,  // next: STOPPED
        };

        /** Construct a new channel. */
        EQSERVER_EXPORT Channel( Window* parent );

        /** Construct a new deep copy of a channel. */
        Channel( const Channel& from, Window* window );

        /** Destruct this channel. */
        virtual ~Channel();

        /** 
         * @return the state of this channel.
         */
        State getState()    const { return _state.get(); }

        /**
         * @name Data Access
         */
        //@{
        Config* getConfig();
        const Config* getConfig() const;

        EQSERVER_EXPORT Node* getNode();
        EQSERVER_EXPORT const Node* getNode() const;

        Pipe* getPipe();
        const Pipe* getPipe() const;

        const CompoundVector& getCompounds() const;

        net::CommandQueue* getMainThreadQueue();
        net::CommandQueue* getCommandThreadQueue();

        /** Increase channel activition count. */
        void activate();

        /** Decrease channel activition count. */
        void deactivate();

        /** @return if this channel is actively used for rendering. */
        bool isActive() const { return (_active != 0); }

        /**
         * Add additional tasks this channel, and all its parents, might
         * potentially execute.
         */
        void addTasks( const uint32_t tasks );

        /** Set the output view and segment for this channel. */
        void setOutput( View* view, Segment* segment );

        /** Unset the output view and segment for this channel. */
        void unsetOutput();

        /** @return the channel's view. */
        const View* getView() const { return _view; }

        /** @return the channel's layout. */
        const Layout* getLayout() const;

        /** @return the channel's segment. */
        const Segment* getSegment() const { return _segment; }

        /** @return the channel's segment. */
        Segment* getSegment() { return _segment; }

        /** The last drawing compound for this entity. */
        void setLastDrawCompound( const Compound* compound )
            { _lastDrawCompound = compound; }
        const Compound* getLastDrawCompound() const { return _lastDrawCompound;}

        void setIAttribute( const IAttribute attr, const int32_t value )
            { fabric::Channel< Window, Channel >::setIAttribute( attr, value );}
        void setDrawable( const uint32_t drawable )
            { fabric::Channel< Window, Channel >::setDrawable( drawable ); }
        //@}

        /**
         * @name Operations
         */
        //@{
        /** Update (init and exit) this channel as needed. */
        void updateRunning( const uint32_t initID );

        /** Finalize the last updateRunning changes. */
        bool syncRunning();

        /** 
         * Starts exiting this node.
         */
        void startConfigExit();

        /** 
         * Synchronize the exit of the node.
         * 
         * @return <code>true</code> if the node exited cleanly,
         *         <code>false</code> if not.
         */
        bool syncConfigExit();

        /** 
         * Update one frame.
         *
         * @param frameID a per-frame identifier passed to all rendering
         *                methods.
         * @param frameNumber the number of the frame.
         * @return true if at least one rendering task was sent.
         */
        bool update( const uint32_t frameID, const uint32_t frameNumber );

        void send( net::ObjectPacket& packet );
        template< typename T >
        void send( net::ObjectPacket &packet, const std::vector<T>& data );
        //@}

        /** @name Channel listener interface. */
        //@{
        /** Register a channel listener. */
        void addListener( ChannelListener* listener );
        /** Deregister a channel listener. */
        void removeListener( ChannelListener* listener );
        //@}

    protected:
        /** @sa net::Object::attachToSession. */
        virtual void attachToSession( const uint32_t id, 
                                      const uint32_t instanceID, 
                                      net::Session* session );
        virtual void deserialize( eq::net::DataIStream&, const uint64_t );

    private:
        //-------------------- Members --------------------
        /** Number of activations for this channel. */
        uint32_t _active;

        /** The view used by this channel. */
        View* _view;

        /** The segment used by this channel. */
        Segment* _segment;

        Vector4i    _overdraw;

        /** The current state for state change synchronization. */
        base::Monitor< State > _state;

        /** The last draw compound for this entity */
        const Compound* _lastDrawCompound;

        typedef std::vector< ChannelListener* > ChannelListeners;
        ChannelListeners _listeners;

        CHECK_THREAD_DECLARE( _serverThread );

        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };

        //-------------------- Methods --------------------
        /** common code for all constructors */
        void _construct();

        Vector3ub _getUniqueColor() const;

        void _configInit( const uint32_t initID );
        bool _syncConfigInit();
        void _configExit();
        bool _syncConfigExit();

        void _setupRenderContext( const uint32_t frameID,
                                  RenderContext& context );

        void _fireLoadData( const uint32_t frameNumber, 
                            const uint32_t nStatistics,
                            const eq::Statistic* statistics );

        /* command handler functions. */
        net::CommandResult _cmdConfigInitReply( net::Command& command );
        net::CommandResult _cmdConfigExitReply( net::Command& command );
        net::CommandResult _cmdFrameFinishReply( net::Command& command );

        // For access to _fixedPVP
        friend std::ostream& operator << ( std::ostream&, const Channel& );
    };

    std::ostream& operator << ( std::ostream& os, const Channel& channel);
}
}
#endif // EQSERVER_CHANNEL_H
