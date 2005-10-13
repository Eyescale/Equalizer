
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_REQUESTHANDLER_H
#define EQBASE_REQUESTHANDLER_H

#include "base.h"
#include "hash.h"
#include "thread.h"
#include "timedLock.h"

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
     */
    class RequestHandler 
    {

    public:
        /** 
         * Constructs a new requestHandler of the given type.
         * 
         * @param type the type of threads accessing the requestHandler.
         */
        RequestHandler( const Thread::Type type = Thread::PTHREAD );

        /** Destructs the requestHandler. */
        ~RequestHandler();

        /** 
         * Registers a new request.
         * 
         * @param data a pointer to user-specific data for the request, can be
         *             <code>NULL</code>.
         * @return the request identifier.
         */
        uint registerRequest( void* data=NULL );

        /** 
         * Waits a given time for the completion of a request and retrieves the
         * result. 
         * 
         * @param requestID the request identifier.
         * @param timeout the timeout in milliseconds to wait for the request,
         *                or <code>EQ_TIMEOUT_INDEFINITE</code> to wait
         *                indefinitely.
         * @return the result of the request, or <code>NULL</code> if the
         *         request timed out or the requestID is invalid.
         */
        void* waitRequest( const uint requestID, 
                           const uint timeout = EQ_TIMEOUT_INDEFINITE );

        /** 
         * Retrieves the user-specific data for a request.
         * 
         * @param requestID the request identifier.
         * @return the user-specific data for the request.
         */
        void* getRequestData( const uint requestID );

        /** 
         * Server a request.
         * 
         * @param requestID the request identifier.
         * @param result the result of the request.
         */
        void serveRequest( const uint requestID, void* result );

    private:
        Thread::Type _type;

        struct Request
        {
            Request( Thread::Type type )
                {
                    lock = new TimedLock( type );
                    lock->set();
                }
            ~Request(){ delete lock; }
            
            TimedLock* lock;
            void*      data;
            void*      result;
        };
        
        uint                          _requestID;
        Sgi::hash_map<uint, Request*> _requests;
        std::list<Request*>           _freeRequests;
    };
}

#endif //EQBASE_REQUESTHANDLER_H
