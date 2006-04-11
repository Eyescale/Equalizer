
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_WINDOW_H
#define EQS_WINDOW_H

#include "pipe.h"

#include <eq/client/object.h>
#include <eq/client/pixelViewport.h>

#include <iostream>
#include <vector>

namespace eqs
{
    class Channel;
    class Pipe;

    /**
     * The window.
     */
    class Window : public eqNet::Object
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
         * Constructs a new Window.
         */
        Window();

        /** 
         * Constructs a new deep copy of a window.
         */
        Window( const Window& from );

        virtual ~Window(){}

        Server* getServer() const
            { return _pipe ? _pipe ->getServer() : NULL; }

        /** 
         * @return the state of this pipe.
         */
        State getState()    const { return _state; }

        /**
         * @name Data Access
         */
        //*{
        /** 
         * Adds a new channel to this window.
         * 
         * @param channel the channel.
         */
        void addChannel( Channel* channel );

        /** 
         * Removes a channel from this window.
         * 
         * @param channel the channel
         * @return <code>true</code> if the channel was removed, <code>false</code>
         *         otherwise.
         */
        bool removeChannel( Channel* channel );

        /** 
         * Returns the number of channels on this window.
         * 
         * @return the number of channels on this window. 
         */
        uint32_t nChannels() const { return _channels.size(); }

        /** 
         * Gets a channel.
         * 
         * @param index the channel's index. 
         * @return the channel.
         */
        Channel* getChannel( const uint32_t index ) const
            { return _channels[index]; }

        Node* getNode() const { return (_pipe ? _pipe->getNode() : NULL); }
        Config* getConfig() const 
            { return (_pipe ? _pipe->getConfig() : NULL); }
        
        /** 
         * References this window as being actively used.
         */
        void refUsed();

        /** 
         * Unreferences this window as being actively used.
         */
        void unrefUsed();

        /** 
         * Return if this window is actively used.
         *
         * @return <code>true</code> if this window is actively used,
         *         <code>false</code> if not.
         */
        bool isUsed() const { return (_used!=0); }

        /** 
         * Set the window's pixel viewport wrt its parent pipe.
         * 
         * @param pvp the viewport in pixels.
         */
        void setPixelViewport( const eq::PixelViewport& pvp ) { _pvp = pvp; }

        /** 
         * Return this window's pixel viewport.
         * 
         * @return the pixel viewport.
         */
        const eq::PixelViewport& getPixelViewport() const { return _pvp; }

        /** 
         * Clear the swap group of the window.
         */
        void resetSwapGroup();

        /** 
         * Add the window to the swap group owned by the master.
         * 
         * @param master the owner of the swap group.
         */
        void setSwapGroup( Window* master );
        //*}

        /**
         * @name Operations
         */
        //*{
        /** 
         * Start initializing this node.
         *
         * @param initID an identifier to be passed to all init methods.
         */
        void startInit( const uint32_t initID );

        /** 
         * Synchronize the initialisation of the node.
         * 
         * @return <code>true</code> if the node was initialised successfully,
         *         <code>false</code> if not.
         */
        bool syncInit();
        
        /** 
         * Starts exiting this node.
         */
        void startExit();

        /** 
         * Synchronize the exit of the node.
         * 
         * @return <code>true</code> if the node exited cleanly,
         *         <code>false</code> if not.
         */
        bool syncExit();
        
        /** 
         * Update the per-frame data of this window.
         *
         * @param frameID a per-frame identifier passed to all rendering
         *                methods.
         */
        void update( const uint32_t frameID );
        //*}

    private:
        /** The current operational state. */
        State _state;

        /** The child channels. */
        std::vector<Channel*> _channels;

        /** Number of entitities actively using this window. */
        uint32_t _used;

        /** The parent pipe. */
        Pipe* _pipe;
        friend class Pipe;

        /** The request id for pending asynchronous operations. */
        uint32_t _pendingRequestID;

        /** The absolute size and position of the window. */
        eq::PixelViewport _pvp;
        
        /** The fractional size and position of the window. */
        eq::Viewport _vp;
        
        /** The master window of the swap group, can be <code>this</code> */
        Window*              _swapMaster;
        /** The list of windows participating in the swap group. */
        std::vector<Window*> _swapGroup;
        /** The id of the current swap barrier. */
        eqNet::Barrier*      _swapBarrier;
        
        /** common code for all constructors */
        void _construct();

        void _send( const eqNet::Packet& packet ) { getNode()->send( packet ); }

        void _sendInit( const uint32_t initID );
        void _sendExit();

        /* command handler functions. */
        eqNet::CommandResult _cmdInitReply(eqNet::Node* node, 
                                           const eqNet::Packet* packet);
        eqNet::CommandResult _cmdExitReply(eqNet::Node* node, 
                                           const eqNet::Packet* packet);
        eqNet::CommandResult _cmdSwap(eqNet::Node* node,
                                      const eqNet::Packet* packet);
        eqNet::CommandResult _cmdSwapWithBarrier(eqNet::Node* node,
                                                 const eqNet::Packet* packet);
    };

    std::ostream& operator << ( std::ostream& os, const Window* window );
};
#endif // EQS_WINDOW_H
