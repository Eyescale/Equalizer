
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_MONITOR_H
#define EQBASE_MONITOR_H

#include <eq/base/nonCopyable.h> // base class

namespace eqBase
{
    class MonitorPrivate;

    /**
     * A monitor primitive.
     *
     * A monitor has a value, which can be monitored to reach a certain
     * state. The caller is blocked until the condition is fulfilled.
     *
     * Template instantiation are at the end of the .cpp file.
     */
    template< typename T > class Monitor : public NonCopyable
    {
    public:
        /** 
         * Constructs a new monitor for the given thread type.
         */
        Monitor() : _value( static_cast<T>( 0 ))    { _construct(); }
        Monitor( const T& value ) : _value( value ) { _construct(); }

        void _construct();
        
        /** Destructs the monitor. */
        ~Monitor();

        /** @name Changing the monitored value. */
        //*{
        Monitor& operator++ ();    // prefix only
        Monitor& operator-- ();    // prefix only
        Monitor& operator = ( const T& val )
            {
                set( val );
                return *this;
            }

        void set( const T& val );
        //*}

        /** @name Monitor the value. */
        //*{
        void waitEQ( const T& val ) const;
        void waitNE( const T& val ) const;
        void waitGE( const T& val ) const;
        void waitLE( const T& val ) const;
        //*}

        /** @name Comparison Operators. */
        //*{
        bool operator == ( const T& val ) const { return _value == val; }
        bool operator != ( const T& val ) const { return _value != val; }
        bool operator < ( const T& val ) const { return _value < val; }
        bool operator > ( const T& val ) const { return _value > val; }
        bool operator <= ( const T& val ) const { return _value <= val; }
        bool operator >= ( const T& val ) const { return _value >= val; }

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
        //*}

        const T& get() const { return _value; }

    private:
        T _value;
        MonitorPrivate* _data;
    };

typedef Monitor< bool >     Monitorb;
typedef Monitor< uint32_t > Monitoru;
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

// Monitor for uint32_t and bool are explicitly instantiated in monitor.cpp
// Monitors for other types can be created by including pthread.h before this
// file.  
// The application has to include pthread.h since on Windows the use of
// pthreads-Win32 library includes might create hard to resolve type conflicts
// with other header files.

#ifdef HAVE_PTHREAD_H

#include <eq/base/debug.h>
#include <eq/base/log.h>

namespace eqBase
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

    int error = pthread_cond_init( &_data->cond, NULL );
    if( error )
    {
        EQERROR << "Error creating pthread condition: " 
                << strerror( error ) << std::endl;
        return;
    } 
    
    error = pthread_mutex_init( &_data->mutex, NULL );
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
inline void Monitor<T>::set( const T& val )
{
    pthread_mutex_lock( &_data->mutex );
    _value = val;
    pthread_cond_broadcast( &_data->cond );
    pthread_mutex_unlock( &_data->mutex );
}

template< typename T > 
inline void Monitor<T>::waitEQ( const T& val ) const
{
    pthread_mutex_lock( &_data->mutex );
    while( _value != val )
        pthread_cond_wait( &_data->cond, &_data->mutex);
    pthread_mutex_unlock( &_data->mutex );
}

template< typename T > 
inline void Monitor<T>::waitNE( const T& val ) const
{
    pthread_mutex_lock( &_data->mutex );
    while( _value == val )
        pthread_cond_wait( &_data->cond, &_data->mutex);
    pthread_mutex_unlock( &_data->mutex );
}

template< typename T > 
inline void Monitor<T>::waitGE( const T& val ) const
{
    pthread_mutex_lock( &_data->mutex );
    while( _value < val )
        pthread_cond_wait( &_data->cond, &_data->mutex);
    pthread_mutex_unlock( &_data->mutex );
}

template< typename T > 
inline void Monitor<T>::waitLE( const T& val ) const
{
    pthread_mutex_lock( &_data->mutex );
    while( _value > val )
        pthread_cond_wait( &_data->cond, &_data->mutex);
    pthread_mutex_unlock( &_data->mutex );
}

template< typename T >
std::ostream& operator << ( std::ostream& os, const Monitor<T>& monitor )
{
    os << "Monitor< " << monitor.get() << " >";
    return os;
}
}
#endif // HAVE_PTHREAD_H
#endif //EQBASE_MONITOR_H
