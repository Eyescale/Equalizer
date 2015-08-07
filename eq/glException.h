
/* Copyright (c) 2012-2014, Stefan.Eilemann@epfl.ch
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

#include <eq/api.h>
#include <eq/exception.h> // base class

namespace eq
{
/** OpenGL Exception. */
class GLException : public Exception
{
public:
    /** Construct a new OpenGL Exception. */
    EQ_API explicit GLException( const uint32_t glError_ );

    /** Destruct this exception. */
    virtual ~GLException() throw() {}

    EQ_API const char* what() const throw() override;

    const uint32_t glError;

private:
    std::string _what;
};
}

#endif // EQ_GLEXCEPTION_H
