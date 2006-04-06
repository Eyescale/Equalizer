
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_PIPE_H
#define EQS_PIPE_H

#include "node.h"
#include <eq/net/base.h>
#include <eq/net/object.h>

#include <ostream>
#include <vector>

namespace eqs
{
    class Config;
    class Window;

    /**
     * The pipe.
     */
    class Pipe : public eqNet::Base, public eqNet::Object
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

        /**
         * @name Data Access
         */
        //*{
        void setDisplay( const uint32_t display ){ _display = display; }
        uint32_t getDisplay() const              { return _display; }
        void setScreen( const uint32_t screen )  { _screen = screen; }
        uint32_t getScreen() const               { return _screen; }
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
        void startInit( const uint32_t initID );

        /** 
         * Synchronize the initialisation of the pipe.
         * 
         * @return <code>true</code> if the pipe was initialised successfully,
         *         <code>false</code> if not.
         */
        bool syncInit();
        
        /** 
         * Starts exiting this pipe.
         */
        void startExit();

        /** 
         * Synchronize the exit of the pipe.
         * 
         * @return <code>true</code> if the pipe exited cleanly,
         *         <code>false</code> if not.
         */
        bool syncExit();

        /** 
         * Update one frame.
         *
         * @param frameID a per-frame identifier passed to all rendering
         *                methods.
         */
        void update( const uint32_t frameID );

        /**
         * Finish one frame.
         */
        void syncUpdate() { _frameSync.wait(); }

        /** 
         * Adjust the latency of this pipe.
         * 
         * If decrementing the latency, the function blocks until the
         * corresponding frames have been finished.
         *
         * @param delta the delta of the latency change.
         */
        void adjustLatency( const int delta ){ _frameSync.adjust( delta ); }
        //*}

    private:
        /** The current operational state. */
        State _state;

        /** The list of windows. */
        std::vector<Window*>   _windows;

        /** Number of entitities actively using this pipe. */
        uint32_t _used;

        /** The parent node. */
        Node* _node;
        friend class Node;

        /** The request id for pending asynchronous operations. */
        uint32_t _pendingRequestID;

        
        /** The display (X11) or ignored (Win32, CGL). */
        uint32_t _display;

        /** The screen (X11), adapter (Win32) or virtual screen (CGL). */
        uint32_t _screen;

        /* The display (CGL) or output channel (X11, Win32). */
        //uint32_t _channel;

        /** A counter for the number of allowed pending frames. */
        eqBase::Sema _frameSync;


        /** common code for all constructors */
        void _construct();

        void _send( const eqNet::Packet& packet ){ _node->send( packet ); }

        void _sendInit( const uint32_t initID );
        void _sendExit();

        /* command handler functions. */
        eqNet::CommandResult _cmdInitReply(eqNet::Node* node,
                                           const eqNet::Packet* packet);
        eqNet::CommandResult _cmdExitReply(eqNet::Node* node,
                                           const eqNet::Packet* packet);
        eqNet::CommandResult _cmdFrameSync( eqNet::Node* node,
                                            const eqNet::Packet* packet );
    };

    std::ostream& operator << ( std::ostream& os, const Pipe* pipe );
};
#endif // EQS_PIPE_H
