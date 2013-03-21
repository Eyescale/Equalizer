
/* Copyright (c) 2007       Maxim Makhinya
 *
 */

#version 110

// updated per frame
uniform float   sliceDistance;

uniform vec3    memBlockSize;
uniform vec3    memShift;

uniform vec3    coordOffset;
uniform vec3    coordDim;
uniform vec3    memOffset;


void main(void)
{
    vec4 vDir = normalize(gl_ModelViewMatrixInverse * vec4(0.0,0.0,-1.0,1.0));

// compute position of virtual back vertex
    vec4 vPosition = gl_ModelViewMatrixInverse*vec4(0.0,0.0,0.0,1.0);

    vec4 eyeToVert = normalize(gl_Vertex - vPosition);

// perspective/parallel projection sellection
    vec3 shift = eyeToVert.xyz * (sliceDistance / dot(vDir,eyeToVert));
    shift = shift / coordDim;

    // direct and virtual back vertex texture coordinates, normalized to [0..1]
    vec3 coords0 = (gl_Vertex.xyz - coordOffset) / coordDim;
    vec3 coords1 = coords0 + shift;

    vec3 memBlockScale = memBlockSize - memShift * 2.0;
    vec3 texCoord0 =  memOffset + memShift*1.0 + coords0 * memBlockScale;
    vec3 texCoord1 =  memOffset + memShift*1.0 + coords1 * memBlockScale;

    gl_TexCoord[0].xyz  = clamp( texCoord0, memOffset, memOffset + memBlockSize );
    gl_TexCoord[1].xyz  = clamp( texCoord1, memOffset, memOffset + memBlockSize );

    gl_Position = ftransform();
}
