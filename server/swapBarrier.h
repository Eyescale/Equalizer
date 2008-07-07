
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_SWAPBARRIER_H
#define EQSERVER_SWAPBARRIER_H

#include <eq/net/node.h>

namespace eq
{
namespace server
{
    class Barrier;

    /**
     * A swapbarrier is set on a Compound to synchronize the swap buffer between
     * windows.
     *
     * Swap barriers with the same name are linked together, that is, all
     * compounds holding a swap barrier with the same name synchronize their
     * window's swap command. The same Barrier is set up for all swap barriers
     * of the same name during compound init.
     */
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
        std::string _name;
    };

    std::ostream& operator << ( std::ostream& os, const SwapBarrier* barrier );
}
}
#endif // EQSERVER_SWAPBARRIER_H
