
/* Copyright (c) 2008-2010, Stefan Eilemann <eile@equalizergraphics.com>
 * Copyright (c)      2010, Cedric Stalder <cedric.stalder@gmail.com> 
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

#ifndef EQFABRIC_TASK_H
#define EQFABRIC_TASK_H

#include <iostream>
#include <co/base/types.h>

namespace eq
{
namespace fabric
{
    /**
     * Tasks define the actions executed by a channel during rendering.
     *
     * The enum values are spaced apart to leave room for future additions
     * without breaking binary backward compatibility.
     */
    enum Task
    {
        TASK_NONE     = EQ_BIT_NONE,
        TASK_DEFAULT  = EQ_BIT1,   //!< leaf: all, other ASSEMBLE|READBACK|VIEW
        TASK_VIEW     = EQ_BIT2,   //!< View start/finish
        TASK_CLEAR    = EQ_BIT5,   //!< Clear the framebuffer
        TASK_CULL     = EQ_BIT9,   //!< Cull data [unused]
        TASK_DRAW     = EQ_BIT13,  //!< Draw data to the framebuffer
        TASK_ASSEMBLE = EQ_BIT17,  //!< Combine input frames
        TASK_READBACK = EQ_BIT21,  //!< Read results to output frames
        TASK_ALL      = EQ_BIT_ALL
    };
}
}
#endif // EQ_TASK_H
