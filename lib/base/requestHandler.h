
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_REQUESTHANDLER_H
#define EQBASE_REQUESTHANDLER_H

#include <eq/base/base.h>
#include <eq/base/hash.h>
#include <eq/base/timedLock.h>

#include <list>

namespace eqBase
{
    class Lock;

    /**
     * A request handler.
     * 
     * Different execution threads can synchronize using a request handler. One
     * thread registers a request. Another thread can serve the request. The
     * original thread can wait for the request to be served and retrieve the
     * result.
     *
     * A note on thread-safety: Unless threadSafe is set in the constructor, the
     * functions registerRequest(), unregisterRequest() and waitRequest() are
     * supposed to be called from one 'waiting' thread, and the functions
     * serveRequest() and deleteRequest() are supposed to be called only from
     * one 'serving' thread.
     */
    class EQ_EXPORT RequestHandler
    {

    public:
        /** 
         * Constructs a new request handler.
         * 
         * @param threadSafe if <code>true</code>, all public functions are
         *                   thread-safe.
         */
        RequestHandler( const bool threadSafe = false );

        /** Destructs the requestHandler. */
        ~RequestHandler();

        /** 
         * Registers a new request.
         * 
         * @param data a pointer to user-specific data for the request, can be 0
         * @return the request identifier.
         */
        uint32_t registerRequest( void* data = 0 );

        /** 
         * Unregisters a request.
         *
         * Note that waitRequest() automatically unregisters the request when it
         * was successful.
         * 
         * @param requestID the request identifier.
         */
        void unregisterRequest( const uint32_t requestID );

        /** 
         * Waits a given time for the completion of a request.
         *
         * The request is unregistered.
         * 
         * @param requestID the request identifier.
         * @param result  the result code of the operation.
         * @param timeout the timeout in milliseconds to wait for the request,
         *                or <code>EQ_TIMEOUT_INDEFINITE</code> to wait
         *                indefinitely.
         * @return true if the request was served, false if not.
         */
        bool waitRequest( const uint32_t requestID, void*& result,
                          const uint32_t timeout = EQ_TIMEOUT_INDEFINITE );
        bool waitRequest( const uint32_t requestID, uint32_t& result,
                          const uint32_t timeout = EQ_TIMEOUT_INDEFINITE );
        bool waitRequest( const uint32_t requestID, bool& result,
                          const uint32_t timeout = EQ_TIMEOUT_INDEFINITE );
        bool waitRequest( const uint32_t requestID );

        /** 
         * Polls for the completion of a request.
         *
         * The request is never unregistered.
         * 
         * @param requestID the request identifier.
         * @return true if the request has been served, false if it is pending.
         */
        bool isServed( const uint32_t requestID ) const;

        /** 
         * Retrieves the user-specific data for a request.
         * 
         * @param requestID the request identifier.
         * @return the user-specific data for the request.
         */
        void* getRequestData( const uint32_t requestID );

        /** 
         * Serve a request.
         * 
         * @param requestID the request identifier.
         * @param result the result of the request.
         */
        void serveRequest( const uint32_t requestID, void* result = 0 );
        void serveRequest( const uint32_t requestID, uint32_t result );
        void serveRequest( const uint32_t requestID, bool result );

		bool isThreadSafe() const { return ( _mutex != 0 ); }
        bool empty()        const { return _requests.empty( ); }

    private:
        Lock*        _mutex;

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
        
        typedef stde::hash_map<uint32_t, Request*> RequestHash;

        uint32_t            _requestID;
#pragma warning(push)
#pragma warning(disable: 4251)
        RequestHash         _requests;
        std::list<Request*> _freeRequests;

        bool _waitRequest( const uint32_t requestID, Request::Result& result,
                           const uint32_t timeout );

        CHECK_THREAD_DECLARE( _thread );
#pragma warning(pop)
    };
}

#endif //EQBASE_REQUESTHANDLER_H
