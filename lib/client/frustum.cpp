
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "frustum.h"

using namespace eq;

void Frustum::adjustNear( const float _near )
{
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

void Frustum::computeMatrix( float matrix[16] ) const
{
    matrix[0]  =  2 * near / (right - left);
    matrix[4]  =  0.;
    matrix[8]  =  (right + left) / (right - left );
    matrix[12]  =  0;
    
    matrix[1]  =  0;
    matrix[5]  =  2 * near / (top - bottom);
    matrix[9]  =  (top + bottom) / (top - bottom);
    matrix[13]  =  0;
    
    matrix[2]  =  0;
    matrix[6]  =  0;
    matrix[10] =  -(far + near) / (far - near);
    matrix[14] = -2 * far * near / (far - near);

    matrix[3] =  0;
    matrix[7] =  0;
    matrix[11] = -1;
    matrix[15] =  0;
}
