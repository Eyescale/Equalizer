
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_LOG_H
#define EQBASE_LOG_H

#include <iostream>
#include <pthread.h>
#include <unistd.h>

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
}

#define LOG_EXTRA  << getpid()  << "." << pthread_self() << " " << __FILE__ \
  << " " << __LINE__ << ": "

#define ERROR (eqBase::Log::level >= eqBase::LOG_ERROR) && \
    std::cout << "[E]" LOG_EXTRA
#define WARN  (eqBase::Log::level >= eqBase::LOG_WARN)  && \
    std::cout << "[W]"  LOG_EXTRA
#define INFO  (eqBase::Log::level >= eqBase::LOG_INFO)  && \
    std::cout << "[I]"  LOG_EXTRA
#define VERB  (eqBase::Log::level >= eqBase::LOG_VERBATIM)  && \
    std::cout << "[V]"  LOG_EXTRA

#endif //EQBASE_LOG_H
