uniform float Fcoef;
varying float flogz;
uniform float u_transparency;

#define DEPTH_OFFSET -0.0005

#if 1
const vec3 normal = vec3(0.0,0.0,1.0);                                                      
varying vec3 eyeVec;                                                        
varying vec3 lightDirs[8];

uniform bool lightsEnabled[8];                                              
uniform float   lightsBrightness[8]; //Adjust brightness of lights on water

const float cos_outer_cone_angle = 0.4; // 36 degrees   

        
void computeColorForLightSource(int i, inout vec4 color)                    
{           
    if (!lightsEnabled[i]) return;

    vec4 diffuseColor = gl_LightSource[i].diffuse*lightsBrightness[i];
    diffuseColor.a = gl_LightSource[i].diffuse.a;

    vec4 final_color =  vec4(0.0,0.0,0.0,1.0);
                        //(gl_FrontLightModelProduct.sceneColor * gl_FrontMaterial.ambient) +
                        //(gl_LightSource[i].ambient * gl_FrontMaterial.ambient);

    float distSqr = dot(lightDirs[i],lightDirs[i]);
    float invRadius = gl_LightSource[i].constantAttenuation;
    float att = clamp(1.0-invRadius * sqrt(distSqr), 0.0, 1.0);
    //att *= lightsBrightness;

    vec3 L = lightDirs[i] * inversesqrt(distSqr);
    vec3 D = normalize(gl_LightSource[i].spotDirection);

    float cos_cur_angle = dot(-L, D);

    float cos_inner_cone_angle = gl_LightSource[i].spotCosCutoff;

    float cos_inner_minus_outer_angle =
            cos_inner_cone_angle - cos_outer_cone_angle;

    float spot = 0.0;
    spot = clamp((cos_cur_angle - cos_outer_cone_angle) /
                 cos_inner_minus_outer_angle, 0.0, 1.0);

    vec3 N = normalize(normal);

    float lambertTerm = max( dot(N,L), 0.0);
    if(lambertTerm > 0.0)
    {
        final_color += diffuseColor *
                gl_FrontMaterial.diffuse *
                lambertTerm * spot * att;

        vec3 E = normalize(eyeVec);
        vec3 R = reflect(-L, N);

        float specular = pow( max(dot(R, E), 0.0),
                gl_FrontMaterial.shininess );

        final_color += gl_LightSource[i].specular *
                gl_FrontMaterial.specular *
                specular * spot * att;
    }

    color.xyz += final_color.xyz;
}                                                               
                                                                            
void lighting( inout vec4 color )                                          
{               
    vec4 clr = vec4(0.,0.,0.,1.0);
    for (int i = 1; i < 4; i++)
    {
            computeColorForLightSource(i,clr);
    }
    color += clr;

}
#endif


// Light, view, and normal vectors are all in world space.
// This function may be used to modify the ambient, diffuse, and specular light computed by Triton's fragment shaders.
void user_lighting(in vec3 L
                   , in vec3 vVertex_World_Space, in vec3 vNormal_World_Space
                   , in vec4 vVertex_Projection_Space
                   , inout vec3 ambient, inout vec3 diffuse, inout vec3 specular)
{

}

// View in world space. The final waterColor will be alpha-blended with the fogColor by fogBlend.
void user_fog(in vec3 vNorm, inout vec4 waterColor, inout vec4 fogColor, inout float fogBlend)
{

}

// The final computed color is normally just clamped to (0,1), but you may override this behavior here.
void user_tonemap(in vec4 preToneMapColor, inout vec4 postToneMapColor)
{
    vec4 color = postToneMapColor;
    lighting( color );

    postToneMapColor.rgb = color.rgb;
    postToneMapColor.a = u_transparency > 0.0 ? u_transparency : postToneMapColor.a;
}

// Override spray particle colors. Note these are drawn with an additive blending
// mode, so darker colors just result in more transparent particles. You're given
// the position in eye and world coordinates, and the texture lookup results for the
// spray particle. "Transparency" represents the overall transparency of the particles.
// "Decay" is used to fade out the particle over time. The final output color should
// be written to additive Color.
// The default implementation is:
// additiveColor = texColor * lightColor * decay * transparency

void user_particle_color(in vec3 vEye, in vec3 vWorld, in vec4 texColor,
                         in vec4 lightColor, in float transparency, in float decay,
                         inout vec4 additiveColor)
{

}

// Override the shading of volumetric decals on the water. You are given the texture lookup value,
// alpha value for the decal, and light color for the decal. The incoming default finalColor
// is the textureColor times the lightColor, with the alpha component further multiplied by alpha.
void user_decal_color(in vec4 textureColor, in float alpha, in vec4 lightColor, inout vec4 finalColor)
{

}

//adjust the reflection color prior to it being used by triton.
void user_reflection_adjust(in vec4 envColor, in vec4 planarColor, in float planarReflectionBlend, inout vec4 reflectedColor)
{
}

// Shadows the fragment; 1.0 = no shadow, 0 = black.
float user_cloud_shadow_fragment()
{
    return 1.0;
}

// Adjust the water diffuse to include color from breaking waves, propeller wash, and some refraction
void user_diffuse_color( inout vec3 Cdiffuse, in vec3 CiNoLight, in vec4 reflectedColor,
                         in float reflectivity )
{

}

// Output to MRT
void writeFragmentData(in vec4 finalColor, in vec4 Cdiffuse, in vec3 lightColor, in vec3 nNorm )
{
#ifdef OPENGL32
    fragColor = finalColor;
#else
    gl_FragColor = finalColor;
    gl_FragDepth = log2(flogz) * Fcoef * 0.5 + DEPTH_OFFSET;
#endif
}
