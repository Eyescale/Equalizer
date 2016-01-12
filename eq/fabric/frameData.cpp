
/* Copyright (c) 2006-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
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

#include "frameData.h"

#include <co/dataIStream.h>
#include <co/dataOStream.h>

namespace eq
{
namespace fabric
{

void FrameData::serialize( co::DataOStream& os ) const
{
    os << _pvp << _frameType << _buffers << _period << _phase << _range
       << _pixel << _subpixel << _zoom;
}

void FrameData::deserialize( co::DataIStream& is )
{
    is >> _pvp >> _frameType >> _buffers >> _period >> _phase >> _range
       >> _pixel >> _subpixel >> _zoom;
}

}
}
