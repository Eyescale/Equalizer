
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

#ifdef EQUALIZERSERVERLIBRARY_EXPORTS
   // We need to instantiate a Monitor< State > when compiling the library,
   // but we don't want to have <pthread.h> for a normal build, hence this hack
#  include <pthread.h>
#endif
#include <eq/base/monitor.h>

#include "types.h"
#include "visitorResult.h"  // enum

#include <eq/net/barrier.h>
#include <eq/fabric/window.h> // base class

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
        
        Channel* getChannel( const ChannelPath& path );

        net::CommandQueue* getServerThreadQueue();
        net::CommandQueue* getCommandThreadQueue();

        /** @return the state of this window. */
        State getState() const { return _state.get(); }

        /** Increase window activition count. */
        void activate();

        /** Decrease window activition count. */
        void deactivate();

        /** @return if this window is actively used for rendering. */
        bool isActive() const { return (_active != 0); }

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
        net::Barrier* joinSwapBarrier( net::Barrier* barrier );

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
        net::Barrier* joinNVSwapBarrier( const SwapBarrier* swapBarrier,
                                         net::Barrier* netBarrier );

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
        /** Update (init and exit) this window and its children as needed. */
        void updateRunning( const uint32_t initID );

        /** Finalize the last updateRunning changes. */
        bool syncRunning();

        /** 
         * Update one frame.
         *
         * @param frameID a per-frame identifier passed to all rendering
         *                methods.
         * @param frameNumber the number of the frame.
         */
        void updateDraw( const uint32_t frameID, const uint32_t frameNumber );

        /** 
         * Trigger the post-draw operations.
         *
         * @param frameID a per-frame identifier passed to all rendering
         *                methods.
         * @param frameNumber the number of the frame.
         */
        void updatePost( const uint32_t frameID, const uint32_t frameNumber );
        //@}

        void send( net::ObjectPacket& packet );

    protected:

        /** @sa net::Object::attachToSession. */
        virtual void attachToSession( const uint32_t id, 
                                      const uint32_t instanceID, 
                                      net::Session* session );

        virtual void deserialize( eq::net::DataIStream&, const uint64_t );

    private:

        /** Number of activations for this window. */
        uint32_t _active;

        /** The current state for state change synchronization. */
        base::Monitor< State > _state;
        
        /** The maximum frame rate allowed for this window. */
        float _maxFPS;

        /** The flag if the window has to execute a finish */
        bool _swapFinish;

        /** 
         * The flag if the window has to swap, i.e, something was done during
         * the last update.
         */
        bool _swap;

        /** The list of master swap barriers for the current frame. */
        net::BarrierVector _masterSwapBarriers;
        /** The list of slave swap barriers for the current frame. */
        net::BarrierVector _swapBarriers;

        /** The hardware swap barrier to use. */
        const SwapBarrier* _nvSwapBarrier;

        /** The network barrier used to protect hardware barrier entry. */
        net::Barrier* _nvNetBarrier;

        /** The last draw channel for this entity */
        const Channel* _lastDrawChannel;

        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };

        /** common code for all constructors */
        void _construct();

        /** Clears all swap barriers of the window. */
        void _resetSwapBarriers();

        void _send( net::ObjectPacket& packet, const std::string& string );

        void _configInit( const uint32_t initID );
        bool _syncConfigInit();
        void _configExit();
        bool _syncConfigExit();

        void _updateSwap( const uint32_t frameNumber );

        /* command handler functions. */
        net::CommandResult _cmdConfigInitReply( net::Command& command ); 
        net::CommandResult _cmdConfigExitReply( net::Command& command ); 

        // For access to _fixedPVP
        friend std::ostream& operator << ( std::ostream&, const Window*);
    };

    std::ostream& operator << ( std::ostream& os, const Window* window );
}
}

#endif // EQSERVER_WINDOW_H
