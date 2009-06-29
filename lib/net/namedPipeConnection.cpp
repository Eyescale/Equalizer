
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <limits>

#include "namedPipeConnection.h"

#include "connectionDescription.h"
#include "node.h"

#include <eq/base/base.h>
#include <eq/base/log.h>

#include <errno.h>
#include <sstream>
#include <string.h>
#include <sys/types.h>

#ifdef WIN32
#  define EQ_SOCKET_ERROR getErrorString( GetLastError( )) << \
    "(" << GetLastError() << ")"
#  include "namedPipeConnectionWin32.cpp"

#else
#  define EQ_PIPE_ERROR strerror( errno ) << "(" << errno << ")"

#  include "namedPipeConnectionPosix.cpp"
#endif

namespace eq
{
namespace net
{

}
}