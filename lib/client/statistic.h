
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_STATISTIC_H
#define EQ_STATISTIC_H

#include <eq/base/base.h>

#include <iostream>

namespace eq
{
    /** A statistics event. */
    struct Statistic
    {
        enum Type // Also update string table in statistic.cpp
        {
            NONE = 0,
            CHANNEL_CLEAR,
            CHANNEL_DRAW,
            CHANNEL_DRAW_FINISH,
            CHANNEL_ASSEMBLE,
            CHANNEL_READBACK,
            CHANNEL_WAIT_FRAME,
            WINDOW_FINISH,
            WINDOW_SWAP_BARRIER,
            WINDOW_SWAP,
            WINDOW_THROTTLE_FRAMERATE,
            PIPE_IDLE,
            FRAME_TRANSMIT,
            FRAME_COMPRESS,
            FRAME_DECOMPRESS,
            CONFIG_START_FRAME,
            CONFIG_FINISH_FRAME,
            CONFIG_WAIT_FINISH_FRAME,
            TYPE_ALL          // must be last
        };

        Type     type;
        uint32_t frameNumber;
        uint32_t task;
        union
        {
            int64_t  startTime;
            int64_t  idleTime;
        };
        union
        {
            int64_t  endTime;
            int64_t  totalTime;
        };
        float ratio; // compression ratio
        char resourceName[32];
    };

    EQ_EXPORT std::ostream& operator << ( std::ostream&, const Statistic& );
}

#endif // EQ_STATISTIC_H

