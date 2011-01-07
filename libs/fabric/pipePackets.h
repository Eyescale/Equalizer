
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQFABRIC_PIPEPACKETS_H
#define EQFABRIC_PIPEPACKETS_H

#include <eq/fabric/packets.h> // base structs

/** @cond IGNORE */
namespace eq
{
namespace fabric
{
    struct PipeNewWindowPacket : public PipePacket
    {
        PipeNewWindowPacket()
            {
                command = fabric::CMD_PIPE_NEW_WINDOW;
                size    = sizeof( PipeNewWindowPacket );
            }

        uint32_t requestID;
    };

    struct PipeNewWindowReplyPacket : public PipePacket
    {
        PipeNewWindowReplyPacket( const PipeNewWindowPacket* request )
                : requestID( request->requestID )
            {
                command = fabric::CMD_PIPE_NEW_WINDOW_REPLY;
                size    = sizeof( PipeNewWindowReplyPacket );
            }

        co::base::UUID windowID;
        const uint32_t requestID;
    };
}
}
/** @endcond */
#endif //EQFABRIC_PIPEPACKETS_H
