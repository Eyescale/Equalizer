/* Copyright (c) 2007       Maxim Makhinya
   All rights reserved. */


// updated per frame
uniform vec3    vecView;
uniform int     frontIndex;
uniform float   sliceDistance;


uniform float   W;   //scale for x
uniform float   H;   //scale for y
uniform float   D;   //scale for z
uniform float   Do;  //shift of z
uniform float   Db;  //z offset 


void main(void)
{
//    gl_Position = ftransform();
    gl_Position = gl_ModelViewProjectionMatrix * vec4( gl_Vertex.xyz, 1.0 );

    gl_TexCoord[0] = 0.5 * gl_Vertex + 0.5;

    vec4 vDir = normalize(gl_ModelViewMatrixInverse * vec4(0.,0.,-1.,1.));

#ifndef PARALLEL_PROJECTION
    //compute position of virtual back vertex
    vec4 vPosition = gl_ModelViewMatrixInverse*vec4(0,0,0,1);

    vec4 eyeToVert = normalize(gl_Vertex - vPosition);
    vec4 backVert = vec4(1,1,1,1);

    gl_TexCoord[1] =
            gl_Vertex + eyeToVert * (sliceDistance / dot(vDir,eyeToVert));
#else
    gl_TexCoord[1] = gl_Vertex + vDir * sliceDistance; 
#endif
    //compute texture coordinates for virtual back vertex

    gl_TexCoord[1] = 0.5 * gl_TexCoord[1] + 0.5;


//Scaling of texture coordinates
    gl_TexCoord[0].x  *= W;
    gl_TexCoord[0].y  *= H;
    gl_TexCoord[0].z   = Db + (gl_TexCoord[0].z - Do) * D;

    gl_TexCoord[1].x  *= W;
    gl_TexCoord[1].y  *= H;
    gl_TexCoord[1].z   = Db + (gl_TexCoord[1].z - Do) * D;

}
