
#include "log.h"

using namespace eqBase;

static int getLogLevel();

int eqBase::logLevel = getLogLevel();


int getLogLevel()
{

    const char *env = getenv("EQLOGLEVEL");
    if( env != NULL )
    {
        if( strcmp( env, "ERROR" ))
            return LOG_ERROR;
        if( strcmp( env, "WARN" ))
            return LOG_WARN;
        if( strcmp( env, "INFO" ))
            return LOG_INFO;
    }

#ifdef NDEBUG
    return LOG_WARN;
#else
    return LOG_INFO;
#endif
}
