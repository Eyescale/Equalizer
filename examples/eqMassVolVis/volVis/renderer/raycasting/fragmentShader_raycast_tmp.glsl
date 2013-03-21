
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

uniform vec3    memOffset;
uniform vec3    voxSize;

uniform vec2    pvpOffset;

uniform int     drawBB;

uniform float shininess;
uniform vec3  viewVec;
uniform vec4  taint; // .rgb should be pre-multiplied with .a

//uniform int   normalsQuality;

float edgeStrength( vec3 coord )
{
    float tr = 0.02;
    float tr1 = 1.0 - tr;

    coord = abs( coord-0.5 )*2.f;
    coord = abs( coord-tr1 );

    vec3 ds = vec3(1.f,1.f,1.f) - smoothstep( 0.f, tr, coord );

    // median computation
    if( ds.x <= ds.y )
    {
        if( ds.y <= ds.z ) return ds.y;
        if( ds.x <= ds.z ) return ds.z;
        return ds.x;
    }
    if( ds.x <= ds.z ) return ds.x;
    if( ds.y <= ds.z ) return ds.z;
    return ds.y; 
}

bool isEdge( vec3 coord )
{
    float tr = 0.97-0.5;

    coord = abs( coord-0.5 );
    int i = 0;

    if( coord.x > tr )  i++;
    if( coord.y > tr )  i++;
    if( coord.z > tr )  i++;

    return i > 1;
}

void main (void)
{
    vec4 color = texture2DRect( frameBuffer, gl_FragCoord.xy-pvpOffset.xy );
    if( color.a < .0001 )
        discard;

    vec3 fCrd = gl_Color.rgb;
    vec3 bCrd = texture2DRect( backCoordsTexture, gl_FragCoord.xy ).rgb;

    vec3  blockSize = memBlockSize - memShift*2.0;
    float steps =  distance(  bCrd,  fCrd ) / samplingDistance;
    vec3  deltaAbs = (( bCrd - fCrd ) / steps );
    vec3  delta = deltaAbs * blockSize;

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

        float edgeWeight = 0.f;
        if( drawBB > 0 )
        {
            fCrd += deltaAbs; 
            edgeWeight = edgeStrength( fCrd );
        }

        if( col.a > .999 && edgeWeight == 0.f )
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

        if( edgeWeight != 0.f )
            col = mix( col, vec4( 1.0, 0.0, 0.0, 0.001 ), edgeWeight );

        color.rgb += col.rgb * color.a;
        color.a  *= col.a;

        if( color.a < .0001 )
            i = steps + 1.0;
    }
    gl_FragColor = color;

//    gl_FragColor = vec4( bCrd, 0.5 );
    gl_FragColor = vec4( fCrd, 0.1 );
}

