
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_PACKETCACHE_H
#define EQNET_PACKETCACHE_H

#include <list>

namespace eqNet
{
    class Node;
    class Packet;

    struct Request
    {
        Request() : packet( NULL ) {}
        Node*   node;
        Packet* packet;
    };
    
    /**
     * A RequestCache handles the creation of request for the RequestQueue and
     * the node reschedule queue.
     *
     * The packets are copied and can therefore be retained in the queues.
     */
    class RequestCache
    {
    public:
        RequestCache();
        ~RequestCache();

        /** 
         * Create a new request.
         *
         * @param node the node sending the packet.
         * @param packet the command packet.
         * @return the request.
         */
        Request *alloc( Node* node, const Packet* packet );

        /** 
         * Release a request.
         *
         * @param request the request.
         */
        void release( Request* request );

    private:
        /** The free request cache. */
        std::list<Request*>       _freeRequests;
    };
};

#endif //EQNET_PACKETCACHE_H
