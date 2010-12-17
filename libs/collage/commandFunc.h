
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

#ifndef EQNET_COMMANDFUNC_H
#define EQNET_COMMANDFUNC_H

namespace eq
{
namespace net
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
                  _func(
                      static_cast<bool (T::*)( Command& )>(from._func))
            {}

        bool operator()( Command& command )
        {
            return (_object->*_func)( command );
        }

        // Can't make private because copy constructor needs access. Can't
        // declare friend because C++ does not allow generic template friends in
        // template classes.
        //private:
        T* _object;
        bool (T::*_func)( Command& );
    };
}
}

#endif //EQNET_COMMANDFUNC_H
