
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
            virtual bool startNode( const uint nodeID );
            virtual bool connect( const uint nodeID );

            ssize_t runReceiver();
            
        private:
            Connection*     _listener;
            eqBase::Thread* _receiver;

            bool _startListener();
            bool _startReceiver();
            bool _startNodes();
            bool   _launchNodes();
            bool     _launchNode( const uint nodeID, 
                                  const ConnectionDescription* description );
            bool   _connectNodes();
            
            void  _stopNodes();
            void  _stopReceiver();
            void  _stopListener();

            ConnectionDescription* _getConnectionDescription( const uint nodeID)
                {
                    IDHash<ConnectionDescription*>::iterator iter =
                        _descriptions.find( nodeID);

                    if( iter == _descriptions.end() )
                        return NULL;
                    return (*iter).second;
                }

        };
    }
}

#endif //EQNET_SOCKET_NETWORK_H
