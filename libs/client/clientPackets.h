
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

#ifndef EQ_CLIENTPACKETS_H
#define EQ_CLIENTPACKETS_H

#include <eq/packets.h> // base structs

/** @cond IGNORE */
namespace eq
{
  struct ClientExitPacket : public ClientPacket
    {
        ClientExitPacket()
            {
                command = fabric::CMD_CLIENT_EXIT;
                size    = sizeof( ClientExitPacket );
            }
    };
}
/** @endcond */
#endif //EQ_CLIENTPACKETS_H
