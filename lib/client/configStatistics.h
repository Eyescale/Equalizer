
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_CONFIGSTATISTICS_H
#define EQ_CONFIGSTATISTICS_H

#include <eq/client/configEvent.h> // member
#include <eq/client/event.h>

namespace eq
{
    class Config;

    /**
     * Holds one statistics event, used for profiling.
     */
    class ConfigStatistics
    {
    public:
        ConfigStatistics( const Statistic::Type type, Config* config );
        ~ConfigStatistics();

        ConfigEvent event;
        bool        ignore;

    private:
        Config*     _config;
    };
}

#endif // EQ_CONFIGSTATISTICS_H
