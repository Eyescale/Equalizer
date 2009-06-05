/*  
 * vertexShader.glsl
 * Copyright (c) 2007, Tobias Wolf <twolf@access.unizh.ch>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */ 
    
// Vertex shader for Phong/Blinn-Phong Shading with one light source.


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
