
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PLY_PIPE_H
#define EQ_PLY_PIPE_H

#include <eq/eq.h>

#include "frameData.h"

namespace eqPly
{
    /**
     * The representation of one GPU.
     *
     * The pipe object is responsible for maintaining GPU-specific and
     * frame-specific data. The identifier passed by the configuration contains
     * the version of the frame data corresponding to the rendered frame. The
     * pipe's start frame callback synchronizes the thread-local instance of the
     * frame data to this version.
     */
    class Pipe : public eq::Pipe
    {
    public:
        Pipe( eq::Node* parent ) : eq::Pipe( parent ) {}

        const FrameData& getFrameData() const { return _frameData; }

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
