
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EVOLVE_PIPE_H
#define EVOLVE_PIPE_H

#include <eq/eq.h>

#include "frameData.h"

#include "shader.h"

namespace eVolve
{
    class Pipe : public eq::Pipe
    {
    public:
        Pipe() : eq::Pipe(), _shadersLoaded(false) {}

        const FrameData& getFrameData() const { return _frameData; }

        bool LoadShaders();

        GLhandleARB getShader()  const { return _shader;  }
        Model*      getModel()   const { return _model;   }

    protected:
        virtual ~Pipe() {}

        virtual eq::WindowSystem selectWindowSystem() const;
        virtual bool configInit( const uint32_t initID );
        virtual bool configExit();
        virtual void frameStart( const uint32_t frameID, 
                                 const uint32_t frameNumber );
    private:
        GLhandleARB _shader;

        bool        _shadersLoaded;
        FrameData   _frameData;

        Model*   _model;      //!< equal to RawVolume Model
    };
}

#endif // EVOLVE_PIPE_H
