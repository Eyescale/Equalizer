
/* Copyright (c) 2006-2012, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include <cstdio>

#ifdef _MSC_VER
#  define snprintf _snprintf
#endif

namespace eq
{

ChannelStatistics::ChannelStatistics( const Statistic::Type type,
                                      Channel* channel, const uint32_t frame,
                                      const int32_t hint )
        : StatisticSampler< Channel >( type, channel, frame )
        , _hint( hint )
{
    if( _hint == AUTO )
        _hint = channel->getIAttribute( Channel::IATTR_HINT_STATISTICS );
    if( _hint == OFF )
        return;

    event.statistic.task = channel->getTaskID();

    const std::string& name = channel->getName();
    if( name.empty( ))
        snprintf( event.statistic.resourceName, 32, "Channel %s",
                  channel->getID().getShortString().c_str( ));
    else
        snprintf( event.statistic.resourceName, 32, "%s", name.c_str( ));
    event.statistic.resourceName[31] = 0;

    if( _hint == NICEST &&
        type != Statistic::CHANNEL_ASYNC_READBACK &&
        type != Statistic::CHANNEL_FRAME_TRANSMIT &&
        type != Statistic::CHANNEL_FRAME_COMPRESS &&
        type != Statistic::CHANNEL_FRAME_WAIT_SENDTOKEN )
    {
        channel->getWindow()->finish();
    }

    event.statistic.startTime  = channel->getConfig()->getTime();
}


ChannelStatistics::~ChannelStatistics()
{
    if( _hint == OFF )
        return;

    const Statistic::Type type = event.statistic.type;
    if( _hint == NICEST &&
        type != Statistic::CHANNEL_ASYNC_READBACK &&
        type != Statistic::CHANNEL_FRAME_TRANSMIT &&
        type != Statistic::CHANNEL_FRAME_COMPRESS &&
        type != Statistic::CHANNEL_FRAME_WAIT_SENDTOKEN )
    {
        _owner->getWindow()->finish();
    }

    if( event.statistic.endTime == 0 )
        event.statistic.endTime = _owner->getConfig()->getTime();
    if( event.statistic.endTime <= event.statistic.startTime )
        event.statistic.endTime = event.statistic.startTime + 1;

    _owner->addStatistic( event );
}

}
