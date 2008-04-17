/* Copyright (c) 2007       Maxim Makhinya
   All rights reserved. */


// input variables to function

uniform sampler3D volume; //gx, gy, gz, v
uniform sampler2D preInt; // r,  g,  b, a

uniform float shininess;
uniform vec3  viewVec;

void main (void)
{
    vec4 lookupSF;
    vec4 lookupSB;

    lookupSF = texture3D(volume, gl_TexCoord[0].xyz);
    lookupSB = texture3D(volume, gl_TexCoord[1].xyz);

    vec4 preInt_ =  texture2D(preInt, vec2(lookupSF.a, lookupSB.a));

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

