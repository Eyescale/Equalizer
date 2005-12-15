
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "requestQueue.h"

#include "packets.h"

#define SMALL_PACKET_SIZE 1024

using namespace eqNet;

RequestQueue::RequestQueue()
        : _lastRequest(NULL)
{}

RequestQueue::~RequestQueue()
{
    if( _lastRequest )
        _freeRequest( _lastRequest );

    while( !_freeRequests.empty( ))
    {
        Request* request = _freeRequests.front();
        _freeRequests.pop_front();
        if( request->packet )
            free( request->packet );
        delete request;
    }
}

void RequestQueue::push( Node* node, const Packet* packet )
{
    Request* request;

    _freeRequestsLock.set();
    if( _freeRequests.empty( ))
        request = new Request;
    else
    {
        request = _freeRequests.front();
        _freeRequests.pop_front();
    }
    _freeRequestsLock.unset();

    request->node = node;

    const size_t packetSize = packet->size;
    if( packetSize <= SMALL_PACKET_SIZE )
    {
        if( !request->packet )
            request->packet = (Packet*)malloc( SMALL_PACKET_SIZE );
    }
    else
    {
        if( request->packet )
            free( request->packet );
        
        request->packet = (Packet*)malloc( packetSize );
    }
        
    memcpy( request->packet, packet, packetSize );
    request->packet->command++; // REQ must always follow CMD
    _requests.push( request );
}

void RequestQueue::pop( Node** node, Packet** packet )
{
    if( _lastRequest )
        _freeRequest( _lastRequest );
    
    _lastRequest = _requests.pop();
    *node   = _lastRequest->node;
    *packet = _lastRequest->packet;
}
    
void RequestQueue::_freeRequest( Request* request )
{
    if( request->packet->size > SMALL_PACKET_SIZE )
    {
        free( request->packet );
        request->packet = NULL;
    }
    
    _freeRequestsLock.set();
    _freeRequests.push_back( request );
    _freeRequestsLock.unset();
}

