
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EVOLVE_PIPE_H
#define EVOLVE_PIPE_H

#include <eq/eq.h>

#include "frameData.h"

namespace eVolve
{
    class Pipe : public eq::Pipe
    {
    public:
        Pipe( eq::Node* parent ) : eq::Pipe( parent ) {}

        const FrameData& getFrameData() const { return _frameData; }

        Model*      getModel()   const { return _model;   }

    protected:
        virtual ~Pipe() {}

        virtual eq::WindowSystem selectWindowSystem() const;
        virtual bool configInit( const uint32_t initID );
        virtual bool configExit();
        virtual void frameStart( const uint32_t frameID, 
                                 const uint32_t frameNumber );
    private:

        FrameData   _frameData;

        Model*      _model;      //!< equal to RawVolumeModelRenderer
    };
}

#endif // EVOLVE_PIPE_H
