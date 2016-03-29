#define SELF_SHADOW_STAGE 1
#define CLOUDS_SHADOW_STAGE 6
uniform mat4 cloudShadowCoordMatrix;                                                      
void DynamicShadow( in vec4 ecPosition )                                                  
{                                                                                         
	// generate coords for shadow mapping                                                 
	gl_TexCoord[SELF_SHADOW_STAGE].s = dot( ecPosition, gl_EyePlaneS[SELF_SHADOW_STAGE] );
	gl_TexCoord[SELF_SHADOW_STAGE].t = dot( ecPosition, gl_EyePlaneT[SELF_SHADOW_STAGE] );
	gl_TexCoord[SELF_SHADOW_STAGE].p = dot( ecPosition, gl_EyePlaneR[SELF_SHADOW_STAGE] );
	gl_TexCoord[SELF_SHADOW_STAGE].q = dot( ecPosition, gl_EyePlaneQ[SELF_SHADOW_STAGE] );
	gl_TexCoord[CLOUDS_SHADOW_STAGE] = cloudShadowCoordMatrix * ecPosition;               
}                                                                                         