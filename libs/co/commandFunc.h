
/* Copyright (c) 2006-2012, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef CO_COMMANDFUNC_H
#define CO_COMMANDFUNC_H

#include <co/types.h>
#include <co/base/debug.h>

// If you get a warning in your code, add:
//  #pragma warning( disable: 4407 )
// This warning is 'fixed' by https://github.com/Eyescale/Equalizer/issues/100

namespace co
{
    /**
     * A wrapper to register a function callback on an object instance.
     * 
     * This wrapper is used by the Dispatcher to register and save callback
     * methods of derived classes.
     */
    template< typename T > class CommandFunc
    {
    public:
        CommandFunc( T* object, 
                     bool (T::*func)( Command& ))
            : _object( object ), _func( func ) {}


        template< typename O > CommandFunc( const CommandFunc< O >& from )
                : _object( _convertThis< O, T >( from._object )),
                  _func( static_cast<bool (T::*)( Command& )>(from._func))
            {}

        bool operator()( Command& command )
        {
            EQASSERT( _object );
            EQASSERT( _func );
            return (_object->*_func)( command );
        }

        void clear() { _object = 0; _func = 0; }
        bool isValid() const { return _object && _func; }

        // Can't make private because copy constructor needs access. Can't
        // declare friend because C++ does not allow generic template friends in
        // template classes.
        //private:
        T* _object;
        bool (T::*_func)( Command& );

    private:
        template< class F, class T > T* _convertThis( F* ptr )
        {
#ifdef _MSC_VER
            // https://github.com/Eyescale/Equalizer/issues/100
            return reinterpret_cast< T* >( ptr );
#else
            return ptr;
#endif
        }
    };

    template< typename T >
    inline std::ostream& operator << ( std::ostream& os,
                                       const CommandFunc<T>& func )
    {
        if( func.isValid( ))
            os << "CommandFunc of " << base::className( func._object );
        else
            os << "NULL CommandFunc";
        return os;
    }
}

#endif //CO_COMMANDFUNC_H
