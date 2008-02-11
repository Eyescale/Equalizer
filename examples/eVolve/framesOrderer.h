/* Copyright (c) 2007       Maxim Makhinya
   All rights reserved. */

#ifndef EVOLVE_FRAMES_ORDERER_H
#define EVOLVE_FRAMES_ORDERER_H

#include <eq/eq.h>

namespace eVolve
{
    struct Frame
    {
        Frame( eq::Frame* f=0, eq::Image* i=0, eq::Range r=eq::Range(0.f, 0.f) )
        : frame( f ), image( i ), range( r ) {}

        eq::Range getRange() const 
        { return frame ? frame->getRange() : range; }

        eq::Frame* frame;
        eq::Image* image;
    
        eq::Range  range;
    };


    void orderFrames(       std::vector< Frame > &frames,
                      const vmml::Matrix4d       &modelviewM,
                      const vmml::Matrix3d       &modelviewITM,
                      const vmml::Matrix4f       &rotation,
                      const bool                  perspective    );
}

#endif //EVOLVE_FRAMES_ORDERER_H
