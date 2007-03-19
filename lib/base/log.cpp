
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "log.h"
#include "perThread.h"

using namespace eqBase;
using namespace std;

#ifdef WIN32
#  define atoll _atoi64
#endif

static int      getLogLevel();
static unsigned getLogTopics();

namespace eqBase
{
    class LogTable
    {
    public:
        LogTable( const LogLevel _level, const string& _name )
                : level( _level ), name( _name ) {}

        LogLevel level;
        string   name;
    };

#   define LOG_TABLE_ENTRY( name ) LogTable( LOG_ ## name, string( #name ))
#   define LOG_TABLE_SIZE (4)
    
    static LogTable _logTable[ LOG_TABLE_SIZE ] =
    {
        LOG_TABLE_ENTRY( ERROR ),
        LOG_TABLE_ENTRY( WARN ),
        LOG_TABLE_ENTRY( INFO ),
        LOG_TABLE_ENTRY( VERBATIM )
    };
}

EQ_EXPORT int           eqBase::Log::level  = getLogLevel();
EQ_EXPORT unsigned      eqBase::Log::topics = getLogTopics();
EQ_EXPORT eqBase::Clock eqBase::LogBuffer::_clock;

static PerThread<Log*> _logInstance;

int getLogLevel()
{
    const char *env = getenv("EQ_LOG_LEVEL");
    if( env )
    {
        const int level = atoi( env );
        if( level )
            return level;

        const string envString( env );
        for( uint32_t i=0; i<LOG_TABLE_SIZE; ++i )
            if( _logTable[i].name == envString )
                return _logTable[i].level;
    }

#ifdef NDEBUG
    return LOG_WARN;
#else
    return LOG_INFO;
#endif
}

std::string& Log::getLogLevelString()
{
    for( uint32_t i=0; i<LOG_TABLE_SIZE; ++i )
        if( _logTable[i].level == level )
                return _logTable[i].name;

    return _logTable[0].name;
}

unsigned getLogTopics()
{
    const char *env = getenv("EQ_LOG_TOPICS");
    if( env )
        return atoll(env);

    return 0;
}

EQ_EXPORT Log& Log::instance( const char* subdir, const char* file, const int line )
{
    Log* log = _logInstance.get();
    if( !log )
    {
        log = new Log();
        _logInstance = log;
    }

    log->setLogInfo( subdir, file, line );
    return *log;
}

EQ_EXPORT std::ostream& eqBase::indent( std::ostream& os )
{
    Log* log = dynamic_cast<Log*>(&os);
    if( log )
        log->indent();
    return os;
}
EQ_EXPORT std::ostream& eqBase::exdent( std::ostream& os )
{
    Log* log = dynamic_cast<Log*>(&os);
    if( log )
        log->exdent();
        return os;
}

EQ_EXPORT std::ostream& eqBase::disableFlush( std::ostream& os )
{
    Log* log = dynamic_cast<Log*>(&os);
    if( log )
        log->disableFlush();
    return os;
}
EQ_EXPORT std::ostream& eqBase::enableFlush( std::ostream& os )
{
    Log* log = dynamic_cast<Log*>(&os);
    if( log )
        log->enableFlush();
    return os;
}
EQ_EXPORT std::ostream& eqBase::forceFlush( std::ostream& os )
{
    Log* log = dynamic_cast<Log*>(&os);
    if( log )
        log->forceFlush();
    return os;
}

EQ_EXPORT std::ostream& eqBase::disableHeader( std::ostream& os )
{
    Log* log = dynamic_cast<Log*>(&os);
    if( log )
        log->disableHeader();
    return os;
}
EQ_EXPORT std::ostream& eqBase::enableHeader( std::ostream& os )
{
    Log* log = dynamic_cast<Log*>(&os);
    if( log )
        log->enableHeader();
    return os;
}
