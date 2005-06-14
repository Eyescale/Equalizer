
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_LOG_H
#define EQBASE_LOG_H

#include <iostream>

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
        LOG_INFO
    };

    /** The current log level. */
    extern int logLevel;
}

#define ERROR \
    (eqBase::logLevel >= eqBase::LOG_ERROR) && std::cout << "   Error: " 
#define WARN \
    (eqBase::logLevel >= eqBase::LOG_WARN)   && std::cout << "Warning: " 
#define INFO \
    (eqBase::logLevel >= eqBase::LOG_INFO)   && std::cout << "   Info: " 

#endif //EQBASE_LOG_H
