
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "configStatistics.h"

#include "config.h"
#include "global.h"

#ifdef _MSC_VER
#  define snprintf _snprintf
#endif

namespace eq
{

ConfigStatistics::ConfigStatistics( const Statistic::Type type, 
                                    Config* config )
        : StatisticSampler< Config >( type, config, config->getCurrentFrame( ))
{
    const std::string& name = config->getName();
    if( name.empty( ))
        snprintf( event.data.statistic.resourceName, 32, "config" );
    else
        snprintf( event.data.statistic.resourceName, 32, "%s", name.c_str( ));
    event.data.statistic.resourceName[31] = 0;
    event.data.statistic.startTime = config->getTime();
}


ConfigStatistics::~ConfigStatistics()
{
    event.data.statistic.endTime = _owner->getTime();
    _owner->sendEvent( event );
}

}
