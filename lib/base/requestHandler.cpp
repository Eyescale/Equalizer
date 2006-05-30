
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
    _deletedRequestsLock = threadSafe ? NULL : new Lock( type );
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
    if( _deletedRequestsLock )
        delete _deletedRequestsLock;
}

uint32_t RequestHandler::registerRequest( void* data )
{
    if( _mutex )
        _mutex->set();
    else
        CHECK_THREAD( _threadID );

    _recycleRequests();

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

    _recycleRequests();
    _unregisterRequest( requestID );

    if( _mutex )
        _mutex->unset();
}

void RequestHandler::_unregisterRequest( const uint32_t requestID )
{
    RequestHash::iterator iter = _requests.find( requestID );
    if( iter == _requests.end( ))
        return;

    Request* request = iter->second;
    _requests.erase( iter );
    _freeRequests.push_front( request );
}

void* RequestHandler::waitRequest( const uint32_t requestID, bool* success,
                                   const uint32_t timeout )
{
    if( _mutex )
        _mutex->set();
    else
        CHECK_THREAD( _threadID );

    _recycleRequests();

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

void RequestHandler::deleteRequest( const uint32_t requestID )
{
    if( _mutex && _mutex->trySet( )) // only try to get lock, waitReq can hold
    {                                // lock for a long time, deadlock possible
        unregisterRequest( requestID );
        _mutex->unset();
        return;
    }

    RequestHash::iterator iter = _requests.find( requestID );
    if( iter == _requests.end( ))
    {
        EQWARN << "Attempt to serve unregistered request" << endl;
        return;
    }
    
    _deletedRequestsLock->set();
    _deletedRequests.push_back( requestID );
    _deletedRequestsLock->unset();
}

void RequestHandler::_recycleRequests()
{
    if( _deletedRequests.empty( ))
        return;

    _deletedRequestsLock->set();
    for( vector<uint32_t>::iterator iter = _deletedRequests.begin();
         iter != _deletedRequests.end(); ++iter )
        unregisterRequest( *iter );

    _deletedRequests.clear();
    _deletedRequestsLock->unset();
}
