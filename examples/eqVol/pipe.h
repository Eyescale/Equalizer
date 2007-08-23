
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_VOL_PIPE_H
#define EQ_VOL_PIPE_H

#include <eq/eq.h>

#include "frameData.h"

#define CG_SHADERS

#ifdef CG_SHADERS
#include <eq/gloo/cg_program.h>
#else
#include "shader.h"
#endif

namespace eqVol
{
#ifdef CG_SHADERS
	struct eqCgShaders
	{
		eqCgShaders() : cgVertex(NULL), cgFragment(NULL) {};
		
		gloo::cg_program*  cgVertex;    //!< Cg vertex shader
	    gloo::cg_program*  cgFragment;  //!< Cg fragment shader
	};
#endif
	
    class Pipe : public eq::Pipe
    {
    public:
 		Pipe() : eq::Pipe()
		{
			_shadersLoaded = false;
		}

        const FrameData& getFrameData() const { return _frameData; }

		void LoadShaders();

#ifdef CG_SHADERS
		eqCgShaders getShaders() const { return _shaders; }
#else
		GLhandleARB getShader() const { return _shader; }
#endif

    protected:
        virtual ~Pipe() {}

        virtual bool configInit( const uint32_t initID );
        virtual bool configExit();
        virtual void frameStart( const uint32_t frameID, 
                                 const uint32_t frameNumber );
    private:
#ifdef CG_SHADERS
		CGcontext   _cgContext;
 		eqCgShaders _shaders;
#else
		GLhandleARB _shader;
#endif
		bool		_shadersLoaded;
        FrameData   _frameData;
    };
}

#endif // EQ_VOL_PIPE_H
