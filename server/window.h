
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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
#include "swapBarrier.h"
#include <eq/client/pixelViewport.h>
#include <eq/client/window.h>
#include <eq/net/barrier.h>
#include <eq/net/object.h>

#include <iostream>
#include <vector>

namespace eq
{
namespace server
{
    class WindowVisitor;
    class ConstWindowVisitor;
    struct ChannelPath;
    struct WindowPath;

    /**
     * The window.
     */
    class Window : public net::Object
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
        EQSERVER_EXPORT Window();

        /** 
         * Constructs a new deep copy of a window.
         */
        Window( const Window& from, Pipe* pipe );

        /**
         * @name Data Access
         */
        //@{
        /** 
         * Add a new channel to this window.
         * 
         * @param channel the channel.
         */
        EQSERVER_EXPORT void addChannel( Channel* channel );

        /** 
         * Insert a new channel after the given channel.
         * 
         * @param position the channel after which to insert.
         * @param channel the channel to insert.
         */
        void insertChannel( const Channel* position, Channel* channel );

        /** 
         * Removes a channel from this window.
         * 
         * @param channel the channel
         * @return <code>true</code> if the channel was removed,
         *         <code>false</code> otherwise.
         */
        bool removeChannel( Channel* channel );

        /** @return the vector of channels. */
        const ChannelVector& getChannels() const { return _channels; }

        Pipe*       getPipe()       { return _pipe; }
        const Pipe* getPipe() const { return _pipe; }

        Node* getNode();
        const Node* getNode() const;

        Config* getConfig();
        const Config* getConfig() const;
        
        /** @return the index path to this window. */
        WindowPath getPath() const;

        Channel* getChannel( const ChannelPath& path );

        const eq::Window::DrawableConfig& getDrawableConfig() const
            { return _drawableConfig; }

        net::CommandQueue* getServerThreadQueue();
        net::CommandQueue* getCommandThreadQueue();

        /** @return the state of this window. */
        State getState() const { return _state.get(); }

        /** 
         * Traverse this window and all children using a window visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         */
        EQSERVER_EXPORT VisitorResult accept( WindowVisitor& visitor );
        EQSERVER_EXPORT VisitorResult accept( ConstWindowVisitor& ) const;

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

        void setName( const std::string& name ) { _name = name; }
        const std::string& getName() const      { return _name; }

        /** 
         * Set the window's pixel viewport wrt its parent pipe.
         * 
         * @param pvp the viewport in pixels.
         */
        EQSERVER_EXPORT void setPixelViewport( const eq::PixelViewport& pvp );

        /** 
         * Return this window's pixel viewport.
         * 
         * @return the pixel viewport.
         */
        const eq::PixelViewport& getPixelViewport() const { return _pvp; }

        /** 
         * Set the window's viewport wrt its parent pipe.
         * 
         * @param vp the fractional viewport.
         */
        void setViewport( const eq::Viewport& vp );

        /** 
         * Return this window's viewport.
         * 
         * @return the fractional viewport.
         */
        const eq::Viewport& getViewport() const { return _vp; }

        /**
         * Notify this window that the viewport has changed. 
         *
         * The window updates the viewport or pixel viewport accordingly. 
         */
        void notifyViewportChanged();

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

        /**
         * @name Attributes
         */
        //@{
        void setIAttribute( const eq::Window::IAttribute attr,
                            const int32_t value )
            { _iAttributes[attr] = value; }
        int32_t  getIAttribute( const eq::Window::IAttribute attr ) const
            { return _iAttributes[attr]; }
        //@}

        /** @name Error information. */
        //@{
        /** @return the error message from the last operation. */
        const std::string& getErrorMessage() const { return _error; }
        //@}

        void send( net::ObjectPacket& packet );

    protected:
        virtual ~Window();

        /** Registers request packets waiting for a return value. */
        base::RequestHandler _requestHandler;

        /** @sa net::Object::attachToSession. */
        virtual void attachToSession( const uint32_t id, 
                                      const uint32_t instanceID, 
                                      net::Session* session );
    private:
        eq::Window::DrawableConfig _drawableConfig;

        /** The window's name */
        std::string _name;

        /** The reason for the last error. */
        std::string            _error;

        /** Integer attributes. */
        int32_t _iAttributes[eq::Window::IATTR_ALL];

        /** The child channels. */
        ChannelVector _channels;

        /** Number of activations for this window. */
        uint32_t _active;

        /** The parent pipe. */
        Pipe* _pipe;
        friend class Pipe;

        /** Worst-case set of tasks. */
        uint32_t _tasks;

        /** The current state for state change synchronization. */
        base::Monitor< State > _state;
            
        /** The absolute size and position of the window. */
        eq::PixelViewport _pvp;
        
        /** The fractional size and position of the window. */
        eq::Viewport _vp;
        
        /** The maximum frame rate allowed for this window. */
        float _maxFPS;

        /** true if something was done during the last update. */
        bool _doSwap;

        /** 
         * true if the pixel viewport is immutable, false if the viewport is
         * immutable
         */
        bool _fixedPVP;

        /** The list of master swap barriers for the current frame. */
        std::vector<net::Barrier*> _masterSwapBarriers;
        /** The list of slave swap barriers for the current frame. */
        std::vector<net::Barrier*> _swapBarriers;
        
        /** The hardware swap barrier to use. */
        const SwapBarrier* _nvSwapBarrier;

        /** The network barrier used to protect hardware barrier entry. */
        net::Barrier* _nvNetBarrier;

        /** The last draw channel for this entity */
        const Channel* _lastDrawChannel;

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
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

        virtual void getInstanceData( net::DataOStream& os ) { EQDONTCALL }
        virtual void applyInstanceData( net::DataIStream& is ) { EQDONTCALL }

        /* command handler functions. */
        net::CommandResult _cmdConfigInitReply( net::Command& command ); 
        net::CommandResult _cmdConfigExitReply( net::Command& command ); 
        net::CommandResult _cmdSetPixelViewport( net::Command& command );

        // For access to _fixedPVP
        friend std::ostream& operator << ( std::ostream&, const Window*);
    };

    std::ostream& operator << ( std::ostream& os, const Window* window );
}
}

#endif // EQSERVER_WINDOW_H
