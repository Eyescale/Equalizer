
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com>
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

#include "frameDataStatistics.h"

#include "config.h"
#include "global.h"
#include "pipe.h"
#include "frameData.h"

#ifdef _MSC_VER
#  define snprintf _snprintf
#endif

namespace eq
{

FrameDataStatistics::FrameDataStatistics( const Statistic::Type type, 
                                          FrameData* frameData, 
                                          const uint32_t frameNumber,
                                          const uint128_t& originator )
        : StatisticSampler< FrameData >( type, frameData, frameNumber )
{
    snprintf( event.data.statistic.resourceName, 32, "Node %s",
              originator.getShortString().c_str( ));
    event.data.statistic.resourceName[31] = 0;

    const net::Session* session = frameData->getSession();
    EQASSERT( session );
    if( !session )
    {
        event.data.statistic.frameNumber = 0;
        return;
    }

    const Config* config = EQSAFECAST( const Config*, session );
    event.data.statistic.startTime = config->getTime();
    EQASSERT( originator != base::UUID::ZERO );
    event.data.originator = originator;
}


FrameDataStatistics::~FrameDataStatistics()
{
    if( event.data.statistic.frameNumber == 0 ) // does not belong to a frame
        return;

    net::Session* session = _owner->getSession();
    EQASSERT( session );
    if( !session )
        return;

    Config* config = EQSAFECAST( Config*, session );
    event.data.statistic.endTime = config->getTime();
    config->sendEvent( event );
}

}
