#define SELF_SHADOW_STAGE  1
#define CLOUDS_SHADOW_STAGE 6
uniform sampler2DShadow    shadowTexture;                                                      
uniform sampler2D          cloudShadowTexture;                                                 
float computeShadowFactor()                                                                          
{                                                                                              
   float selfShadow = shadow2DProj( shadowTexture, gl_TexCoord[SELF_SHADOW_STAGE] ).r;         
   // PPP: TO DO  
 //float cloudsShadow = texture2D( cloudShadowTexture, gl_TexCoord[CLOUDS_SHADOW_STAGE].xy ).r;
   return selfShadow/* * cloudsShadow*/;                                                           
}                                                                                        