
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "requestHandler.h"

using namespace eqBase;
using namespace std;

RequestHandler::RequestHandler( const Thread::Type type )
        : _type(type),
          _requestID(1)
{
#ifdef CHECK_THREADSAFETY
    _threadID = 0;
#endif;
}

RequestHandler::~RequestHandler()
{
    while( !_freeRequests.empty( ))
    {
        Request* request = _freeRequests.front();
        _freeRequests.pop_front();
        delete request;
    }
}

uint32_t RequestHandler::registerRequest( void* data )
{
    CHECK_THREAD;

    Request* request;
    if( _freeRequests.empty( ))
        request = new Request( _type );
    else
    {
        request = _freeRequests.front();
        _freeRequests.pop_front();
    }

    request->data = data;
    const uint32_t requestID = _requestID++;
    _requests[requestID] = request;
    
    return requestID;
}

void RequestHandler::unregisterRequest( const uint32_t requestID )
{
    CHECK_THREAD;

    Request* request = _requests[requestID];
    if( !request )
        return;

    _requests.erase( requestID );
    _freeRequests.push_front( request );
}

void* RequestHandler::waitRequest( const uint32_t requestID, bool* success,
                                   const uint32_t timeout )
{
    CHECK_THREAD;
    Request* request = _requests[requestID];

    if( !request || !request->lock->set( timeout ))
    {
        if( success ) *success = false;
        return NULL;
    }

    void* result = request->result;
    
    _requests.erase( requestID );
    _freeRequests.push_front( request );

    if( success ) *success = true;
    return result;
}

void* RequestHandler::getRequestData( const uint32_t requestID )
{
    const Request* request = _requests[requestID];
    if( !request )
        return NULL;

    return request->data;
}

void RequestHandler::serveRequest( const uint32_t requestID, void* result )
{
    Request* request = _requests[requestID];

    if( !request )
    {
        WARN << "Attempt to serve unregistered request" << endl;
        return;
    }

    request->result = result;
    request->lock->unset();
}
