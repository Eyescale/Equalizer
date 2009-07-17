
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQBASE_PERTHREAD_H
#define EQBASE_PERTHREAD_H

#include <errno.h>
#include <string.h>

#include <eq/base/base.h>
#include <eq/base/executionListener.h>
#include <eq/base/nonCopyable.h>

namespace eq
{
namespace base
{
    class PerThreadPrivate;

    /**
     * Implements thread-specific storage for C++ objects.
     * 
     * The object has to implement notifyPerThreadDelete().
     */
    template<typename T> class PerThread : public ExecutionListener, 
                                           public NonCopyable
    {
    public:
        /** Construct a new per-thread variable. */
        PerThread();
        /** Destruct the per-thread variable. */
        ~PerThread();

        /** Assign an object to the thread-local storage. */        
        PerThread<T>& operator = ( const T* data );
        /** Assign an object from another thread-local storage. */
        PerThread<T>& operator = ( const PerThread<T>& rhs );

        /** @return the held object pointer. */
        T* get();
        /** @return the held object pointer. */
        const T* get() const;
        /** Access the thread-local object. */
        T* operator->();
        /** Access the thread-local object. */
        const T* operator->() const;

        /** @return true if the thread-local variables hold the same object. */
        bool operator == ( const PerThread& rhs ) const 
            { return ( get() == rhs.get( )); }
        /** @return true if the thread-local variable holds the same object. */
        bool operator == ( const T* rhs ) const { return ( get()==rhs ); }
        /** @return true if the thread-local variable holds another object. */
        bool operator != ( const T* rhs ) const { return ( get()!=rhs ); }

    protected:
        virtual void notifyExecutionStopping();

    private:
        PerThreadPrivate* _data;
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

// The application has to include pthread.h if it wants to instantiate new
// types, since on Windows the use of pthreads-Win32 library includes might
// create hard to resolve type conflicts with other header files.

#ifdef HAVE_PTHREAD_H

#include <eq/base/debug.h>
#include <eq/base/thread.h>

namespace eq
{
namespace base
{

class PerThreadPrivate
{
public:
    pthread_key_t key;
};

template< typename T >
PerThread<T>::PerThread() 
        : _data( new PerThreadPrivate )
{
    const int error = pthread_key_create( &_data->key, 0 );
    if( error )
    {
        EQERROR << "Can't create thread-specific key: " 
                << strerror( error ) << std::endl;
        EQASSERT( !error );
    }

    Thread::addListener( this );
}

template< typename T >
PerThread<T>::~PerThread()
{
    Thread::removeListener( this );

    T* object = get();
    if( object )
        object->notifyPerThreadDelete();

    pthread_key_delete( _data->key );
    delete _data;
    _data = 0;
}

template< typename T >
void PerThread<T>::notifyExecutionStopping()
{
    T* object = get();
    pthread_setspecific( _data->key, 0 );

    if( object )
        object->notifyPerThreadDelete();
}

template< typename T >
PerThread<T>& PerThread<T>::operator = ( const T* data )
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
T* PerThread<T>::get()
{
    return static_cast< T* >( pthread_getspecific( _data->key )); 
}
template< typename T >
const T* PerThread<T>::get() const
{
    return static_cast< const T* >( pthread_getspecific( _data->key )); 
}

template< typename T >
T* PerThread<T>::operator->() 
{
    return static_cast< T* >( pthread_getspecific( _data->key )); 
}
template< typename T >
const T* PerThread<T>::operator->() const 
{ 
    return static_cast< const T* >( pthread_getspecific( _data->key )); 
}

}
}
#endif // HAVE_PTHREAD_H

#endif //EQBASE_PERTHREAD_H
