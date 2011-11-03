
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch> 
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

#include "module.h"

#include <gpusd1/gpuInfo.h>
#include <dns_sd.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <arpa/inet.h>

namespace gpusd
{
namespace remote
{
namespace dns_sd
{
namespace
{

Module instance;

static void handleEvents( DNSServiceRef serviceRef )
{
    const int fd = DNSServiceRefSockFD( serviceRef );
    const int nfds = fd + 1;

    while( true )
    {
        fd_set fdSet;
        FD_ZERO( &fdSet );
        FD_SET( fd, &fdSet );

        const int result = select( nfds, &fdSet, 0, 0, 0 );
        if( result > 0 )
        {
            DNSServiceErrorType error = kDNSServiceErr_NoError;
            if( FD_ISSET( fd, &fdSet ))
                error = DNSServiceProcessResult( serviceRef );
            if( error )
                return;
        }
        else
        {
            std::cerr << "Select error: " << strerror( errno ) << " (" << errno
                      << ")" << std::endl;
            if( errno != EINTR )
                return;
        }
    }
}

static void browseCallback( DNSServiceRef service,
                            DNSServiceFlags flags,
                            uint32_t interfaceIndex,
                            DNSServiceErrorType errorCode,
                            const char* name,
                            const char* type,
                            const char* domain,
                            void* context )
{
    if( errorCode != kDNSServiceErr_NoError)
    {
        std::cerr << "Browse callback error" << errorCode << std::endl;
        return;
    }

    if( !( flags & kDNSServiceFlagsAdd ))
        return;

    std::cout << "Found " << name << " " << type << "." << domain
              << " interface " << interfaceIndex << std::endl;
}

}

GPUInfos Module::discoverGPUs_() const
{
    DNSServiceRef serviceRef;
    const DNSServiceErrorType error = DNSServiceBrowse( &serviceRef, 0, 0,
                                                        "_gpu-sd._tcp", "",
                                                        browseCallback, 0 );
    if( error == kDNSServiceErr_NoError )
    {
        handleEvents(serviceRef);
        DNSServiceRefDeallocate(serviceRef);
    }
    else
        std::cerr << "DNSServiceDiscovery error: " << error << std::endl;

    GPUInfos result;
    return result;
}

}
}
}

