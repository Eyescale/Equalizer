/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_CHANNEL_H
#define EQ_CHANNEL_H

#include "commands.h"
#include "window.h"

#include <eq/base/pixelViewport.h>
#include <eq/net/base.h>
#include <eq/net/object.h>

namespace eq
{
    class Channel;
    class Node;

    class Channel : public eqNet::Base, public eqNet::Object
    {
    public:
        /** 
         * Constructs a new channel.
         */
        Channel();

        /**
         * Destructs the channel.
         */
        virtual ~Channel();

        Pipe* getPipe() const { return ( _window ? _window->getPipe() : NULL );}

        /**
         * @name Callbacks
         *
         * The callbacks are called by Equalizer during rendering to execute
         * various actions.
         */
        //@{

        /** 
         * Initialises this channel.
         * TODO: Create thread, get display info, etc.
         */
        virtual bool init(){ return true; }

        /** 
         * Exit this channel.
         */
        virtual void exit(){}

        /** 
         * Clear the frame buffer.
         */
        virtual void clear();
        //@}

        /**
         * @name Operations
         *
         * Operations are only available from within certain callbacks.
         */
        //*{

        /** 
         * Apply the current rendering buffer.
         */
        void applyBuffer();

        /** 
         * Apply the OpenGL viewport for the current rendering task.
         */
        void applyViewport();
        //*}
    private:
        /** The parent node. */
        friend class Window;
        Window*      _window;

        /** The draw buffer for the current task. */
        GLenum                _drawBuffer;

        /** The pixel viewport for the current task. */
        eqBase::PixelViewport _pvp;

        void _pushRequest( eqNet::Node* node, const eqNet::Packet* packet );
        void _reqInit( eqNet::Node* node, const eqNet::Packet* packet );
        void _reqExit( eqNet::Node* node, const eqNet::Packet* packet );
        void _reqClear( eqNet::Node* node, const eqNet::Packet* packet );
    };
}

#endif // EQ_CHANNEL_H

