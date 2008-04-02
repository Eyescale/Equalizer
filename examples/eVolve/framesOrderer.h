/* Copyright (c) 2007       Maxim Makhinya
   All rights reserved. */

#ifndef EVOLVE_FRAMES_ORDERER_H
#define EVOLVE_FRAMES_ORDERER_H

#include <eq/eq.h>

namespace eVolve
{
void orderFrames( eq::FrameVector&      frames,
                  const vmml::Matrix4d& modelviewM,
                  const vmml::Matrix3d& modelviewITM,
                  const vmml::Matrix4f& rotation,
                  const bool            perspective    );
}

#endif //EVOLVE_FRAMES_ORDERER_H
