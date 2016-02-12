#ifdef SL_USE_FORWARD_PLUS_LIGHTING   

varying vec3 vVertex_View_Space;
varying vec4 vVertex_Projection_Space;
varying vec3 vNormal_View_Space;

uniform float LightBrightnessOnClouds;

void compute_forward_plus_lighting(vec4 cDiffuseColor, vec4 cSpecularColor, float fSpecularExponent
								 , vec3 vPosition, vec3 vNormal, vec3 vEyePosition
								 , vec4 vPositionProj
							 	 , out vec4 fAmbientColor
								 , out vec4 fDiffuseColor
								 , out vec4 fSpecularColor);

void user_function_forward_plus_lighting_override(float lightType
	, inout vec4 ambientColor, inout vec4 diffuseColor, inout vec4 specularColor
	, vec3 fCustomFloats)
{
	// Triton's lighting for sun/moon comes directly from Triton
	if (fCustomFloats.x==99)
	{
		ambientColor  = vec4(0,0,0,0);
		diffuseColor  = vec4(0,0,0,0);
		specularColor = vec4(0,0,0,0);
	}
}
void overrideBillboardFragment_forward_plus_sl_ps(in vec4 texColor, in vec4 lightColor, inout vec4 finalColor)
{

   vec3 vNormal_View_Space1 = normalize(vec3(-vVertex_View_Space.x,-vVertex_View_Space.y,-vVertex_View_Space.z));

   vec4 fAmbientColor = vec4(0.0,0.0,0.0,0.0);
   vec4 fDiffuseColor = vec4(0.0,0.0,0.0,0.0);
   vec4 fSpecularColor = vec4(0.0,0.0,0.0,0.0);
	   
   compute_forward_plus_lighting(vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), 10.0
							   , vVertex_View_Space, vNormal_View_Space1, vec3(0.0,0.0,0.0)
							   , vVertex_Projection_Space
							   , fAmbientColor, fDiffuseColor, fSpecularColor);

   float factor = LightBrightnessOnClouds*texColor.a;
   vec4 lc = vec4(lightColor.r,lightColor.g,lightColor.b,0.0);
   finalColor = texColor*(lc+vec4(fAmbientColor.xyz+fDiffuseColor.xyz+fSpecularColor.xyz,0.0)*factor);
   finalColor.a = texColor.a;
}

#else
void overrideBillboardFragment_forward_plus_sl_ps(in vec4 texColor, in vec4 lightColor, inout vec4 finalColor)
{
	finalColor = texColor * lightColor;
}
#endif