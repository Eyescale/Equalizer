
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_BARRIER_H
#define EQBASE_BARRIER_H

#include <eq/base/base.h>

namespace eq
{
namespace base
{
    class BarrierPrivate;

    /**
     * A barrier primitive.
     */
    class EQ_EXPORT Barrier 
    {
    public:
        /** 
         * Constructs a new barrier.
         */
        Barrier();

        /** Destructs the barrier. */
        ~Barrier();

        /** 
         * Enters the barrier.
         * 
         * @param size the size of the barrier, i.e., the number of
         *             participants.
         * @return the position in which the barrier was entered.
         */
        size_t enter( const size_t size );

    private:
        BarrierPrivate *_data;
    };
}

}
#endif //EQBASE_BARRIER_H
