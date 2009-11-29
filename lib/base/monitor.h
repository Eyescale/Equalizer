
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQBASE_MONITOR_H
#define EQBASE_MONITOR_H

#include <eq/base/nonCopyable.h> // base class

#include <errno.h>
#include <string.h>
#include <iostream>
#include <typeinfo>

#ifdef PTHREAD_MUTEX_INITIALIZER // Crude test if pthread.h was included
#  ifndef HAVE_PTHREAD_H
#    define HAVE_PTHREAD_H
#  endif
#  ifndef EQUALIZER_EXPORTS // pthread.h included, not building Eq (inline impl)
#    define EQ_PT_EXPORT
#  endif
#endif
#ifndef EQ_PT_EXPORT // pthread.h not included or building eq (explicit export)
#  define EQ_PT_EXPORT EQ_EXPORT 
#endif 

namespace eq
{
namespace base
{
    class MonitorPrivate;

    /**
     * A monitor primitive.
     *
     * A monitor has a value, which can be monitored to reach a certain
     * state. The caller is blocked until the condition is fulfilled. The
     * concept is similar to a pthread condition, with more usage convenience.
     *
     * Template instantiations for uint32_t and bool are at the end of
     * monitor.cpp. Monitors for other types can be created by including 
     * pthread.h before this file. pthread.h is not automatically included to
     * avoid hard to resolve type conflicts with other header files on Windows. 
     */
    template< typename T > class Monitor : public NonCopyable
    {
    public:
        /** Constructs a new monitor with a default value of 0. @version 1.0 */
        Monitor() : _value( static_cast<T>( 0 )) { _construct(); }

        /** Constructs a new monitor with a given default value. @version 1.0 */
        Monitor( const T& value ) : _value( value ) { _construct(); }
        
        /** Destructs the monitor. @version 1.0 */
        EQ_PT_EXPORT ~Monitor();

        /** @name Changing the monitored value. */
        //@{
        /** Increment the monitored value, prefix only. @version 1.0 */
        EQ_PT_EXPORT Monitor& operator++ ();
        /** Decrement the monitored value, prefix only. @version 1.0 */
        EQ_PT_EXPORT Monitor& operator-- ();

        /** Assign a new value. @version 1.0 */
        Monitor& operator = ( const T& value )
            {
                set( value );
                return *this;
            }

        /** Set a new value. @version 1.0 */
        EQ_PT_EXPORT void set( const T& value );
        //@}

        /** @name Monitor the value. */
        //@{
        /**
         * Block until the monitor has the given value.
         * @return the value when reaching the condition.
         * @version 1.0
         */
        EQ_PT_EXPORT const T& waitEQ( const T& value ) const;

        /**
         * Block until the monitor has not the given value.
         * @return the value when reaching the condition.
         * @version 1.0
         */
        EQ_PT_EXPORT const T& waitNE( const T& value ) const;

        /**
         * Block until the monitor has a value greater or equal to the given
         * value.
         * @return the value when reaching the condition.
         * @version 1.0
         */
        EQ_PT_EXPORT const T& waitGE( const T& value ) const;

        /**
         * Block until the monitor has a value less or equal to the given
         * value.
         * @return the value when reaching the condition.
         * @version 1.0
         */
        EQ_PT_EXPORT const T& waitLE( const T& value ) const;
        //@}

        /** @name Comparison Operators. @version 1.0 */
        //@{
        bool operator == ( const T& value ) const { return _value == value; }
        bool operator != ( const T& value ) const { return _value != value; }
        bool operator < ( const T& value ) const { return _value < value; }
        bool operator > ( const T& value ) const { return _value > value; }
        bool operator <= ( const T& value ) const { return _value <= value; }
        bool operator >= ( const T& value ) const { return _value >= value; }

        bool operator == ( const Monitor<T>& rhs ) const
            { return _value == rhs._value; }
        bool operator != ( const Monitor<T>& rhs ) const
            { return _value != rhs._value; }
        bool operator < ( const Monitor<T>& rhs ) const
            { return _value < rhs._value; }
        bool operator > ( const Monitor<T>& rhs ) const
            { return _value > rhs._value; }
        bool operator <= ( const Monitor<T>& rhs ) const
            { return _value <= rhs._value; }
        bool operator >= ( const Monitor<T>& rhs ) const
            { return _value >= rhs._value; }
        //@}

        /** @name Data Access. */
        //@{
        /** @return the current value. @version 1.0 */
        const T& operator->() const { return _value; }

        /** @return the current value. @version 1.0 */
        const T& get() const { return _value; }

        /** @return the current plus the given value. @version 1.0 */
        T operator + ( const T& value ) const { return _value + value; }
        //@}

    private:
        T _value;
        MonitorPrivate* _data;

        EQ_PT_EXPORT void _construct();
    };

typedef Monitor< bool >     Monitorb;
typedef Monitor< uint32_t > Monitoru;

/** Print the monitor to the given output stream. @version 1.0 */
template< typename T >
inline std::ostream& operator << ( std::ostream& os, const Monitor<T>& monitor )
{
    os << "Monitor< " << monitor.get() << " >";
    return os;
}

}
}

//----------------------------------------------------------------------
// implementation
//----------------------------------------------------------------------

#ifdef HAVE_PTHREAD_H

#include <eq/base/debug.h>
#include <eq/base/log.h>

namespace eq
{
namespace base
{
class MonitorPrivate
{
public:
    pthread_cond_t  cond;
    pthread_mutex_t mutex;
};

template< typename T > 
inline void Monitor<T>::_construct()
{
    _data = new MonitorPrivate;

    int error = pthread_cond_init( &_data->cond, 0 );
    if( error )
    {
        EQERROR << "Error creating pthread condition: " 
                << strerror( error ) << std::endl;
        return;
    } 
    
    error = pthread_mutex_init( &_data->mutex, 0 );
    if( error )
    {
        EQERROR << "Error creating pthread mutex: "
                << strerror( error ) << std::endl;
        return;
    }
}
        
template< typename T > 
inline Monitor<T>::~Monitor()
{
    pthread_cond_destroy( &_data->cond );
    pthread_mutex_destroy( &_data->mutex );
    delete _data;
    _data = 0;
}

template< typename T > 
inline Monitor<T>& Monitor<T>::operator++ ()
{
    pthread_mutex_lock( &_data->mutex );
    ++_value;
    pthread_cond_broadcast( &_data->cond );
    pthread_mutex_unlock( &_data->mutex );
    return *this;
}

template<>  
inline Monitor< bool >& Monitor< bool >::operator++ ()
{
   pthread_mutex_lock( &_data->mutex );
   assert( !_value );
   _value = !_value;
   pthread_cond_broadcast( &_data->cond );
   pthread_mutex_unlock( &_data->mutex );
   return *this;
}

template< typename T > 
inline Monitor<T>& Monitor<T>::operator-- ()
{
    pthread_mutex_lock( &_data->mutex );
    --_value;
    pthread_cond_broadcast( &_data->cond );
    pthread_mutex_unlock( &_data->mutex );
    return *this;
}

template<> 
inline Monitor< bool >& Monitor< bool >::operator-- ()
{
    EQUNIMPLEMENTED;
    return *this;
}

template< typename T > 
inline void Monitor<T>::set( const T& value )
{
    pthread_mutex_lock( &_data->mutex );
    _value = value;
    pthread_cond_broadcast( &_data->cond );
    pthread_mutex_unlock( &_data->mutex );
}

template< typename T > 
inline const T& Monitor<T>::waitEQ( const T& value ) const
{
    pthread_mutex_lock( &_data->mutex );
    while( _value != value )
        pthread_cond_wait( &_data->cond, &_data->mutex);
    pthread_mutex_unlock( &_data->mutex );
    return value;
}

template< typename T > 
inline const T& Monitor<T>::waitNE( const T& value ) const
{
    pthread_mutex_lock( &_data->mutex );
    while( _value == value )
        pthread_cond_wait( &_data->cond, &_data->mutex);
    const T& newValue = _value;
    pthread_mutex_unlock( &_data->mutex );
    return newValue;
}

template< typename T > 
inline const T& Monitor<T>::waitGE( const T& value ) const
{
    pthread_mutex_lock( &_data->mutex );
    while( _value < value )
        pthread_cond_wait( &_data->cond, &_data->mutex);
    const T& newValue = _value;
    pthread_mutex_unlock( &_data->mutex );
    return newValue;
}

template< typename T > 
inline const T& Monitor<T>::waitLE( const T& value ) const
{
    pthread_mutex_lock( &_data->mutex );
    while( _value > value )
        pthread_cond_wait( &_data->cond, &_data->mutex);
    const T& newValue = _value;
    pthread_mutex_unlock( &_data->mutex );
    return newValue;
}

}
}
#endif // HAVE_PTHREAD_H
#endif //EQBASE_MONITOR_H
