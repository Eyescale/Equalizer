
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PLY_CHANNEL_H
#define EQ_PLY_CHANNEL_H

#include "eqPly.h"

#include "plyModel.h"
#include "frustum.h"

#include <eq/eq.h>

class FrameData;
class InitData;

class Channel : public eq::Channel
{
public:
    Channel(){}

protected:
    virtual bool init( const uint32_t initID );
    virtual void draw( const uint32_t frameID );

private:
    static void _drawBBoxCB( Model::BBox *bbox, void *userData );
    void _drawBBox( const Model::BBox* bbox );
    void _initFrustum( Frustumf& frustum );
};




#endif // EQ_PLY_CHANNEL_H

