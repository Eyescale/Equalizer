
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "channelStatistics.h"

#include "channel.h"
#include "config.h"
#include "global.h"
#include "pipe.h"
#include "window.h"

#ifdef _MSC_VER
#  define snprintf _snprintf
#endif

namespace eq
{

ChannelStatistics::ChannelStatistics( const Statistic::Type type, 
                                      Channel* channel )
        : StatisticSampler< Channel >( type, channel, 
                                       channel->getPipe()->getCurrentFrame( ))
        , statisticsIndex( channel->_statisticsIndex )
{
    const int32_t hint = channel->getIAttribute(Channel::IATTR_HINT_STATISTICS);
    if( hint == OFF )
        return;

    event.data.statistic.task = channel->getTaskID();

    const std::string& name = channel->getName();
    if( name.empty( ))
        snprintf( event.data.statistic.resourceName, 32, "Channel %s",
                  channel->getID().getShortString().c_str( ));
    else
        snprintf( event.data.statistic.resourceName, 32, "%s", name.c_str( ));
    event.data.statistic.resourceName[31] = 0;

    if( hint == NICEST &&
        type != Statistic::CHANNEL_FRAME_TRANSMIT &&
        type != Statistic::CHANNEL_FRAME_COMPRESS &&
        type != Statistic::CHANNEL_FRAME_WAIT_SENDTOKEN )
    {
        channel->getWindow()->finish();
    }

    event.data.statistic.startTime  = channel->getConfig()->getTime();
}


ChannelStatistics::~ChannelStatistics()
{
    const int32_t hint = _owner->getIAttribute( Channel::IATTR_HINT_STATISTICS);
    if( hint == OFF )
        return;

    const Statistic::Type type = event.data.statistic.type;
    if( hint == NICEST &&
        type != Statistic::CHANNEL_FRAME_TRANSMIT &&
        type != Statistic::CHANNEL_FRAME_COMPRESS &&
        type != Statistic::CHANNEL_FRAME_WAIT_SENDTOKEN )
    {
        _owner->getWindow()->finish();
    }

    if( event.data.statistic.endTime == 0 )
        event.data.statistic.endTime = _owner->getConfig()->getTime();
    if( event.data.statistic.endTime == event.data.statistic.startTime )
        ++event.data.statistic.endTime;

    _owner->addStatistic( event.data, statisticsIndex );
}

}
