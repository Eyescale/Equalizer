
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

#ifndef EQSERVER_PIPE_H
#define EQSERVER_PIPE_H

#include "types.h"
#include "visitorResult.h" // enum

#include <eq/client/global.h>           // eq::OFF enum
#include <eq/fabric/pipe.h>             // parent
#include <eq/fabric/pixelViewport.h>    // member
#include <eq/base/monitor.h>            // member

#include <ostream>
#include <vector>

namespace eq
{
namespace server
{

    /**
     * The pipe.
     */
    class Pipe : public fabric::Pipe< Node, Pipe, Window >
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
         * Constructs a new Pipe.
         */
        EQSERVER_EXPORT Pipe(  Node* parent );

        /** 
         * Constructs a new deep copy of a pipe.
         */
        Pipe( const Pipe& from, Node* parent );

        virtual ~Pipe();

        ServerPtr getServer();
        const ServerPtr getServer() const;

        Config* getConfig();
        const Config* getConfig() const;

        net::CommandQueue* getServerThreadQueue();
        net::CommandQueue* getCommandThreadQueue();

        Channel* getChannel( const ChannelPath& path );

        /** @return the state of this pipe. */
        State getState()    const { return _state.get(); }

        /** 
         * Adds a new window to this config.
         * 
         * @param window the window.
         */
        EQSERVER_EXPORT void addWindow( Window* window );

        /** 
         * Removes a window from this config.
         * 
         * @param window the window
         * @return <code>true</code> if the window was removed, 
         *         <code>false</code> otherwise.
         */
        bool removeWindow( Window* window );

        /** 
         * Traverse this pipe and all children using a pipe visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         */
        VisitorResult accept( PipeVisitor& visitor );
        VisitorResult accept( PipeVisitor& visitor ) const;

        /** Increase pipe activition count. */
        void activate();

        /** Decrease pipe activition count. */
        void deactivate();

        /** @return if this pipe is actively used for rendering. */
        bool isActive() const { return (_active != 0); }

        /**
         * Add additional tasks this pipe, and all its parents, might
         * potentially execute.
         */
        void addTasks( const uint32_t tasks );

        /**
         * @name Data Access
         */
        //@{


        /** The last drawing compound for this entity. @internal */
        void setLastDrawWindow( const Window* window )
            { _lastDrawWindow = window; }
        const Window* getLastDrawWindow() const { return _lastDrawWindow; }
        //@}

        /**
         * @name Operations
         */
        //@{
        /** Update (init and exit) this pipe and its children as needed. */
        void updateRunning( const uint32_t initID, const uint32_t frameNumber );

        /** Finalize the last updateRunning changes. */
        bool syncRunning();

        /** 
         * Trigger the rendering of a new frame.
         *
         * @param frameID a per-frame identifier passed to all rendering
         *                methods.
         * @param frameNumber the number of the frame.
         */
        void update( const uint32_t frameID, const uint32_t frameNumber );
        //@}

        /**
         * @name Attributes
         */
        //@{
        /** Set a pipe attribute. */
        void setIAttribute( const IAttribute attr, const int32_t value )
            { _iAttributes[attr] = value; }

        /** @return the value of a pipe attribute. */
        int32_t  getIAttribute( const IAttribute attr ) const
            { return _iAttributes[attr]; }

        bool isThreaded() const
            { return (getIAttribute( IATTR_HINT_THREAD ) != eq::OFF ); }
        //@}

        void send( net::ObjectPacket& packet );

    protected:

        /** @sa net::Object::attachToSession. */
        virtual void attachToSession( const uint32_t id, 
                                      const uint32_t instanceID, 
                                      net::Session* session );
        virtual void deserialize( eq::net::DataIStream&, const uint64_t );

    private:

        /** Integer attributes. */
        int32_t _iAttributes[IATTR_ALL];

        /** Number of activations for this pipe. */
        uint32_t _active;

        friend class Node;

        /** The current state for state change synchronization. */
        base::Monitor< State > _state;

        /* The display (AGL) or output channel (X11?, Win32). */
        //uint32_t _monitor;

        /** The last draw window for this entity. */
        const Window* _lastDrawWindow;

        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };

        /** common code for all constructors */
        void _construct();

        void _send( net::ObjectPacket& packet, const std::string& string ) ;

        void _configInit( const uint32_t initID, const uint32_t frameNumber );
        bool _syncConfigInit();
        void _configExit();
        bool _syncConfigExit();

        /* command handler functions. */
        net::CommandResult _cmdConfigInitReply( net::Command& command );
        net::CommandResult _cmdConfigExitReply( net::Command& command );
    };

    std::ostream& operator << ( std::ostream& os, const Pipe* pipe );
}
}
#endif // EQSERVER_PIPE_H
