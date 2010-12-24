
/* Copyright (c) 2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

namespace co
{
namespace base
{

#ifdef Linux
int RNG::_fd = -1;
#endif

bool RNG::_init()
{
#ifdef Linux
    EQASSERT( _fd == -1 );
    _fd = ::open( "/dev/urandom", O_RDONLY );
    if( _fd < 0 )
    {
        EQERROR << "Failed to open /dev/urandom :" << sysError << std::endl;
        EQASSERT( _fd >= 0 );
        return false;
    }
#endif

    return true;
}

bool RNG::_exit()
{
#ifdef Linux
    if( _fd > 0 )
    {
        ::close( _fd );
        _fd = -1;
    }
#endif

    return true;
}

}
}
