
/* Copyright (c) 2010-2011, Stefan Eilemann <eile@eyescale.ch>
 */

#ifndef MASS_VOL__ERROR_H
#define MASS_VOL__ERROR_H

#include <eq/eq.h>

namespace massVolVis
{


/** Defines errors produced by volVis. */
enum Error
{
    ERROR_VOLVIS_ARB_SHADER_OBJECTS_MISSING = eq::ERROR_CUSTOM,
    ERROR_VOLVIS_EXT_BLEND_FUNC_SEPARATE_MISSING,
    ERROR_VOLVIS_ARB_MULTITEXTURE_MISSING,
    ERROR_VOLVIS_NV_TEXTURE_BARRIER_MISSING,
//    ERROR_VOLVIS_LOADSHADERS_FAILED,
//    ERROR_VOLVIS_LOADMODEL_FAILED,
    ERROR_VOLVIS_MAP_CONFIG_OBJECT_FAILED,
    ERROR_VOLVIS_MAP_VOLUME_INFO_OBJECT_FAILED
};

/** Set up eVolve-specific error codes. */
void initErrors();

/** Clear eVolve-specific error codes. */
void exitErrors();


}//namespace massVolVis
#endif // MASS_VOL__ERROR_H
