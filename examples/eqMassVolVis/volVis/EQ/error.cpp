
/* Copyright (c) 2010-2011, Stefan Eilemann <eile@eyescale.ch>
 *                    2012, David Steiner   <steiner@ifi.uzh.ch>
 */

#include "error.h"

#ifndef __APPLE__
#  include <eq/fabric/errorRegistry.h>
#  include <co/global.h>
#else
#  include <eq/fabric/errorRegistry.h>
#  include <co/global.h>
#endif

namespace massVolVis
{

namespace
{
struct ErrorData
{
    const uint32_t code;
    const std::string text;
};

ErrorData _errors[] = {
    { ERROR_VOLVIS_ARB_SHADER_OBJECTS_MISSING,      "GL_ARB_shader_objects extension missing" },
    { ERROR_VOLVIS_EXT_BLEND_FUNC_SEPARATE_MISSING, "GL_EXT_blend_func_separate extension missing" },
    { ERROR_VOLVIS_ARB_MULTITEXTURE_MISSING,        "GL_ARB_multitexture extension missing" },
    { ERROR_VOLVIS_NV_TEXTURE_BARRIER_MISSING,      "GL_NV_texture_barrier extension missing" },
//    { ERROR_VOLVIS_LOADSHADERS_FAILED,              "Can't load shaders" },
//    { ERROR_VOLVIS_LOADMODEL_FAILED,                "Can't load model" },
//    { ERROR_VOLVIS_MAPOBJECT_FAILED,                "Mapping data from application process failed" },
    { ERROR_VOLVIS_MAP_CONFIG_OBJECT_FAILED,        "Mapping of config data from application process failed"      },
    { ERROR_VOLVIS_MAP_VOLUME_INFO_OBJECT_FAILED,   "Mapping of volume info data from application process failed" },

    { 0, "" } // last!
};
}

void initErrors()
{
    eq::fabric::ErrorRegistry& registry = eq::Global::getErrorRegistry();

    for( size_t i=0; _errors[i].code != 0; ++i )
        registry.setString( _errors[i].code, _errors[i].text );
}

void exitErrors()
{
    eq::fabric::ErrorRegistry& registry = eq::Global::getErrorRegistry();

    for( size_t i=0; _errors[i].code != 0; ++i )
        registry.eraseString( _errors[i].code );
}

}//namespace massVolVis

