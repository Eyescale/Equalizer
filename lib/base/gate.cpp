
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "gate.h"

#include <errno.h>

using namespace eqBase;
using namespace std;

Gate::Gate( const Thread::Type type )
        : _type( type ),
          _open( false )
{
    switch( _type )
    {
        case Thread::PTHREAD:
        {
            int error = pthread_cond_init( &_pthread.cond, NULL );
            if( error )
            {
                EQERROR << "Error creating pthread condition: " 
                        << strerror( error ) << endl;
                return;
            } 
            
            error = pthread_mutex_init( &_pthread.mutex, NULL );
            if( error )
            {
                EQERROR << "Error creating pthread mutex: " 
                        << strerror( error ) << endl;
                return;
            } 
        } break;

        default: EQUNIMPLEMENTED;
    }
}

Gate::~Gate()
{
    switch( _type )
    {
        case Thread::PTHREAD:
            pthread_cond_destroy( &_pthread.cond );
            pthread_mutex_destroy( &_pthread.mutex );
            return;

        default: EQUNIMPLEMENTED;
    }
}

void Gate::up()
{
    switch( _type )
    {
        case Thread::PTHREAD:
            pthread_mutex_lock( &_pthread.mutex );
            _open = true;
            pthread_cond_broadcast( &_pthread.cond );
            pthread_mutex_unlock( &_pthread.mutex );
            return;

        default:
            EQERROR << "not implemented" << endl;
    }
}

void Gate::down()
{
    switch( _type )
    {
        case Thread::PTHREAD:
            _open = false;
            return;

        default:
            EQERROR << "not implemented" << endl;
    }
}

void Gate::enter()
{
    if( _open )
        return;

    switch( _type )
    {
        case Thread::PTHREAD:
            pthread_mutex_lock( &_pthread.mutex );
            while( !_open )
                pthread_cond_wait( &_pthread.cond, &_pthread.mutex );
            pthread_mutex_unlock( &_pthread.mutex );
            return;

        default:
            EQERROR << "not implemented" << endl;
    }
}
