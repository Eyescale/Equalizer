/* Copyright (c) 2007       Maxim Makhinya
   All rights reserved. */

#ifndef EVOLVE_GLSL_SHADERS_H
#define EVOLVE_GLSL_SHADERS_H

namespace eVolve
{
    class glslShaders
    {
    public:
        glslShaders(): _program( 0 ), _shadersLoaded( false ) {}

        bool loadShaders( const std::string &vShader,
                          const std::string &fShader,
                                GLEWContext* glewCtx );

        void unloadShaders();

        GLhandleARB getProgram() { return _program; }

    private:
        GLhandleARB     _program;       //!< GLSL vertex and fragment shaders
        bool            _shadersLoaded; //!< flag of loaded shaders
        GLEWContext*    _glewCtx;       //!< OpenGL rendering context
    };

}
#endif // EVOLVE_GLSL_SHADERS_H
