
#include "log.h"

using namespace eqBase;

static int getLogLevel();

int eqBase::Log::level = getLogLevel();


int getLogLevel()
{
    static bool first = true;

    if( first )
    {
        first = false;
        std::cout << "getLogLevel: " << getLogLevel() << std::endl;
    }

    const char *env = getenv("EQLOGLEVEL");
    if( env != NULL )
    {
        if( strcmp( env, "ERROR" ) == 0 )
            return LOG_ERROR;
        if( strcmp( env, "WARN" ) == 0 )
            return LOG_WARN;
        if( strcmp( env, "INFO" ) == 0 )
            return LOG_INFO;
    }

#ifdef NDEBUG
    return LOG_WARN;
#else
    return LOG_INFO;
#endif
}
