
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch> 
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
    /** Defines selective logging classes enabled by setting EQ_LOG_TOPICS. */
    enum LogTopics
    {
        LOG_INIT     = co::LOG_CUSTOM << 0,   //!< Log initialization (256)
        LOG_CUSTOM   = 0x400 //!< 1024
    };
}
}
#endif // EQFABRIC_LOG_H
