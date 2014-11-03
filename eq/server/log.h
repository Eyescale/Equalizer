
/* Copyright (c) 2006-2011, Stefan Eilemann <eile@equalizergraphics.com>
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

#include <eq/fabric/log.h>

namespace eq
{
namespace server
{
    enum LogTopics
    {
        LOG_INIT     = fabric::LOG_INIT,        // 512
        LOG_ASSEMBLY = fabric::LOG_ASSEMBLY,    // 1024
        LOG_TASKS    = fabric::LOG_TASKS,       // 2048
        LOG_LB1      = fabric::LOG_LB1,         // 4096
        LOG_LB2      = fabric::LOG_LB2,         // 8192
        LOG_VIEW     = fabric::LOG_VIEW         // 16384
    };
}
}
#endif // EQSERVER_LOG_H
