#define SELF_SHADOW_STAGE  1
#define CLOUDS_SHADOW_STAGE 4
uniform sampler2DShadow    shadowTexture;                                                      
uniform sampler2D          cloudShadowTexture; 
uniform sampler2DShadow    shadowTexture0;   
uniform int shadowTextureUnit0;
float computeShadowFactor()                                                                          
{                                                                                              
   float selfShadow = shadow2DProj( shadowTexture0, gl_TexCoord[shadowTextureUnit0] ).r;         
   // PPP: TO DO  
   float cloudsShadow = texture2D( cloudShadowTexture, gl_TexCoord[CLOUDS_SHADOW_STAGE].xy ).r;
   return selfShadow * cloudsShadow;                                                           
}                                                                                        