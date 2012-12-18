
/* Copyright (c) 2012, Stefan.Eilemann@epfl.ch
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

#ifndef EQ_GLEXCEPTION_H
#define EQ_GLEXCEPTION_H

#include <eq/client/api.h>
#include <eq/client/exception.h> // base class

namespace eq
{
    class GLException;
    EQ_API std::ostream& operator << ( std::ostream& os, const GLException& e );

    /** OpenGL Exception. */
    class GLException : public Exception
    {
    public:
        /** Construct a new OpenGL Exception. */
        GLException( const uint32_t glError_ )
                : Exception( GL_ERROR ), glError( glError_ ) {}

        virtual const char* what() const throw()
        {
            std::stringstream os;
            os << *this;
            return os.str().c_str();
        }

        const uint32_t glError;
    };
}

#endif // EQ_GLEXCEPTION_H
