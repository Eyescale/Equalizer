
/* Copyright (c) 2016, Stefan.Eilemann@epfl.ch
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Eyescale Software GmbH nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once
#include <hwsd/netInfo.h>
#include <hwsd/hwsd.h>
#include <hwsd/net/dns_sd/module.h>

#include <co/node.h>

namespace
{
/** Browse hwsd and report found hosts as Collage nodes */
co::Nodes discoverHWSDHosts()
{
    hwsd::net::dns_sd::Module::use();

    hwsd::FilterPtr filter = hwsd::FilterPtr( new hwsd::DuplicateFilter );
    filter = filter | new hwsd::SessionFilter( "default" );

    const hwsd::NetInfos& netInfos = hwsd::discoverNetInfos( filter );
    typedef std::unordered_map< co::uint128_t, co::NodePtr > NodeMap;
    NodeMap nodes;

    for( const hwsd::NetInfo& info : netInfos )
    {
        if( info.inetAddress.empty( ))
            continue;

        co::NodePtr node = nodes[ info.id ];
        const bool addRSP = false;//!node;
        if( !node )
        {
            node = new co::Node;
            nodes[ info.id ] = node;
        }
        if( node->getHostname().empty( ))
            node->setHostname( info.hostname );

        co::ConnectionDescriptionPtr desc = new co::ConnectionDescription;
        switch( info.type )
        {
        case hwsd::NetInfo::TYPE_ETHERNET:
            desc->type = co::CONNECTIONTYPE_TCPIP;
            desc->bandwidth = 1250000;   // 10Gbit
            break;

        case hwsd::NetInfo::TYPE_LOOPBACK:
            desc->type = co::CONNECTIONTYPE_TCPIP;
            desc->bandwidth = 12500000;   // 100Gbit
            break;

        case hwsd::NetInfo::TYPE_INFINIBAND:
            desc->type = co::CONNECTIONTYPE_RDMA;
            desc->bandwidth = 2500000;   // 20Gbit
            break;

        default:
            continue;
        }

        if( info.linkspeed != hwsd::NetInfo::defaultValue )
            desc->bandwidth = info.linkspeed * 125; // MBit -> KByte

        desc->hostname = info.inetAddress;
        node->addConnectionDescription( desc );

        if( addRSP )
        {
            desc = new co::ConnectionDescription( *desc );
            desc->type = co::CONNECTIONTYPE_RSP;
            desc->hostname.clear();
            node->addConnectionDescription( desc );
        }
    }

    co::Nodes result;
    for( auto node : nodes )
    {
        result.push_back( node.second );
        LBDEBUG << "Discovered " << *node.second << std::endl;
    }

    hwsd::net::dns_sd::Module::dispose();
    return result;
}
}
