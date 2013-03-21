
/* Copyright (c) 2007-2011, Maxim Makhinya  <maxmah@gmail.com>
 *
 */


// input variables to function

uniform sampler3D volume; //gx, gy, gz, v
uniform sampler2D preIntRGBA;
uniform sampler2D preIntSDA;
uniform sampler2DRect backCoordsTexture;
uniform sampler2DRect frameBuffer;

uniform vec3    memBlockSize;
uniform vec3    memShift;

uniform float   samplingDistance; // 1.0 / samplingRate
uniform float   samplingDelta;

uniform vec3    memOffset;
uniform vec3    voxSize;

uniform vec2    pvpOffset;

uniform int     drawBB;

uniform float shininess;
uniform vec3  viewVec;
uniform vec4  taint; // .rgb should be pre-multiplied with .a

//uniform int   normalsQuality;

bool isEdge( vec3 coord )
{
    float minT = 0.018;
    float maxT = 1.0 - minT;
    int i = 0;

    if( coord.x < minT || coord.x > maxT ) i++;
    if( coord.y < minT || coord.y > maxT ) i++;
    if( coord.z < minT || coord.z > maxT ) i++;

    return i > 1;
}

void main (void)
{
    vec4 bbColor = vec4( 1.0, 0.0, 0.0, 0.1 );

    vec4 color = texture2DRect( frameBuffer, gl_FragCoord.xy-pvpOffset.xy );
    if( color.a < .0001 )
        discard;

    vec3 fCrd = gl_Color.rgb;
    vec3 bCrd = texture2DRect( backCoordsTexture, gl_FragCoord.xy ).rgb;

    vec3  blockSize = memBlockSize - memShift*2.0;
    float steps =  distance(  bCrd,  fCrd ) / samplingDistance;
    vec3  delta = (( bCrd - fCrd ) / steps ) * blockSize;

    vec3 pos = memOffset + memShift + fCrd * blockSize;

    float i = 0.0;
    float vol1 = texture3D( volume, pos ).a;
    while( i < steps )
    {
        i   += 1.0;
        pos += delta;
        float vol2 = texture3D( volume, pos ).a;
        vec4  col  = texture2D( preIntRGBA, vec2(vol1, vol2) );
        vec3  sda  = texture2D( preIntSDA , vec2(vol1, vol2) ).rgb;
        vol1 = vol2;

        float newAlpha = pow( col.a, samplingDelta );
        vec4 newColor = vec4( col.rgb*samplingDelta, newAlpha );
        col = mix( col, newColor, smoothstep( 0.27, 0.67, color.a*newAlpha ));


        if( !(drawBB > 0 && isEdge( fCrd )) )
        {
            if( col.a > .999 ) // transparent
                continue;

            vec3 pos1 = pos - delta/2.0;

            vec3 lookupMP = vec3(
                texture3D( volume, pos1 + vec3( voxSize.x,0.0,0.0) ).a -
                texture3D( volume, pos1 + vec3(-voxSize.x,0.0,0.0) ).a,
                texture3D( volume, pos1 + vec3(0.0, voxSize.y,0.0) ).a -
                texture3D( volume, pos1 + vec3(0.0,-voxSize.y,0.0) ).a,
                texture3D( volume, pos1 + vec3(0.0,0.0, voxSize.z) ).a -
                texture3D( volume, pos1 + vec3(0.0,0.0,-voxSize.z) ).a ) * 6.0;

            vec3 tnorm = -normalize( lookupMP );
            vec3 lightVec = normalize( gl_LightSource[0].position.xyz );
            vec3 reflect  = reflect( -lightVec, tnorm );

            float diffuse  = dot(lightVec, tnorm);
            float specular = pow(clamp(dot(reflect, viewVec), 0.0, 1.0), shininess);


            if( taint.a != 0.0 )
                col = vec4( col.rgb*(1.0-taint.a) + taint.rgb*taint.a*(-log(col.a)), col.a );


            col *= vec4(       gl_LightSource[0].ambient.rgb                        * sda.r +
                        clamp( gl_LightSource[0].diffuse.rgb  * diffuse, 0.0, 1.0 ) * sda.g +
                        clamp( gl_LightSource[0].specular.rgb * specular,0.0, 1.0 ) * sda.b, 1.0 );
        }else
            col = bbColor;

        color.rgb += col.rgb * color.a;
        color.a  *= col.a;

        if( color.a < .0001 )
            i = steps + 1.0;
    }
    if( drawBB > 0 && isEdge( bCrd ) && color.a >= .0001 )
    {
        color.rgb += bbColor.rgb * color.a;
        color.a  *= bbColor.a;
    }

    gl_FragColor = color;

//    gl_FragColor = vec4( 1., 1., 1., 0.1 );
//    gl_FragColor = vec4( bCrd, 0.1 );
//    gl_FragColor = vec4( fCrd, 0.1 );
}

