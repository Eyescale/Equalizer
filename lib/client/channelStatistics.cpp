
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

#include "channelStatistics.h"

#include "channel.h"
#include "config.h"
#include "global.h"
#include "pipe.h"
#include "window.h"

#ifdef WIN32_VC
#  define snprintf _snprintf
#endif

namespace eq
{

ChannelStatistics::ChannelStatistics( const Statistic::Type type, 
                                      Channel* channel )
        : _channel( channel )
{
    const int32_t hint = channel->getIAttribute(Channel::IATTR_HINT_STATISTICS);
    if( hint == OFF )
        return;

    event.type                  = Event::STATISTIC;
    event.originator            = channel->getID();
    event.statistic.type        = type;
    event.statistic.frameNumber = channel->getPipe()->getCurrentFrame();
    event.statistic.task        = channel->getTaskID();

    const std::string& name = channel->getName();
    if( name.empty( ))
        snprintf( event.statistic.resourceName, 32, "channel %d",
                  channel->getID( ));
    else
        snprintf( event.statistic.resourceName, 32, "%s", name.c_str( ));

    if( hint == NICEST )
        channel->getWindow()->finish();

    event.statistic.startTime  = channel->getConfig()->getTime();
    event.statistic.endTime    = 0;
}


ChannelStatistics::~ChannelStatistics()
{
    const int32_t hint =_channel->getIAttribute(Channel::IATTR_HINT_STATISTICS);
    if( hint == OFF )
        return;

    if( hint == NICEST )
        _channel->getWindow()->finish();

    if( event.statistic.endTime == 0 )
        event.statistic.endTime = _channel->getConfig()->getTime();

    _channel->addStatistic( event );
}

}
