
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_SEMA_H
#define EQBASE_SEMA_H

#include <eq/base/base.h>

namespace eqBase
{
    class SemaPrivate;

    /**
     * A semaphore primitive.
     */
    class Sema 
    {
    public:
        /** 
         * Constructs a new semaphore.
         */
        Sema();


        /** Destructs the sema. */
        ~Sema();

        /** 
         * Post (v) operation.
         */
        void post();

        /** 
         * Wait (p) operation. 
         */
        void wait();

        /** 
         * Bulk post or wait.
         * 
         * @param delta the resource delta to be applied.
         */
        void adjust( const int delta );

    private:
        SemaPrivate*   _data;
        uint32_t       _value;
    };
}

#endif //EQBASE_SEMA_H
