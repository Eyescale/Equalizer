
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

#ifndef COBASE_REQUESTHANDLER_H
#define COBASE_REQUESTHANDLER_H

#include <co/base/api.h>       // COBASE_API definition
#include <co/base/thread.h>    // thread-safety macros
#include <co/base/types.h>

#include <list>

namespace co
{
namespace base
{
namespace detail { class RequestHandler; }

    /**
     * A thread-safe request handler.
     * 
     * Different execution threads can synchronize using a request handler. One
     * thread registers a request, and later waits for the request to be
     * served. Another thread can serve the request, providing a result value.
     *
     * A note on thread-safety: Unless threadSafe is set in the constructor, the
     * functions registerRequest(), unregisterRequest() and waitRequest() are
     * supposed to be called from one 'waiting' thread, and the functions
     * serveRequest() and deleteRequest() are supposed to be called only from
     * one 'serving' thread.
     */
    class RequestHandler : public NonCopyable
    {
    public:
        /** Construct a new request handler.  @version 1.0 */
        COBASE_API RequestHandler();

        /** Destruct the request handler. */
        COBASE_API ~RequestHandler();

        /** 
         * Register a new request.
         * 
         * @param data a pointer to user-specific data for the request, can be
         *             0.
         * @return the request identifier.
         * @version 1.0
         */
        COBASE_API uint32_t registerRequest( void* data = 0 );

        /** 
         * Unregister a request.
         *
         * Note that waitRequest automatically unregisters the request when it
         * was successful. This method is only used when a waitRequest has timed
         * out and the request will no longer be used.
         * 
         * @param requestID the request identifier.
         * @version 1.0
         */
        COBASE_API void unregisterRequest( const uint32_t requestID );

        /**
         * Wait a given time for the completion of a request.
         *
         * The request is unregistered upon successful completion, i.e, the
         * when the method returns true.
         * 
         * @param requestID the request identifier.
         * @param result  the result code of the operation.
         * @param timeout the timeout in milliseconds to wait for the request,
         *                or <code>EQ_TIMEOUT_INDEFINITE</code> to wait
         *                indefinitely.
         * @return true if the request was served, false if not.
         * @version 1.0
         */
        COBASE_API bool waitRequest( const uint32_t requestID, void*& result,
                               const uint32_t timeout = EQ_TIMEOUT_INDEFINITE );

        /** Wait for a request with an uint32_t result. @version 1.0 */
        COBASE_API bool waitRequest( const uint32_t requestID, uint32_t& result,
                               const uint32_t timeout = EQ_TIMEOUT_INDEFINITE );
        /** Wait for a request with a bool result. @version 1.0 */
        COBASE_API bool waitRequest( const uint32_t requestID, bool& result,
                               const uint32_t timeout = EQ_TIMEOUT_INDEFINITE );
        /** Wait for a request with an uint128_t result. @version 1.0 */
        COBASE_API bool waitRequest(const uint32_t requestID, uint128_t& result,
                               const uint32_t timeout = EQ_TIMEOUT_INDEFINITE );
        /** Wait for a request without a result. @version 1.0 */
        COBASE_API bool waitRequest( const uint32_t requestID );

        /** 
         * Poll for the completion of a request.
         *
         * Does not unregister the request.
         * 
         * @param requestID the request identifier.
         * @return true if the request has been served, false if it is pending.
         * @version 1.0
         */
        COBASE_API bool isRequestServed( const uint32_t requestID ) const;

        /** 
         * Retrieve the user-specific data for a request.
         * 
         * @param requestID the request identifier.
         * @return the user-specific data for the request.
         * @version 1.0
         */
        COBASE_API void* getRequestData( const uint32_t requestID );

        /** 
         * Serve a request with a void* result.
         * 
         * @param requestID the request identifier.
         * @param result the result of the request.
         * @version 1.0
         */
        COBASE_API void serveRequest( const uint32_t requestID,
                                        void* result = 0 );
        /** Serve a request with an uint32_t result. @version 1.0 */
        COBASE_API void serveRequest( const uint32_t requestID,
                                        uint32_t result );
        /** Serve a request with a bool result. @version 1.0 */
        COBASE_API void serveRequest( const uint32_t requestID, bool result );
        /** Serve a request with an uint128_t result. @version 1.0 */
        COBASE_API void serveRequest( const uint32_t requestID,
                                      const uint128_t& result );
        /**
         * @return true if this request handler has pending requests.
         * @version 1.0
         */
        COBASE_API bool hasPendingRequests() const;

    private:
        detail::RequestHandler* const _impl;
        friend COBASE_API std::ostream& operator << ( std::ostream&,
                                                     const RequestHandler& );
        EQ_TS_VAR( _thread );
    };

    COBASE_API std::ostream& operator << ( std::ostream&,
                                           const RequestHandler& );
}

}
#endif //COBASE_REQUESTHANDLER_H
