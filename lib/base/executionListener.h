
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_EXECUTIONLISTENER_H
#define EQBASE_EXECUTIONLISTENER_H

#include "base.h"

namespace eqBase
{
    /**
     * A listener interface to monitor execution unit (Thread, Process) state
     * changes.
     */
    class ExecutionListener
    {
    public:
        virtual ~ExecutionListener() {}

        /**
         * Notify that a new execution started.
         * @sa Execution::start
         */
        virtual void notifyExecutionStarted(){};

        /**
         * Notify that the execution is about to stop.
         * @sa Execution::exit
         */
        virtual void notifyExecutionStopping(){};        
    };
}

#endif //EQBASE_EXECUTIONLISTENER_H
