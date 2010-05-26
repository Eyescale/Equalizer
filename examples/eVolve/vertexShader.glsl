
/* Copyright (c) 2007       Maxim Makhinya
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Eyescale Software GmbH nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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
