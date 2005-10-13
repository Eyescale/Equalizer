
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "requestHandler.h"

using namespace eqBase;
using namespace std;

RequestHandler::RequestHandler( const Thread::Type type )
        : _type(type),
          _requestID(0)
{}

RequestHandler::~RequestHandler()
{
    while( !_freeRequests.empty( ))
    {
        Request* request = _freeRequests.front();
        _freeRequests.pop_front();
        delete request;
    }
}

uint RequestHandler::registerRequest( void* data )
{
    Request* request;
    if( _freeRequests.empty( ))
        request = new Request( _type );
    else
    {
        request = _freeRequests.front();
        _freeRequests.pop_front();
    }

    request->data = data;
    const uint requestID = _requestID++;
    _requests[requestID] = request;
    return requestID;
}

void* RequestHandler::waitRequest( const uint requestID, const uint timeout )
{
    Request* request = _requests[requestID];
    if( !request )
        return NULL;

    if( !request->lock->set( timeout ))
        return false;

    void* result = request->result;

    _requests.erase( requestID );
    _freeRequests.push_front( request );

    return result;
}

void* RequestHandler::getRequestData( const uint requestID )
{
    const Request* request = _requests[requestID];
    if( !request )
        return NULL;

    return request->data;
}

void RequestHandler::serveRequest( const uint requestID, void* result )
{
    Request* request = _requests[requestID];
    if( !request )
        return;

    request->result = result;
    request->lock->unset();
}
