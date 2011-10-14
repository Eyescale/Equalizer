
/* Copyright (c) 2007-2011, Maxim Makhinya  <maxmah@gmail.com>
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


// input variables to function

uniform sampler3D volume; //gx, gy, gz, v
uniform sampler2D preInt; // r,  g,  b, a

uniform float shininess;
uniform vec3  viewVec;
uniform vec4  taint; // .rgb should be pre-multiplied with .a

void main (void)
{
    vec4 lookupSF;
    vec4 lookupSB;

    lookupSF = texture3D(volume, gl_TexCoord[0].xyz);
    lookupSB = texture3D(volume, gl_TexCoord[1].xyz);

    vec4 preInt_ =  texture2D(preInt, vec2(lookupSF.a, lookupSB.a));

    if( taint.a != 0.0 )
        preInt_ = vec4( preInt_.rgb*(1.0-taint.a) + 
                        taint.rgb*taint.a*(-log(preInt_.a)),
                        preInt_.a );

    // lighting
    vec3 normalSF = lookupSF.rgb-0.5;
    vec3 normalSB = lookupSB.rgb-0.5;
    vec3 tnorm   = -normalize(normalSF+normalSB);

    vec3 lightVec = normalize( gl_LightSource[0].position.xyz );
    vec3 reflect  = reflect( -lightVec, tnorm );

    float diffuse = max( dot(lightVec, tnorm), 0.0 );

    float specular = pow(max(dot(reflect, viewVec), 0.0), shininess);

    vec4 color = vec4(gl_LightSource[0].ambient.rgb  * preInt_.rgb +
                      gl_LightSource[0].diffuse.rgb  * preInt_.rgb * diffuse +
                      gl_LightSource[0].specular.rgb * preInt_.rgb * specular,
                      preInt_.a);

    gl_FragColor = color;
}

