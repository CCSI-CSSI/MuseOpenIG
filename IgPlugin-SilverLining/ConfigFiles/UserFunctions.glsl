varying vec3 normal;
varying vec3 eyeVec;
varying vec3 lightDirs[8];

uniform mat4 sl_modelView;
uniform mat4 sl_billboard;
uniform vec4 sl_fade;
uniform mat4 sl_basis;

uniform vec4 upVectorAndThickness;
uniform int museIREnabled;

uniform float Fcoef;
varying float flogz;

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
    vec4 texCoord = gl_MultiTexCoord0.xyzw;

    vec4 vertPos = vec4(abs(texCoord.x) * sign(texCoord.y), abs(texCoord.x) * sign(texCoord.z), 0.0, 1.0);

    vec4 rotatedPos;

    float angle = abs(texCoord.z) + abs(texCoord.y) * sign(texCoord.x) * sl_fade.w;
    angle = mod(angle, 3.14159265 * 2.0);
    float cosLength = cos(angle);
    float sinLength = sin(angle);

    rotatedPos.x = (cosLength * vertPos.x + -sinLength * vertPos.y);
    rotatedPos.y = (sinLength * vertPos.x + cosLength * vertPos.y);
    rotatedPos.z = 0.0;
    rotatedPos.w = 1.0;

    vec4 vert = gl_Vertex * sl_basis;

    mat4 xlate = mat4(1.0, 0.0, 0.0, 0.0,
                  0.0, 1.0, 0.0, 0.0,
                  0.0, 0.0, 1.0, 0.0,
                  vert.x, vert.y, vert.z, 1.0);

    mat4 xlatebb = (xlate * sl_billboard);

    #if 1
    // I think this formula may be more precise

    mat4 mv = (sl_modelView * xlatebb);
    vec3 vVertex = vec3(mv * rotatedPos);
    #else
    // This old one is also crudely correct but does not include
    // bilboard rotation into account
        vec3 vVertex = vec3(gl_ModelViewMatrix * gl_Vertex);
    #endif

    lightDirs[0] = vec3(gl_LightSource[0].position.xyz);
    lightDirs[1] = vec3(gl_LightSource[1].position.xyz - vVertex);
    lightDirs[2] = vec3(gl_LightSource[2].position.xyz - vVertex);
    lightDirs[3] = vec3(gl_LightSource[3].position.xyz - vVertex);
    lightDirs[4] = vec3(gl_LightSource[4].position.xyz - vVertex);
    lightDirs[5] = vec3(gl_LightSource[5].position.xyz - vVertex);
    lightDirs[6] = vec3(gl_LightSource[6].position.xyz - vVertex);
    lightDirs[7] = vec3(gl_LightSource[7].position.xyz - vVertex);

    eyeVec = -vVertex;

    // Apparently Bilboards do not use normals because
    // following does not produce reasonable normal values :

    //  normal = gl_NormalMatrix * gl_Normal;

    // However, these are bilboards, aren't they ? (See this shader file name in doubts)
    // So they are always facing the camera thus the normal is the same as eyeVec

    #if 0
    normal = eyeVec;
    #else
    // Trick which sets opposite normals at corners of quad bilboard
    // this will make normals spread into -45 to +45 degrees range over bilboard
    // I suppose it can behave nicer with specular highlight math
    normal = vec3( sign( texCoord.yz ) * 2.0 - 1.0, 1. );
    #endif

}

// Override the vertex color of a point on the sky box. The position in world space (relative to the camera) is given, as is
// our sky color before and after applying fog.
void overrideSkyColor(in vec3 vertexPos, in vec4 preFogColor, inout vec4 finalColor)
{
    //Turn Sky grey for a simulated IR mode on POD
    if(museIREnabled == 1)
    {
        float Color = 0.0;

        //Get lowest color setting and ensure
        //they are all the same so that we still
        //have a shade of grey for the sky color
        //and also it adjusts a bit for day/night sky
        if(finalColor.x < finalColor.y)
            Color = finalColor.x;
        else
            Color = finalColor.y;

        if(finalColor.z < Color)
            Color = finalColor.z;

        //clamp the color level between 0.1 and 0.5
        //so we have a limited shade of grey
        float outcolor = clamp(Color, 0.1, 0.5);
        //Send out finalColor now
        finalColor.x = outcolor;
        finalColor.y = outcolor;
        finalColor.z = outcolor;
        finalColor.w = 1.0;
    }
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

// override the color calculation for the Stratus clouds.
// finalColor = vec4(color.x, color.y, color.z, color.w * fadeAndDisplacementFactor);
void overrideStratusColor(in vec4 color, in float fadeAndDisplacementFactor, inout vec4 finalColor)
{

}

// Provides a point to override the final value of gl_Position.
// Useful for implementing logarithmic depth buffers etc.
vec4 overridePosition(in vec4 position)
{
	vec4 adjustedClipPosition = position;
	if ( Fcoef > 0.0 )
	{
		adjustedClipPosition.z = (log2(max(1e-6, position.w+1.0))*Fcoef - 1.0) * position.w;
		flogz = 1.0 + adjustedClipPosition.w;
	}
	return adjustedClipPosition;
}
