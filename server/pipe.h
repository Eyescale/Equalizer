
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
        /** 
         * Constructs a new Pipe.
         */
        Pipe();

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
         */
        void startInit();

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
         * Update the per-frame data of this pipe.
         */
        void update();
        //*}

    private:
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


        void _send( const eqNet::Packet& packet ){ _node->send( packet ); }

        void _sendInit();
        void _sendExit();

        void _cmdInitReply(eqNet::Node* node, const eqNet::Packet* packet);
        void _cmdExitReply(eqNet::Node* node, const eqNet::Packet* packet);
    };

    std::ostream& operator << ( std::ostream& os, const Pipe* pipe );
};
#endif // EQS_PIPE_H
