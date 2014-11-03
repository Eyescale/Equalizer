
/* Copyright (c) 2008-2012, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQS_CHANNEL_LISTENER_H
#define EQS_CHANNEL_LISTENER_H

#include "types.h"

namespace eq
{
namespace server
{
    /** A listener on various channel operations. */
    class ChannelListener
    {
    public:
        virtual ~ChannelListener(){}

        /**
         * Notify that the channel has received new load data.
         *
         * @param channel the channel
         * @param frameNumber the frame number.
         * @param statistics the frame's statistic.
         * @param region the draw area wrt the channels viewport
         */
        virtual void notifyLoadData( Channel* channel,
                                     const uint32_t frameNumber,
                                     const Statistics& statistics,
                                     const Viewport& region ) = 0;
    };
}
}
#endif // EQS_CHANNEL_LISTENER_H

