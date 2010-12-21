
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQSERVER_LOG_H
#define EQSERVER_LOG_H

#include <eq/log.h>

namespace eq
{
namespace server
{
    enum LogTopics
    {
        LOG_LB1      = LOG_SERVER << 0,   // 8192
        LOG_LB2      = LOG_SERVER << 1,   // 16384
        LOG_VIEW     = LOG_SERVER << 2    // 32768
    };
}
}
#endif // EQSERVER_LOG_H
