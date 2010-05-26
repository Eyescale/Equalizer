/*  
 *  fragmentShader.glsl
 *  Copyright (c) 2007, Tobias Wolf <twolf@access.unizh.ch>
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
    
// Fragment shader for Phong/Blinn-Phong Shading with one light source.


varying vec3 normalEye;
varying vec4 positionEye;


void main()
{
    // normalize interpolated normal, compute view vector from position
    vec3 normal = normalize( normalEye );
    vec3 view = normalize( -positionEye ).xyz;
    
    // compute light vector
    vec3 light;
    if( gl_LightSource[0].position.w == 0.0 )
        // directional light
        light = normalize( gl_LightSource[0].position ).xyz;
    else
        // point light
        light = normalize( gl_LightSource[0].position - positionEye ).xyz;
    
    // compute the ambient component
    //vec4 ambient = gl_FrontLightProduct[0].ambient;
    vec4 ambient = gl_LightSource[0].ambient * gl_Color;
    
    // compute the diffuse component
    float dotLN = dot( light, normal );
    //vec4 diffuse = gl_FrontLightProduct[0].diffuse * max( dotLN, 0.0 );
    vec4 diffuse = gl_LightSource[0].diffuse * gl_Color * max( dotLN, 0.0 );
    
    // compute the specular component
    float factor;
    if( dotLN > 0.0 )
        factor = 1.0;
    else
        factor = 0.0;
    
    // pure Phong
    //vec3 reflect = normalize( reflect( -light, normal ) );
    //vec4 specular = 
    //    gl_FrontLightProduct[0].specular * factor *
    //    max( pow( dot( reflect, view ), gl_FrontMaterial.shininess ), 0.0 );
    
    // modified Blinn-Phong
    vec3 halfway = normalize( light + view );
    vec4 specular = 
        gl_FrontLightProduct[0].specular * factor *
        max( pow( dot( normal, halfway ), gl_FrontMaterial.shininess ), 0.0 );
    
    // sum the components up, defaulting alpha to 1.0
    gl_FragColor = 
        vec4( vec3( gl_FrontLightModelProduct.sceneColor + 
                    ambient + diffuse + specular ), 1.0 );
}
