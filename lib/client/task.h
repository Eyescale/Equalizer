
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_TASK_H
#define EQ_TASK_H

#include <iostream>

namespace eq
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
        TASK_DEFAULT  = EQ_BIT1,   //!< ALL for leaf, else ASSEMBLE|READBACK
        TASK_CLEAR    = EQ_BIT5,   //!< Clear the framebuffer
        TASK_CULL     = EQ_BIT9,   //!< Cull data [unused]
        TASK_DRAW     = EQ_BIT13,  //!< Draw data to the framebuffer
        TASK_ASSEMBLE = EQ_BIT17,  //!< Combine input frames
        TASK_READBACK = EQ_BIT21,  //!< Read results to output frames
        TASK_ALL      = EQ_BIT_ALL
    };
}

#endif // EQ_TASK_H
