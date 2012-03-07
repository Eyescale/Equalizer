
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder  <cedric.stalder@gmail.com>
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

#ifndef EQFABRIC_WINDOWPACKETS_H
#define EQFABRIC_WINDOWPACKETS_H

#include <eq/fabric/packets.h> // base structs

/** @cond IGNORE */
namespace eq
{
namespace fabric
{
    struct WindowNewChannelPacket : public WindowPacket
    {
        WindowNewChannelPacket( const uint32_t requestID_ )
                : requestID( requestID_ )
                , pad( 0 )
            {
                command = CMD_WINDOW_NEW_CHANNEL;
                size    = sizeof( WindowNewChannelPacket );
            }

        const uint32_t requestID;
        const uint32_t pad;
    };

    struct WindowNewChannelReplyPacket : public WindowPacket
    {
        WindowNewChannelReplyPacket( const WindowNewChannelPacket* request )
                : requestID( request->requestID )
                , pad( 0 )
            {
                command = CMD_WINDOW_NEW_CHANNEL_REPLY;
                size    = sizeof( WindowNewChannelReplyPacket );
            }

        co::base::UUID channelID;
        const uint32_t requestID; 
        const uint32_t pad;       
    };
}
}
/** @endcond */
#endif //EQFABRIC_WINDOWPACKETS_H
