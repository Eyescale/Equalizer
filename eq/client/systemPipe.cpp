
/* Copyright (c) 2009-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "systemPipe.h"

#include "pipe.h"

namespace eq
{

SystemPipe::SystemPipe( Pipe* parent )
    : _maxOpenGLVersion( fabric::AUTO )
    , _pipe( parent )
{
    LBASSERT( _pipe );
}

SystemPipe::~SystemPipe()
{
}

EventOCommand SystemPipe::sendError( const uint32_t error )
{
    return _pipe->sendError( error );
}

}
