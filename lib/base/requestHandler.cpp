
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "requestHandler.h"

#include "lock.h"
#include "scopedMutex.h"

#include <eq/base/debug.h>

using namespace eqBase;
using namespace std;

RequestHandler::RequestHandler( const bool threadSafe )
        : _requestID(1)
{
    _mutex               = threadSafe ? new Lock() : 0;
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
        CHECK_THREAD( _thread );

    Request* request;
    if( _freeRequests.empty( ))
        request = new Request;
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
        CHECK_THREAD( _thread );

    RequestHash::iterator iter = _requests.find( requestID );
    if( iter == _requests.end( ))
        return;

    Request* request = iter->second;
    _requests.erase( iter );
    _freeRequests.push_front( request );

    if( _mutex )
        _mutex->unset();
}

void* RequestHandler::waitRequest( const uint32_t requestID, bool* success,
                                   const uint32_t timeout )
{
    ScopedMutex mutex( _mutex );

    if( !_mutex )
        CHECK_THREAD( _thread );

    RequestHash::iterator iter = _requests.find( requestID );
    if( iter == _requests.end( ))
    {
        if( success )
            *success = false;
        return 0;
    }

    Request* request = iter->second;
    const bool requestServed = request->lock.set( timeout );
    void*      result        = requestServed ? request->result : 0;

    _requests.erase( iter );
    _freeRequests.push_front( request );
    
    if( success )
        *success = requestServed;

    return result;
}

void* RequestHandler::getRequestData( const uint32_t requestID )
{
    RequestHash::iterator iter = _requests.find( requestID );
    if( iter == _requests.end( ))
        return 0;

    return iter->second->data;
}

void RequestHandler::serveRequest( const uint32_t requestID, void* result )
{
    RequestHash::const_iterator iter = _requests.find( requestID );
    EQASSERTINFO( iter != _requests.end(),
                  "Attempt to serve unregistered request " << requestID );

    Request* request = iter->second;
    request->result = result;
    request->lock.unset();
}
    
bool RequestHandler::isServed( const uint32_t requestID ) const
{
    RequestHash::const_iterator iter = _requests.find( requestID );
    if( iter == _requests.end( ))
        return false;

    Request* request = iter->second;
    if( !request->lock.test( ))
        return true;

    return false;
}
    
