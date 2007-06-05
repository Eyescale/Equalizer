
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PLY_PIPE_H
#define EQ_PLY_PIPE_H

#include <eq/eq.h>

#include "frameData.h"

namespace eqPly
{
    class Pipe : public eq::Pipe
    {
    public:
        const FrameData& getFrameData() const { return _frameData; }

    protected:
        virtual ~Pipe() {}

        virtual bool configInit( const uint32_t initID );
        virtual bool configExit();
        virtual void frameStart( const uint32_t frameID, 
                                 const uint32_t frameNumber );

    private:
        FrameData _frameData;
    };
}

#endif // EQ_PLY_PIPE_H
