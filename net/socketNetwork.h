
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_SOCKET_NETWORK_H
#define EQNET_SOCKET_NETWORK_H

#include "dynamicNetwork.h"

#include <sys/param.h>
#include <vector>

namespace eqBase
{
    class Thread;
}

namespace eqNet
{
    namespace priv
    {
#       define STARTUP_TIMEOUT 2000 // ms
        class SocketNetwork : public DynamicNetwork
        {
        public:
            SocketNetwork( const uint id, Session* session );
            virtual ~SocketNetwork();

            virtual bool start();
            virtual void stop();
            virtual bool startNode( Node* node );

            ssize_t runReceiver();
            
        private:
            Connection*     _listener;
            eqBase::Thread* _receiver;

            bool _startListener();
            bool _startReceiver();
            bool _startNodes();
            bool   _launchNodes();
            bool     _launchNode( Node* node, 
                                  const ConnectionDescription* description );
            bool   _connectNodes();
            
            void  _stopNodes();
            void  _stopReceiver();
            void  _stopListener();

        };
    }
}

#endif //EQNET_SOCKET_NETWORK_H
