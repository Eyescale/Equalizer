
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PIPE_H
#define EQ_PIPE_H

#include "commands.h"

#include "node.h"

#include <eq/net/base.h>
#include <eq/net/object.h>

namespace eq
{
    class Window;

    class Pipe : public eqNet::Base, public eqNet::Object
    {
    public:
        /** 
         * Constructs a new pipe.
         */
        Pipe();

        /**
         * Destructs the pipe.
         */
        virtual ~Pipe();

        /** 
         * Returns the config of this pipe.
         * 
         * @return the config of this pipe. 
         */
        Config* getConfig() const { return (_node ? _node->getConfig() : NULL);}

        /**
         * @name Callbacks
         *
         * The callbacks are called by Equalizer during rendering to execute
         * various actions.
         */
        //@{

        /** 
         * Initialises this pipe.
         * TODO: Create thread, get display info, etc.
         */
        virtual bool init(){ return true; }

        /** 
         * Exit this pipe.
         */
        virtual void exit(){}
        //@}

    private:
        /** The parent node. */
        friend class Node;
        Node*                _node;

        /** The windows of this pipe. */
        std::vector<Window*>     _windows;

        void _addWindow( Window* window );
        void _removeWindow( Window* window );

        /** The command functions. */
        void _cmdCreateWindow( eqNet::Node* node, const eqNet::Packet* packet );
        void _cmdDestroyWindow( eqNet::Node* node, const eqNet::Packet* packet);
        void _cmdInit( eqNet::Node* node, const eqNet::Packet* packet );
        void _cmdExit( eqNet::Node* node, const eqNet::Packet* packet );
    };
}

#endif // EQ_PIPE_H

