
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <vector>

namespace eq
{
namespace util
{

class Accum;
class FrameBufferObject;
class Texture;
template< class > class BitmapFont;
template< class > class ObjectManager;


/** A vector of pointers to eq::util::Texture */
typedef std::vector< Texture* >  Textures;

class GPUCompressor; //!< @internal

#ifdef EQ_USE_DEPRECATED
typedef Textures TextureVector
#endif
}
}
#endif // EQUTIL_TYPES_H
