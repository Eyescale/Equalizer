
/* Copyright (c) 2008-2016, Stefan Eilemann <eile@equalizergraphics.com>
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

#include <cstdio>

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

namespace eq
{
ConfigStatistics::ConfigStatistics(const Statistic::Type type, Config* config)
    : StatisticSampler<Config>(type, config, config->getCurrentFrame())
{
    const std::string& name = config->getName();
    if (name.empty())
        snprintf(statistic.resourceName, 32, "config");
    else
        snprintf(statistic.resourceName, 32, "%s", name.c_str());
    statistic.resourceName[31] = 0;
    statistic.startTime = config->getTime();
}

ConfigStatistics::~ConfigStatistics()
{
    statistic.endTime = _owner->getTime();
    if (statistic.endTime <= statistic.startTime)
        statistic.endTime = statistic.startTime + 1;
    _owner->sendEvent(EVENT_STATISTIC) << statistic;
}
}
