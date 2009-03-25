
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include <pthread.h>

#include "log.h"
#include "perThread.h"
#include "thread.h"

using namespace std;

#ifdef WIN32_VC
#  define atoll _atoi64
#endif


namespace eq
{
namespace base
{

static int      getLogLevel();
static unsigned getLogTopics();

class LogTable
{
public:
    LogTable( const LogLevel _level, const string& _name )
            : level( _level ), name( _name ) {}

    LogLevel level;
    string   name;
};

#define LOG_TABLE_ENTRY( name ) LogTable( LOG_ ## name, string( #name ))
#define LOG_TABLE_SIZE (5)

static LogTable _logTable[ LOG_TABLE_SIZE ] =
{
    LOG_TABLE_ENTRY( ERROR ),
    LOG_TABLE_ENTRY( WARN ),
    LOG_TABLE_ENTRY( INFO ),
    LOG_TABLE_ENTRY( VERB ),
    LOG_TABLE_ENTRY( ALL )
};

EQ_EXPORT int        Log::level  = getLogLevel();
EQ_EXPORT unsigned   Log::topics = getLogTopics();
EQ_EXPORT Clock      LogBuffer::_clock;

static PerThread< Log > _logInstance;

#ifdef NDEBUG
    static std::ostream* _logStream = &std::cout;
#else
    static std::ostream* _logStream = &std::cerr;
#endif

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

    if( getLogLevel() == LOG_ALL )
        return 0xffffffffu;

    return 0;
}

EQ_EXPORT Log& Log::instance( const char* subdir, const char* file,
                              const int line )
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

EQ_EXPORT void Log::exit()
{
    Log* log = _logInstance.get();
    _logInstance = 0;
    delete log;
}

EQ_EXPORT void Log::setOutput( std::ostream& stream )
{
    _logStream = &stream;
    exit();
}


std::ostream& Log::getOutput()
{
    return *_logStream;
}


LogBuffer::int_type LogBuffer::overflow( LogBuffer::int_type c )
{
    if( c == EOF )
        return EOF;

    if( _newLine )
    {
        if( !_noHeader )
        {
            _stringStream << getpid()  << " " 
                          << eq::base::Thread::getSelfThreadID()
                          << " " << _file << ":" << _line << " ";
#ifndef NDEBUG
            const int prec  = _stringStream.precision();

            _stringStream.precision( 4 );
            _stringStream << std::setw(5) << _clock.getMSf() << " ";
            _stringStream.precision( prec );
#endif
        }

        for( int i=0; i<_indent; ++i )
            _stringStream << "    ";
        _newLine = false;
    }

    _stringStream << (char)c;
    return c;
}


EQ_EXPORT std::ostream& indent( std::ostream& os )
{
    Log* log = dynamic_cast<Log*>(&os);
    if( log )
        log->indent();
    return os;
}
EQ_EXPORT std::ostream& exdent( std::ostream& os )
{
    Log* log = dynamic_cast<Log*>(&os);
    if( log )
        log->exdent();
        return os;
}

EQ_EXPORT std::ostream& disableFlush( std::ostream& os )
{
    Log* log = dynamic_cast<Log*>(&os);
    if( log )
        log->disableFlush();
    return os;
}
EQ_EXPORT std::ostream& enableFlush( std::ostream& os )
{
    Log* log = dynamic_cast<Log*>(&os);
    if( log )
        log->enableFlush();
    return os;
}
EQ_EXPORT std::ostream& forceFlush( std::ostream& os )
{
    Log* log = dynamic_cast<Log*>(&os);
    if( log )
        log->forceFlush();
    return os;
}

EQ_EXPORT std::ostream& disableHeader( std::ostream& os )
{
    Log* log = dynamic_cast<Log*>(&os);
    if( log )
        log->disableHeader();
    return os;
}
EQ_EXPORT std::ostream& enableHeader( std::ostream& os )
{
    Log* log = dynamic_cast<Log*>(&os);
    if( log )
        log->enableHeader();
    return os;
}

}
}
