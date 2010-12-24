
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <co/base/os.h>
#include <co/base/debug.h>

namespace co
{
    class Command;

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

        template< typename O > CommandFunc( const O& from )
                : _object( from._object ),
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
    };

    template< typename T >
    inline std::ostream& operator << ( std::ostream& os,
                                       const CommandFunc<T>& func )
    {
        if( func._object && func._func )
            os << "CommandFunc of " << co::base::className( func._object );
        else
            os << "invalid CommandFunc";
        return os;
    }
}

#endif //CO_COMMANDFUNC_H
