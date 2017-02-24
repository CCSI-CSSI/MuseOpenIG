#pragma import_defines (SHADOWING, ENVIRONMENTAL, AO, USER, ENVIRONMENTAL_FACTOR, HAS_NORMAL_MAP )

uniform float DepthOffset;
uniform sampler2D baseTexture;
uniform float todBasedLightBrightness;
uniform float todBasedEnvironmentalLightingFactor;

in vec4 vPositionProj;
in vec3 eyeVec;

in vec3 vPositionEyeSpace;
in vec3 vNormalEyeSpace;

#ifdef AO

uniform float ambientOcclusionFactor;
uniform sampler2D ambientOcclusionTexture;

vec4 computeAmbientOcclusion()
{
   vec4 aoColor = texture2D(ambientOcclusionTexture,gl_TexCoord[0].xy);
   aoColor.rgb *= ambientOcclusionFactor;
   return vec4(aoColor.rgb,1);
}
#endif

#ifdef SHADOWING
uniform float shadowsFactor;
float computeShadowFactor();
#endif

#ifdef ENVIRONMENTAL
uniform float environmentalFactor;
uniform samplerCube environmentalMapTexture;

vec4 computeEnvironmentalMap()
{																		
	vec3 v = gl_TexCoord[4].xzy;
    v.y *= -1.0;
    vec3 cubeColor = textureCube(environmentalMapTexture, v).rgb;
	return vec4(cubeColor,1);
}												
#endif

#ifdef HAS_NORMAL_MAP
uniform sampler2D normalMapSampler;
#endif

float computeFogFactor()
{
    float fogExp = gl_Fog.density * length(eyeVec); 
	float fogFactor = exp(-(fogExp * fogExp));
	fogFactor = clamp(fogFactor, 0.0, 1.0);

	return fogFactor;
}

#if defined(USE_LOG_DEPTH_BUFFER)
in float flogz;
uniform float Fcoef;
#endif

void user_function_forward_plus_lighting_override(float lightType
	, inout vec4 ambientColor, inout vec4 diffuseColor, inout vec4 specularColor
	, vec3 fCustomFloats)
{
	float todMultiplier = (fCustomFloats.x==99)? 1.0 : todBasedLightBrightness;
	ambientColor.xyz  *= todMultiplier;
	diffuseColor.xyz  *= todMultiplier;
	specularColor.xyz *= todMultiplier;
}

vec3 expand(vec3 v)
{
	return (v - 0.5) * 2.0;
}
// http://www.thetenthplanet.de/archives/1180
mat3 cotangent_frame( vec3 N, vec3 p, vec2 uv )
{
    // get edge vectors of the pixel triangle
    vec3 dp1 = dFdx( p );
    vec3 dp2 = dFdy( p );
    vec2 duv1 = dFdx( uv );
    vec2 duv2 = dFdy( uv );
 
    // solve the linear system
    vec3 dp2perp = cross( dp2, N );
    vec3 dp1perp = cross( N, dp1 );
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
 
    // construct a scale-invariant frame 
    float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
    return mat3( T * invmax, B * invmax, N );
}

vec3 compute_perturbed_normal_eye_space(vec3 vNormalEyeSpace, vec3 vPositionEyeSpace, sampler2D samplerNormalMap, vec2 uv)
{
	mat3 TBN = cotangent_frame(vNormalEyeSpace, vPositionEyeSpace, uv);
	vec3 vNormalMapValue = normalize(expand(texture2D(samplerNormalMap, uv).xyz));
	return normalize(TBN*vNormalMapValue);
}

vec4 compute_forward_plus_lighting(vec4 cDiffuseColor, vec4 cSpecularColor, float fSpecularExponent
								 , vec3 vPosition, vec3 vNormal, vec3 vEyePosition
								 , vec4 vPositionProj);

void main()
{
	vec3 vNormalEyeSpaceN = normalize(vNormalEyeSpace);
	vec3 vEyePosition = vec3(0,0,0);

#ifdef HAS_NORMAL_MAP
	vNormalEyeSpaceN = compute_perturbed_normal_eye_space(vNormalEyeSpaceN, vPositionEyeSpace, normalMapSampler, gl_TexCoord[0].xy);
#endif

	//if (lightCount==0)
	//{
	//	gl_FragColor = vec4(1,0,0,1);
	//	return;
	//}

	vec4 cDiffuseColor = texture2D( baseTexture, gl_TexCoord[0].xy ) * gl_FrontMaterial.diffuse;
	float alpha = cDiffuseColor.a;

#ifdef AO
	cDiffuseColor *= computeAmbientOcclusion();
#endif

//#ifdef SHADOWING
//	float fShadowFactor = computeShadowFactor();	
//	cDiffuseColor.rgb = mix(cDiffuseColor.rgb*0.5, cDiffuseColor.rgb, fShadowFactor);
//#endif
	vec4 fColor = compute_forward_plus_lighting(cDiffuseColor, gl_FrontMaterial.specular, gl_FrontMaterial.shininess
										 , vPositionEyeSpace, vNormalEyeSpaceN, vEyePosition
										 , vPositionProj);

	fColor = vec4(fColor.x, fColor.y, fColor.z, 1.0);

#ifdef ENVIRONMENTAL
    vec4 fReflectColor = computeEnvironmentalMap(); 
	//fColor.rgb = mix(fColor.xyz,fReflectColor.xyz,ENVIRONMENTAL_FACTOR).xyz;
#if 0
	// This suppose to be the proper way	fColor.rgb = ((1.0-ENVIRONMENTAL_FACTOR)*fColor.rgb + ENVIRONMENTAL_FACTOR*fReflectColor.rgb);
#else
	// But I like it this way better. Nick
	vec3 mixed_color = mix(fReflectColor.rgb, fColor.rgb, 1.0-ENVIRONMENTAL_FACTOR).rgb;
	fColor.rgb *= mixed_color * todBasedEnvironmentalLightingFactor;
#endif
#endif

	float fFogFactor = computeFogFactor();
    fColor.rgb = mix(gl_Fog.color, fColor, fFogFactor).rgb;

	gl_FragColor = vec4(fColor.rgb, alpha);

#if defined(USE_LOG_DEPTH_BUFFER)
	gl_FragDepth = log2(flogz) * Fcoef * 0.5 + DepthOffset;
#endif
}
