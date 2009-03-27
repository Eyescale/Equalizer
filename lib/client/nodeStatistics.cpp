
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "nodeStatistics.h"

#include "config.h"
#include "node.h"
#include "global.h"

#ifdef WIN32_VC
#  define snprintf _snprintf
#endif

namespace eq
{

NodeStatistics::NodeStatistics( const Statistic::Type type, Node* node,
                                const uint32_t frameNumber )
        : _node( node )
{
    const int32_t hint = _node->getIAttribute( Node::IATTR_HINT_STATISTICS);
    if( hint == OFF )
        return;

    event.data.type                  = Event::STATISTIC;
    event.data.originator            = node->getID();
    event.data.statistic.type        = type;
    event.data.statistic.frameNumber = frameNumber;

    const std::string& name = node->getName();
    if( name.empty( ))
        snprintf( event.data.statistic.resourceName, 32, "node %d",
                  node->getID( ));
    else
        snprintf( event.data.statistic.resourceName, 32, "%s", name.c_str( ));

    event.data.statistic.startTime  = node->getConfig()->getTime();
    event.data.statistic.endTime    = 0;
}


NodeStatistics::~NodeStatistics()
{
    const int32_t hint = _node->getIAttribute( Node::IATTR_HINT_STATISTICS);
    if( hint == OFF )
        return;

    if( event.data.statistic.frameNumber == 0 ) // does not belong to a frame
        return;

    Config* config = _node->getConfig();
    if( event.data.statistic.endTime == 0 )
        event.data.statistic.endTime = config->getTime();

    config->sendEvent( event );
}

}
