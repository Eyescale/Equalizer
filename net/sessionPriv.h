
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_SESSION_PRIV_H
#define EQNET_SESSION_PRIV_H

#include <eq/base/base.h>
#include <eq/net/global.h>
#include <eq/net/network.h>

#include "base.h"
#include "session.h"

#ifdef __GNUC__              // GCC 3.1 and later
#  include <ext/hash_map>
namespace Sgi = ::__gnu_cxx; 
#else                        //  other compilers
#  include <hash_map>
namespace Sgi = std;
#endif

namespace eqNet
{
    class Node;

    namespace priv
    {
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


        protected:
            Session(const uint id);

        private:
            /** 
             * The list of nodes in this session, the first node is always the
             * server node.
             */
            Sgi::hash_map<uint, Node*> _nodes;

            /** The list of networks in this session. */
            //std::vector<Network*>       _networks;

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

