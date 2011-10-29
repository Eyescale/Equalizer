
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

#include <gpusd1/local/module.h>

#include <dns_sd.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>

#ifdef _WIN32
#  include <process.h>
typedef int pid_t;
#  define getpid _getpid
#  define strcasecmp _stricmp
#  define snprintf _snprintf
#else
#  include <sys/time.h>
#  include <unistd.h>
#  include <arpa/inet.h>
#endif

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

static DNSServiceErrorType registerService()
{
    DNSServiceRef serviceRef = 0;
    const DNSServiceErrorType error =
        DNSServiceRegister( &serviceRef, 0 /* flags */, 0 /* all interfaces */,
                            0 /* computer name */, "_gpu-sd._tcp",
                            0 /* default domains */, 0 /* default hostname */,
                            htons( 4242 ) /* port */,
                            0, // text record length
                            0, // text record
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
    DNSServiceErrorType error = registerService();
    std::cout << "DNSServiceDiscovery returned: " << error << std::endl;
    return EXIT_SUCCESS;
}
