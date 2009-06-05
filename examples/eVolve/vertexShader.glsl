/* Copyright (c) 2007       Maxim Makhinya
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

#version 110

// updated per frame
uniform float   sliceDistance;
uniform float   perspProj;


uniform float   W;   //scale for x
uniform float   H;   //scale for y
uniform float   D;   //scale for z
uniform float   Do;  //shift of z
uniform float   Db;  //z offset 

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
}
