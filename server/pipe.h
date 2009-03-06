
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_PIPE_H
#define EQSERVER_PIPE_H

#include "types.h"
#include "visitorResult.h" // enum

#include <eq/client/global.h>           // eq::OFF enum
#include <eq/client/pixelViewport.h>    // member
#include <eq/net/object.h>              // base class
#include <eq/base/monitor.h>            // member

#include <ostream>
#include <vector>

namespace eq
{
namespace server
{
    class PipeVisitor;
    class ConstPipeVisitor;
    struct ChannelPath;
    struct PipePath;

    /**
     * The pipe.
     */
    class Pipe : public net::Object
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
        Pipe();

        /** 
         * Constructs a new deep copy of a pipe.
         */
        Pipe( const Pipe& from, Node* node );

        Server* getServer();
        const Server* getServer() const;

        Node*       getNode()       { return _node; }
        const Node* getNode() const { return _node; }

        Config* getConfig();
        const Config* getConfig() const;

        net::CommandQueue* getServerThreadQueue();
        net::CommandQueue* getCommandThreadQueue();

        /** @return the index path to this pipe. */
        PipePath getPath() const;

        Channel* getChannel( const ChannelPath& path );

        /** @return the state of this pipe. */
        State getState()    const { return _state.get(); }

        /** 
         * Adds a new window to this config.
         * 
         * @param window the window.
         */
        void addWindow( Window* window );

        /** 
         * Removes a window from this config.
         * 
         * @param window the window
         * @return <code>true</code> if the window was removed, 
         *         <code>false</code> otherwise.
         */
        bool removeWindow( Window* window );

        /** @return the vector of windows. */
        const WindowVector& getWindows() const { return _windows; }

        /** 
         * Traverse this pipe and all children using a pipe visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         */
        VisitorResult accept( PipeVisitor& visitor );
        VisitorResult accept( ConstPipeVisitor& visitor ) const;

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

        void setName( const std::string& name ) { _name = name; }
        const std::string& getName() const      { return _name; }

        /**
         * @name Data Access
         */
        //*{
        void setPort( const uint32_t port )      { _port = port; }
        uint32_t getPort() const                 { return _port; }
        void setDevice( const uint32_t device )  { _device = device; }
        uint32_t getDevice() const               { return _device; }

        /** 
         * Set (force) the pixel viewport.
         *
         * If the pixel viewport is invalid, it is determined automatically
         * during initialisation of the pipe. If it is valid, it force this pipe
         * to assume the given size and position.
         * 
         * @param pvp the pixel viewport.
         */
        void setPixelViewport( const eq::PixelViewport& pvp );

        /** @return the pixel viewport. */
        const eq::PixelViewport& getPixelViewport() const { return _pvp; }

        /** The last drawing compound for this entity. @internal */
        void setLastDrawWindow( const Window* window )
            { _lastDrawWindow = window; }
        const Window* getLastDrawWindow() const { return _lastDrawWindow; }
        //*}

        /**
         * @name Operations
         */
        //*{
        /** Update (init and exit) this pipe and its children as needed. */
        void updateRunning( const uint32_t initID );

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
        //*}

        /**
         * @name Attributes
         */
        //*{
        // Note: also update string array initialization in pipe.cpp
        enum IAttribute
        {
            IATTR_HINT_THREAD,
            IATTR_ALL
        };

        void setIAttribute( const IAttribute attr, const int32_t value )
            { _iAttributes[attr] = value; }
        int32_t  getIAttribute( const IAttribute attr ) const
            { return _iAttributes[attr]; }
        static const std::string&  getIAttributeString( const IAttribute attr )
            { return _iAttributeStrings[attr]; }

        bool isThreaded() const
            { return (getIAttribute( IATTR_HINT_THREAD ) != eq::OFF ); }
        //*}

        /** @name Error information. */
        //@{
        /** @return the error message from the last operation. */
        const std::string& getErrorMessage() const { return _error; }
        //@}

    protected:
        virtual ~Pipe();

        /** Registers request packets waiting for a return value. */
        base::RequestHandler _requestHandler;

        /** @sa net::Object::attachToSession. */
        virtual void attachToSession( const uint32_t id, 
                                      const uint32_t instanceID, 
                                      net::Session* session );
    private:
        /** The pipe's name */
        std::string _name;

        /** Integer attributes. */
        int32_t _iAttributes[IATTR_ALL];
        /** String representation of integer attributes. */
        static std::string _iAttributeStrings[IATTR_ALL];

        /** The list of windows. */
        WindowVector _windows;

        /** Number of activations for this pipe. */
        uint32_t _active;

        /** The reason for the last error. */
        std::string _error;

        /** The parent node. */
        Node* _node;
        friend class Node;

        /** Worst-case set of tasks. */
        uint32_t _tasks;

        /** The current state for state change synchronization. */
        base::Monitor< State > _state;
            
        /** The display (X11) or ignored (Win32, AGL). */
        uint32_t _port;

        /** The screen (X11), GPU (Win32) or virtual screen (AGL). */
        uint32_t _device;

        /* The display (AGL) or output channel (X11?, Win32). */
        //uint32_t _monitor;

        /** The absolute size and position of the pipe. */
        eq::PixelViewport _pvp;

        /** The last draw window for this entity. */
        const Window* _lastDrawWindow;

        /** common code for all constructors */
        void _construct();

        void _send( net::ObjectPacket& packet );
        void _send( net::ObjectPacket& packet, const std::string& string ) ;

        void _startWindows();
        void _stopWindows();

        void _configInit( const uint32_t initID );
        bool _syncConfigInit();
        void _configExit();
        bool _syncConfigExit();

        virtual void getInstanceData( net::DataOStream& os ) { EQDONTCALL }
        virtual void applyInstanceData( net::DataIStream& is ) { EQDONTCALL }

        /* command handler functions. */
        net::CommandResult _cmdConfigInitReply( net::Command& command );
        net::CommandResult _cmdConfigExitReply( net::Command& command );
    };

    std::ostream& operator << ( std::ostream& os, const Pipe* pipe );
}
}
#endif // EQSERVER_PIPE_H
