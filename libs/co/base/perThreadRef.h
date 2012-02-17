
/* Copyright (c) 2008-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef COBASE_PERTHREADREF_H
#define COBASE_PERTHREADREF_H

#include <co/base/debug.h>
#include <co/base/refPtr.h>

namespace co
{
namespace base
{
namespace detail { class PerThreadRef; }

    /** 
     * Thread-specific storage for a RefPtr.
     * 
     * To instantiate the template code for this class, applications have to
     * include pthread.h before this file. pthread.h is not automatically
     * included to avoid hard to resolve type conflicts with other header files
     * on Windows.
     */
    template< typename T > class PerThreadRef : public NonCopyable
    {
    public:
        /** Construct a new per-thread RefPtr. @version 1.0 */
        PerThreadRef();
        /** Destruct a per-thread RefPtr. @version 1.0 */
        ~PerThreadRef();

        /** Assign a RefPtr to the thread-local storage. @version 1.0 */
        PerThreadRef<T>& operator = ( RefPtr< T > data );

        /** Assign a RefPtr to the thread-local storage. @version 1.0 */
        PerThreadRef<T>& operator = ( const PerThreadRef<T>& rhs );

        /** @return the RefPtr from the thread-local storage. @version 1.0 */
        RefPtr< const T > get() const;
        /** @return the RefPtr from the thread-local storage. @version 1.0 */
        RefPtr< T > get();

        /**
         * @return the C pointer of the RefPtr from the thread-local storage.
         * @version 1.0
         */
        T* getPointer();

        /**
         * @return the object held by the RefPtr in the thread-local storage.
         * @version 1.0
         */
        T* operator->();

        /**
         * @return the object held by the RefPtr in the thread-local storage.
         * @version 1.0
         */
        const T* operator->() const;

        /**
         * @return true if the two objects hold the same C pointer.
         * @version 1.0
         */
        bool operator == ( const PerThreadRef& rhs ) const 
            { return ( get() == rhs.get( )); }

        /**
         * @return true if the two objects hold the same C pointer.
         * @version 1.0
         */
        bool operator == ( const RefPtr< T >& rhs ) const
            { return ( get()==rhs ); }

        /**
         * @return true if the two objects hold the same C pointer.
         * @version 1.0
         */
        bool operator != ( const RefPtr< T >& rhs ) const
            { return ( get()!=rhs ); }

        /**
         * @return true if the thread-local storage holds a 0 pointer.
         * @version 1.0
         */
        bool operator ! () const;

        /**
         * @return true if the thread-local storage holds a non-0 pointer.
         * @version 1.0
         */
        bool isValid() const;

    private:
        PerThreadRef* const _impl;
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

#ifdef HAVE_PTHREAD_H
namespace detail
{
class PerThreadRef
{
public:
    pthread_key_t key;
};
}

template< typename T >
PerThreadRef<T>::PerThreadRef() 
        : _impl( new detail::PerThreadRef )
{
    const int error = pthread_key_create( &_impl->key, 0 );
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

    pthread_key_delete( _impl->key );
    delete _impl;

    object.unref();
}

template< typename T >
PerThreadRef<T>& PerThreadRef<T>::operator = ( RefPtr< T > data )
{ 
    data.ref(); // ref new

    RefPtr< T > object = get();
    pthread_setspecific( _impl->key, static_cast<const void*>( data.get( )));

    object.unref(); // unref old
    return *this; 
}

template< typename T >
PerThreadRef<T>& PerThreadRef<T>::operator = ( const PerThreadRef<T>& rhs )
{
    RefPtr< T > newObject = rhs.get();
    newObject.ref(); // ref new

    RefPtr< T > object = get();
    pthread_setspecific( _impl->key, pthread_getspecific( rhs._impl->key ));

    object.unref(); // unref old
    return *this;
}

template< typename T >
RefPtr< const T > PerThreadRef<T>::get() const
{
    return static_cast< const T* >( pthread_getspecific( _impl->key )); 
}

template< typename T >
RefPtr< T > PerThreadRef<T>::get()
{
    return static_cast< T* >( pthread_getspecific( _impl->key )); 
}

template< typename T >
T* PerThreadRef<T>::getPointer()
{
    return static_cast< T* >( pthread_getspecific( _impl->key )); 
}

template< typename T >
T* PerThreadRef<T>::operator->() 
{
    EQASSERT( pthread_getspecific( _impl->key ));
    return static_cast< T* >( pthread_getspecific( _impl->key )); 
}

template< typename T >
const T* PerThreadRef<T>::operator->() const 
{ 
    EQASSERT( pthread_getspecific( _impl->key ));
    return static_cast< const T* >( pthread_getspecific( _impl->key )); 
}

template< typename T >
bool PerThreadRef<T>::operator ! () const
{
    return pthread_getspecific( _impl->key ) == 0;
}

template< typename T >
bool PerThreadRef<T>::isValid() const
{
    return pthread_getspecific( _impl->key ) != 0;
}
#endif // HAVE_PTHREAD_H

}
}
#endif //COBASE_PERTHREADREF_H
