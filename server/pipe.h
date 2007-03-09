
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_PIPE_H
#define EQS_PIPE_H

#include "node.h"                       // used in inline method
#include <eq/net/object.h>              // base class

#include <ostream>
#include <vector>

namespace eqs
{
    class Config;
    class Server;
    class Window;
    
    /**
     * The pipe.
     */
    class Pipe : public eqNet::Object
    {
    public:
        enum State
        {
            STATE_STOPPED,       // initial     <----+
            STATE_INITIALISING,  // init sent        |
            STATE_RUNNING,       // init successful  |
            STATE_STOPPING       // exit send   -----+
        };

        /** 
         * Constructs a new Pipe.
         */
        Pipe();

        /** 
         * Constructs a new deep copy of a pipe.
         */
        Pipe( const Pipe& from );

        Server* getServer() const
            { return _node ? _node->getServer() : NULL; }

        /** 
         * @return the state of this pipe.
         */
        State getState()    const { return _state; }

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

        /** 
         * Returns the number of windows on this config.
         * 
         * @return the number of windows on this config. 
         */
        uint32_t nWindows() const { return _windows.size(); }

        /** 
         * Gets a window.
         * 
         * @param index the window's index. 
         * @return the window.
         */
        Window* getWindow( const uint32_t index ) const
            { return _windows[index]; }

        Node*   getNode()   const { return _node; }
        Config* getConfig() const { return (_node ? _node->getConfig() : NULL);}

        /** 
         * References this pipe as being actively used.
         */
        void refUsed();

        /** 
         * Unreferences this pipe as being actively used.
         */
        void unrefUsed();

        /** 
         * Returns if this pipe is actively used.
         *
         * @return <code>true</code> if this pipe is actively used,
         *         <code>false</code> if not.
         */
        bool isUsed() const { return (_used!=0); }

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
        //*}

        /**
         * @name Operations
         */
        //*{
        /** 
         * Start initializing this pipe.
         *
         * @param initID an identifier to be passed to all init methods.
         */
        void startConfigInit( const uint32_t initID );

        /** 
         * Synchronize the initialisation of the pipe.
         * 
         * @return <code>true</code> if the pipe was initialised successfully,
         *         <code>false</code> if not.
         */
        bool syncConfigInit();
        
        /** 
         * Starts exiting this pipe.
         */
        void startConfigExit();

        /** 
         * Synchronize the exit of the pipe.
         * 
         * @return <code>true</code> if the pipe exited cleanly,
         *         <code>false</code> if not.
         */
        bool syncConfigExit();

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
        //*}

        /** @name Error information. */
        //@{
        /** @return the error message from the last operation. */
        const std::string& getErrorMessage() const { return _error; }
        //@}

    protected:
        virtual ~Pipe();

        /** Registers request packets waiting for a return value. */
        eqBase::RequestHandler _requestHandler;

    private:
        /** The current operational state. */
        State _state;

        /** The pipe's name */
        std::string _name;

        /** Integer attributes. */
        int32_t _iAttributes[IATTR_ALL];
        /** String representation of integer attributes. */
        static std::string _iAttributeStrings[IATTR_ALL];

        /** The list of windows. */
        std::vector<Window*> _windows;

        /** Number of entitities actively using this pipe. */
        uint32_t _used;

        /** The reason for the last error. */
        std::string _error;

        /** The parent node. */
        Node* _node;
        friend class Node;

        /** The request id for pending asynchronous operations. */
        uint32_t _pendingRequestID;

        /** The display (X11) or ignored (Win32, CGL). */
        uint32_t _port;

        /** The screen (X11), GPU (Win32) or virtual screen (CGL). */
        uint32_t _device;

        /* The display (CGL) or output channel (X11?, Win32). */
        //uint32_t _monitor;

        /** The absolute size and position of the pipe. */
        eq::PixelViewport _pvp;

        /** common code for all constructors */
        void _construct();

        void _send( eqNet::ObjectPacket& packet )
            { packet.objectID = getID(); _node->send( packet ); }
        void _send( eqNet::ObjectPacket& packet, const std::string& string ) 
            { packet.objectID = getID(); _node->send( packet, string ); }

        void _sendConfigInit( const uint32_t initID );
        void _sendConfigExit();

        /* command handler functions. */
        eqNet::CommandResult _cmdConfigInitReply(eqNet::Command& command );
        eqNet::CommandResult _cmdConfigExitReply(eqNet::Command& command );
    };

    std::ostream& operator << ( std::ostream& os, const Pipe* pipe );
};
#endif // EQS_PIPE_H
