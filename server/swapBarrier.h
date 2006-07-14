
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_SWAPBARRIER_H
#define EQS_SWAPBARRIER_H

#include <eq/net/barrier.h>
#include <eq/net/node.h>

namespace eqs
{
    class SwapBarrier
    {
    public:
        /** 
         * Constructs a new SwapBarrier.
         */
        SwapBarrier() {}

        /** @name Data Access. */
        //*{
        void setName( const std::string& name ) { _name = name; }
        const std::string getName() const { return _name; }
        //*}

    private:
        std::string     _name;
        eqNet::Barrier* _barrier;
    };

    std::ostream& operator << ( std::ostream& os, const SwapBarrier* barrier );
}

#endif // EQS_SWAPBARRIER_H
