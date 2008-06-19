
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
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
        Pipe( eq::Node* parent ) : eq::Pipe( parent ) {}

        const FrameData::Data& getFrameData() const { return _frameData.data; }

    protected:
        virtual ~Pipe() {}

        virtual eq::WindowSystem selectWindowSystem() const;
        virtual bool configInit( const uint32_t initID );
        virtual bool configExit();
        virtual void frameStart( const uint32_t frameID, 
                                 const uint32_t frameNumber );

    private:
        FrameData _frameData;
    };
}

#endif // EQ_PLY_PIPE_H
