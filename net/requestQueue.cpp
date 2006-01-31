
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "requestQueue.h"

#include "packets.h"

using namespace eqNet;

RequestQueue::RequestQueue()
        : _lastRequest(NULL)
{}

RequestQueue::~RequestQueue()
{
    if( _lastRequest )
        _requestCache.release( _lastRequest );
}

void RequestQueue::push( Node* node, const Packet* packet )
{
    _requestCacheLock.set();
    Request* request = _requestCache.alloc( node, packet );
    _requestCacheLock.unset();

    request->packet->command++; // REQ must always follow CMD
    _requests.push( request );
}

void RequestQueue::pop( Node** node, Packet** packet )
{
    if( _lastRequest )
    {
        _requestCacheLock.set();
        _requestCache.release( _lastRequest );
        _requestCacheLock.unset();
    }
    
    _lastRequest = _requests.pop();
    *node   = _lastRequest->node;
    *packet = _lastRequest->packet;
}
