
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

#ifndef EQ_LOG_H
#define EQ_LOG_H

#include <eq/net/log.h>

namespace eq
{
    /** Defines selective logging classes enabled by setting EQ_LOG_TOPICS. */
    enum LogTopics
    {
        LOG_PLUGIN   = base::LOG_PLUGIN,       //!< Plugin usage (1)

        LOG_ASSEMBLY = net::LOG_CUSTOM << 0,   //!< Log compositing tasks (256)
        LOG_TASKS    = net::LOG_CUSTOM << 1,   //!< Log rendering tasks (512)
        LOG_STATS    = net::LOG_CUSTOM << 3,   //!< Log statistic events (2048)
        LOG_INIT     = net::LOG_CUSTOM << 4,   //!< Log initialization (4096)
        LOG_SERVER   = net::LOG_CUSTOM << 5,   //!< Server-side logging (8192)
        LOG_CUSTOM   = 0x10000              //!< User-defined log topics (65536)
    };
}
#endif // EQ_LOG_H
