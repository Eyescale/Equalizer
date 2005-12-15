/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_WINDOW_H
#define EQ_WINDOW_H

#include "commands.h"
#include "pipe.h"

#include <eq/net/base.h>
#include <eq/net/object.h>

namespace eq
{
    class Channel;

    class Window : public eqNet::Base, public eqNet::Object
    {
    public:
        /** 
         * Constructs a new window.
         */
        Window();

        /**
         * Destructs the window.
         */
        virtual ~Window();

        /** 
         * Returns the config of this window.
         * 
         * @return the config of this window. 
         */
        Config* getConfig() const { return (_pipe ? _pipe->getConfig() : NULL);}

        /**
         * @name Callbacks
         *
         * The callbacks are called by Equalizer during rendering to execute
         * various actions.
         */
        //@{

        /** 
         * Initialises this window.
         * TODO: Create thread, get display info, etc.
         */
        virtual bool init(){ return true; }

        /** 
         * Exit this window.
         */
        virtual void exit(){}
        //@}

    private:
        /** The parent node. */
        friend class Pipe;
        Pipe*        _pipe;

        /** The channels of this window. */
        std::vector<Channel*>     _channels;

        void _addChannel( Channel* channel );
        void _removeChannel( Channel* channel );

        /** The command functions. */
        void _cmdCreateChannel( eqNet::Node* node, const eqNet::Packet* packet);
        void _cmdDestroyChannel(eqNet::Node* node, const eqNet::Packet* packet);
        void _cmdInit( eqNet::Node* node, const eqNet::Packet* packet );
        void _cmdExit( eqNet::Node* node, const eqNet::Packet* packet );
    };
}

#endif // EQ_WINDOW_H

