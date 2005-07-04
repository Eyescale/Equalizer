
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "lock.h"

#include <errno.h>

using namespace eqBase;
using namespace std;

Lock::Lock( const Thread::Type type )
        : _type( type )
{
    switch( _type )
    {
        case Thread::PTHREAD:
        {
            int nTries = 10;
            while( nTries-- )
            {
                const int error = pthread_mutex_init( &_lock.pthread, NULL );
                switch( error )
                {
                    case 0: // ok
                        return;
                    case EAGAIN:
                        break;
                    default:
                        ERROR << "Error creating pthread mutex: " 
                              << strerror( error ) << endl;
                        return;
                }
            } 
            ERROR << "Error creating pthread mutex"  << endl;
        } return;

        default:
            ERROR << "not implemented" << endl;
    }
}

Lock::~Lock()
{
    switch( _type )
    {
        case Thread::PTHREAD:
            pthread_mutex_destroy( &_lock.pthread );
            return;

        default:
            ERROR << "not implemented" << endl;
    }
}

void Lock::set()
{
    switch( _type )
    {
        case Thread::PTHREAD:
            pthread_mutex_lock( &_lock.pthread );
            return;

        default:
            ERROR << "not implemented" << endl;
    }
}


void Lock::unset()
{
    switch( _type )
    {
        case Thread::PTHREAD:
            pthread_mutex_unlock( &_lock.pthread );
            return;

        default:
            ERROR << "not implemented" << endl;
    }
}


bool Lock::trySet()
{
    switch( _type )
    {
        case Thread::PTHREAD:
            return ( pthread_mutex_trylock( &_lock.pthread ) == 0 );

        default:
            ERROR << "not implemented" << endl;
            return false;
    }
}


bool Lock::test()
{
    switch( _type )
    {
        case Thread::PTHREAD:
            if( pthread_mutex_trylock( &_lock.pthread ) == 0 )
            {
                pthread_mutex_unlock( &_lock.pthread );
                return false;
            }
            return true;

        default:
            ERROR << "not implemented" << endl;
            return false;
    }
}


