
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef COBASE_PERTHREAD_H
#define COBASE_PERTHREAD_H

#include <co/base/nonCopyable.h>

#include <cstdio>
#include <string.h>

namespace co
{
namespace base
{
namespace detail { class PerThread; }

    /** Default PerThread destructor deleting the object. @version 1.1.2 */
    template< class T > void perThreadDelete( T* object ) { delete object; }

    /** Empty PerThread destructor. @version 1.1.2 */
    template< class T > void perThreadNoDelete( T* object ) {}

    /**
     * Implements thread-specific storage for C++ objects.
     * 
     * The default destructor function deletes the object on thread exit.
     *
     * To instantiate the template code for this class, applications have to
     * include pthread.h before this file. The pthread.h header is not
     * automatically included to avoid hard to resolve type conflicts with other
     * header files on Windows.
     *
     * @param T the type of data to store in thread-local storage
     * @param D the destructor callback function.
     */
    template< class T, void (*D)( T* ) = perThreadDelete< T > >
    class PerThread : public NonCopyable
    {
    public:
        /** Construct a new per-thread variable. @version 1.0 */
        PerThread();
        /** Destruct the per-thread variable. @version 1.0 */
        ~PerThread();

        /** Assign an object to the thread-local storage. @version 1.0 */ 
        PerThread<T, D>& operator = ( const T* data );
        /** Assign an object from another thread-local storage. @version 1.0 */
        PerThread<T, D>& operator = ( const PerThread<T, D>& rhs );

        /** @return the held object pointer. @version 1.0 */
        T* get();
        /** @return the held object pointer. @version 1.0 */
        const T* get() const;
        /** Access the thread-local object. @version 1.0 */
        T* operator->();
        /** Access the thread-local object. @version 1.0 */
        const T* operator->() const;

        /** @return the held object reference. @version 1.0 */
        T& operator*()
            { EQASSERTINFO( get(), className( this )); return *get(); }
        /** @return the held object reference. @version 1.0 */
        const T& operator*() const
            { EQASSERTINFO( get(), className( this )); return *get(); }

        /**
         * @return true if the thread-local variables hold the same object.
         * @version 1.0
         */
        bool operator == ( const PerThread& rhs ) const 
            { return ( get() == rhs.get( )); }

        /**
         * @return true if the thread-local variable holds the same object.
         * @version 1.0
         */
        bool operator == ( const T* rhs ) const { return ( get()==rhs ); }

        /**
         * @return true if the thread-local variable holds another object.
         * @version 1.0
         */
        bool operator != ( const T* rhs ) const { return ( get()!=rhs ); }

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
        detail::PerThread* const _impl;
    };
}
}

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

#include <co/base/debug.h>
#include <co/base/thread.h>

namespace co
{
namespace base
{
namespace detail
{
class PerThread
{
public:
    pthread_key_t key;
};
}

template< class T, void (*D)( T* ) >
PerThread<T, D>::PerThread() 
        : _impl( new detail::PerThread )
{
    typedef void (*PThreadDtor_t)(void*);
    const int error = pthread_key_create( &_impl->key, (PThreadDtor_t)( D ));
    if( error )
    {
        EQERROR << "Can't create thread-specific key: " 
                << strerror( error ) << std::endl;
        EQASSERT( !error );
    }
}

template< class T, void (*D)( T* ) >
PerThread<T, D>::~PerThread()
{
    T* object = get();
    if( object )
        D( object );

    pthread_key_delete( _impl->key );
    delete _impl;
}

template< class T, void (*D)( T* ) >
PerThread<T, D>& PerThread<T, D>::operator = ( const T* data )
{ 
    pthread_setspecific( _impl->key, static_cast<const void*>( data ));
    return *this; 
}

template< class T, void (*D)( T* ) >
PerThread<T, D>& PerThread<T, D>::operator = ( const PerThread<T, D>& rhs )
{ 
    pthread_setspecific( _impl->key, pthread_getspecific( rhs._impl->key ));
    return *this;
}

template< class T, void (*D)( T* ) >
T* PerThread<T, D>::get()
{
    return static_cast< T* >( pthread_getspecific( _impl->key )); 
}
template< class T, void (*D)( T* ) >
const T* PerThread<T, D>::get() const
{
    return static_cast< const T* >( pthread_getspecific( _impl->key )); 
}

template< class T, void (*D)( T* ) >
T* PerThread<T, D>::operator->() 
{
    return static_cast< T* >( pthread_getspecific( _impl->key )); 
}
template< class T, void (*D)( T* ) >
const T* PerThread<T, D>::operator->() const 
{ 
    return static_cast< const T* >( pthread_getspecific( _impl->key )); 
}

template< class T, void (*D)( T* ) >
bool PerThread<T, D>::operator ! () const
{
    return pthread_getspecific( _impl->key ) == 0;
}

template< class T, void (*D)( T* ) >
bool PerThread<T, D>::isValid() const
{
    return pthread_getspecific( _impl->key ) != 0;
}


}
}
#endif // HAVE_PTHREAD_H
#endif //COBASE_PERTHREAD_H
