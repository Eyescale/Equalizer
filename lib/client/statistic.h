
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

#include <eq/client/types.h>
#include <eq/base/base.h>

#include <iostream>

namespace eq
{
    /** A statistics event. */
    struct Statistic
    {
        enum Type // Also update string and color table in statistic.cpp
        {
            NONE = 0,
            CHANNEL_CLEAR,
            CHANNEL_DRAW,
            CHANNEL_DRAW_FINISH,
            CHANNEL_ASSEMBLE,
            CHANNEL_WAIT_FRAME,
            CHANNEL_READBACK,
            CHANNEL_VIEW_FINISH,
            WINDOW_FINISH,
            WINDOW_THROTTLE_FRAMERATE,
            WINDOW_SWAP_BARRIER,
            WINDOW_SWAP,
            PIPE_IDLE,
            FRAME_TRANSMIT,
            FRAME_COMPRESS,
            FRAME_RECEIVE,
            CONFIG_START_FRAME,
            CONFIG_FINISH_FRAME,
            CONFIG_WAIT_FINISH_FRAME,
            ALL          // must be last
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

        static const std::string& getName( const Type type );
        static const Vector3f& getColor( const Type type );
    };

    EQ_EXPORT std::ostream& operator << ( std::ostream&, const Statistic& );
}

#endif // EQ_STATISTIC_H

