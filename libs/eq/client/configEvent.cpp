
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "configEvent.h"

#include <co/buffer.h>

namespace eq
{

ConfigEvent::ConfigEvent( co::BufferPtr buffer )
    : co::ObjectCommand( buffer )
{
    *this >> data;
}

std::ostream& operator << ( std::ostream& os, const ConfigEvent* event )
{
    os << "config event " << ", event "
       << event->data;

    return os;
}

}
