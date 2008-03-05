/* Copyright (c) 2007,       Maxim Makhinya
   Copyright (c) 2008,       Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EVOLVE_GLSL_SHADERS_H
#define EVOLVE_GLSL_SHADERS_H

#include <eq/eq.h>

namespace eVolve
{
    class glslShaders
    {
    public:
        glslShaders(): _program( 0 ), _shadersLoaded( false ), _glewCtx( 0 ) {}

        bool loadShaders( const std::string &vShader,
                          const std::string &fShader,
                                GLEWContext* glewCtx );

        void unloadShaders();

        GLhandleARB getProgram() { return _program; }
        GLEWContext* glewGetContext() { return _glewCtx; }

    private:
        GLhandleARB     _program;       //!< GLSL vertex and fragment shaders
        bool            _shadersLoaded; //!< flag of loaded shaders
        GLEWContext*    _glewCtx;       //!< OpenGL rendering context

        GLhandleARB _loadShader( const std::string &shader, GLenum shaderType );
        void _printLog( GLhandleARB shader, const std::string &type );
    };

}
#endif // EVOLVE_GLSL_SHADERS_H
