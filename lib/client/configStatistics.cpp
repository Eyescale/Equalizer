
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "configStatistics.h"

#include "config.h"
#include "global.h"

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
