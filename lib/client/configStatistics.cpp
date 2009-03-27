
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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

#include "configStatistics.h"

#include "config.h"
#include "global.h"

#ifdef WIN32_VC
#  define snprintf _snprintf
#endif

namespace eq
{

ConfigStatistics::ConfigStatistics( const Statistic::Type type, 
                                    Config* config )
        : ignore( false )
        , _config( config )
{
    event.data.type                  = Event::STATISTIC;
    event.data.originator            = config->getID();
    event.data.statistic.type        = type;
    event.data.statistic.frameNumber = config->getCurrentFrame();

    const std::string& name = config->getName();
    if( name.empty( ))
        snprintf( event.data.statistic.resourceName, 32, "config" );
    else
        snprintf( event.data.statistic.resourceName, 32, "%s", name.c_str( ));

    event.data.statistic.startTime   = config->getTime();
}


ConfigStatistics::~ConfigStatistics()
{
    if( ignore )
        return;

    event.data.statistic.endTime     = _config->getTime();
    _config->sendEvent( event );
}

}
