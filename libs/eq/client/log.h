
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

#include <eq/fabric/log.h>

namespace eq
{
    /** Defines selective logging classes enabled by setting EQ_LOG_TOPICS. */
    enum LogTopics
    {
        LOG_PLUGIN   = co::base::LOG_PLUGIN,       //!< Plugin usage (1)
        LOG_INIT     = fabric::LOG_INIT,       //!< Initialization (256)

        LOG_ASSEMBLY = fabric::LOG_CUSTOM << 0,   //!< Compositing tasks (1024)
        LOG_TASKS    = fabric::LOG_CUSTOM << 1,   //!< Rendering tasks (2048)
        LOG_STATS    = fabric::LOG_CUSTOM << 2,   //!< Statistic events (4096)
        LOG_SERVER   = fabric::LOG_CUSTOM << 3,   //!< Server-side logs (8192)
        LOG_CUSTOM   = 0x10000              //!< User-defined log topics (65536)
    };
}
#endif // EQ_LOG_H
