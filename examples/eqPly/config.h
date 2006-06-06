
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PLY_CONFIG_H
#define EQ_PLY_CONFIG_H

#include <eq/eq.h>

#include "frameData.h"
#include "initData.h"

class Config : public eq::Config
{
protected:
    eqNet::Object* instanciateObject( const uint32_t type, const void* data, 
                                       const uint64_t dataSize )
        {
            switch( type )
            {
                case TYPE_INITDATA:
                    return new InitData( data, dataSize );
                case TYPE_FRAMEDATA:
                    return new FrameData( data, dataSize );
                default:
                    return eqNet::Session::instanciateObject( type, data,
                                                              dataSize );
            }
        }
};

#endif // EQ_PLY_CONFIG_H
