
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
    while( _freeRequests.size() > 0 )
    {
        Request* request = _freeRequests.front();
        _freeRequests.pop_front();
        delete request;
    }
}

uint RequestHandler::registerRequest()
{
    Request* request = _freeRequests.front();
    _freeRequests.pop_front();

    if( !request )
        request = new Request( _type );

    const uint requestID = _requestID++;
    _requests[requestID] = request;
    return requestID;
}

void* RequestHandler::waitRequest( const uint requestID )
{
    Request* request = _requests[requestID];
    if( !request )
        return NULL;

    request->lock->set();
    void* result = request->result;

    _requests.erase( requestID );
    _freeRequests.push_front( request );

    return result;
}

void RequestHandler::serveRequest( const uint requestID, void* result )
{
    Request* request = _requests[requestID];
    if( !request )
        return;

    request->result = result;
    request->lock->unset();
}
