// Tested agains SilverLining Version 4.040 January 12, 2016

varying vec3 vVertex_View_Space;
varying vec3 vNormal_View_Space;
varying vec4 vVertex_Projection_Space;

// Hook to override computation of fog on clouds
// fogExponent is squared to determine the blending of clouds with the background
// fogBlend is used for blending fogColor with the cloud color.
void overrideBillboardFog(in vec3 eyePosition, inout vec4 fogColor, inout float fogExponent, inout float fogBlend)
{

}

// Cloud puffs are drawn additively, so dark = more transparent. This allows you to influence the color used for a given
// puff. This color is multiplied with the puff texture in the fragment shader, and nothing more.
// By default the fogged color is multiplied by the cloudFade, voxelFade, and fogFade, which apply alpha effects on the
// cloud layer as a whole, on the individual puff (ie while clouds are growing in real time,) and from fog (we simulate atmospheric
// scattering by blending the puff into the sky behind it based on distance.)
// modelPos is the position of the puff vertex, before billboarding has been applied, and rotated into a Y-is-up system. So, the altitude
// of this vertex will be in modelPos.y.
void overrideBillboardColor(in vec3 eyePosition, in vec4 modelPos, in vec4 foggedColor, in float cloudFade, in float voxelFade, in float fogFade, inout vec4 finalColor)
{
	vVertex_View_Space = eyePosition;

	vec4 texCoord = gl_MultiTexCoord0.xyzw;
	vNormal_View_Space = vec3( sign( texCoord.yz ) * 2.0 - 1.0, 1.0 );
}

// Override the vertex color of a point on the sky box. The position in world space (relative to the camera) is given, as is
// our sky color before and after applying fog.
void overrideSkyColor(in vec3 vertexPos, in float fogDensity, in vec4 preFogColor, inout vec4 finalColor)
{

}


// Overrides colors of cirrus and cirrocumulus clouds
void overrideCirrusLighting(in vec3 lightColor, in vec3 fogColor, in float fogBlend, in float alpha, inout vec4 finalColor)
{

}

// Override the star colors.
void overrideStars(in vec4 worldPos, in float magnitude, in vec4 starColor, in vec4 fogColor, in float fogDensity, inout vec4 finalColor)
{

}

// override the color calculation for the Stratus clouds.
// finalColor = vec4(color.x, color.y, color.z, color.w * fadeAndDisplacementFactor);
void overrideStratocumulusColor(inout vec4 finalColor)
{

}

vec4 log_z_vs(in vec4 position);

// Provides a point to override the final value of gl_Position.
// Useful for implementing logarithmic depth buffers etc.
vec4 overridePosition(in vec4 position)
{
    vec4 adjustedClipPosition = log_z_vs(position);
	vVertex_Projection_Space = adjustedClipPosition;
	return adjustedClipPosition;
}