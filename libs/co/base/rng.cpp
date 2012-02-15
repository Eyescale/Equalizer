
/* Copyright (c) 2010-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "rng.h"

#include <cstdlib>

namespace co
{
namespace base
{
#ifdef Linux
int RNG::_fd = -1;
#elif defined (_WIN32)
HCRYPTPROV RNG::_provider = 0;
#endif

bool RNG::_init()
{
#ifdef Darwin
    srandomdev();
#elif defined (Linux)
    static int fd = -1; // prevent static initializer fiasco
    if( fd >= 0 )
        return true;

    fd = ::open( "/dev/urandom", O_RDONLY );
    if( fd >= 0 )
        ::atexit( RNG::_exit );
    else
    {
        EQERROR << "Failed to open /dev/urandom: " << sysError << std::endl;
        return false;
    }
    _fd = fd;

#elif defined (_WIN32)

    static HCRYPTPROV provider = 0; // prevent static initializer fiasco
    if( provider )
        return true;

    if( CryptAcquireContext( &provider, 0, 0, PROV_RSA_FULL,
                              CRYPT_VERIFYCONTEXT ) || !provider )
    {
        ::atexit( RNG::_exit );
    }
    else
    {
        EQERROR << "Failed to acquire crypto context: " << sysError <<std::endl;
        return false;
    }

    _provider = provider;
#endif
    return true;
}

void RNG::_exit()
{
#ifdef Linux
    if( _fd >= 0 )
    {
        ::close( _fd );
        _fd = -1;
    }
#elif defined (_WIN32)
    if( _provider && !CryptReleaseContext( _provider, 0 ))
        EQERROR << "Failed to release crypto context: " << sysError
                << std::endl;
    _provider = 0;
#endif
}

}
}
