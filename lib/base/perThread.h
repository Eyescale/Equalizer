
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
     *
     * To instantiate the template code for this class, applications have to
     * include pthread.h before this file. pthread.h is not automatically
     * included to avoid hard to resolve type conflicts with other header files
     * on Windows.
     */
    template<typename T> class PerThread : public ExecutionListener, 
                                           public NonCopyable
    {
    public:
        /** Construct a new per-thread variable. @version 1.0 */
        PerThread();
        /** Destruct the per-thread variable. @version 1.0 */
        ~PerThread();

        /** Assign an object to the thread-local storage. @version 1.0 */ 
        PerThread<T>& operator = ( const T* data );
        /** Assign an object from another thread-local storage. @version 1.0 */
        PerThread<T>& operator = ( const PerThread<T>& rhs );

        /** @return the held object pointer. @version 1.0 */
        T* get();
        /** @return the held object pointer. @version 1.0 */
        const T* get() const;
        /** Access the thread-local object. @version 1.0 */
        T* operator->();
        /** Access the thread-local object. @version 1.0 */
        const T* operator->() const;

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

template< typename T >
bool PerThread<T>::operator ! () const
{
    return pthread_getspecific( _data->key ) == 0;
}

template< typename T >
bool PerThread<T>::isValid() const
{
    return pthread_getspecific( _data->key ) != 0;
}


}
}
#endif // HAVE_PTHREAD_H

#endif //EQBASE_PERTHREAD_H
