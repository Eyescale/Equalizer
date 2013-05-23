
/* Copyright (c) 2007-2011, Maxim Makhinya  <maxmah@gmail.com>
 *
 */


// input variables to function

uniform sampler3D volume; //gx, gy, gz, v
uniform sampler1D preIntRGBA;
uniform sampler1D preIntSDA;
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

uniform vec3  bbColor;


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


vec3 computeNormal( sampler3D volume, vec3 pos, vec3 voxSize )
{
    return vec3(
        texture3D( volume, pos + vec3( voxSize.x,0.0,0.0) ).a -
        texture3D( volume, pos + vec3(-voxSize.x,0.0,0.0) ).a,
        texture3D( volume, pos + vec3(0.0, voxSize.y,0.0) ).a -
        texture3D( volume, pos + vec3(0.0,-voxSize.y,0.0) ).a,
        texture3D( volume, pos + vec3(0.0,0.0, voxSize.z) ).a -
        texture3D( volume, pos + vec3(0.0,0.0,-voxSize.z) ).a );
}


vec3 computeLight( vec3 normal, vec3 sda, bool applyLight )
{
    if( !applyLight )
        return vec3( 1.0 );

    vec3 tnorm = -normalize( normal );
    vec3 lightVec = normalize( gl_LightSource[0].position.xyz );
    vec3 reflectVec  = reflect( -lightVec, tnorm );

    float diffuse  = dot(lightVec, tnorm);
    float specular = pow(clamp(dot(reflectVec, viewVec), 0.0, 1.0), shininess);

    return vec3(       gl_LightSource[0].ambient.rgb                        * sda.r +
                clamp( gl_LightSource[0].diffuse.rgb  * diffuse, 0.0, 1.0 ) * sda.g +
                clamp( gl_LightSource[0].specular.rgb * specular,0.0, 1.0 ) * sda.b );
}


vec3 applyTaintColor( vec3 color, vec4 taint )
{
    return mix( color.rgb, taint.rgb, taint.a );
}


#define ALPHA_TH 0.0001
#define EXTINCTION_TH -10.0

void main (void)
{
//    vec3 bbColor = vec4( 1.0, 0.0, 0.0 );

    vec4 color = texture2DRect( frameBuffer, gl_FragCoord.xy-pvpOffset.xy );
    float extinctionSum = log( color.a );

    if( extinctionSum < EXTINCTION_TH )
        discard;

    vec3 fCrd = gl_Color.rgb;
    vec3 bCrd = texture2DRect( backCoordsTexture, gl_FragCoord.xy ).rgb;

    vec3  blockSize = memBlockSize - memShift*2.0;
    float steps =  distance(  bCrd,  fCrd ) / samplingDistance;
    vec3  delta = (( bCrd - fCrd ) / steps ) * blockSize;

    vec3 pos = memOffset + memShift + fCrd*blockSize - delta;

    float i = 0.0;
    while( i < steps )
    {
        i   += 1.0;
        pos += delta;
        float vol = texture3D( volume, pos ).a;

        vec3  sda  = texture1D( preIntSDA , vol );
        vec4  col  = texture1D( preIntRGBA, vol );

        if( drawBB > 0 && isEdge( fCrd ) )
        {
            col.rgb = bbColor.rgb * exp( extinctionSum );

            extinctionSum = EXTINCTION_TH-1.0;
        }else
        {
            if( col.a < ALPHA_TH ) // transparent
                continue;
//            col.a = -log( 1.00000001-col.a );

            vec3 normal = computeNormal( volume, pos, voxSize );
            bool applyLight = true;
            if( taint.a > 0.0 )
            {
                if( taint.a < 2.0/255.0 )
                {
                    taint.rgb  = normalize( normal )/2.0 + 0.5;
                    taint.a    = 1.0;
                    applyLight = false;
                }
                col.rgb = applyTaintColor( col.rgb, taint );
            }

            float samplingDeltaLocal = samplingDelta;
            if( extinctionSum - col.a*samplingDelta < -1.0 )
                samplingDeltaLocal = 1.0;

            col.rgb = col.rgb *
                      computeLight( normal, sda, applyLight ) *
                      col.a *
                      samplingDeltaLocal *
                      exp( extinctionSum );

            extinctionSum -= col.a*samplingDeltaLocal;
        }

        color.rgb += col.rgb;

        if( extinctionSum < EXTINCTION_TH )
            i = steps + 1.0;
    }
    if( drawBB > 0 && isEdge( bCrd ) && extinctionSum >= EXTINCTION_TH )
    {
        color.rgb += bbColor.rgb * exp( extinctionSum );
        extinctionSum = EXTINCTION_TH-1.0;
    }

    gl_FragColor = vec4( color.rgb, exp( extinctionSum ));

//    gl_FragColor = vec4( 1., 1., 1., 0.1 );
//    gl_FragColor = vec4( bCrd, 0.1 );
//    gl_FragColor = vec4( fCrd, 0.1 );
}

