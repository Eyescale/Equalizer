 
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "requestHandler.h"

#include "scopedMutex.h"

#include <co/base/debug.h>

namespace co
{
namespace base
{
RequestHandler::RequestHandler()
        : _requestID( 1 ) 
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

uint32_t RequestHandler::registerRequest( void* data )
{
    ScopedMutex< SpinLock > mutex( _mutex );

    Request* request;
    if( _freeRequests.empty( ))
        request = new Request;
    else
    {
        request = _freeRequests.front();
        _freeRequests.pop_front();
    }

    request->data = data;
    _requestID = ( _requestID + 1 ) % EQ_MAX_UINT32;
    _requests[ _requestID ] = request;
    return _requestID;
}

void RequestHandler::unregisterRequest( const uint32_t requestID )
{
    ScopedMutex< SpinLock > mutex( _mutex );

    RequestHash::iterator i = _requests.find( requestID );
    if( i == _requests.end( ))
        return;

    Request* request = i->second;
    _requests.erase( i );
    _freeRequests.push_front( request );
}

bool RequestHandler::waitRequest( const uint32_t requestID, void*& rPointer,
                                  const uint32_t timeout )
{
    Request::Result result;
    if( !_waitRequest( requestID, result, timeout ))
        return false;

    rPointer = result.rPointer;
    return true;
}
bool RequestHandler::waitRequest( const uint32_t requestID, uint32_t& rUint32,
                                  const uint32_t timeout )
{
    Request::Result result;
    if( !_waitRequest( requestID, result, timeout ))
        return false;

    rUint32 = result.rUint32;
    return true;
}

bool RequestHandler::waitRequest( const uint32_t requestID, uint128_t& rUint128,
                                  const uint32_t timeout )
{
    Request::Result result;
    if( !_waitRequest( requestID, result, timeout ))
        return false;

    rUint128.high() = result.rUint128.high;
    rUint128.low() = result.rUint128.low;
    return true;
}

bool RequestHandler::waitRequest( const uint32_t requestID, bool& rBool,
                                  const uint32_t timeout )
{
    Request::Result result;
    if( !_waitRequest( requestID, result, timeout ))
        return false;

    rBool = result.rBool;
    return true;
}
bool RequestHandler::waitRequest( const uint32_t requestID )
{
    Request::Result result;
    return _waitRequest( requestID, result, EQ_TIMEOUT_INDEFINITE );
}

bool RequestHandler::_waitRequest( const uint32_t requestID, 
                                   Request::Result& result,
                                   const uint32_t timeout )
{
    Request* request = 0;
    {
        ScopedMutex< SpinLock > mutex( _mutex );

        RequestHash::const_iterator i = _requests.find( requestID );
        if( i == _requests.end( ))
            return false;

        request = i->second;
    }

    const bool requestServed = request->lock.set( timeout );
    if( requestServed )
        result = request->result;

    unregisterRequest( requestID );
    
    //EQINFO << "Cleared " << requestID << std::endl;
    return requestServed;
}

void* RequestHandler::getRequestData( const uint32_t requestID )
{
    ScopedMutex< SpinLock > mutex( _mutex );
    RequestHash::const_iterator i = _requests.find( requestID );
    if( i == _requests.end( ))
        return 0;

    return i->second->data;
}

void RequestHandler::serveRequest( const uint32_t requestID, void* result )
{
    Request* request = 0;
    {
        ScopedMutex< SpinLock > mutex( _mutex );
        RequestHash::const_iterator i = _requests.find( requestID );

        if( i != _requests.end( ))
            request = i->second;
    }
    if( request )
    {
        request->result.rPointer = result;
        request->lock.unset();
    }
}

void RequestHandler::serveRequest( const uint32_t requestID, uint32_t result )
{
    Request* request = 0;
    {
        ScopedMutex< SpinLock > mutex( _mutex );
        RequestHash::const_iterator i = _requests.find( requestID );

        if( i != _requests.end( ))
            request = i->second;
    }
    if( request )
    {
        request->result.rUint32 = result;
        request->lock.unset();
    }
}

void RequestHandler::serveRequest( const uint32_t requestID, bool result )
{
    Request* request = 0;
    {
        ScopedMutex< SpinLock > mutex( _mutex );
        RequestHash::const_iterator i = _requests.find( requestID );

        if( i != _requests.end( ))
            request = i->second;
    }
    if( request )
    {
        request->result.rBool = result;
        request->lock.unset();
    }
}

void RequestHandler::serveRequest( const uint32_t requestID, const uint128_t& result )
{
    Request* request = 0;
    {
        ScopedMutex< SpinLock > mutex( _mutex );
        RequestHash::const_iterator i = _requests.find( requestID );

        if( i != _requests.end( ))
            request = i->second;
    }
        
    if( request )
    {

        request->result.rUint128.low = result.low();
        request->result.rUint128.high = result.high();
        request->lock.unset();
    }
}
    
bool RequestHandler::isRequestServed( const uint32_t requestID ) const
{
    ScopedMutex< SpinLock > mutex( _mutex );
    RequestHash::const_iterator i = _requests.find( requestID );
    if( i == _requests.end( ))
        return false;

    Request* request = i->second;
    if( !request->lock.isSet( ))
        return true;

    return false;
}

std::ostream& operator << ( std::ostream& os, const RequestHandler& rh )
{
    ScopedMutex< SpinLock > mutex( rh._mutex );
    for( RequestHandler::RequestHash::const_iterator i = rh._requests.begin();
         i != rh._requests.end(); ++i )
    {
        os << "request " << i->first << " served " << i->second->lock.isSet()
           << std::endl;
    }

    return os;    
}

}
}
