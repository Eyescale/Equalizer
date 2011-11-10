
/* Copyright (c) 2006-2011, Stefan Eilemann <eile@equalizergraphics.com> 
 *                    2011, Cedric Stalder <cedric.stalder@gmail.com>
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

#ifndef COBASE_MONITOR_H
#define COBASE_MONITOR_H

#include <co/base/nonCopyable.h> // base class
#include <co/base/condition.h>   // member
#include <co/base/types.h>

#include <errno.h>
#include <string.h>
#include <iostream>
#include <typeinfo>

namespace co
{
namespace base
{
    /**
     * A monitor primitive.
     *
     * A monitor has a value, which can be monitored to reach a certain
     * state. The caller is blocked until the condition is fulfilled. The
     * concept is similar to a pthread condition, with more usage convenience.
     */
    template< typename T > class Monitor
    {
    public:
        /** Construct a new monitor with a default value of 0. @version 1.0 */
        Monitor() : _value( T( 0 )) {}

        /** Construct a new monitor with a given default value. @version 1.0 */
        explicit Monitor( const T& value ) : _value( value ) {}

        /** Ctor initializing with the given monitor value. @version 1.1.5 */
        Monitor( const Monitor< T >& from ) : _value( from._value ) {}

        /** Destructs the monitor. @version 1.0 */
        ~Monitor() {}

        /** @name Changing the monitored value. */
        //@{
        /** Increment the monitored value, prefix only. @version 1.0 */
        Monitor& operator++ ()
            {
                _cond.lock();
                ++_value;
                _cond.broadcast();
                _cond.unlock();
                return *this;
            }

        /** Decrement the monitored value, prefix only. @version 1.0 */
        Monitor& operator-- ()
            {
                _cond.lock();
                --_value;
                _cond.broadcast();
                _cond.unlock();
                return *this;
            }

        /** Assign a new value. @version 1.0 */
        Monitor& operator = ( const T& value )
            {
                set( value );
                return *this;
            }

        /** Assign a new value. @version 1.1.5 */
        const Monitor& operator = ( const Monitor< T >& from )
            {
                set( from._value );
                return *this;
            }

        /** Perform an or operation on the value. @version 1.0 */
        Monitor& operator |= ( const T& value )
            {
                _cond.lock();
                _value |= value;
                _cond.broadcast();
                _cond.unlock();
                return *this;
            }

        /** Set a new value. @version 1.0 */
        void set( const T& value )
            {
                _cond.lock();
                _value = value;
                _cond.broadcast();
                _cond.unlock();
            }
        //@}

        /** @name Monitor the value. */
        //@{
        /**
         * Block until the monitor has the given value.
         * @return the value when reaching the condition.
         * @version 1.0
         */
        const T& waitEQ( const T& value ) const
            {
                if( _value == value )
                    return value;
                _cond.lock();
                while( _value != value )
                    _cond.wait();
                _cond.unlock();
                return value;
            }

        /**
         * Block until the monitor has not the given value.
         * @return the value when reaching the condition.
         * @version 1.0
         */
        const T waitNE( const T& value ) const
            {
                const T current = _value;
                if( current != value )
                    return current;

                _cond.lock();
                while( _value == value )
                    _cond.wait();
                const T newValue = _value;
                _cond.unlock();
                return newValue;
            }

        /**
         * Block until the monitor has none of the given values.
         * @return the value when reaching the condition.
         * @version 1.0
         */
        const T waitNE( const T& v1, const T& v2 ) const
            {
                const T current = _value;
                if( current != v1 && current != v2 )
                    return current;

                _cond.lock();
                while( _value == v1 || _value == v2 )
                    _cond.wait();
                const T newValue = _value;
                _cond.unlock();
                return newValue;
            }

        /**
         * Block until the monitor has a value greater or equal to the given
         * value.
         * @return the value when reaching the condition.
         * @version 1.0
         */
         const T waitGE( const T& value ) const
            {
                const T current = _value;
                if( current >= value )
                    return current;

                _cond.lock();
                while( _value < value )
                    _cond.wait();
                const T newValue = _value;
                _cond.unlock();
                return newValue;
            }
        /**
         * Block until the monitor has a value less or equal to the given
         * value.
         * @return the value when reaching the condition.
         * @version 1.0
         */
        const T waitLE( const T& value ) const
            {
                const T current = _value;
                if( current <= value )
                    return current;

                _cond.lock();
                while( _value > value )
                    _cond.wait();
                const T newValue = _value;
                _cond.unlock();
                return newValue;
            }

        /** @name Monitor the value with a timeout. */
        //@{
        /**
         * Block until the monitor has the given value.
         * @param value the exact value to monitor.
         * @param timeout the timeout in milliseconds to wait for the value.
         * @return true on success, false on timeout.
         * @version 1.1
         */
        bool timedWaitEQ( const T& value, const uint32_t timeout ) const
            {
                if( _value == value )
                    return true;

                _cond.lock();
                while( _value != value )
                {
                    if( !_cond.timedWait( timeout ) )
                    {
                        _cond.unlock();
                        return false;
                    }
                }
                _cond.unlock();
                return true;
            }

        /**
         * Block until the monitor has a value greater or equal to the given
         * value.
         * @param value the exact value to monitor.
         * @param timeout the timeout in milliseconds to wait for the value.
         * @return true on success, false on timeout.
         * @version 1.1
         */
       bool timedWaitGE( const T& value, const uint32_t timeout ) const
            {
                if( _value >= value )
                    return true;

                _cond.lock();
                while( _value < value )
                {
                    if ( !_cond.timedWait( timeout ) )
                    {
                        _cond.unlock();
                        return false;
                    }
                }
                _cond.unlock();
                return true;
            }
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

        /** @return the current or'ed with the given value. @version 1.0 */
        T operator | ( const T& value ) const
            { return static_cast< T >( _value | value ); }

        /** @return the current and the given value. @version 1.0 */
        T operator & ( const T& value ) const
            { return static_cast< T >( _value & value ); }
        //@}

    private:
        T _value;
        mutable Condition _cond;
    };

    typedef Monitor< bool >     Monitorb; //!< A boolean monitor variable
    typedef Monitor< uint32_t > Monitoru; //!< An unsigned 32bit integer monitor

    /** Print the monitor to the given output stream. @version 1.0 */
    template< typename T >
    inline std::ostream& operator << ( std::ostream& os,
                                       const Monitor<T>& monitor )
    {
        os << "Monitor< " << monitor.get() << " >";
        return os;
    }

    template<> inline Monitor< bool >& Monitor< bool >::operator++ ()
    {
        _cond.lock();
        assert( !_value );
        _value = !_value;
        _cond.broadcast();
        _cond.unlock();
        return *this;
    }

    template<> inline Monitor< bool >& Monitor< bool >::operator-- ()
    {
        _cond.lock();
        assert( !_value );
        _value = !_value;
        _cond.broadcast();
        _cond.unlock();
        return *this;
    }

    template<> 
    inline Monitor< bool >& Monitor< bool >::operator |= ( const bool& value )
    {
        if( value )
            _value = value;
        return *this;
    }
}
}
#endif //COBASE_MONITOR_H
