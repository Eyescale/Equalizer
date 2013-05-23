
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef MASS_VOL__MODEL_RENDER_PARAMETERS_H
#define MASS_VOL__MODEL_RENDER_PARAMETERS_H

#include <msv/types/vmmlTypes.h>
#include <eq/fabric/range.h>

namespace massVolVis
{

struct ModelRenderParameters
{
    const Matrix4f&     modelviewM;
    const Matrix4f&     projectionMV;
    const Vec2_f&       screenCenter;
    const Vec2_f&       screenSize;
    const Vector3f&     viewVector;
    const eq::fabric::Range& range;
    const Vector4f&     taintColor;
          bool          drawBB;
    const uint32_t      renderBudget;
    const uint8_t       maxTreeDepth;
    const float         cameraSpin;
    const float         cameraTranslatoin;
    const bool          useRenderingError;
    const uint16_t      renderingError;
    const Channel*      channel;
};


} //namespace massVolVis

#endif //MASS_VOL__MODEL_RENDER_PARAMETERS_H

