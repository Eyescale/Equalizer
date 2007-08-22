
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_VOL_PIPE_H
#define EQ_VOL_PIPE_H

#include <eq/eq.h>

#include "frameData.h"

#include <eq/gloo/cg_program.h>

namespace eqVol
{
	struct eqCgShaders
	{
		eqCgShaders() : cgVertex(NULL), cgFragment(NULL) {};
		
		gloo::cg_program*  cgVertex;    //!< Cg vertex shader
	    gloo::cg_program*  cgFragment;  //!< Cg fragment shader
	};
	
    class Pipe : public eq::Pipe
    {
    public:
 		Pipe() : eq::Pipe()
		{
			_cgContext = NULL;
		}

        const FrameData& getFrameData() const { return _frameData; }

		void LoadShaders();

		eqCgShaders getShaders() const { return _shaders; }
		void        setShaders( eqCgShaders shaders ) { _shaders = shaders; }

    protected:
        virtual ~Pipe() {}

        virtual bool configInit( const uint32_t initID );
        virtual bool configExit();
        virtual void frameStart( const uint32_t frameID, 
                                 const uint32_t frameNumber );

    private:
		CGcontext   _cgContext;
 		eqCgShaders _shaders;
        FrameData   _frameData;
    };
}

#endif // EQ_VOL_PIPE_H
