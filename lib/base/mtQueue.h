
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQBASE_MTQUEUE_H
#define EQBASE_MTQUEUE_H

#include <eq/base/base.h>
#include <eq/base/debug.h>

#include <queue>
#include <string.h>
#include <sys/timeb.h>

namespace eq
{
namespace base
{
    class MTQueuePrivate;

    /**
     * A thread-safe queue with a blocking read access.
     *
     * Typically used to communicate between two execution threads.
     *
     * To instantiate the template code for this class, applications have to
     * include pthread.h before this file. pthread.h is not automatically
     * included to avoid hard to resolve type conflicts with other header files
     * on Windows.
     */
    template< typename T > class MTQueue
    {
    public:
        /** Construct a new queue. @version 1.0 */
        MTQueue();

        /** Construct a copy of a queue. @version 1.0 */
        MTQueue( const MTQueue< T >& from );

        /** Destruct this Queue. @version 1.0 */
        ~MTQueue();

        /** Assign the values of another queue. @version 1.0 */
        MTQueue< T >& operator = ( const MTQueue< T >& from ); 

        /** @return true if the queue is empty, false otherwise. @version 1.0 */
        bool isEmpty() const { return _queue.empty(); }

        /** @return the number of items currently in the queue. @version 1.0 */
        size_t getSize() const { return _queue.size(); }

        /** Reset (empty) the queue. @version 1.0 */
        void clear();

        /** 
         * Retrieve and pop the front element from the queue, may block.
         * @version 1.0
         */
        T pop();

        /** 
         * Retrieve and pop the front element from the queue, may block.
         *
         * @param result the front value or unmodified.
         * @return true if an element was placed in result, false if the queue
         *         is empty.
         * @version 1.0
         */
        bool tryPop( T& result );

        /** 
         * @param result the front value or unmodified.
         * @return true if an element was placed in result, false if the queue
         *         is empty.
         * @version 1.0
         */
        bool getFront( T& result ) const;

        /** 
         * @param result the lasr value or unmodified.
         * @return true if an element was placed in result, false if the queue
         *         is empty.
         * @version 1.0
         */
        bool getBack( T& result ) const;

        /** Push a new element to the back of the queue. @version 1.0 */
        void push( const T& element );

        /** Push a vector of elements to the back of the queue. @version 1.0 */
        void push( const std::vector< T >& elements );

        /** Push a new element to the front of the queue. @version 1.0 */
        void pushFront( const T& element );

    private:
        std::deque< T > _queue;
        MTQueuePrivate* _data;

        void _init();
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

class MTQueuePrivate
{
public:
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
};

template< typename T >
MTQueue<T>::MTQueue()
        : _data( new MTQueuePrivate )
{
    _init();
}

template< typename T >
MTQueue< T >::MTQueue( const MTQueue< T >& from )
        : _queue( from._queue )
        , _data( new MTQueuePrivate )
{
    _init();
}

template< typename T >
void MTQueue< T >::_init()
{
    // mutex init
    int error = pthread_mutex_init( &_data->mutex, 0 );
    if( error )
    {
        EQERROR << "Error creating pthread mutex: " 
                << strerror( error ) << std::endl;
        return;
    }
    // condvar init
    error = pthread_cond_init( &_data->cond, 0 );
    if( error )
    {
        EQERROR << "Error creating pthread condition: " 
                << strerror( error ) << std::endl;
        return;
    }
}

template< typename T >
MTQueue< T >& MTQueue< T >::operator = ( const MTQueue< T >& from )
{
    pthread_mutex_lock( &_data->mutex );
    _queue = from._queue;
    pthread_cond_signal( &_data->cond );
    pthread_mutex_unlock( &_data->mutex );
    return *this;
}


template< typename T >
MTQueue<T>::~MTQueue()
{
    pthread_mutex_destroy( &_data->mutex );
    pthread_cond_destroy( &_data->cond );
    delete _data;
    _data = 0;
}

template< typename T >
void MTQueue<T>::clear()
{
    pthread_mutex_lock( &_data->mutex );
    _queue.clear();
    pthread_mutex_unlock( &_data->mutex );
}

template< typename T >
T MTQueue<T>::pop()
{
    pthread_mutex_lock( &_data->mutex );
    while( _queue.empty( ))
        pthread_cond_wait( &_data->cond, &_data->mutex );
                
    EQASSERT( !_queue.empty( ));
    T element = _queue.front();
    _queue.pop_front();
    pthread_mutex_unlock( &_data->mutex );
    return element;
}

template< typename T >
bool MTQueue<T>::tryPop( T& result )
{
    if( _queue.empty( ))
        return false;
    
    pthread_mutex_lock( &_data->mutex );
    if( _queue.empty( ))
    {
        pthread_mutex_unlock( &_data->mutex );
        return false;
    }
    
    result = _queue.front();
    _queue.pop_front();
    pthread_mutex_unlock( &_data->mutex );
    return true;
}   

template< typename T >
bool MTQueue<T>::getFront( T& result ) const
{
    pthread_mutex_lock( &_data->mutex );
    if( _queue.empty( ))
    {
        pthread_mutex_unlock( &_data->mutex );
        return false;
    }
    // else
    result = _queue.front();
    pthread_mutex_unlock( &_data->mutex );
    return true;
}

template< typename T >
bool MTQueue<T>::getBack( T& result ) const
{
    pthread_mutex_lock( &_data->mutex );
    if( _queue.empty( ))
    {
        pthread_mutex_unlock( &_data->mutex );
        return false;
    }
    // else
    result = _queue.back();
    pthread_mutex_unlock( &_data->mutex );
    return true;
}

template< typename T >
void MTQueue<T>::push( const T& element )
{
    pthread_mutex_lock( &_data->mutex );
    _queue.push_back( element );
    pthread_cond_signal( &_data->cond );
    pthread_mutex_unlock( &_data->mutex );
}

template< typename T >
void MTQueue<T>::push( const std::vector< T >& elements )
{
    pthread_mutex_lock( &_data->mutex );
    _queue.insert( _queue.end(), elements.begin(), elements.end( ));
    pthread_cond_signal( &_data->cond );
    pthread_mutex_unlock( &_data->mutex );
}

template< typename T >
void MTQueue<T>::pushFront( const T& element )
{
    pthread_mutex_lock( &_data->mutex );
    _queue.push_front( element );
    pthread_cond_signal( &_data->cond );
    pthread_mutex_unlock( &_data->mutex );
}
#endif //HAVE_PTHREAD_H
}

}
#endif //EQBASE_MTQUEUE_H
