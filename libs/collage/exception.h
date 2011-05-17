
/* Copyright (c) 2011, Cedric Stalder <cedric.stalder@gmail.com>
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

#ifndef CO_EXCEPTION_H
#define CO_EXCEPTION_H
#include <sstream>

namespace co 
{
    /** A base Exception for collage operations. */
    class Exception : public std::exception
    {
    public:
        
        enum Type
        {
            EXCEPTION_WRITE_TIMEOUT,   //!< A write timeout operation 
            EXCEPTION_READ_TIMEOUT,    //!< A read timeout operation
            EXCEPTION_BARRIER_TIMEOUT, //!< A barrier timeout operation
            EXCEPTION_COMMANDQUEUE_TIMEOUT, //!< A command queue timeout
            EXCEPTION_MONITOR_TIMEOUT,      //!< A moitor timeout
            EXCEPTION_CUSTOM      = 20 // leave some room
        };

        /** Construct a new Exception. */
        Exception( const uint32_t type ) : _type( type ) {}

        /** Destruct this exception. */
        virtual ~Exception() throw() {}

        /** @return the type of this exception */
        virtual uint32_t getType() const { return _type; }

        virtual const char* what() const throw() 
        { 
            std::stringstream os;
            os << this;
            return os.str().c_str(); 
        }

    private:
        /** The type of this eception instance **/
        const uint32_t _type;
    };

    inline std::ostream& operator << ( std::ostream& os, const Exception& e )
    {
        switch( e.getType() )
        {
        case Exception::EXCEPTION_WRITE_TIMEOUT :
            os << " Timeout on write operation";
            break;
        case Exception::EXCEPTION_READ_TIMEOUT :
            os << " Timeout on read operation";
            break;
        case Exception::EXCEPTION_BARRIER_TIMEOUT :
            os << " Timeout on barrier ";
            break;
        case Exception::EXCEPTION_COMMANDQUEUE_TIMEOUT :
            os << " Timeout on command queue ";
            break;
        case Exception::EXCEPTION_MONITOR_TIMEOUT :
            os << " Timeout on monitor ";
            break;
        default:
            {
                EQASSERT( false );
            }
        }
        return os;
    }
}

#endif // CO_EXCEPTION_H
