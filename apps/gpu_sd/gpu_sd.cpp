
/*
  Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch>

  This file is part of the GPU-SD daemon.

  The GPU-SD daemon is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the Free
  Software Foundation, either version 3 of the License, or (at your option) any
  later version.

  GPU-SD is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with GPU-SD. If not, see <http://www.gnu.org/licenses/>.
*/

#include <gpusd/gpuInfo.h>
#include <gpusd/module.h>
#ifdef GPUSD_CGL
#  include <gpusd/cgl/module.h>
#endif
#ifdef GPUSD_GLX
#  include <gpusd/glx/module.h>
#endif

#include <arpa/inet.h>
#include <dns_sd.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>

using gpusd::GPUInfo;
using gpusd::GPUInfos;

static void setTXTRecordValue( TXTRecordRef& record, const size_t gpuIndex,
                               const std::string& name, const unsigned value )
{
    std::ostringstream out;
    out << "GPU" << gpuIndex << " " << name;
    const std::string string = out.str();

    out.str("");
    out << value;
    TXTRecordSetValue( &record, string.c_str(),
                       out.str().length(), out.str().c_str( ));
}

static void createTXTRecord( TXTRecordRef& record, const GPUInfos& gpus )
{
    // GPU Count=<integer>
    std::ostringstream out;
    out << gpus.size();
    TXTRecordSetValue( &record, "GPU Count",
                       out.str().length(), out.str().c_str( ));

    for( GPUInfos::const_iterator i = gpus.begin(); i != gpus.end(); ++i )
    {
        const GPUInfo& info = *i;
        const size_t index = i - gpus.begin();

        // GPU<integer> Type=GLX | WGL | WGLn | CGL
        out.str("");
        out << "GPU" << index << " Type";
        TXTRecordSetValue( &record, out.str().c_str(),
                           4, info.getName().c_str( ));

        if( info.port != GPUInfo::defaultValue )
            // GPU<integer> Port=<integer> // X11 display number, 0 otherwise
            setTXTRecordValue( record, index, "Port", info.port );

        if( info.device != GPUInfo::defaultValue )
            // GPU<integer> Device=<integer> // X11 display number, 0 otherwise
            setTXTRecordValue( record, index, "Device", info.device );

        if( info.pvp[2] > 0 && info.pvp[3] > 0 )
        {
            setTXTRecordValue( record, index, "X", info.pvp[0] );
            setTXTRecordValue( record, index, "Y", info.pvp[1] );
            setTXTRecordValue( record, index, "Width", info.pvp[2] );
            setTXTRecordValue( record, index, "Height", info.pvp[3] );
        }
    }
}

void handleEvents( DNSServiceRef serviceRef )
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

static void registerCB( DNSServiceRef service, DNSServiceFlags flags,
                        DNSServiceErrorType error, const char* name,
                        const char* type, const char* domain,
                        void* context )
{
    if( error != kDNSServiceErr_NoError)
        std::cerr << "Register callback error: " << error << std::endl;
    else
        std::cout << "Registered " << name << "." << type << "." << domain
                  << std::endl;
}

static DNSServiceErrorType registerService( const TXTRecordRef& record )
{
    DNSServiceRef serviceRef = 0;
    const DNSServiceErrorType error =
        DNSServiceRegister( &serviceRef, 0 /* flags */, 0 /* all interfaces */,
                            0 /* computer name */, "_gpu-sd._tcp",
                            0 /* default domains */, 0 /* default hostname */,
                            htons( 4242 ) /* port */,
                            TXTRecordGetLength( &record ),
                            TXTRecordGetBytesPtr( &record ),
                            registerCB, 0 /* context* */ );
    if( error == kDNSServiceErr_NoError )
    {
        handleEvents( serviceRef );
        DNSServiceRefDeallocate( serviceRef );
    }
    return error;
}

int main (int argc, const char * argv[])
{
#ifdef GPUSD_CGL
    gpusd::cgl::Module::use();
#endif
#ifdef GPUSD_GLX
    gpusd::glx::Module::use();
#endif
    const GPUInfos gpus = gpusd::Module::discoverGPUs();
    if( gpus.empty( ))
    {
        std::cerr << "No GPUs found, quitting";
        return EXIT_FAILURE;
    }

    TXTRecordRef record;
    TXTRecordCreate( &record, 0, 0 );
    createTXTRecord( record, gpus );

    DNSServiceErrorType error = registerService( record );
    std::cout << "DNSServiceDiscovery returned: " << error << std::endl;

    TXTRecordDeallocate( &record );
    return EXIT_SUCCESS;
}
