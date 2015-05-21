
/* Copyright (c) 2006-2013, Stefan Eilemann <eile@equalizergraphics.com>
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
#include <pression/log.h>

namespace eq
{
    /** Defines selective logging classes enabled by setting LB_LOG_TOPICS. */
    enum LogTopics
    {
        LOG_BUG      = lunchbox::LOG_BUG,       //!< Potential bugs (4)
        LOG_PLUGIN   = pression::LOG_PLUGIN,    //!< Plugin usage (16)
        LOG_INIT     = fabric::LOG_INIT,        //!< Initialization (512)
        LOG_ASSEMBLY = fabric::LOG_ASSEMBLY,    //!< Compositing tasks (1024)
        LOG_TASKS    = fabric::LOG_TASKS,       //!< Rendering tasks (2048)

        LOG_STATS    = fabric::LOG_CUSTOM << 0, //!< Statistic events (32768)
        LOG_CUSTOM   = 0x10000             //!< User-defined log topics (65536)
    };
}
#endif // EQ_LOG_H
