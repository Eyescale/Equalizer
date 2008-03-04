/* Copyright (c) 2007       Maxim Makhinya
   All rights reserved. */

#version 110

// updated per frame
uniform vec3    vecView;
uniform int     frontIndex;
uniform float   sliceDistance;
uniform float   perspProj;


uniform float   W;   //scale for x
uniform float   H;   //scale for y
uniform float   D;   //scale for z
uniform float   Do;  //shift of z
uniform float   Db;  //z offset 

varying vec3 ecPosition;

void main(void)
{
    gl_TexCoord[0] = 0.5 * gl_Vertex + 0.5;

    vec4 vDir = normalize(gl_ModelViewMatrixInverse * vec4(0.,0.,-1.,1.));

    //compute position of virtual back vertex
    vec4 vPosition = gl_ModelViewMatrixInverse*vec4(0,0,0,1);

    vec4 eyeToVert = normalize(gl_Vertex - vPosition);

// perspective/parallel projection sellection
    vec4 shift = eyeToVert * (sliceDistance / dot(vDir,eyeToVert)) * perspProj +
                 vDir * sliceDistance * ( 1.0 - perspProj );

    gl_TexCoord[1] = gl_Vertex + shift;

    //compute texture coordinates for virtual back vertex
    gl_TexCoord[1] = 0.5 * gl_TexCoord[1] + 0.5;


//Scaling of texture coordinates
    gl_TexCoord[0].x  *= W;
    gl_TexCoord[0].y  *= H;
    gl_TexCoord[0].z   = Db + (gl_TexCoord[0].z - Do) * D;

    gl_TexCoord[1].x  *= W;
    gl_TexCoord[1].y  *= H;
    gl_TexCoord[1].z   = Db + (gl_TexCoord[1].z - Do) * D;

    gl_Position = ftransform();

    vec4 ecPosition4 = gl_ModelViewMatrix * gl_Vertex;
    ecPosition       = ecPosition4.xyz / ecPosition4.w;
}
