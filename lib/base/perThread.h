
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_PERTHREAD_H
#define EQBASE_PERTHREAD_H

#include "base.h"

namespace eqBase
{
    /**
     * Implements a thread-specific storage.
     * 
     * The size of the stored data has not to exceed sizeof(void*).
     */
    template<typename T> class PerThread
    {
    public:
        PerThread() 
            {
                EQASSERTINFO( sizeof(T) <= sizeof(void*), 
                              "Data too large for thread-specific storage" );

                const int error = pthread_key_create( &_key, NULL );
                if( error )
                {
                    EQERROR << "Can't create thread-specific key: " 
                            << strerror( error ) << std::endl;
                    EQASSERT( !error );
                }
            }

        PerThread<T>& operator = ( const T data )
            { 
                pthread_setspecific( _key, (const void*)data );
                return *this; 
            }

        PerThread<T>& operator = ( const PerThread<T>& rhs )
            { 
                pthread_setspecific( _key, pthread_getspecific( rhs._key ));
                return *this;
            }

        virtual ~PerThread()
            {
                pthread_key_delete( _key );
            }

        T get() const { return (T)pthread_getspecific( _key ); }

    private:
        pthread_key_t _key;
    };
}

#endif //EQBASE_PERTHREAD_H
