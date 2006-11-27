
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "frustum.h"

using namespace eq;

void Frustum::adjustNear( const float _near )
{
    if( _near == near )
        return;

    const float ratio_2 = 0.5f * _near / near;

    const float hMiddle = (right + left) * 0.5f;
    const float width_2 = (right - left) * ratio_2;
    right = hMiddle + width_2;
    left  = hMiddle - width_2; 

    const float vMiddle  = (top + bottom) * 0.5f;
    const float height_2 = (top - bottom) * ratio_2;
    top    = vMiddle + height_2;
    bottom = vMiddle - height_2;

    near = _near;
}

vmml::Matrix4f Frustum::computeMatrix() const
{
    vmml::Matrix4f matrix = vmml::Matrix4f::IDENTITY;

    matrix.m00 = 2 * near / (right - left);
    matrix.m02 = (right + left) / (right - left );
    
    matrix.m11 = 2 * near / (top - bottom);
    matrix.m12 = (top + bottom) / (top - bottom);

    matrix.m22 = (far + near) / (far - near);
    matrix.m23 = 2 * far * near / (far - near);

    matrix.m32 = -1;
    matrix.m33 =  0;

    return matrix;
}
