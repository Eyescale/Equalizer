
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

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
        SwapBarrier() : _nvSwapGroup( 0 ), _nvSwapBarrier( 0 ) {}

        /** @name Data Access. */
        //*{
        void setName( const std::string& name ) { _name = name; }
        const std::string getName() const { return _name; }
        //*}

        const uint32_t getNVSwapGroup() const   { return _nvSwapGroup ; }
        void setNVSwapGroup( uint32_t nvGroup ) { _nvSwapGroup = nvGroup; }

        const uint32_t getNVSwapBarrier() const { return _nvSwapBarrier; }
        void setNVSwapBarrier( uint32_t nvBarrier ) 
            { _nvSwapBarrier = nvBarrier; }

        bool isNvSwapBarrier() const
            { return ( _nvSwapBarrier || _nvSwapGroup ); }

    private:
        std::string _name;

        uint32_t _nvSwapGroup;
        uint32_t _nvSwapBarrier;
    };

    std::ostream& operator << ( std::ostream& os, const SwapBarrier* barrier );
}
}
#endif // EQSERVER_SWAPBARRIER_H
