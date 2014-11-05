
/* Copyright (c) 2010-2012, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef EQFABRIC_LOG_H
#define EQFABRIC_LOG_H

#include <co/log.h>

namespace eq
{
namespace fabric
{
    /** Defines selective logging classes enabled by setting LB_LOG_TOPICS. */
    enum LogTopics
    {
        LOG_INIT     = co::LOG_CUSTOM << 0, //!< Log initialization (512)
        LOG_ASSEMBLY = co::LOG_CUSTOM << 1, //!< Compositing tasks (1024)
        LOG_TASKS    = co::LOG_CUSTOM << 2, //!< Rendering tasks (2048)
        LOG_LB1      = co::LOG_CUSTOM << 3, // 4096
        LOG_LB2      = co::LOG_CUSTOM << 4, // 8192
        LOG_VIEW     = co::LOG_CUSTOM << 5, // 16384
        LOG_CUSTOM   = co::LOG_CUSTOM << 6  //!< 32768
    };
}
}
#endif // EQFABRIC_LOG_H
