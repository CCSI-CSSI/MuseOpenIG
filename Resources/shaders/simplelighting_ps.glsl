#version 120
#pragma import_defines(SHADOWING,ENVIRONMENTAL,AO,ENVIRONMENTAL_FACTOR,USE_LOG_DEPTH_BUFFER)

varying vec3 normal;
varying vec3 eyeVec;
varying vec3 lightDirs[8];

uniform bool lightsEnabled[8];

uniform bool ViewOptions_EO;
uniform bool ViewOptions_IR;

const float cos_outer_cone_angle = 0.4; // 36 degrees


uniform sampler2D baseTexture;
uniform float todBasedLightBrightness;


#ifdef AO

uniform float ambientOcclusionFactor;
uniform sampler2D ambientOcclusionTexture;

vec4 computeAmbientOcclusion()
{
	vec4 aoColor = texture2D(ambientOcclusionTexture, gl_TexCoord[0].xy);
	aoColor.rgb *= ambientOcclusionFactor;
	return vec4(aoColor.rgb, 1);
}
#endif

#ifdef SHADOWING
uniform float shadowsFactor;
float DynamicShadow();
#endif

#ifdef ENVIRONMENTAL
uniform float environmentalFactor;
uniform samplerCube environmentalMapTexture;

vec4 computeEnvironmentalMap()
{
	vec3 v = gl_TexCoord[4].xzy;
	v.y *= -1.0;
	vec3 cubeColor = textureCube(environmentalMapTexture, v).rgb;
	return vec4(cubeColor, 1);
}
#endif

#ifdef USE_LOG_DEPTH_BUFFER
varying float flogz;
uniform float Fcoef;
#endif

float computeFogFactor()
{
	float fogExp = gl_Fog.density * length(eyeVec);
	float fogFactor = exp(-(fogExp * fogExp));
	fogFactor = clamp(fogFactor, 0.0, 1.0);

	return fogFactor;
}

void computeAmbientColor(inout vec4 color)
{
	vec4 final_color =
   (gl_FrontLightModelProduct.sceneColor * gl_FrontMaterial.ambient) +
	(gl_LightSource[0].ambient * gl_FrontMaterial.ambient);

	vec3 N = normalize(normal);
	vec3 L = normalize(gl_LightSource[0].position.xyz);

	float lambertTerm = max(dot(N,L),0.0);

	//if(lambertTerm > 0.0)
	{
		final_color += gl_LightSource[0].diffuse *
		               gl_FrontMaterial.diffuse *
					   lambertTerm;

		vec3 E = normalize(eyeVec);
		vec3 R = reflect(-L, N);
		float specular = pow( max(dot(R, E), 0.0),
		                 gl_FrontMaterial.shininess );
		final_color +=  gl_LightSource[0].specular *
                               gl_FrontMaterial.specular *
					   specular;
	}

	color += final_color;
}

void computeColorForLightSource(int i, inout vec4 color)
{
   if (!lightsEnabled[i]) return;

	vec4 final_color = vec4(0.0,0.0,0.0,1.0);

		float distSqr = dot(lightDirs[i],lightDirs[i]);
		float invRadius = gl_LightSource[i].constantAttenuation;
		float att = clamp(1.0-invRadius * sqrt(distSqr), 0.0, 1.0);
		vec3 L = lightDirs[i] * inversesqrt(distSqr);
		vec3 D = normalize(gl_LightSource[i].spotDirection);

		float cos_cur_angle = dot(-L, D);

		float cos_inner_cone_angle = gl_LightSource[i].spotCosCutoff;

		float cos_inner_minus_outer_angle =
			cos_inner_cone_angle - cos_outer_cone_angle;

		float spot = 0.0;
		spot = clamp((cos_cur_angle - cos_outer_cone_angle) /
			cos_inner_minus_outer_angle, 0.0, 1.0);

		vec3 N = normalize(normal);

		float lambertTerm = max( dot(N,L), 0.0);
		if(lambertTerm > 0.0)
		{
			final_color += gl_LightSource[i].diffuse *
				gl_FrontMaterial.diffuse *
				lambertTerm * spot * att;

			vec3 E = normalize(eyeVec);
			vec3 R = reflect(-L, N);

			float specular = pow( max(dot(R, E), 0.0),
				gl_FrontMaterial.shininess );

			final_color += gl_LightSource[i].specular *
				gl_FrontMaterial.specular *
				specular * spot * att;
		}


		color += final_color;
}

void lighting( inout vec4 color )
{
	vec4 clr = vec4(0.0);

	for (int i = 1; i < 8; i++)
	{
		computeColorForLightSource(i,clr);
	}
	computeAmbientColor(clr);

	color *= clr;

}

float snoise(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
	vec4 cDiffuseColor = texture2D( baseTexture, gl_TexCoord[0].xy );
	float alpha = cDiffuseColor.a;

#ifdef AO
	cDiffuseColor *= computeAmbientOcclusion();
#endif

#ifdef SHADOWING
	float fShadowFactor = DynamicShadow();
	cDiffuseColor.rgb = mix(cDiffuseColor.rgb*(1.0 - shadowsFactor), cDiffuseColor.rgb, fShadowFactor);
#endif

   lighting(cDiffuseColor);

#ifdef ENVIRONMENTAL
	vec4 fReflectColor = computeEnvironmentalMap();
	cDiffuseColor.rgb = ((1.0 - ENVIRONMENTAL_FACTOR)*cDiffuseColor.rgb + ENVIRONMENTAL_FACTOR*fReflectColor.rgb);
#endif

	float fFogFactor = computeFogFactor();
	cDiffuseColor.rgb = mix(gl_Fog.color, cDiffuseColor, fFogFactor).rgb;

	gl_FragColor = vec4(cDiffuseColor.rgb, alpha);

	if (ViewOptions_IR)
	{
		//uses NTSC conversion weights
		float gray = dot(cDiffuseColor.rgb, vec3(0.299, 0.587, 0.114));		
		vec4 gray_color = vec4( gray, gray, gray, alpha);

		// This one causes issues on Mac
		float n = snoise(vec2(gl_TexCoord[0].x,gl_TexCoord[0].y)); 		
		vec4 noise_color = vec4(n, n, n, alpha )*1.5;		

		gl_FragColor = mix(noise_color,gray_color,0.7);
	}
	else
	if (ViewOptions_EO)
	{
		float gray = dot(cDiffuseColor.rgb, vec3(0.299, 0.587, 0.114));		
		gl_FragColor = vec4( gray, gray, gray, alpha);
	}
#if defined(USE_LOG_DEPTH_BUFFER)
	//gl_FragDepth = log2(flogz) * Fcoef * 0.5;
#endif
}