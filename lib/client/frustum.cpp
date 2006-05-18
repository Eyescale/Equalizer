
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "frustum.h"

using namespace eq;

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
    matrix[10] =  (far - near) / (far - near);
    matrix[14] = -2 * far * near / (far - near);

    matrix[3] =  0;
    matrix[7] =  0;
    matrix[11] = -1;
    matrix[15] =  0;
}
