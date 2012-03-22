
/* Copyright (c) 2007-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <lunchbox/debug.h>
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
        CONNECTIONTYPE_RDMA,      //!< Infiniband RDMA CM
        CONNECTIONTYPE_UDT,       //!< UDT connection
        CONNECTIONTYPE_MULTICAST = 0x100,
        CONNECTIONTYPE_PGM,       //!< IP-based multicast connection (PGM)
        CONNECTIONTYPE_RSP        //!< UDP-based reliable stream protocol
    };

    inline std::ostream& operator << ( std::ostream& os,
                                       const ConnectionType& type )
    {
        switch( type )
        {
            case CONNECTIONTYPE_TCPIP: return os << "TCPIP";
            case CONNECTIONTYPE_SDP: return os << "SDP";
            case CONNECTIONTYPE_PIPE: return os << "ANON_PIPE";
            case CONNECTIONTYPE_NAMEDPIPE: return os << "PIPE";
            case CONNECTIONTYPE_IB: return os << "IB";
            case CONNECTIONTYPE_PGM: return os << "PGM";
            case CONNECTIONTYPE_RSP: return os << "RSP";
            case CONNECTIONTYPE_NONE: return os << "NONE";
            case CONNECTIONTYPE_RDMA: return os << "RDMA";
            case CONNECTIONTYPE_UDT: return os << "UDT";
                
            default:
                EQASSERTINFO( false, "Not implemented" );
                return os << "ERROR";
        }
        return os;
    }
}

#endif // CO_CONNECTIONTYPE_H
