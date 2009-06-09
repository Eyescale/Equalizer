
/* 
 * Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQNET_COMMANDRESULT_H
#define EQNET_COMMANDRESULT_H

#include <iostream>

namespace eq
{
namespace net
{
    enum CommandResult
    {
        COMMAND_HANDLED,     //!< The command was handled
        COMMAND_DISCARD,     //!< Discard command, used by Objects
        COMMAND_ERROR        //!< An unrecoverable error occured
    };
    inline std::ostream& operator << ( std::ostream& os, 
                                       const CommandResult& result )
    {
        os << ( result == COMMAND_HANDLED ? "handled" :
                result == COMMAND_DISCARD ? "discard" :
                result == COMMAND_ERROR   ? "error" : "UNKNOWN" );
        return os;
    }
}
}

#endif // EQNET_COMMANDRESULT_H
