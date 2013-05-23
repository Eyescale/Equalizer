
/* Copyright (c) 2007       Maxim Makhinya
 *
 */

#version 110

// updated per frame
//uniform float   sliceDistance;

//uniform vec3    coordOffset;
//uniform vec3    coordDim;

void main(void)
{
    gl_Position = ftransform();
    gl_FrontColor = gl_Color;
}
