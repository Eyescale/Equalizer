
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQBASE_REQUESTHANDLER_H
#define EQBASE_REQUESTHANDLER_H

#include <eq/base/base.h>      // EQ_EXPORT definition
#include <eq/base/hash.h>      // member
#include <eq/base/thread.h>    // thread-safety macros
#include <eq/base/timedLock.h> // member

#include <list>

namespace eq
{
namespace base
{
    class Lock;

    /**
     * A request handler.
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
        /** 
         * Construct a new request handler.
         * 
         * @param threadSafe if <code>true</code>, all public functions are
         *                   thread-safe and can be called from multiple
         *                   threads.
         * @version 1.0
         */
        EQ_EXPORT RequestHandler( const bool threadSafe = false );

        /** Destruct the request handler. */
        EQ_EXPORT ~RequestHandler();

        /** 
         * Register a new request.
         * 
         * @param data a pointer to user-specific data for the request, can be
         *             0.
         * @return the request identifier.
         * @version 1.0
         */
        EQ_EXPORT uint32_t registerRequest( void* data = 0 );

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
        EQ_EXPORT void unregisterRequest( const uint32_t requestID );

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
        EQ_EXPORT bool waitRequest( const uint32_t requestID, void*& result,
                               const uint32_t timeout = EQ_TIMEOUT_INDEFINITE );

        /** Wait for a request with a uint32_t result. @version 1.0 */
        EQ_EXPORT bool waitRequest( const uint32_t requestID, uint32_t& result,
                               const uint32_t timeout = EQ_TIMEOUT_INDEFINITE );
        /** Wait for a request with a bool result. @version 1.0 */
        EQ_EXPORT bool waitRequest( const uint32_t requestID, bool& result,
                               const uint32_t timeout = EQ_TIMEOUT_INDEFINITE );
        /** Wait for a request without a result. @version 1.0 */
        EQ_EXPORT bool waitRequest( const uint32_t requestID );

        /** 
         * Poll for the completion of a request.
         *
         * Does not unregister the request.
         * 
         * @param requestID the request identifier.
         * @return true if the request has been served, false if it is pending.
         * @version 1.0
         */
        EQ_EXPORT bool isRequestServed( const uint32_t requestID ) const;

        /** 
         * Retrieve the user-specific data for a request.
         * 
         * @param requestID the request identifier.
         * @return the user-specific data for the request.
         * @version 1.0
         */
        EQ_EXPORT void* getRequestData( const uint32_t requestID );

        /** 
         * Serve a request with a void* result.
         * 
         * @param requestID the request identifier.
         * @param result the result of the request.
         * @version 1.0
         */
        EQ_EXPORT void serveRequest( const uint32_t requestID, void* result=0 );
        /** Serve a request with a uint32_t result. @version 1.0 */
        EQ_EXPORT void serveRequest( const uint32_t requestID, uint32_t result);
        /** Serve a request with a bool result. @version 1.0 */
        EQ_EXPORT void serveRequest( const uint32_t requestID, bool result );

        /**
         * @return true if this request handler has pending requests.
         * @version 1.0
         */
        bool hasPendingRequests() const { return !_requests.empty( ); }

    private:
        Lock*        _mutex;

        //! @cond IGNORE
        struct Request
        {
            Request()
                {
                    lock.set();
                }
            ~Request(){}
            
            TimedLock lock;
            void*     data;

            union Result
            {
                void*    rPointer;
                uint32_t rUint32;
                bool     rBool;
            } result;
        };
        // @endcond

        typedef stde::hash_map< uint32_t, Request* > RequestHash;

        uint32_t            _requestID;
        RequestHash         _requests;
        std::list<Request*> _freeRequests;

        bool _waitRequest( const uint32_t requestID, Request::Result& result,
                           const uint32_t timeout );

        CHECK_THREAD_DECLARE( _thread );
    };
}

}
#endif //EQBASE_REQUESTHANDLER_H
