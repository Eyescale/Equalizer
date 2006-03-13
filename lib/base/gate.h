
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_GATE_H
#define EQBASE_GATE_H

#include "thread.h"

#include <pthread.h>

namespace eqBase
{
    /**
     * A gate for different thread types.
     * 
     * When the gate is open, other threads can enter.
     */
    class Gate 
    {
    public:
        /** 
         * Constructs a new gate for the given thread type.
         * 
         * The default state is closed.
         *
         * @param type the type of threads accessing the gate.
         */
        Gate( const Thread::Type type = Thread::PTHREAD );


        /** Destructs the gate. */
        ~Gate();

        /** 
         * Opens the gate and lets everybody enter.
         */
        void up();

        /** 
         * Closes the gate.
         */
        void down();

        /** 
         * Enters the gate.
         *
         * Blocks the caller until the gate is opened.
         */
        void enter();

    private:
        Thread::Type _type;

        union
        {
            struct
            {
                pthread_cond_t  cond;
                pthread_mutex_t mutex;

            } _pthread;
        };

        volatile bool _open;
    };
}

#endif //EQBASE_GATE_H
