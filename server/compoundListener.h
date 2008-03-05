
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_COMPOUND_LISTENER_H
#define EQS_COMPOUND_LISTENER_H

#include <eq/base/base.h>

namespace eqs
{
    class Compound;

    /** A listener on various compound operations. */
    class CompoundListener
    {
    public:
        virtual ~CompoundListener(){}

        /** 
         * Notify that the compound tree below and including compound is about
         * to be updated.
         *
         * Called on each compound of the tree during update.
         *
         * @param compound the root compound of the tree to be updated.
         * @param frameNumber the new frame number.
         */
        virtual void notifyUpdatePre( Compound* compound, 
                                      const uint32_t frameNumber ) {}
    };
}
#endif // EQS_COMPOUND_LISTENER_H
