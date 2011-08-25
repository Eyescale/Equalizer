/* Copyright (c) 2008-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQFABRIC_EQUALIZERTYPES_H
#define EQFABRIC_EQUALIZERTYPES_H

#include <co/base/defines.h>

namespace eq
{
namespace fabric
{

static const uint32_t LOAD_EQUALIZER        = 1;
static const uint32_t TREE_EQUALIZER        = LOAD_EQUALIZER << 1;
static const uint32_t VIEW_EQUALIZER        = LOAD_EQUALIZER << 2;
static const uint32_t TILE_EQUALIZER        = LOAD_EQUALIZER << 3;
static const uint32_t MONITOR_EQUALIZER     = LOAD_EQUALIZER << 4;
static const uint32_t DFR_EQUALIZER         = LOAD_EQUALIZER << 5;
static const uint32_t FRAMERATE_EQUALIZER   = LOAD_EQUALIZER << 6;
static const uint32_t EQUALIZER_ALL         = EQ_UNDEFINED_UINT32;

}
}
#endif //EQFABRIC_EQUALIZERTYPES_H
