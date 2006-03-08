
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "requestCache.h"

#include "packets.h"

#define SMALL_PACKET_SIZE 1024

using namespace eqNet;

RequestCache::RequestCache()
{}

RequestCache::~RequestCache()
{
    while( !_freeRequests.empty( ))
    {
        Request* request = _freeRequests.front();
        _freeRequests.pop_front();
        if( request->packet )
            free( request->packet );
        delete request;
    }
}

Request* RequestCache::alloc( Node* node, const Packet* packet )
{
    Request* request;

    if( _freeRequests.empty( ))
        request = new Request;
    else
    {
        request = _freeRequests.front();
        _freeRequests.pop_front();
    }

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
    return request;
}

void RequestCache::release( Request* request )
{
    if( request->packet->size > SMALL_PACKET_SIZE )
    {
        free( request->packet );
        request->packet = NULL;
    }
    
    _freeRequests.push_back( request );
}

