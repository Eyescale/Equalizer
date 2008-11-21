
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_PERTHREADREF_H
#define EQBASE_PERTHREADREF_H

#include <eq/base/base.h>
#include <eq/base/debug.h>
#include <eq/base/refPtr.h>

namespace eq
{
namespace base
{
    class PerThreadRefPrivate;

    /**
     * Implements a thread-specific storage for RefPtr's.
     * 
     * OPT: using __thread storage where available might be beneficial.
     */
    template<typename T> class PerThreadRef
    {
    public:
        PerThreadRef();
        virtual ~PerThreadRef();

        PerThreadRef<T>& operator = ( RefPtr< T > data );
        PerThreadRef<T>& operator = ( const PerThreadRef<T>& rhs );

        RefPtr< T > get() const;
        T* operator->();
        const T* operator->() const;

        bool operator == ( const PerThreadRef& rhs ) const 
            { return ( get() == rhs.get( )); }
        bool operator == ( const RefPtr< T >& rhs ) const
            { return ( get()==rhs ); }
        bool operator != ( const RefPtr< T >& rhs ) const
            { return ( get()!=rhs ); }

        bool operator ! () const;
        bool isValid() const;

    private:
        PerThreadRefPrivate* _data;
    };


//----------------------------------------------------------------------
// implementation
//----------------------------------------------------------------------

// Crude test if pthread.h was included
#ifdef PTHREAD_MUTEX_INITIALIZER
#  ifndef HAVE_PTHREAD_H
#    define HAVE_PTHREAD_H
#  endif
#endif

// The application has to include pthread.h if it wants to instantiate new
// types, since on Windows the use of pthreads-Win32 library includes might
// create hard to resolve type conflicts with other header files.

#ifdef HAVE_PTHREAD_H

class PerThreadRefPrivate
{
public:
    pthread_key_t key;
};

template< typename T >
PerThreadRef<T>::PerThreadRef() 
        : _data( new PerThreadRefPrivate )
{
    const int error = pthread_key_create( &_data->key, 0 );
    if( error )
    {
        EQERROR << "Can't create thread-specific key: " 
                << strerror( error ) << std::endl;
        EQASSERT( !error );
    }
}

template< typename T >
PerThreadRef<T>::~PerThreadRef()
{
    RefPtr< T > object = get();

    pthread_key_delete( _data->key );
    delete _data;
    _data = 0;

    object.unref();
}

template< typename T >
PerThreadRef<T>& PerThreadRef<T>::operator = ( RefPtr< T > data )
{ 
    data.ref(); // ref new

    RefPtr< T > object = get();
    pthread_setspecific( _data->key, static_cast<const void*>( data.get( )));

    object.unref(); // unref old
    return *this; 
}

template< typename T >
PerThreadRef<T>& PerThreadRef<T>::operator = ( const PerThreadRef<T>& rhs )
{
    RefPtr< T > newObject = rhs.get();
    newObject.ref(); // ref new

    RefPtr< T > object = get();
    pthread_setspecific( _data->key, pthread_getspecific( rhs._data->key ));

    object.unref(); // unref old
    return *this;
}

template< typename T >
RefPtr< T > PerThreadRef<T>::get() const
{
    return static_cast< T* >( pthread_getspecific( _data->key )); 
}

template< typename T >
T* PerThreadRef<T>::operator->() 
{
    EQASSERT( pthread_getspecific( _data->key ));
    return static_cast< T* >( pthread_getspecific( _data->key )); 
}

template< typename T >
const T* PerThreadRef<T>::operator->() const 
{ 
    EQASSERT( pthread_getspecific( _data->key ));
    return static_cast< const T* >( pthread_getspecific( _data->key )); 
}

template< typename T >
bool PerThreadRef<T>::operator ! () const
{
    return pthread_getspecific( _data->key ) == 0;
}

template< typename T >
bool PerThreadRef<T>::isValid() const
{
    return pthread_getspecific( _data->key ) != 0;
}

#endif // HAVE_PTHREAD_H
}

}
#endif //EQBASE_PERTHREADREF_H
