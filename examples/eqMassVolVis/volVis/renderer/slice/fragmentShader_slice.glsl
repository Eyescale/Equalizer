
/* Copyright (c) 2007-2011, Maxim Makhinya  <maxmah@gmail.com>
 *
 */


// input variables to function 

uniform sampler3D volume; //gx, gy, gz, v
uniform sampler2D preInt; // r,  g,  b, a

uniform float shininess;
uniform int   normalsQuality;
uniform vec3  viewVec;
uniform vec3  sizeVec;
uniform vec4  taint; // .rgb should be pre-multiplied with .a

void main (void)
{
// Normals
    vec3 coords = (gl_TexCoord[0].xyz+gl_TexCoord[1].xyz)/2.0;
//    vec3 coords = gl_TexCoord[0].xyz;
    vec3 lookupMP;

if( sizeVec.x > 0.5 )
{
    lookupMP = texture3D( volume, coords ).rgb - 0.5;
}
else
{
// basic
    lookupMP = vec3(
        texture3D( volume, coords + vec3( sizeVec.x,0.0,0.0) ).a -
        texture3D( volume, coords + vec3(-sizeVec.x,0.0,0.0) ).a,
        texture3D( volume, coords + vec3(0.0, sizeVec.y,0.0) ).a -
        texture3D( volume, coords + vec3(0.0,-sizeVec.y,0.0) ).a,
        texture3D( volume, coords + vec3(0.0,0.0, sizeVec.z) ).a -
        texture3D( volume, coords + vec3(0.0,0.0,-sizeVec.z) ).a ) * 6.0;

// medium
    if( normalsQuality < 2 )
    {
        float lu = texture3D( volume, coords + vec3(-sizeVec.x, sizeVec.y,0.0) ).a;
        float ld = texture3D( volume, coords + vec3(-sizeVec.x,-sizeVec.y,0.0) ).a;
        float lb = texture3D( volume, coords + vec3(-sizeVec.x,0.0,-sizeVec.z) ).a;
        float lf = texture3D( volume, coords + vec3(-sizeVec.x,0.0, sizeVec.z) ).a;

        float ru = texture3D( volume, coords + vec3( sizeVec.x, sizeVec.y,0.0) ).a;
        float rd = texture3D( volume, coords + vec3( sizeVec.x,-sizeVec.y,0.0) ).a;
        float rb = texture3D( volume, coords + vec3( sizeVec.x,0.0,-sizeVec.z) ).a;
        float rf = texture3D( volume, coords + vec3( sizeVec.x,0.0, sizeVec.z) ).a;

        float ub = texture3D( volume, coords + vec3(0.0, sizeVec.y,-sizeVec.z) ).a;
        float uf = texture3D( volume, coords + vec3(0.0, sizeVec.y, sizeVec.z) ).a;
        float db = texture3D( volume, coords + vec3(0.0,-sizeVec.y,-sizeVec.z) ).a;
        float df = texture3D( volume, coords + vec3(0.0,-sizeVec.y, sizeVec.z) ).a;

        lookupMP += vec3(
            ru + rd + rb + rf - lu - ld - lb - lf,
            lu + ru + ub + uf - ld - rd - db - df,
            lf + rf + uf + df - lb - rb - ub - db ) * 3.0;
    }

// full
    if( normalsQuality < 1 )
    {
        float lub = texture3D( volume, coords + vec3(-sizeVec.x, sizeVec.y,-sizeVec.z) ).a;
        float luf = texture3D( volume, coords + vec3(-sizeVec.x, sizeVec.y, sizeVec.z) ).a;
        float ldb = texture3D( volume, coords + vec3(-sizeVec.x,-sizeVec.y,-sizeVec.z) ).a;
        float ldf = texture3D( volume, coords + vec3(-sizeVec.x,-sizeVec.y, sizeVec.z) ).a;

        float rub = texture3D( volume, coords + vec3( sizeVec.x, sizeVec.y,-sizeVec.z) ).a;
        float ruf = texture3D( volume, coords + vec3( sizeVec.x, sizeVec.y, sizeVec.z) ).a;
        float rdb = texture3D( volume, coords + vec3( sizeVec.x,-sizeVec.y,-sizeVec.z) ).a;
        float rdf = texture3D( volume, coords + vec3( sizeVec.x,-sizeVec.y, sizeVec.z) ).a;

        lookupMP += vec3(
            lub + luf + ldb + ldf - rub - ruf - rdb - rdf,
            lub + luf + rub + ruf - ldb - ldf - rdb - rdf,
            luf + ldf + ruf + rdf - lub - ldb - rub - rdb );
    }
}

// Preintegration (colors)
    float lookupSF = texture3D(volume, gl_TexCoord[0].xyz).a;
    float lookupSB = texture3D(volume, gl_TexCoord[1].xyz).a;

    vec4 preInt_ = texture2D(preInt, vec2(lookupSF, lookupSB));

    // don't mess z-buffer value when region is transparent
    if( preInt_.a > .999 )
        discard;

    if( taint.a != 0.0 )
        preInt_ = vec4( preInt_.rgb*(1.0-taint.a) + 
                        taint.rgb*taint.a*(-log(preInt_.a)),
                        preInt_.a );

    // lighting
    vec3 tnorm = -normalize( lookupMP );

    vec3 lightVec = normalize( gl_LightSource[0].position.xyz );
    vec3 reflect  = reflect( -lightVec, tnorm );

    float diffuse = max( dot(lightVec, tnorm), 0.0 );

    float specular = pow(max(dot(reflect, viewVec), 0.0), shininess);

    vec4 color = vec4(gl_LightSource[0].ambient.rgb  * preInt_.rgb +
                      gl_LightSource[0].diffuse.rgb  * preInt_.rgb * diffuse +
                      gl_LightSource[0].specular.rgb * preInt_.rgb * specular,
                      preInt_.a);

//    gl_FragColor = vec4( vec3(tnorm/2.0+0.5)*(-log(preInt_.a)), preInt_.a);
    gl_FragColor = color;
//    gl_FragColor = vec4( 1.0, 1.0, 1.0, 1.0 );
}

