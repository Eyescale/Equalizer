
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "sema.h"

#include <eq/base/log.h>

#include <errno.h>

using namespace eqBase;
using namespace std;

Sema::Sema()
        : _value( 0 )
{
    int error = pthread_cond_init( &_cond, NULL );
    if( error )
    {
        EQERROR << "Error creating pthread condition: " << strerror( error )
                << endl;
        return;
    } 
            
    error = pthread_mutex_init( &_mutex, NULL );
    if( error )
    {
        EQERROR << "Error creating pthread mutex: " << strerror(error) << endl;
        return;
    } 
}

Sema::~Sema()
{
    pthread_cond_destroy( &_cond );
    pthread_mutex_destroy( &_mutex );
}

void Sema::post()
{
    pthread_mutex_lock( &_mutex );
    ++_value;
    pthread_cond_signal( &_cond );
    pthread_mutex_unlock( &_mutex );
}

void Sema::wait()
{
    pthread_mutex_lock( &_mutex );
    while( _value == 0 )
        pthread_cond_wait( &_cond, &_mutex );

    --_value;
    pthread_mutex_unlock( &_mutex );
}

void Sema::adjust( const int delta )
{
    if( delta == 0 )
        return;

    pthread_mutex_lock( &_mutex );
    
    if( delta > 0 )
    {
        _value += delta;
        pthread_cond_broadcast( &_cond );
        pthread_mutex_unlock( &_mutex );
        return;
    };

    uint32_t amount = (uint32_t)-delta;
    while( amount )
    {
        while( _value == 0 )
            pthread_cond_wait( &_cond, &_mutex );
                
        if( _value < amount )
        {
            amount -= _value;
            _value = 0;
        }
        else
        {
            _value -= amount;
            amount = 0;
        }
    }
    pthread_mutex_unlock( &_mutex );
}

