
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "pipeStatistics.h"

#include "config.h"
#include "pipe.h"
#include "global.h"

#ifdef WIN32_VC
#  define snprintf _snprintf
#endif

namespace eq
{

PipeStatistics::PipeStatistics( const Statistic::Type type, Pipe* pipe )
        : _pipe( pipe )
{
#if 0
    const int32_t hint = _pipe->getIAttribute( Pipe::IATTR_HINT_STATISTICS );
    if( hint == OFF )
        return;
#endif

    event.data.type                  = Event::STATISTIC;
    event.data.originator            = pipe->getID();
    event.data.statistic.type        = type;
    event.data.statistic.frameNumber = pipe->getCurrentFrame();

    const std::string& name = pipe->getName();
    if( name.empty( ))
        snprintf( event.data.statistic.resourceName, 32, "pipe %d",
                  pipe->getID( ));
    else
        snprintf( event.data.statistic.resourceName, 32, "%s", name.c_str( ));

    event.data.statistic.startTime  = pipe->getConfig()->getTime();
    event.data.statistic.endTime    = 0;
}


PipeStatistics::~PipeStatistics()
{
#if 0
    const int32_t hint = _pipe->getIAttribute( Pipe::IATTR_HINT_STATISTICS);
    if( hint == OFF )
        return;
#endif

    if( event.data.statistic.frameNumber == 0 ) // does not belong to a frame
        return;

    Config* config = _pipe->getConfig();
    if( event.data.statistic.endTime == 0 )
        event.data.statistic.endTime = config->getTime();
    config->sendEvent( event );
}

}
