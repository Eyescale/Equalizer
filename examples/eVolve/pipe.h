
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_VOL_PIPE_H
#define EQ_VOL_PIPE_H

#include <eq/eq.h>

#include "frameData.h"

#ifdef CG_INSTALLED
#include <gloo/cg_program.hpp>
#endif
#include "shader.h"

namespace eVolve
{
    struct eqCgShaders
    {
#ifdef CG_INSTALLED
        eqCgShaders() : cgVertex(NULL), cgFragment(NULL) {};
        
        gloo::cg_program*  cgVertex;    //!< Cg vertex shader
        gloo::cg_program*  cgFragment;  //!< Cg fragment shader
#endif
    };
    
    class Pipe : public eq::Pipe
    {
    public:
         Pipe() : eq::Pipe(), _shadersLoaded( false ) { }

        const FrameData& getFrameData() const { return _frameData; }

        void LoadShaders();

        eqCgShaders getShaders() const { return _shaders; }
        GLhandleARB getShader()  const { return _shader; }

    protected:
        virtual ~Pipe() {}

        virtual eq::WindowSystem selectWindowSystem() const;
        virtual bool configInit( const uint32_t initID );
        virtual bool configExit();
        virtual void frameStart( const uint32_t frameID, 
                                 const uint32_t frameNumber );
    private:
#ifdef CG_INSTALLED
        CGcontext   _cgContext;
#endif
        eqCgShaders _shaders;
        GLhandleARB _shader;

        bool        _shadersLoaded;
        FrameData   _frameData;
    };
}

#endif // EQ_VOL_PIPE_H
