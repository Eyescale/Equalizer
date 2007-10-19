
/*
// updated per cube
varying vec3  vecTranslate;

// updated per frame
uniform vec3  vecView;
uniform int   frontIndex;

// const: never updated
uniform float dPlaneStart;
uniform float dPlaneIncr;
uniform int   sequence[64];
uniform vec3  vecVertices[8];
uniform int   v1[24];
uniform int   v2[24];

void main(void)
{
    float dPlaneDist = dPlaneStart + gl_Vertex.y * dPlaneIncr;
        
    vec3 Position  = vecTranslate;
    
    for( int e = 0; e < 4; e++ )
    {
    
        int vidx1 = sequence[ int( frontIndex * 8 + v1[ int( gl_Vertex.x ) * 4 + e ] ) ];
        int vidx2 = sequence[ int( frontIndex * 8 + v2[ int( gl_Vertex.x ) * 4 + e ] ) ];
    
        vec3 vecV1 = vecVertices[ vidx1 ];
        vec3 vecV2 = vecVertices[ vidx2 ];
        
        vec3 vecStart = vecV1 + vecTranslate;
        vec3 vecDir   = vecV2 - vecV1;
    
        float denom = dot( vecDir, vecView );
        float lambda = ( denom != 0.0 ) ? ( dPlaneDist - dot( vecStart, vecView ) ) / denom : -1.0;

        if( (lambda >= 0.0) && (lambda <= 1.0) )
        {
            Position = vecStart + lambda * vecDir;
            break;
        }
        
    } 
    
    gl_Position = gl_ModelViewProjectionMatrix * vec4( Position, 1.0 );
}
*/

uniform float sliceDistance;

void main(void)
{
    gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;
    
    gl_Position = ftransform();
    
    gl_TexCoord[0] = gl_TextureMatrixInverse[0] * gl_Vertex;
    gl_TexCoord[0] = gl_TexCoord[0] / 2.0 + 0.5;
    
    vec4 vPosition = gl_ModelViewMatrixInverse*vec4(0,0,0,1);
    vec4 vDir = normalize(gl_ModelViewMatrixInverse * vec4(0.,0.,-1.,1.));

    //compute position of virtual back vertex
    
    vec4 eyeToVert = normalize(gl_Vertex - vPosition);
    vec4 backVert = vec4(1,1,1,1);
    backVert = gl_Vertex - eyeToVert * (sliceDistance / dot(vDir,eyeToVert)); 
    
    //compute texture coordinates for virtual back vertex
    
    gl_TexCoord[1] = gl_TextureMatrixInverse[0] * backVert;
    gl_TexCoord[1] = gl_TexCoord[1] / 2.0 + 0.5;
}
