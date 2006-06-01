
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_CONFIG_H
#define EQ_CONFIG_H

#include "client.h"
#include "commands.h"

#include <eq/base/base.h>
#include <eq/base/requestHandler.h>
#include <eq/net/packets.h>
#include <eq/net/session.h>


namespace eq
{
    class Node;
    class SceneObject;
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
         * @param initID an identifier to be passed to all init methods.
         * @return <code>true</code> if the initialisation was successful,
         *         <code>false</code> if not.
         */
        bool init( const uint32_t initID );

        /** 
         * Exits this configuration.
         * 
         * A config which could not be exited properly may not be
         * re-initialised. The exit function does not provide the possibility to
         * pass an exit identifier to the exit methods, because individual
         * entities may stopped dynamically by the server when running a config,
         * i.e., before exit() is called.
         *
         * @return <code>true</code> if the exit was successful,
         *         <code>false</code> if not.
         */
        bool exit();

        /**
         * @name Frame Control
         */
        //*{
        /** 
         * Requests a new frame of rendering.
         * 
         * @param frameID a per-frame identifier passed to all rendering
         *                methods.
         * @return the frame number of the new frame.
         */
        uint32_t beginFrame( const uint32_t frameID );

        /** 
         * Sends frame data to 
         * 
         * @param data 
         */
        void sendSceneObject( SceneObject* object );
        void flushSceneObjects();

        /** 
         * Synchronizes the end of a frame.
         * 
         * @return the frame number of the finished frame, or <code>0</code> if
         *         no frame has been finished.
         */
        uint32_t endFrame();
        //*}

        /** 
         * Push a request from the receiver to the app thread to be handled
         * asynchronously.
         * 
         * @param node the node sending the packet.
         * @param packet the command packet.
         */
        void pushRequest( eqNet::Node* node, const eqNet::Packet* packet )
            { ((eq::Client*)getLocalNode())->pushRequest( node, packet ); }

    private:
        /** The local proxy of the server hosting the session. */
        friend class Server;
        Server* _server;

        void _addNode( Node* node );
        void _removeNode( Node* node );

        /** Registers pending requests waiting for a return value. */
        eqBase::RequestHandler _requestHandler;

        /** The command functions. */
        eqNet::CommandResult _cmdCreateNode( eqNet::Node* node,
                                             const eqNet::Packet* packet );
        eqNet::CommandResult _cmdDestroyNode( eqNet::Node* node,
                                              const eqNet::Packet* packet );
        eqNet::CommandResult _cmdInitReply( eqNet::Node* node,
                                            const eqNet::Packet* packet );
        eqNet::CommandResult _cmdExitReply( eqNet::Node* node,
                                            const eqNet::Packet* packet );
        eqNet::CommandResult _cmdBeginFrameReply( eqNet::Node* node, 
                                                  const eqNet::Packet* packet );
        eqNet::CommandResult _cmdEndFrameReply( eqNet::Node* node,
                                                const eqNet::Packet* packet);
    };
}

#endif // EQ_CONFIG_H

