//Current SL versions supported 3.023 - 3.025
//
uniform float Fcoef;
varying float flogz;

varying vec3 normal;
varying vec3 eyeVec;
varying vec3 lightDirs[8];

uniform bool	lightsEnabled[8];
uniform float	lightsBrightness[8]; //Adjust brightness of lights on clouds
uniform float	todLightsBrightness;
uniform bool	todLightsBrightnessEnabled;

const float cos_outer_cone_angle = 0.14; // 36 degrees

void computeFogColor(inout vec4 color)
{
    if (gl_FragCoord.w > 0.0)
    {
        const float LOG2 = 1.442695;
        float z = gl_FragCoord.z / gl_FragCoord.w;
        float fogFactor = exp2( -gl_Fog.density *
                gl_Fog.density *
                z *
                z *
                LOG2 );
        fogFactor = clamp(fogFactor, 0.0, 1.0);

        vec4 clr = color;
        color = mix(gl_Fog.color, color, fogFactor );
        color.a = clr.a;
    }
}

void computeColorForLightSource(int i, inout vec4 color)
{
    if ( lightsEnabled[i] == 0) return;

    vec4 final_color = vec4(0,0,0,0);

    float distSqr = dot(lightDirs[i],lightDirs[i]);
    float d = sqrt( distSqr );

    // This is what the attenuation should be, but you need the right
    // values in your light sources first.
    float invRadius = gl_LightSource[i].constantAttenuation;
    float att = clamp(1.0-invRadius * sqrt(distSqr), 0.0, 1.0);

    // Just discard if attenuation is high enough to effectively kill
    // the lights. Otherwise weird precision issues crop up.
    //if (att < (1.0 / 256.0)) return;

    vec3 L = lightDirs[i] * inversesqrt(distSqr);
    vec3 D = normalize(gl_LightSource[i].spotDirection);

    float cos_cur_angle = dot(-L, D);

    float cos_inner_cone_angle = gl_LightSource[i].spotCosCutoff;

    float cos_inner_minus_outer_angle =
            cos_inner_cone_angle - cos_outer_cone_angle;

    float spot = 0.0;
    spot = clamp((cos_cur_angle - cos_outer_cone_angle) /
            cos_inner_minus_outer_angle, 0.0, 1.0);

    spot = clamp((cos_cur_angle - 0.1) /
            cos_inner_minus_outer_angle, 0.0, 1.0);

    vec3 N = normalize(normal);

    float lambertTerm = max( dot(N,L), 0.0);

	float todBrightness = todLightsBrightnessEnabled ? todLightsBrightness : 1.0;

	vec4 diffuse = gl_LightSource[i].diffuse * lightsBrightness[i] * todBrightness;
	diffuse.a = gl_LightSource[i].diffuse.a;

    if(lambertTerm > 0.0)
    {
            final_color +=  diffuse * 
                    gl_FrontMaterial.diffuse *
                    lambertTerm * spot * att;
    }

    color.rgb += final_color.rgb;
}

void lighting( inout vec4 color, in vec4 texColor )
{
    vec4 clr = color;
    for (int i = 1; i < 8; i++)
    {
        computeColorForLightSource(i,clr);
    }	
    color = texColor * clr;
}

// Allow overriding of the final sky fragment color
void overrideSkyFragColor(inout vec4 finalColor)
{
}

// Allows overriding of the fog color, fog blend factor, underlying cloud color, and alpha blending of the cloud.
void overrideStratusLighting(in vec3 fogColor, in float fogFactor, in vec3 cloudColor, in float cloudFade, inout vec4 finalColor)
{

}

// Overrides fragment colors in stratocumulus clouds. The pre-lit color (which incorporates light scattering within the cloud) is
// given as well as the light color. These are multiplied together to provide the default finalColor.
// finalColor = color * lightColor;
void overrideStratocumulus(in vec4 color, in vec4 lightColor, inout vec4 finalColor)
{

}

// Overrides fragment colors of billboards (cloud puffs, sun, moon.)
void overrideBillboardFragment(in vec4 texColor, in vec4 lightColor, inout vec4 finalColor)
{    
    lighting(finalColor,texColor);
}

//Overrides the final color of the Cirrus Clouds
// original finalColor is texel * gl_Color
void overrideCirrusColor(in vec4 texel, in vec4 litColor, inout vec4 finalColor)
{

}

// Overrides the particle color used to light rain, snow, and sleet.
void overrideParticleColor(in vec4 textureColor, in vec4 lightColor, inout vec4 particleColor)
{

}

// Write the final fragment color. Implement this if you need to write to multiple render targets, for example.
void writeFragmentData(in vec4 finalColor)
{
    gl_FragColor = finalColor;gl_FragColor = finalColor;
	gl_FragDepth = log2(flogz) * Fcoef * 0.5;
}

