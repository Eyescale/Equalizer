
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_SESSION_PRIV_H
#define EQNET_SESSION_PRIV_H

#include <eq/base/base.h>
#include <eq/net/network.h>

#include "base.h"
#include "session.h"

namespace eqNet
{
    class Node;

    namespace priv
    {
        class Network;
        class Server;

        class Session : public Base, public eqNet::Session
        {
        public:
            /** 
             * Creates a new session on the specified server.
             * 
             * @param server the server address.
             */
            static Session* create( const char *server );

            /**
             * Adds a new network to this session.
             *
             * @param protocol the network protocol.
             */
            Network* addNetwork( const NetworkProtocol protocol );

            Session(const uint id);

        private:
            /** The list of nodes in this session. */
            IDHash<Node*> _nodes;

            /** The list of networks in this session. */
            IDHash<Network*> _networks;
            /** The identifier of the next network. */
            uint _networkID;

            bool _create( const char* serverAddress );

            /** 
             * Opens and returns a session to the specified server.
             * 
             * @param server the server address.
             * @return the Server object, or <code>NULL</code> if no
             *         server could be contacted.
             */
            Server* _openServer( const char* server );
        };
    }
}
#endif // EQNET_SESSION_PRIV_H

