
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "mtQueue.h"

using namespace eqBase;
using namespace std;

MTQueue::MTQueue( const Thread::Type type )
{
    switch( type )
    {
        case Thread::PTHREAD:
        {
            // mutex init
            int error = pthread_mutex_init( &_sync.pthread.mutex, NULL );
            if( error )
            {
                ERROR << "Error creating pthread mutex: " 
                      << strerror( error ) << endl;
                return;
            }
            // condvar init
            error = pthread_cond_init( &_sync.pthread.cond, NULL );
            if( error )
            {
                ERROR << "Error creating pthread condition: " 
                      << strerror( error ) << endl;
                return;
            }
            break;
        }
        default:
            ASSERT( "not implemented" == NULL );
    }
}

MTQueue::~MTQueue()
{
    switch( _type )
    {
        case Thread::PTHREAD:
            pthread_mutex_destroy( &_sync.pthread.mutex );
            pthread_cond_destroy( &_sync.pthread.cond );
            return;

        default:
            ERROR << "not implemented" << endl;
    }
}
