
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

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
