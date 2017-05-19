
/* Copyright (c) 2009-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
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

#include "nodeStatistics.h"

#include "config.h"
#include "global.h"
#include "node.h"
#include "pipe.h"

#include <cstdio>

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

namespace eq
{
NodeStatistics::NodeStatistics(const Statistic::Type type, Node* node,
                               const uint32_t frameNumber)
    : StatisticSampler<Node>(type, node, frameNumber)
{
    const std::string& name = node->getName();
    if (name.empty())
        snprintf(statistic.resourceName, 32, "Node %s",
                 node->getID().getShortString().c_str());
    else
        snprintf(statistic.resourceName, 32, "%s", name.c_str());

    statistic.resourceName[31] = 0;

    co::LocalNodePtr localNode = node->getLocalNode();
    LBASSERT(localNode);
    if (!localNode)
    {
        statistic.frameNumber = 0;
        return;
    }

    Config* config = _owner->getConfig();
    statistic.startTime = config->getTime();
    LBASSERT(_owner->getID() != 0);
}

NodeStatistics::~NodeStatistics()
{
    if (statistic.frameNumber == 0) // does not belong to a frame
        return;

    co::LocalNodePtr localNode = _owner->getLocalNode();
    LBASSERT(localNode);
    if (!localNode)
        return;

    Config* config = _owner->getConfig();
    statistic.endTime = config->getTime();
    if (statistic.endTime <= statistic.startTime)
        statistic.endTime = statistic.startTime + 1;
    _owner->processEvent(statistic);
}
}
