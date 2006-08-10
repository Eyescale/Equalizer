
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com>
   All rights reserved. */

#include "matrix4.h"

using namespace eq;

//----------------------------------------------------------------------
// float template specialization using cosf/sinf
//----------------------------------------------------------------------

template<>
Matrix4<float>::Matrix4() : Object( DATATYPE_EQ_MATRIX4F,
                                    eqNet::CMD_OBJECT_CUSTOM )
{
    makeIdentity();
}

template<>
void Matrix4<float>::rotateX( const float angle )
{
    //matrix multiplication: _m = _m * rotation x axis
    const float sinus = sinf(angle);
    const float cosin = cosf(angle);
    for( int i=0; i<16; i+=4 )
    {
        const float temp = _m[i];
        _m[i] = _m[i] * cosin - _m[i+2] * sinus;
        _m[i+2] = temp * sinus + _m[i+2] * cosin;
    }
}

template<>
void Matrix4<float>::rotateY( const float angle )
{
    //matrix multiplication: _m = _m * rotation y axis
    const float sinus = sinf(angle);
    const float cosin = cosf(angle);
    for( int i=0; i<16; i+=4 )
    {
        const float temp = _m[i+1];
        _m[i+1] = _m[i+1] * cosin + _m[i+2] * sinus;
        _m[i+2] = temp * -sinus + _m[i+2] * cosin;
    }
}

template<>
void Matrix4<float>::rotateZ( const float angle )
{
    //matrix multiplication: _m = _m * rotation z axis
    const float sinus = sinf(angle);
    const float cosin = cosf(angle);
    for( int i=0; i<16; i+=4 )
    {
        const float temp = _m[i];
        _m[i] = _m[i] * cosin + _m[i+1] * sinus;
        _m[i+1] = temp * -sinus + _m[i+1] * cosin;
    }
}
