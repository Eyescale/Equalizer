
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com>
   All rights reserved. */

#include "matrix4.h"

#include "object.h"
#include "packets.h"

namespace eq
{
// Specialized Constructors
//   We need a new eq::Object type for each specialization. Below are the
//   specialized constructors for all used template types.

    template<>
    Matrix4<float>::Matrix4() 
            : vmml::Matrix4f(),
              Object( eq::Object::TYPE_MATRIX4F, eqNet::CMD_OBJECT_CUSTOM )
    {
        vmml::Matrix4f::operator= ( vmml::Matrix4f::IDENTITY );
        setInstanceData( &ml, 16 * sizeof( float ));
    }

    template<>
    Matrix4<float>::Matrix4( const vmml::Matrix4f& matrix )
            : vmml::Matrix4f( matrix ),
              Object( eq::Object::TYPE_MATRIX4F, eqNet::CMD_OBJECT_CUSTOM )
    {
        setInstanceData( &ml, 16 * sizeof( float ));
    }

    template<>
    Matrix4<float>::Matrix4( const void* data, uint64_t dataSize )
            : vmml::Matrix4f( (float*)data ),
              Object( eq::Object::TYPE_MATRIX4F, eqNet::CMD_OBJECT_CUSTOM )
    {
        setInstanceData( &ml, 16 * sizeof( float ));
    }
}
