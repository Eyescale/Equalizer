
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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
        SwapBarrier(): _nvGroup( 0 )
                     , _nvBarrier( 0 ) {}

        /** @name Data Access. */
        //*{
        void setName( const std::string& name ) { _name = name; }
        const std::string getName() const { return _name; }
        //*}

        const int getNVGroup()const { return _nvGroup ; }
        void setNVGroup( int nvGroup ) { _nvGroup = nvGroup; }

        const int getNVBarrier()const { return _nvBarrier; }
        void setNVBarrier( int nvBarrier )  { _nvBarrier = nvBarrier; }

        bool isNvSwapBarrier()const { return ( _nvBarrier || _nvGroup ); }
    private:
        std::string _name;
        int _nvGroup;
        int _nvBarrier;

    };

    std::ostream& operator << ( std::ostream& os, const SwapBarrier* barrier );
}
}
#endif // EQSERVER_SWAPBARRIER_H
