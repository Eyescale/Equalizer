
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

#include "nodeStatistics.h"

#include "config.h"
#include "global.h"
#include "pipe.h"
#include "node.h"
#include "frameData.h"

#ifdef _MSC_VER
#  define snprintf _snprintf
#endif

namespace eq
{

NodeStatistics::NodeStatistics( const Statistic::Type type, 
                                Node* node, const uint32_t frameNumber )
        : StatisticSampler< Node >( type, node, frameNumber )
{
    const std::string& name = node->getName();
    if( name.empty( ))
        snprintf( event.data.statistic.resourceName, 32, "Node %s",
                  node->getID().getShortString().c_str( ));
    else
        snprintf( event.data.statistic.resourceName, 32, "%s", name.c_str( ));

    event.data.statistic.resourceName[31] = 0;

    co::LocalNodePtr localNode = node->getLocalNode();
    EQASSERT( localNode );
    if( !localNode )
    {
        event.data.statistic.frameNumber = 0;
        return;
    }

    Config* config = _owner->getConfig();
    event.data.statistic.startTime = config->getTime();
    EQASSERT( _owner->getID() != UUID::ZERO );
    event.data.originator = _owner->getID();
}


NodeStatistics::~NodeStatistics()
{
    if( event.data.statistic.frameNumber == 0 ) // does not belong to a frame
        return;

    co::LocalNodePtr localNode = _owner->getLocalNode();
    EQASSERT( localNode );
    if( !localNode )
        return;

    Config* config = _owner->getConfig();
    event.data.statistic.endTime = config->getTime();
    config->sendEvent( event );
}

}
