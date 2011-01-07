
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com>
 * Copyright (c)      2010, Cedric Stalder <cedric.stalder@gmail.com> 
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

#ifndef EQSERVER_WINDOW_H
#define EQSERVER_WINDOW_H

#include "types.h"

#include "state.h"          // enum
#include "visitorResult.h"  // enum

#include <eq/fabric/window.h> // base class
#include <co/barrier.h>
#include <co/base/uint128_t.h> // Member
#include <iostream>
#include <vector>

namespace eq
{
namespace server
{
    class SwapBarrier;

    /**
     * The window.
     */
    class Window : public fabric::Window< Pipe, Window, Channel >
    {
    public:
        /** 
         * Constructs a new Window.
         */
        EQSERVER_EXPORT Window( Pipe* parent );
        
        virtual ~Window();
        /**
         * @name Data Access
         */
        //@{

        Node* getNode();
        const Node* getNode() const;

        Config* getConfig();
        const Config* getConfig() const;

        /** @return the Server of this window. */
        ServerPtr getServer();
        
        Channel* getChannel( const ChannelPath& path );

        co::CommandQueue* getMainThreadQueue();
        co::CommandQueue* getCommandThreadQueue();

        /** Increase window activition count. */
        void activate();

        /** Decrease window activition count. */
        void deactivate();

        /** @return the state of this window. */
        State getState() const { return _state.get(); }

        /** @internal */
        void setState( const State state ) { _state = state; }

        /** @return if this window is actively used for rendering. */
        bool isActive() const { return (_active != 0); }

        /** @return true if this window is stopped. */
        bool isStopped() const { return _state & STATE_STOPPED; }

        /** @return true if this window is running. */
        bool isRunning() const { return _state & STATE_RUNNING; }

        /** @return true if this window should be deleted. */
        bool needsDelete() const { return _state & STATE_DELETE; }

        /** Schedule deletion of this window. */
        void postDelete();

        /**
         * Add additional tasks this window, and all its parents, might
         * potentially execute.
         */
        void addTasks( const uint32_t tasks );

        /** 
         * Join a swap barrier for the next update.
         * 
         * @param barrier the net::Barrier for the swap barrier group, or 0 if
         *                this is the first window.
         * @return the net::Barrier for the swap barrier group.
         */
        co::Barrier* joinSwapBarrier( co::Barrier* barrier );

        /** 
         * Join a NV_swap_group barrier for the next update.
         * 
         * @param swapBarrier the swap barrier containing the NV_swap_group
         *                    parameters.
         * @param netBarrier the net::Barrier to protect the entry from the
         *                   NV_swap_group , or 0 if this is the first window
         *                   entering.
         * @return the net::Barrier for protecting the swap group entry.
         */
        co::Barrier* joinNVSwapBarrier( const SwapBarrier* swapBarrier,
                                         co::Barrier* netBarrier );

        /** @return true if this window has entered a NV_swap_group. */
        bool hasNVSwapBarrier() const { return (_nvSwapBarrier != 0); }

        /** The last drawing channel for this entity. @internal */
        void setLastDrawChannel( const Channel* channel )
            { _lastDrawChannel = channel; }
        const Channel* getLastDrawChannel() const { return _lastDrawChannel; }

        /** The maximum frame rate for this window. @internal */
        void setMaxFPS( const float fps ) { _maxFPS = fps; }
        float getMaxFPS() const { return _maxFPS; }
        //@}

        /**
         * @name Operations
         */
        //@{
        /** Start initializing this entity. */
        void configInit( const uint128_t& initID, const uint32_t frameNumber );

        /** Sync initialization of this entity. */
        bool syncConfigInit();

        /** Start exiting this entity. */
        void configExit();

        /** Sync exit of this entity. */
        bool syncConfigExit();

        /** 
         * Update one frame.
         *
         * @param frameID a per-frame identifier passed to all rendering
         *                methods.
         * @param frameNumber the number of the frame.
         */
        void updateDraw( const uint128_t& frameID, const uint32_t frameNumber );

        /** 
         * Trigger the post-draw operations.
         *
         * @param frameID a per-frame identifier passed to all rendering
         *                methods.
         * @param frameNumber the number of the frame.
         */
        void updatePost( const uint128_t& frameID, const uint32_t frameNumber );
        //@}

        void send( co::ObjectPacket& packet );
        void output( std::ostream& ) const; //!< @internal

    protected:

        /** @sa net::Object::attach. */
        virtual void attach( const UUID& id, const uint32_t instanceID );

        /** @internal Execute the slave remove request. */
        virtual void removeChild( const co::base::UUID& id );

    private:

        /** Number of activations for this window. */
        uint32_t _active;

        /** The current state for state change synchronization. */
        co::base::Monitor< State > _state;
        
        /** The maximum frame rate allowed for this window. */
        float _maxFPS;

        /** The list of master swap barriers for the current frame. */
        co::Barriers _masterSwapBarriers;
        /** The list of slave swap barriers for the current frame. */
        co::Barriers _swapBarriers;

        /** The hardware swap barrier to use. */
        const SwapBarrier* _nvSwapBarrier;

        /** The network barrier used to protect hardware barrier entry. */
        co::Barrier* _nvNetBarrier;

        /** The last draw channel for this entity */
        const Channel* _lastDrawChannel;

        /** The flag if the window has to execute a finish */
        bool _swapFinish;

        /** 
         * The flag if the window has to swap, i.e, something was done during
         * the last update.
         */
        bool _swap;

        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };

        /** Clears all swap barriers of the window. */
        void _resetSwapBarriers();

        void _updateSwap( const uint32_t frameNumber );

        /* command handler functions. */
        bool _cmdConfigInitReply( co::Command& command ); 
        bool _cmdConfigExitReply( co::Command& command ); 

        // For access to _fixedPVP
        friend std::ostream& operator << ( std::ostream&, const Window*);
    };
}
}

#endif // EQSERVER_WINDOW_H
