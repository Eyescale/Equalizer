
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_CONFIG_H
#define EQ_CONFIG_H

#include <eq/base/base.h>
#include <eq/base/requestHandler.h>
#include <eq/net/packets.h>
#include <eq/net/session.h>

#include "commands.h"

namespace eq
{
    struct ConfigPacket;
    class Server;

    class Config : public eqNet::Session
    {
    public:
        /** 
         * Constructs a new config.
         * 
         * @param server the server hosting the config.
         */
        Config();

        /** 
         * Initialises this configuration.
         * 
         * @return <code>true</code> if the initialisation was successful,
         *         <code>false</code> if not.
         */
        bool init();

        /** 
         * Exits this configuration.
         * 
         * A config which could not be exited properly may not be
         * re-initialised.
         *
         * @return <code>true</code> if the exit was successful,
         *         <code>false</code> if not.
         */
        bool exit();

        /** 
         * Requests a new frame of rendering.
         * 
         * @todo per-frame data passed to all rendering methods
         * @return the frame number of the new frame.
         */
        uint32_t frameBegin();

        /** 
         * Synchronizes the end of a frame.
         * 
         * @return the frame number of the finished frame, or <code>0</code> if
         *         no frame has been finished.
         */
        uint32_t frameEnd();

    private:
        /** The local proxy of the server hosting the session. */
        friend class Server;
        Server* _server;

        /** Registers pending requests waiting for a return value. */
        eqBase::RequestHandler _requestHandler;

        /** The command functions. */
        eqNet::CommandResult _cmdInitReply( eqNet::Node* node,
                                            const eqNet::Packet* packet );
        eqNet::CommandResult _cmdExitReply( eqNet::Node* node,
                                            const eqNet::Packet* packet );
        eqNet::CommandResult _cmdFrameBeginReply( eqNet::Node* node, 
                                                  const eqNet::Packet* packet );
        eqNet::CommandResult _cmdFrameEndReply( eqNet::Node* node,
                                                const eqNet::Packet* packet);
    };
}

#endif // EQ_CONFIG_H

