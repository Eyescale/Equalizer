
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_FLIP_CONFIG_H
#define EQ_FLIP_CONFIG_H

#include <eq/eq.h>

#include "frameData.h"
#include "initData.h"

class Config : public eq::Config
{
protected:
    eqNet::Mobject* instanciateMobject( const uint32_t type, const void* data, 
                                        const uint64_t dataSize )
        {
            switch( type )
            {
                case OBJECT_INITDATA:
                    return new InitData( data, dataSize );
                case OBJECT_FRAMEDATA:
                    return new FrameData( data, dataSize );
                default:
                    return eqNet::Session::instanciateMobject( type, data,
                                                               dataSize );
            }
        }
};

#endif // EQ_FLIP_CONFIG_H
