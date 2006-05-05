
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "requestHandler.h"

#include "lock.h"

using namespace eqBase;
using namespace std;

RequestHandler::RequestHandler( const Thread::Type type, const bool threadSafe )
        : _type(type),
          _requestID(1)
{
    _mutex = threadSafe ? new Lock( type ) : NULL;
        
#ifdef CHECK_THREADSAFETY
    _threadID = 0;
#endif
}

RequestHandler::~RequestHandler()
{
    while( !_freeRequests.empty( ))
    {
        Request* request = _freeRequests.front();
        _freeRequests.pop_front();
        delete request;
    }

    if( _mutex )
        delete _mutex;
}

uint32_t RequestHandler::registerRequest( void* data )
{
    if( _mutex )
        _mutex->set();
    else
        CHECK_THREAD( _threadID );

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
    
    if( _mutex )
        _mutex->unset();
    return requestID;
}

void RequestHandler::unregisterRequest( const uint32_t requestID )
{
    if( _mutex )
        _mutex->set();
    else
        CHECK_THREAD( _threadID );

    Request* request = _requests[requestID];
    if( !request )
        return;

    _requests.erase( requestID );
    _freeRequests.push_front( request );
    if( _mutex )
        _mutex->unset();
}

void* RequestHandler::waitRequest( const uint32_t requestID, bool* success,
                                   const uint32_t timeout )
{
    if( _mutex )
        _mutex->set();
    else
        CHECK_THREAD( _threadID );

    Request* request = _requests[requestID];

    if( !request || !request->lock->set( timeout ))
    {
        if( success ) *success = false;
        if( _mutex )
            _mutex->unset();
        return NULL;
    }

    void* result = request->result;
    
    _requests.erase( requestID );
    _freeRequests.push_front( request );

    if( success ) *success = true;
    if( _mutex )
        _mutex->unset();
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
        EQWARN << "Attempt to serve unregistered request" << endl;
        return;
    }

    request->result = result;
    request->lock->unset();
}
