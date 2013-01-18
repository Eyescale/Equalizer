
/* Copyright (c) 2010-2011, Stefan Eilemann <eile@eyescale.ch> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
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

#ifndef EQSERVER_STATE_H
#define EQSERVER_STATE_H

#include <lunchbox/log.h>
#include <lunchbox/types.h>

namespace eq
{
namespace server
{
    /**
     * Running state of a resource entity.
     * <img src="http://www.equalizergraphics.com/documents/design/images/serverState.png">
     */
    enum State
    {
        STATE_UNUSED = LB_BIT1,      //!< next: STOPPED
        STATE_STOPPED = LB_BIT2,      //!< next: INITIALIZING or UNUSED
        STATE_INITIALIZING = LB_BIT3, //!< next: INIT_FAILED or INIT_SUCCESS
        STATE_INIT_SUCCESS = LB_BIT4, //!< next: RUNNING
        STATE_INIT_FAILED = LB_BIT5,  //!< next: EXITING
        STATE_RUNNING = LB_BIT6,      //!< next: EXITING
        STATE_EXITING = LB_BIT7,      //!< next: EXIT_FAILED or EXIT_SUCCESS
        STATE_EXIT_SUCCESS = LB_BIT8, //!< next: STOPPED or FAILED
        STATE_EXIT_FAILED = LB_BIT9,  //!< next: STOPPED or FAILED
        STATE_FAILED = LB_BIT10,       //!< next: STOPPED
        STATE_DELETE = LB_BIT16       //!< additional modifier
    };

    inline std::ostream& operator << ( std::ostream& os, const State& state_ )
    {
        const uint32_t state = state_ & ~STATE_DELETE;

        os << ( state == STATE_STOPPED ? "stopped" :
                state == STATE_INITIALIZING ? "initializing" :
                state == STATE_INIT_SUCCESS ? "init ok" :
                state == STATE_INIT_FAILED ? "init failed" :
                state == STATE_RUNNING ? "running" :
                state == STATE_EXITING ? "exiting" :
                state == STATE_EXIT_SUCCESS ? "exit ok" :
                state == STATE_EXIT_FAILED ? "exit failed" :
                state == STATE_FAILED ? "failed" : "ERROR" );
        return os;
    }
}
}
#endif // EQ_STATE_H
