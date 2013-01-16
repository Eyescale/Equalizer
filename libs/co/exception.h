
/* Copyright (c) 2011, Cedric Stalder <cedric.stalder@gmail.com>
 *               2012, Stefan.Eilemann@epfl.ch
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
    class Exception;
    std::ostream& operator << ( std::ostream& os, const Exception& e );

    /** A base Exception for Collage operations. */
    class Exception : public std::exception
    {
    public:
        enum Type
        {
            TIMEOUT_WRITE,   //!< A write timeout operation
            TIMEOUT_READ,    //!< A read timeout operation
            TIMEOUT_BARRIER, //!< A barrier timeout operation
            TIMEOUT_COMMANDQUEUE, //!< A timeout on a cmd queue operation
            CUSTOM      = 20 // leave some room
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
            os << *this;
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
          case Exception::TIMEOUT_WRITE:
              return os << " Timeout on write operation";
          case Exception::TIMEOUT_READ:
              return os << " Timeout on read operation";
          case Exception::TIMEOUT_BARRIER:
              return os << " Timeout on barrier";
          case Exception::TIMEOUT_COMMANDQUEUE:
              return os << " Timeout on command queue";
          default:
              return os << " Unknown Exception";
        }
    }
}

#endif // CO_EXCEPTION_H
