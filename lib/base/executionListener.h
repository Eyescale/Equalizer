
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_EXECUTIONLISTENER_H
#define EQBASE_EXECUTIONLISTENER_H

#include <eq/base/base.h>

namespace eqBase
{
    /**
     * A listener interface to monitor execution unit (Thread, Process) state
     * changes.
     */
    class EQ_EXPORT ExecutionListener
    {
    public:
        virtual ~ExecutionListener() {}

        /**
         * Notify that a new execution unit started.
         */
        virtual void notifyExecutionStarted(){};

        /**
         * Notify that the execution unit is about to stop.
         */
        virtual void notifyExecutionStopping(){};        
    };
}

#endif //EQBASE_EXECUTIONLISTENER_H
