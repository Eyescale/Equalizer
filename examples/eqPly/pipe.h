
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PLY_PIPE_H
#define EQ_PLY_PIPE_H

#include <eq/eq.h>

#include "frameData.h"

class Pipe : public eq::Pipe
{
public:
    const FrameData& getFrameData() const { return _frameData; }

    // per-pipe display list cache (all windows share the context)
    GLuint getDisplayList( const void* key );
    GLuint newDisplayList( const void* key );

protected:
    virtual bool init( const uint32_t initID );
    virtual bool exit();
    virtual void frameStart( const uint32_t frameID, 
                             const uint32_t frameNumber );

private:
    FrameData _frameData;

    eqBase::PtrHash< const void *, GLuint > _displayLists;
};

#endif // EQ_PLY_PIPE_H
