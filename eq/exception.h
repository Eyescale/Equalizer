
/* Copyright (c) 2011-2014, Stefan Eilemann <eile@eyescale.ch>
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
/** Exception class for Equalizer operations. */
class Exception : public co::Exception
{
public:
    enum Type
    {
        TIMEOUT_INPUTFRAME  = co::Exception::CUSTOM,
        GL_ERROR,
        CUSTOM              = co::Exception::CUSTOM + 20 // leave some room
    };

    /** Construct a new Exception. */
    explicit Exception( const uint32_t type ) : co::Exception( type ) {}

    /** Destruct this exception. */
    virtual ~Exception() throw() {}

    const char* what() const throw() override
    {
        switch( getType( ))
        {
        case Exception::TIMEOUT_INPUTFRAME:
            return " Timeout waiting on input frame";
        case Exception::GL_ERROR:
            return " OpenGL Error";
        default:
            return co::Exception::what();
        }
    }
};
}

#endif // EQ_EXCEPTION_H
