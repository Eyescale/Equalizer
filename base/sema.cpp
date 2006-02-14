
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "sema.h"

#include <errno.h>

using namespace eqBase;
using namespace std;

Sema::Sema( const Thread::Type type )
        : _type( type ),
          _value( 0 )
{
    switch( _type )
    {
        case Thread::PTHREAD:
        {
            int nTries = 10;
            while( nTries-- )
            {
                const int error = pthread_cond_init( &_pthread.cond, NULL );
                switch( error )
                {
                    case 0: // ok
                        return;
                    case EAGAIN:
                        break;
                    default:
                        EQERROR << "Error creating pthread condition: " 
                              << strerror( error ) << endl;
                        return;
                }
            } 
            
            nTries = 10;
            while( nTries-- )
            {
                const int error = pthread_mutex_init( &_pthread.mutex, NULL );
                switch( error )
                {
                    case 0: // ok
                        return;
                    case EAGAIN:
                        break;
                    default:
                        EQERROR << "Error creating pthread mutex: " 
                              << strerror( error ) << endl;
                        return;
                }
            } 
            EQERROR << "Error creating pthread mutex"  << endl;
        } return;

        default: EQUNIMPLEMENTED;
    }
}

Sema::~Sema()
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

void Sema::post()
{
    switch( _type )
    {
        case Thread::PTHREAD:
            pthread_mutex_lock( &_pthread.mutex );
            ++_value;
            pthread_cond_signal( &_pthread.cond );
            pthread_mutex_unlock( &_pthread.mutex );
            return;

        default:
            EQERROR << "not implemented" << endl;
    }
}

void Sema::wait()
{
    switch( _type )
    {
        case Thread::PTHREAD:
            pthread_mutex_lock( &_pthread.mutex );
            while( _value == 0 )
                pthread_cond_wait( &_pthread.cond, &_pthread.mutex );

            --_value;
            pthread_mutex_unlock( &_pthread.mutex );
            return;

        default:
            EQERROR << "not implemented" << endl;
    }
}

void Sema::adjust( int delta )
{
    if( delta == 0 )
        return;

    switch( _type )
    {
        case Thread::PTHREAD:
            pthread_mutex_lock( &_pthread.mutex );

            if( delta > 0 )
            {
                _value += delta;
                pthread_cond_broadcast( &_pthread.cond );
                pthread_mutex_unlock( &_pthread.mutex );
                return;
            };

            while( delta )
            {
                while( _value == 0 )
                    pthread_cond_wait( &_pthread.cond, &_pthread.mutex );
                
                if( _value < delta )
                {
                    delta -= _value;
                    _value = 0;
                }
                else
                {
                    _value -= delta;
                    delta = 0;
                }
            }
            pthread_mutex_unlock( &_pthread.mutex );
            return;

        default:
            EQERROR << "not implemented" << endl;
    }
}

