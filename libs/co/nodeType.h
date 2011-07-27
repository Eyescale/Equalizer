
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef CO_NODETYPE_H
#define CO_NODETYPE_H

#include <iostream>

namespace co
{
    /** Node types to identify connecting nodes. */
    enum NodeType
    {
        NODETYPE_CO_INVALID,         //!< Invalid type
        NODETYPE_CO_NODE,            //!< A plain co::Node
        NODETYPE_CO_USER = 0x100     //!< Application-specific types
    };

    inline std::ostream& operator << ( std::ostream& os, const NodeType& type )
    {
        os << static_cast< unsigned >( type );
        return os;
    }
}

#endif // CO_NODETYPE_H
