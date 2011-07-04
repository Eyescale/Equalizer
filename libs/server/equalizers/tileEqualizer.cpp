
/* Copyright (c) 2008-2011, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2011, Carsten Rohn <carsten.rohn@rtt.ag>
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

#include "types.h"
#include "compound.h"
#include "tileQueue.h"

#include "tileEqualizer.h"

namespace eq
{
namespace server
{

TileEqualizer::TileEqualizer( const TileEqualizer& from )
    : Equalizer( from )
{
    // TODO should add queues and IO frames!?
}

void TileEqualizer::notifyUpdatePre( Compound* compound, 
                                     const uint32_t frameNumber )
{
    const TileQueues& queues = compound->getOutputTileQueues();
    TileQueuesCIter it = queues.begin();
    for ( ; it != queues.end(); ++it )
        (*it)->setActivated( Equalizer::isActivated() );
}

std::ostream& operator << ( std::ostream& os, const TileEqualizer* lb )
{
    if( lb )
        os << "tile_equalizer {}" << std::endl;

    return os;
}


} //server
} //eq
