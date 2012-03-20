
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef CO_COMMANDS_H
#define CO_COMMANDS_H

namespace co
{
    enum
    {
        CMD_NODE_COMMAND, //!< A custom node command (NodeCommandPacket)
        CMD_NODE_INTERNAL, //!< @internal
        CMD_NODE_CUSTOM = 50,  //!< Commands for subclasses of Node start here
        CMD_OBJECT_CUSTOM = 10 //!< Commands for subclasses of Object start here
    };
}

#endif // CO_COMMANDS_H

