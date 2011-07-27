
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef EQ_EXCEPTION_H
#define EQ_EXCEPTION_H

#include <co/exception.h> // base class

namespace eq
{
    class Exception;
    std::ostream& operator << ( std::ostream& os, const Exception& e );

    /** Exception class for Equalizer operations. */
    class Exception : public co::Exception
    {
    public:
        enum Type
        {
            TIMEOUT_INPUTFRAME  = co::Exception::CUSTOM,
            TIMEOUT_FRAMESYNC,
            CUSTOM              = co::Exception::CUSTOM + 20 // leave some room
        };

        /** Construct a new Exception. */
        Exception( const uint32_t type ) : co::Exception( type ) {}

        virtual const char* what() const throw() 
        { 
            std::stringstream os;
            os << *this;
            return os.str().c_str(); 
        }
    };

    inline std::ostream& operator << ( std::ostream& os, const Exception& e )
    {
        switch( e.getType() )
        {
          case Exception::TIMEOUT_INPUTFRAME:
            os << " Timeout waiting on input frame";
            break;
          case Exception::TIMEOUT_FRAMESYNC:
            os << " Timeout during thread frame synchronization";
            break;
          default:
            os << static_cast< const co::Exception& >( e );
        }
        return os;
    }
}

#endif // EQ_EXCEPTION_H
