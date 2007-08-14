
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_MONITOR_H
#define EQBASE_MONITOR_H

#include <eq/base/log.h>
#include <eq/base/nonCopyable.h>
#include <pthread.h>

namespace eqBase
{
    /**
     * A monitor primitive.
     *
     * A monitor has a value, which can be monitored to reach a certain
     * state. The caller is blocked until the condition is fulfilled.
     */
    template< typename T > class Monitor : public NonCopyable
    {
    public:
        /** 
         * Constructs a new monitor for the given thread type.
         */
        Monitor()                 : _value( 0 )     { _construct(); }
        Monitor( const T& value ) : _value( value ) { _construct(); }

        void _construct()
            {
                int error = pthread_cond_init( &_cond, NULL );
                if( error )
                {
                    EQERROR << "Error creating pthread condition: " 
                            << strerror( error ) << std::endl;
                    return;
                } 
                
                error = pthread_mutex_init( &_mutex, NULL );
                if( error )
                {
                    EQERROR << "Error creating pthread mutex: "
                            << strerror( error ) << std::endl;
                    return;
                }
            }
        
        /** Destructs the monitor. */
        ~Monitor()
            {
                pthread_cond_destroy( &_cond );
                pthread_mutex_destroy( &_mutex );
            }

        /** @name Changing the monitored value. */
        //*{
        Monitor& operator++ ()    // prefix only
            {
                pthread_mutex_lock( &_mutex );
                ++_value;
                pthread_cond_broadcast( &_cond );
                pthread_mutex_unlock( &_mutex );
                return *this;
            }
        Monitor& operator-- ()    // prefix only
            {
                pthread_mutex_lock( &_mutex );
                --_value;
                pthread_cond_broadcast( &_cond );
                pthread_mutex_unlock( &_mutex );
                return *this;
            }
        Monitor& operator = ( const T& val )
            {
                set( val );
                return *this;
            }

        void set( const T& val )
            {
                pthread_mutex_lock( &_mutex );
                _value = val;
                pthread_cond_broadcast( &_cond );
                pthread_mutex_unlock( &_mutex );
            }
        //*}

        /** @name Monitor the value. */
        //*{
        void waitEQ( const T& val ) const
            {
                pthread_mutex_lock( &_mutex );
                while( _value != val )
                    pthread_cond_wait( &_cond, &_mutex);
                pthread_mutex_unlock( &_mutex );
            }
        void waitGE( const T& val ) const
            {
                pthread_mutex_lock( &_mutex );
                while( _value < val )
                    pthread_cond_wait( &_cond, &_mutex);
                pthread_mutex_unlock( &_mutex );
            }
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

        mutable pthread_cond_t  _cond;
        mutable pthread_mutex_t _mutex;
    };

    typedef Monitor< bool >     Monitorb;
    typedef Monitor< uint32_t > Monitoru;

    template< typename T >
    std::ostream& operator << ( std::ostream& os, const Monitor<T>& monitor )
    {
        os << "Monitor< " << monitor.get() << " >";
        return os;
    }
}

#endif //EQBASE_MONITOR_H
