
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_REQUESTHANDLER_H
#define EQBASE_REQUESTHANDLER_H

#include "base.h"
#include "hash.h"
#include "lock.h"
#include "thread.h"

#ifdef __GNUC__              // GCC 3.1 and later
#  include <ext/slist>
namespace Sgi = ::__gnu_cxx; 
#else                        //  other compilers
#  include <slist>
namespace Sgi = std;
#endif

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
         * @return the request identifier.
         */
        uint registerRequest();

        /** 
         * Waits for the completion of a request and retrieves the result.
         * 
         * @param requestID the request identifier.
         * @return the result of the request.
         */
        void* waitRequest(const uint requestID);

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
                    lock = new Lock( type );
                    lock->set();
                }
            ~Request(){ delete lock; }

            Lock* lock;
            void* result;
        };
        
        uint                          _requestID;
        Sgi::hash_map<uint, Request*> _requests;
        Sgi::slist<Request*>          _freeRequests;
    };
}

#endif //EQBASE_REQUESTHANDLER_H
