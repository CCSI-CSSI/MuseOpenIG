
#ifdef TRITON_USE_FORWARD_PLUS_LIGHTING

uniform float u_transparency;

uniform mat4 model_view_matrix;
uniform mat4 model_view_inverse_transpose_matrix;

// This will come from Triton
uniform float trit_shininess;

uniform float LightBrightnessOnWater;

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


void compute_forward_plus_lighting(vec4 cDiffuseColor, vec4 cSpecularColor, float fSpecularExponent
								 , vec3 vPosition, vec3 vNormal, vec3 vEyePosition
								 , vec4 vPositionProj
							 	 , out vec4 fAmbientColor
								 , out vec4 fDiffuseColor
								 , out vec4 fSpecularColor);

void triton_user_lighting(in vec3 L
                   , in vec3 vVertex_World_Space, in vec3 vNormal_World_Space
                   , in vec4 vVertex_Projection_Space
                   , inout vec3 ambient, inout vec3 diffuse, inout vec3 specular)
{

   vec4 vVertex_View_Space4 = model_view_matrix                    *  vec4(vVertex_World_Space.x,vVertex_World_Space.y,vVertex_World_Space.z,1);
   vec4 vNormal_View_Space4 = model_view_inverse_transpose_matrix  *  vec4(vNormal_World_Space.x,vNormal_World_Space.y,vNormal_World_Space.z,1);

   vec3 vVertex_View_Space = vec3(vVertex_View_Space4.x,vVertex_View_Space4.y,vVertex_View_Space4.z);
   vec3 vNormal_View_Space = vec3(vNormal_View_Space4.x,vNormal_View_Space4.y,vNormal_View_Space4.z);

   vec4 fAmbientColor, fDiffuseColor, fSpecularColor;

   compute_forward_plus_lighting(vec4(1,1,1,1), vec4(1,1,1,1), 50
							   , vVertex_View_Space, vNormal_View_Space, vec3(0,0,0)
							   , vVertex_Projection_Space
							   , fAmbientColor, fDiffuseColor, fSpecularColor);
   ambient.xyz  += fAmbientColor.xyz;
   diffuse.xyz  += fDiffuseColor.xyz * 1;
   specular.xyz += fSpecularColor.xyz;
}
#else
void triton_user_lighting(in vec3 L
                   , in vec3 vVertex_World_Space, in vec3 vNormal_World_Space
                   , in vec4 vVertex_Projection_Space
                   , inout vec3 ambient, inout vec3 diffuse, inout vec3 specular)
{
}
#endif
