
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_PACKETQUEUE_H
#define EQNET_PACKETQUEUE_H

#include <eq/base/lock.h>
#include <eq/base/mtQueue.h>

#include <list>

namespace eqNet
{
    class Node;
    class Packet;

    /**
     * A RequestQueue is a thread-safe queue for request packets.
     *
     * The packets are copied and can therefore be released after they have been
     * pushed to the queue.
     */
    class RequestQueue
    {
    public:
        RequestQueue();
        ~RequestQueue();

        /** 
         * Push a request to the queue.
         *
         * The request's command is incremented by one, which enables the usage
         * of the same code for handling the arriving packet and the queued
         * request, as they use different commands.
         * 
         * @param node the node sending the packet.
         * @param packet the command packet.
         */
        void push( Node* node, const Packet* packet );

        /** 
         * Pop a request from the queue.
         *
         * The returned packet is valid until the next pop operation.
         * 
         * @param node return value for the node sending the package.
         * @param packet return value for the packet.
         */
        void pop( Node** node, Packet** packet );

    private:
        struct Request
        {
            Request() : packet( NULL ) {}
            Node*   node;
            Packet* packet;
        };

        /** Thread-safe request queue. */
        eqBase::MTQueue<Request*> _requests;
        
        /** The last popped request, to be freed upon the next pop. */
        Request*                  _lastRequest;

        /** The free request cache. */
        std::list<Request*>       _freeRequests;
        eqBase::Lock              _freeRequestsLock;

        void       _freeRequest( Request* request );
    };
};

#endif //EQNET_PACKETQUEUE_H
