
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

#ifndef CO_CONNECTIONTYPE_H
#define CO_CONNECTIONTYPE_H

#include <co/base/debug.h>
#include <iostream>

namespace co
{
    /** The supported network protocols. */
    enum ConnectionType
    {
        CONNECTIONTYPE_NONE = 0,
        CONNECTIONTYPE_TCPIP,     //!< TCP/IP sockets
        CONNECTIONTYPE_SDP,       //!< SDP sockets (InfiniBand)
        CONNECTIONTYPE_PIPE,      //!< pipe() based uni-directional connection
        CONNECTIONTYPE_NAMEDPIPE, //!< Named pipe based bidirectional connection
        CONNECTIONTYPE_IB,        //!< Infiniband based RDMA
        CONNECTIONTYPE_UDP,       //!< Unreliable UDP connection
        CONNECTIONTYPE_MULTICAST = 0x100,
        CONNECTIONTYPE_MCIP = CONNECTIONTYPE_MULTICAST, //!< IP-based multicast
        CONNECTIONTYPE_PGM,       //!< IP-based multicast connection (PGM)
        CONNECTIONTYPE_RSP        //!< UDP-based reliable stream protocol
    };

    inline std::ostream& operator << ( std::ostream& os,
                                       const ConnectionType& type )
    {
        switch( type )
        {
            case CONNECTIONTYPE_TCPIP:
                os << "TCPIP";
                break;

            case CONNECTIONTYPE_SDP:
                os << "SDP";
                break;

            case CONNECTIONTYPE_PIPE:
                os << "ANON_PIPE";
                break;

            case CONNECTIONTYPE_NAMEDPIPE:
                os << "PIPE";
                break;

            case CONNECTIONTYPE_IB:
                os << "IB";
                break;

            case CONNECTIONTYPE_UDP:
                os << "UDP";
                break;

            case CONNECTIONTYPE_MCIP:
                os << "MCIP";
                break;

            case CONNECTIONTYPE_PGM:
                os << "PGM";
                break;

            case CONNECTIONTYPE_RSP:
                os << "RSP";
                break;
                
            default:
                EQASSERTINFO( false, "Not implemented" );
                os << "ERROR";
                break;
        }
        return os;
    }
}

#endif // CO_CONNECTIONTYPE_H
