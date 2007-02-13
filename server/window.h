
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_WINDOW_H
#define EQS_WINDOW_H

#include "pipe.h"

#include <eq/client/object.h>
#include <eq/client/pixelViewport.h>
#include <eq/client/window.h>

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

        virtual uint32_t getTypeID() const { return eq::Object::TYPE_WINDOW; }

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
         * @return <code>true</code> if the channel was removed,
         *         <code>false</code> otherwise.
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
        
        const eq::Window::DrawableConfig& getDrawableConfig() const
            { return _drawableConfig; }

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

        void setName( const std::string& name ) { _name = name; }
        const std::string& getName() const      { return _name; }

        /** 
         * Set the window's pixel viewport wrt its parent pipe.
         * 
         * @param pvp the viewport in pixels.
         */
        void setPixelViewport( const eq::PixelViewport& pvp );

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
         * Create a new swap barrier and join it for the next update.
         *
         * @return the created swap barrier.
         */
        eqNet::Barrier* newSwapBarrier();

        /** 
         * Join a swap barrier for the next update.
         * 
         * @param barrier the swap barrier.
         */
        void joinSwapBarrier( eqNet::Barrier* barrier );
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
         * Update one frame.
         *
         * @param frameID a per-frame identifier passed to all rendering
         *                methods.
         */
        void updateDraw( const uint32_t frameID );

        /** 
         * Trigger the post-draw operations.
         *
         * @param frameID a per-frame identifier passed to all rendering
         *                methods.
         */
        void updatePost( const uint32_t frameID );
        //*}

        /**
         * @name Attributes
         */
        //*{
        void setIAttribute( const eq::Window::IAttribute attr,
                            const int32_t value )
            { _iAttributes[attr] = value; }
        int32_t  getIAttribute( const eq::Window::IAttribute attr ) const
            { return _iAttributes[attr]; }
        //*}

        /** @name Error information. */
        //@{
        /** @return the error message from the last operation. */
        const std::string& getErrorMessage() const { return _error; }
        //@}

    protected:
        virtual ~Window();

        /** Registers request packets waiting for a return value. */
        eqBase::RequestHandler _requestHandler;

    private:
        eq::Window::DrawableConfig _drawableConfig;

        /** The current operational state. */
        State _state;

        std::string _name;

        /** The reason for the last error. */
        std::string            _error;

        /** Integer attributes. */
        int32_t _iAttributes[eq::Window::IATTR_ALL];

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
        
        /** The list of master swap barriers for the current frame. */
        std::vector<eqNet::Barrier*> _masterSwapBarriers;
        /** The list of slave swap barriers for the current frame. */
        std::vector<eqNet::Barrier*> _swapBarriers;
        
        /** common code for all constructors */
        void _construct();

        /** Clears all swap barriers of the window. */
        void _resetSwapBarriers();

        void _send( eqNet::ObjectPacket& packet ) 
            { send( getNode()->getNode(), packet ); }
        void _send( eqNet::ObjectPacket& packet, const std::string& string ) 
            { send( getNode()->getNode(), packet, string ); }

        void _sendInit( const uint32_t initID );
        void _sendExit();

        void _updateSwap();

        /* command handler functions. */
        eqNet::CommandResult _cmdInitReply( eqNet::Command& command ); 
        eqNet::CommandResult _cmdExitReply( eqNet::Command& command ); 
        eqNet::CommandResult _reqSetPixelViewport( eqNet::Command& command );
    };

    std::ostream& operator << ( std::ostream& os, const Window* window );
};
#endif // EQS_WINDOW_H
