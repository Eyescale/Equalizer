
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

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

    /** Thread-specific storage for a RefPtr. */
    template< typename T > class PerThreadRef : public NonCopyable
    {
    public:
        /** Construct a new per-thread reference pointer. */
        PerThreadRef();
        /** Destruct a per-thread reference pointer. */
        ~PerThreadRef();

        /** Assign a reference pointer to the thread-local storage. */
        PerThreadRef<T>& operator = ( RefPtr< T > data );
        /** Assign a reference pointer to the thread-local storage. */
        PerThreadRef<T>& operator = ( const PerThreadRef<T>& rhs );

        /** @return the reference pointer from the thread-local storage. */
        RefPtr< const T > get() const;
        /** @return the reference pointer from the thread-local storage. */
        RefPtr< T > get();

        /** @return the C pointer of the RefPtr from the thread-local storage.*/
        T* getPointer();
        /** Access the object held by the RefPtr in the thread-local storage.*/
        T* operator->();
        /** Access the object held by the RefPtr in the thread-local storage.*/
        const T* operator->() const;

        /** @return true if the two objects hold the same C pointer. */
        bool operator == ( const PerThreadRef& rhs ) const 
            { return ( get() == rhs.get( )); }
        /** @return true if the two objects hold the same C pointer. */
        bool operator == ( const RefPtr< T >& rhs ) const
            { return ( get()==rhs ); }
        /** @return false if the two objects hold the same C pointer. */
        bool operator != ( const RefPtr< T >& rhs ) const
            { return ( get()!=rhs ); }

        /** @return true if the thread-local storage holds a 0 pointer. */
        bool operator ! () const;

        /** @return true if the thread-local storage holds a non-0 pointer. */
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
RefPtr< const T > PerThreadRef<T>::get() const
{
    return static_cast< const T* >( pthread_getspecific( _data->key )); 
}

template< typename T >
RefPtr< T > PerThreadRef<T>::get()
{
    return static_cast< T* >( pthread_getspecific( _data->key )); 
}

template< typename T >
T* PerThreadRef<T>::getPointer()
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
