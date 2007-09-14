
/*

void main (void)
{
    gl_FragColor= vec4( 1.0, 0.0, 0.0, 1.0 );
}

*/

// input variables to function

uniform sampler3D volume; //gx, gy, gz, v
uniform sampler2D preInt; // r,  g,  b, a

uniform float shininess;



void main (void)
{
    
    vec4 lookupSF;
    vec4 lookupSB;
    
    lookupSF = texture3D(volume, gl_TexCoord[0].xyz);
    lookupSB = texture3D(volume, gl_TexCoord[1].xyz);

    vec4 preInt_ =  texture2D(preInt, vec2(lookupSF.a, lookupSB.a));

//    gl_FragColor = preInt_;

    
    // lighting
    vec3 normalSF = (2.*lookupSF.rgb-1.);
    vec3 normalSB = (2.*lookupSB.rgb-1.);
    vec3 normal = normalize(normalSF+normalSB);

    vec3 L = (gl_LightSource[0].position).xyz;
  
    vec3 halfVector = normalize(L);

//    preInt.rgb = vec3(100, 0, 0);
    
    float diffuse = max(dot(gl_LightSource[0].position.xyz, normal), 0.0); 

    float specular = pow(max(dot(halfVector, normal), 0.0), shininess); 

    vec4 color = vec4(gl_LightSource[0].ambient.rgb  * preInt_.rgb + 
                      gl_LightSource[0].diffuse.rgb  * preInt_.rgb * diffuse + 
                      gl_LightSource[0].specular.rgb * preInt_.rgb * specular ,
                      preInt_.a);
  

    gl_FragColor = color;
    
}

