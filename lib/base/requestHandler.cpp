
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
    _mutex               = threadSafe ? new Lock( type ) : NULL;
    CHECK_THREAD_INIT( _threadID );
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
    if( _mutex )
        _mutex->set();
    else
        CHECK_THREAD( _threadID );

    RequestHash::iterator iter = _requests.find( requestID );
    if( iter != _requests.end( ))
    {
        Request* request = iter->second;
        if( request->lock->set( timeout ))
        {
            void* result = request->result;
    
            _requests.erase( iter );
            _freeRequests.push_front( request );

            if( success ) *success = true;
            if( _mutex )
                _mutex->unset();
            return result;
        }
    }

    if( success ) *success = false;
    if( _mutex )
        _mutex->unset();
    return NULL;
}

void* RequestHandler::getRequestData( const uint32_t requestID )
{
    RequestHash::iterator iter = _requests.find( requestID );
    if( iter == _requests.end( ))
        return NULL;

    return iter->second->data;
}

void RequestHandler::serveRequest( const uint32_t requestID, void* result )
{
    RequestHash::iterator iter = _requests.find( requestID );
    if( iter == _requests.end( ))
    {
        EQWARN << "Attempt to serve unregistered request" << endl;
        return;
    }

    Request* request = iter->second;
    request->result = result;
    request->lock->unset();
}
    
void* RequestHandler::peekRequest( const uint32_t requestID, bool* success )
{
    RequestHash::iterator iter = _requests.find( requestID );
    if( iter != _requests.end( ))
    {
        Request* request = iter->second;
        if( !request->lock->test( ))
        {
            if( success ) *success = true;
            return request->result;
        }
    }
    if( success ) *success = false;
    return false;
}
    
