
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef CO_LOG_H
#define CO_LOG_H

#include <co/base/log.h>

namespace co
{
    enum LogTopics
    {
        LOG_OBJECTS = co::base::LOG_CUSTOM << 0,  // 16
        LOG_BARRIER = co::base::LOG_CUSTOM << 1,  // 32
        LOG_RSP     = co::base::LOG_CUSTOM << 2,  // 64
        LOG_PACKETS = co::base::LOG_CUSTOM << 3,  // 128
        LOG_CUSTOM  = co::base::LOG_CUSTOM << 4   // 256
    };
}
#endif // CO_LOG_H
