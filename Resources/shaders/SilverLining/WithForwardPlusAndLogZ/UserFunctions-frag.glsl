// Tested agains SilverLining Version 4.040 January 12, 2016

uniform float u_ForwardPlusRange;

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

void overrideBillboardFragment_forward_plus_sl_ps(in vec4 texColor, in vec4 lightColor, inout vec4 finalColor);


// Overrides fragment colors of billboards (cloud puffs, sun, moon.)
void overrideBillboardFragment(in vec4 texColor, in vec4 lightColor, inout vec4 finalColor)
{
	if (u_ForwardPlusRange > 0.0)
	{
		float z = gl_FragCoord.z / gl_FragCoord.w;
		if (z < u_ForwardPlusRange)
		{
			overrideBillboardFragment_forward_plus_sl_ps(texColor, lightColor, finalColor);
		}
	}
	else
	{
		overrideBillboardFragment_forward_plus_sl_ps(texColor, lightColor, finalColor);
	}
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

void log_z_ps(float depthoffset);

// Write the final fragment color. Implement this if you need to write to multiple render targets, for example.
void writeFragmentData(in vec4 finalColor)
{
    gl_FragColor = finalColor;
	log_z_ps(0.0);
}

