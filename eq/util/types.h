
/* Copyright (c) 2007-2013, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQUTIL_TYPES_H
#define EQUTIL_TYPES_H

#include <eq/types.h>
#include <eq/error.h>
#include <vector>

namespace eq
{
namespace util
{

class Accum;
class AccumBufferObject;
class FrameBufferObject;
class PixelBufferObject;
class Texture;
class BitmapFont;
class ObjectManager;

namespace shader {}

/** A vector of pointers to eq::util::Texture */
typedef std::vector< Texture* >  Textures;
}
}
#endif // EQUTIL_TYPES_H
