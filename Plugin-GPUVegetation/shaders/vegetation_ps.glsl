#version 430 compatibility
#extension GL_ARB_geometry_shader4 : enable

in vec3 vVertexViewSpace;
in vec3 vNormalViewSpace;
in vec4 vPositionProj;

uniform sampler2D color_texture;
uniform sampler2D baseTexture;

#pragma import_defines (USE_LOG_DEPTH_BUFFER)

#ifdef USE_LOG_DEPTH_BUFFER
in float flogz;
uniform float Fcoef;
#endif
uniform float todBasedLightBrightness;

vec4 compute_forward_plus_lighting(vec4 cDiffuseColor, vec4 cSpecularColor, float fSpecularExponent
								 , vec3 vPosition, vec3 vNormal, vec3 vEyePosition
								 , vec4 vPositionProj);

void user_function_forward_plus_lighting_override(float lightType
	, inout vec4 ambientColor, inout vec4 diffuseColor, inout vec4 specularColor
	, vec3 fCustomFloats)
{

}

float computeFogFactor()
{
    float fogExp = gl_Fog.density * length(vVertexViewSpace); 
	float fogFactor = exp(-(fogExp * fogExp));
	fogFactor = clamp(fogFactor, 0.0, 1.0);

	return fogFactor;
}
                                                             
void main() 
{			
	vec4 color = texture2D(baseTexture, gl_TexCoord[0].st);
	if (color.a < 0.6)
	{
	discard;
	}

	vec4 fColor = compute_forward_plus_lighting(vec4(color.xyz,1), vec4(0,0,0,1), 0
												, vVertexViewSpace, vNormalViewSpace, vec3(0,0,0)
												, vPositionProj);

	float fFogFactor = computeFogFactor();
	fColor.rgb = mix(gl_Fog.color, fColor, fFogFactor).rgb;
   
	gl_FragColor = vec4(fColor.rgb, fColor.a);

#ifdef USE_LOG_DEPTH_BUFFER
	gl_FragDepth = log2(flogz) * Fcoef * 0.5;
#endif
} 