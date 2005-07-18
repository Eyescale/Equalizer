
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "log.h"

using namespace eqBase;

static int getLogLevel();

int eqBase::Log::level = getLogLevel();


int getLogLevel()
{
    const char *env = getenv("EQLOGLEVEL");
    if( env != NULL )
    {
        if( strcmp( env, "ERROR" ) == 0 )
            return LOG_ERROR;
        if( strcmp( env, "WARN" ) == 0 )
            return LOG_WARN;
        if( strcmp( env, "INFO" ) == 0 )
            return LOG_INFO;
        if( strcmp( env, "VERBATIM" ) == 0 )
            return LOG_VERBATIM;
    }

#ifdef NDEBUG
    return LOG_WARN;
#else
    return LOG_INFO;
#endif
}
