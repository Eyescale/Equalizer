
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "lock.h"

#include "log.h"

#include <errno.h>

using namespace eqBase;
using namespace std;

Lock::Lock()
{
    const int error = pthread_mutex_init( &_mutex, 0 );
    if( error )
    {
        EQERROR << "Error creating pthread mutex: " << strerror(error) << endl;
        return;
    }
    EQINFO << "New Lock @" << (void*)this << endl;
}

Lock::~Lock()
{
    pthread_mutex_destroy( &_mutex );
}

void Lock::set()
{
    pthread_mutex_lock( &_mutex );
}


void Lock::unset()
{
    pthread_mutex_unlock( &_mutex );
}


bool Lock::trySet()
{
    return ( pthread_mutex_trylock( &_mutex ) == 0 );
}


bool Lock::test()
{
    if( pthread_mutex_trylock( &_mutex ) == 0 )
    {
        pthread_mutex_unlock( &_mutex );
        return false;
    }
    return true;
}


