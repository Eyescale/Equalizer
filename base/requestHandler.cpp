
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "requestHandler.h"

using namespace eqBase;
using namespace std;

#define CHECK_THREADSAFETY

#ifdef CHECK_THREADSAFETY
static pthread_t registerThread = 0;
#endif

RequestHandler::RequestHandler( const Thread::Type type )
        : _type(type),
          _requestID(1)
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
#ifdef CHECK_THREADSAFETY
    if( !registerThread )
        registerThread = pthread_self();

    ASSERT( registerThread == pthread_self( ));
#endif

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

void RequestHandler::unregisterRequest( const uint requestID )
{
#ifdef CHECK_THREADSAFETY
    ASSERT( registerThread == pthread_self( ));
#endif

    Request* request = _requests[requestID];
    if( !request )
        return;

    _requests.erase( requestID );
    _freeRequests.push_front( request );
}

void* RequestHandler::waitRequest( const uint requestID, bool* success,
                                   const uint timeout )
{
#ifdef CHECK_THREADSAFETY
    ASSERT( registerThread == pthread_self( ));
#endif

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
