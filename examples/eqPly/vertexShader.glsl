/*  
    vertexShader.glsl
    Copyright (c) 2007, Tobias Wolf <twolf@access.unizh.ch>
    All rights reserved.  
    
    Vertex shader for Phong/Blinn-Phong Shading with one light source.
*/


varying vec3 normalEye;
varying vec4 positionEye;


void main()
{
    // transform normal to eye coordinates
    normalEye = normalize( gl_NormalMatrix * gl_Normal );
    
    // transform position to eye coordinates
    positionEye = normalize( gl_ModelViewMatrix * gl_Vertex );
    
    // transform position to screen coordinates
    //gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_Position = ftransform();
    
    // pass the vertex colors on to the fragment shader
    gl_FrontColor = gl_Color;
}
