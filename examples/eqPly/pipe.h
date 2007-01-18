
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PLY_PIPE_H
#define EQ_PLY_PIPE_H

#include <eq/eq.h>

#include "initData.h"

class Pipe : public eq::Pipe
{
public:
    // per-pipe display list cache (windows share context)
    GLuint getDisplayList( const void* key );
    GLuint newDisplayList( const void* key );

protected:
    bool init( const uint32_t initID );
    bool exit();
    void startFrame( const uint32_t frameID );

private:
    eqBase::RefPtr<InitData>  _initData;
    eqBase::RefPtr<FrameData> _frameData;

    eqBase::PtrHash< const void *, GLuint > _displayLists;
};

#endif // EQ_PLY_PIPE_H
