
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_LOG_H
#define EQBASE_LOG_H

#include <iostream>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#ifndef NDEBUG
#  include "clock.h"
#endif

/**
 * @namespace eqBase
 * @brief Namespace for basic Equalizer utility code.
 */
namespace eqBase
{
    /** The logging levels. */
    enum LogLevel
    {
        LOG_ERROR,
        LOG_WARN,
        LOG_INFO,
        LOG_VERBATIM
    };

    /** The logging class */
    class Log {
    public:
        /** The current log level. */
        static int level;
    };

    inline void dumpStack( std::ostream& os )
    {
#ifdef backtrace
        void* trace[256];
        const int n = backtrace(trace, 256);
        if (!n)
            return;

        const char** strings = backtrace_symbols (trace, n);
 
        for (int i = 0; i < n; ++i)
            os << i << ": " << strings[i] << std::endl;
        if (strings)
            free (strings);
#else // backtrace
#endif // backtrace
    }
}

#ifdef WIN32
#define SUBDIR " "
#endif

#ifdef NDEBUG
#  define LOG_EXTRA << getpid()  << "." << pthread_self()  \
        << " " << SUBDIR <<"/" << __FILE__ << ":" << (int)__LINE__ << " " 
#else

extern eqBase::Clock eqLogClock;

#  define LOG_EXTRA << getpid()  << "." << pthread_self()               \
        << " " << SUBDIR <<"/" << __FILE__ << ":" << (int)__LINE__ << " t:" \
        << eqLogClock.getMSf() << " "
#endif

#define EQERROR (eqBase::Log::level >= eqBase::LOG_ERROR) && \
    std::cout << "[E]" LOG_EXTRA
#define EQWARN  (eqBase::Log::level >= eqBase::LOG_WARN)  && \
    std::cout << "[W]"  LOG_EXTRA
#define EQINFO  (eqBase::Log::level >= eqBase::LOG_INFO)  && \
    std::cout << "[I]"  LOG_EXTRA
#define EQVERB  (eqBase::Log::level >= eqBase::LOG_VERBATIM)  && \
    std::cout << "[V]"  LOG_EXTRA

#define LOG_MATRIX4x4( m ) endl \
 << "  " << m[0] << " " << m[1] << " " << m[2] << " " << m[3] << " " << endl \
 << "  " << m[4] << " " << m[5] << " " << m[6] << " " << m[7] << " " << endl \
 << "  " << m[8] << " " << m[9] << " " << m[10] << " " << m[11] << " " << endl \
 << "  " << m[12] << " " << m[13] << " " << m[14] << " " << m[15] << " " << endl

#define LOG_VECTOR6( v ) \
    "[" << v[0] << " " << v[1] << " " << v[2] << " " << v[3] << " " \
    << v[4] << " " << v[5] << "]"

#endif //EQBASE_LOG_H
