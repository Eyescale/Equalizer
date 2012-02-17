 
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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
#include <co/base/stdExt.h>
#include <co/base/spinLock.h>
#include <co/base/timedLock.h>

namespace co
{
namespace base
{
//! @cond IGNORE
namespace
{
struct Request
{
    Request() { lock.set(); }
    ~Request(){}

    TimedLock lock;
    void*     data;

    union Result
    {
        void*    rPointer;
        uint32_t rUint32;
        bool     rBool;
        struct
        {
            uint64_t low;
            uint64_t high;
        } rUint128;
    } result;
};
typedef stde::hash_map< uint32_t, Request* > RequestHash;
typedef RequestHash::const_iterator RequestHashCIter;
}

namespace detail
{
class RequestHandler
{
public:
    RequestHandler() : requestID( 1 ) {}

    bool waitRequest( const uint32_t requestID_, Request::Result& result,
                      const uint32_t timeout )
        {
            Request* request = 0;
            {
                ScopedFastWrite mutex( lock );
                RequestHashCIter i = requests.find( requestID_ );
                if( i == requests.end( ))
                    return false;

                request = i->second;
            }

            const bool requestServed = request->lock.set( timeout );
            if( requestServed )
                result = request->result;

            unregisterRequest( requestID_ );
            return requestServed;
        }

    void unregisterRequest( const uint32_t requestID_ )
        {
            ScopedFastWrite mutex( lock );

            RequestHash::iterator i = requests.find( requestID_ );
            if( i == requests.end( ))
                return;

            Request* request = i->second;
            requests.erase( i );
            freeRequests.push_front( request );
        }

    mutable base::SpinLock lock;
    uint32_t requestID;
    RequestHash requests;
    std::list<Request*> freeRequests;
};
}
// @endcond

RequestHandler::RequestHandler()
        : _impl( new detail::RequestHandler )
{}

RequestHandler::~RequestHandler()
{
    while( !_impl->freeRequests.empty( ))
    {
        Request* request = _impl->freeRequests.front();
        _impl->freeRequests.pop_front();
        delete request;
    }
    delete _impl;
}

uint32_t RequestHandler::registerRequest( void* data )
{
    ScopedFastWrite mutex( _impl->lock );

    Request* request;
    if( _impl->freeRequests.empty( ))
        request = new Request;
    else
    {
        request = _impl->freeRequests.front();
        _impl->freeRequests.pop_front();
    }

    request->data = data;
    _impl->requestID = ( _impl->requestID + 1 ) % EQ_MAX_UINT32;
    _impl->requests[ _impl->requestID ] = request;
    return _impl->requestID;
}

void RequestHandler::unregisterRequest( const uint32_t requestID )
{
    _impl->unregisterRequest( requestID );
}

bool RequestHandler::waitRequest( const uint32_t requestID, void*& rPointer,
                                  const uint32_t timeout )
{
    Request::Result result;
    if( !_impl->waitRequest( requestID, result, timeout ))
        return false;

    rPointer = result.rPointer;
    return true;
}
bool RequestHandler::waitRequest( const uint32_t requestID, uint32_t& rUint32,
                                  const uint32_t timeout )
{
    Request::Result result;
    if( !_impl->waitRequest( requestID, result, timeout ))
        return false;

    rUint32 = result.rUint32;
    return true;
}

bool RequestHandler::waitRequest( const uint32_t requestID, uint128_t& rUint128,
                                  const uint32_t timeout )
{
    Request::Result result;
    if( !_impl->waitRequest( requestID, result, timeout ))
        return false;

    rUint128.high() = result.rUint128.high;
    rUint128.low() = result.rUint128.low;
    return true;
}

bool RequestHandler::waitRequest( const uint32_t requestID, bool& rBool,
                                  const uint32_t timeout )
{
    Request::Result result;
    if( !_impl->waitRequest( requestID, result, timeout ))
        return false;

    rBool = result.rBool;
    return true;
}
bool RequestHandler::waitRequest( const uint32_t requestID )
{
    Request::Result result;
    return _impl->waitRequest( requestID, result, EQ_TIMEOUT_INDEFINITE );
}

void* RequestHandler::getRequestData( const uint32_t requestID )
{
    ScopedFastWrite mutex( _impl->lock );
    RequestHashCIter i = _impl->requests.find( requestID );
    if( i == _impl->requests.end( ))
        return 0;

    return i->second->data;
}

void RequestHandler::serveRequest( const uint32_t requestID, void* result )
{
    Request* request = 0;
    {
        ScopedFastWrite mutex( _impl->lock );
        RequestHashCIter i = _impl->requests.find( requestID );

        if( i != _impl->requests.end( ))
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
        ScopedFastWrite mutex( _impl->lock );
        RequestHashCIter i = _impl->requests.find( requestID );

        if( i != _impl->requests.end( ))
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
        ScopedFastWrite mutex( _impl->lock );
        RequestHashCIter i = _impl->requests.find( requestID );

        if( i != _impl->requests.end( ))
            request = i->second;
    }
    if( request )
    {
        request->result.rBool = result;
        request->lock.unset();
    }
}

void RequestHandler::serveRequest( const uint32_t requestID,
                                   const uint128_t& result )
{
    Request* request = 0;
    {
        ScopedFastWrite mutex( _impl->lock );
        RequestHashCIter i = _impl->requests.find( requestID );

        if( i != _impl->requests.end( ))
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
    ScopedFastWrite mutex( _impl->lock );
    RequestHashCIter i = _impl->requests.find( requestID );
    if( i == _impl->requests.end( ))
        return false;

    Request* request = i->second;
    if( !request->lock.isSet( ))
        return true;

    return false;
}

bool RequestHandler::hasPendingRequests() const
{
    return !_impl->requests.empty();
}

std::ostream& operator << ( std::ostream& os, const detail::RequestHandler& rh )
{
    ScopedFastWrite mutex( rh.lock );
    for( RequestHashCIter i = rh.requests.begin(); i != rh.requests.end(); ++i )
    {
        os << "request " << i->first << " served " << i->second->lock.isSet()
           << std::endl;
    }

    return os;    
}

std::ostream& operator << ( std::ostream& os, const RequestHandler& rh )
{
    return os << *rh._impl;
}

}
}
