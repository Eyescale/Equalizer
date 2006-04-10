
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_EX_CHANNEL_H
#define EQ_EX_CHANNEL_H

#include "flip.h"

#include "plyModel.h"

#include <eq/eq.h>

class FrameData;
class InitData;

class Channel : public eq::Channel
{
public:
    Channel() : _initData(NULL), _frameData(NULL) {}

    virtual bool init( const uint32_t initID );
    virtual bool exit();
    virtual void draw( const uint32_t frameID );

private:
    eqBase::RefPtr<InitData> _initData;
    FrameData*               _frameData;

    void _drawBBox( const Model::BBox *bbox );
};




#endif // EQ_EX_CHANNEL_H

