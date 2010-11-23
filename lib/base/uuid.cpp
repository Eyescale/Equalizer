
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "uuid.h"
namespace eq
{
namespace base 
{

const UUID UUID::ZERO;
/** Special object version values */
const UUID EQ_UUID_MAX( 0xfffffffffffffff0u, 0 );
const UUID EQ_UUID_NONE( 0xfffffffffffffffdu, 0 );
const UUID EQ_UUID_INVALID( 0xfffffffffffffffeu, 0 );
const UUID EQ_UUID_ANY( 0xffffffffffffffffu, 0 ) ;
}
}