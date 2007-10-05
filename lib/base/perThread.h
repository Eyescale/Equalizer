
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_PERTHREAD_H
#define EQBASE_PERTHREAD_H

#include <eq/base/base.h>
#include <eq/base/debug.h>

namespace eqBase
{
    class PerThreadPrivate;

    /**
     * Implements a thread-specific storage.
     * 
     * The size of the stored data has not to exceed sizeof(void*).
     * OPT: using __thread storage where available might be benefitial.
     */
    template<typename T> class PerThread
    {
    public:
        PerThread();
        virtual ~PerThread();

        PerThread<T>& operator = ( const T& data );
        PerThread<T>& operator = ( const PerThread<T>& rhs );

        T get() const;
        T operator->();
        const T operator->() const;

        bool operator == ( const PerThread& rhs ) const 
            { return ( get() == rhs.get( )); }
        bool operator == ( const T& rhs ) const { return ( get()==rhs ); }
        bool operator != ( const T& rhs ) const { return ( get()!=rhs ); }

    private:
        PerThreadPrivate* _data;
    };


//----------------------------------------------------------------------
// implementation
//----------------------------------------------------------------------

// Crude test if pthread.h was included
#ifdef PTHREAD_CREATE_JOINABLE
#  ifndef HAVE_PTHREAD_H
#    define HAVE_PTHREAD_H
#  endif
#endif

// The application has to include pthread.h if it wants to instantiate new queue
// types, since on Windows the use of pthreads-Win32 library includes might
// create hard to resolve type conflicts with other header files.

#ifdef HAVE_PTHREAD_H

class PerThreadPrivate
{
public:
    pthread_key_t key;
};

template< typename T >
PerThread<T>::PerThread() 
        : _data( new PerThreadPrivate )
{
    EQASSERTINFO( sizeof(T) <= sizeof(void*), 
                  "Data too large for thread-specific storage" );

    const int error = pthread_key_create( &_data->key, 0 );
    if( error )
    {
        EQERROR << "Can't create thread-specific key: " 
                << strerror( error ) << std::endl;
        EQASSERT( !error );
    }
}

template< typename T >
PerThread<T>::~PerThread()
{
    pthread_key_delete( _data->key );
    delete _data;
    _data = 0;
}

template< typename T >
PerThread<T>& PerThread<T>::operator = ( const T& data )
{ 
    pthread_setspecific( _data->key, static_cast<const void*>( data ));
    return *this; 
}

template< typename T >
PerThread<T>& PerThread<T>::operator = ( const PerThread<T>& rhs )
{ 
    pthread_setspecific( _data->key, pthread_getspecific( rhs._data->key ));
    return *this;
}

template< typename T >
T PerThread<T>::get() const
{
    return static_cast<T>( pthread_getspecific( _data->key )); 
}

template< typename T >
T PerThread<T>::operator->() 
{
    return static_cast<T>( pthread_getspecific( _data->key )); 
}

template< typename T >
const T PerThread<T>::operator->() const 
{ 
    return const_cast<T>( pthread_getspecific( _data->key )); 
}
#endif // HAVE_PTHREAD_H
}

#endif //EQBASE_PERTHREAD_H
