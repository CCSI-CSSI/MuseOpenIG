
// We have to set these up up for Forward+ to work correctly
// x = tile_size_x, y = tile_size_y
// z = viewport_width, w = viewport_height
uniform ivec4 vTilingParams;

uniform samplerBuffer  lightDataTBO;
uniform isamplerBuffer lightIndexListTBO;
uniform isamplerBuffer lightGridOffsetAndSizeTBO;

uniform sampler2D rampTexture;

struct Light
{
    vec3 cAmbientColor;
    vec3 cDiffuseColor;
    vec3 cSpecularColor;

    vec3 vPosition;
    vec3 vDirection;

    vec3 spotparams;

    vec3 rangesandtype;

    vec3 pack;
};

void UnpackLight(int lightIndex, inout Light light)
{
    int texcoord = lightIndex*6;

    vec4 FourFloat1s = texelFetchBuffer(lightDataTBO, texcoord); texcoord += 1;
    vec4 FourFloat2s = texelFetchBuffer(lightDataTBO, texcoord); texcoord += 1;
    vec4 FourFloat3s = texelFetchBuffer(lightDataTBO, texcoord); texcoord += 1;
    vec4 FourFloat4s = texelFetchBuffer(lightDataTBO, texcoord); texcoord += 1;
    vec4 FourFloat5s = texelFetchBuffer(lightDataTBO, texcoord); texcoord += 1;
    vec4 FourFloat6s = texelFetchBuffer(lightDataTBO, texcoord);

    light.cAmbientColor  = vec3(FourFloat1s.x, FourFloat1s.y, FourFloat1s.z);
    light.cDiffuseColor  = vec3(FourFloat1s.w, FourFloat2s.x, FourFloat2s.y);
    light.cSpecularColor = vec3(FourFloat2s.z, FourFloat2s.w, FourFloat3s.x);

    light.vPosition      = vec3(FourFloat3s.y, FourFloat3s.z, FourFloat3s.w);
    light.vDirection     = vec3(FourFloat4s.x, FourFloat4s.y, FourFloat4s.z);
          
    light.spotparams     = vec3(FourFloat4s.w, FourFloat5s.x, FourFloat5s.y);
    light.rangesandtype  = vec3(FourFloat5s.z, FourFloat5s.w, FourFloat6s.x);

	light.pack			 = vec3(FourFloat6s.y, FourFloat6s.z, FourFloat6s.w);
}

vec2 ProjectToNDC(vec4 vPositionProj)
{
    vec2 vNDC = vec2(vPositionProj.x, vPositionProj.y);

    vNDC /= vPositionProj.w;
    vNDC = clamp(vNDC, -1, 1);
    vNDC = vNDC*0.5f + 0.5f;
    
    return vNDC;
}

ivec2 ProjectToWindowCoords(vec4 vPositionProj, ivec2 viewPortSize)
{
    vec2 vNDC = ProjectToNDC(vPositionProj);
    vNDC *= viewPortSize;
    return ivec2(int(vNDC.x), int(vNDC.y));
}

// Client must provide!!
void user_function_forward_plus_lighting_override(float lightType
	, inout vec4 ambientColor, inout vec4 diffuseColor, inout vec4 specularColor
	, vec3 fCustomFloats);

void ComputeLighting(int index
    , vec4 cDiffuseColor, vec4 cSpecularColor, float fSpecularExponent
    , vec3 vPosition, vec3 vNormal, vec3 vEyePosition
	, out vec4 fAmbientColor
	, out vec4 fDiffuseColor
	, out vec4 fSpecularColor)
{
    Light light;
    UnpackLight(index, light);

	float fStartRange = light.rangesandtype.x;
	float fEndRange   = light.rangesandtype.y;
	float lightType   = light.rangesandtype.z;

	float fAttenuation = 0.0f;
    if (lightType == 0)
    {
        fAttenuation = 1.0f; // Attenuation is 1 for directional light
    }
    else if (lightType == 1 || lightType == 2)
    {
        float fBetweenVertexAndLightPos = distance(vPosition,light.vPosition);
        if (fBetweenVertexAndLightPos>fEndRange)
		{
            fAmbientColor  = vec4(0,0,0,0);
			fDiffuseColor  = vec4(0,0,0,0);
			fSpecularColor = vec4(0,0,0,0);
			return;
		}
        fAttenuation = ComputeAttenutation(fStartRange
										 , fEndRange
                                         , fBetweenVertexAndLightPos);
    }

    //LT_DIRECTIONAL = 0
    //LT_POINT = 1
    //LT_SPOTLIGHT = 2
    vec3 vLightDirection = vec3(0,0,0);
    if (lightType == 0)
    {
        vLightDirection = normalize(-light.vDirection);
    }
    else
    {
        vLightDirection = GetLightVector(vPosition, light.vPosition);
		
    }

	float fSpot = 0.0f;
    if (lightType == 0 || lightType == 1)
    {
        fSpot = 1.0f;        // Spot is 1 for non-spot lights

		
    }
    else
    {
        vec3 vSpotLightDirection = normalize(light.vDirection);

        float rho = dot(-vSpotLightDirection, vLightDirection);
        fSpot = ComputeSpotLightFactor(rho, light.spotparams.x, light.spotparams.y, light.spotparams.z);
    }

    float n_dot_l = 0.0f;

	fAmbientColor  = cDiffuseColor*vec4(light.cAmbientColor,1);

    fDiffuseColor  = ComputeDiffuseLighting(cDiffuseColor, light.cDiffuseColor
                               , vNormal, vLightDirection
                               , fAttenuation, fSpot, n_dot_l);

    fSpecularColor = ComputeSpecularLighting(cSpecularColor, light.cSpecularColor
                                    , vNormal, GetHalfVector(GetEyeVector(vPosition, vEyePosition), vLightDirection)
                                    , fAttenuation, fSpot
                                    , fSpecularExponent
                                    , n_dot_l);

	user_function_forward_plus_lighting_override(lightType, fAmbientColor, fDiffuseColor, fSpecularColor, light.pack);
}

void compute_forward_plus_lighting(vec4 cDiffuseColor, vec4 cSpecularColor, float fSpecularExponent
								 , vec3 vPosition, vec3 vNormal, vec3 vEyePosition
								 , vec4 vPositionProj
								 , out vec4 fAmbientColor, out vec4 fDiffuseColor, out vec4 fSpecularColor)
{
	ivec2 tile_size = ivec2(vTilingParams.x, vTilingParams.y);
	ivec2 viewport_size = ivec2(vTilingParams.z, vTilingParams.w);

	ivec2 vWindowCoords = ProjectToWindowCoords(vPositionProj, viewport_size);

	ivec2 grid_max_dim;
	grid_max_dim.x =  ((viewport_size.x + tile_size.x - 1) / tile_size.x);
	grid_max_dim.y =  ((viewport_size.y + tile_size.y - 1) / tile_size.y);

	ivec2 texcoords = ivec2(int(vWindowCoords.x) / tile_size.x, int(vWindowCoords.y) / tile_size.y);
	ivec2 val = texelFetchBuffer(lightGridOffsetAndSizeTBO, texcoords.x + texcoords.y * grid_max_dim.x).xy;
    int lightCount  = val.x;
    int lightOffset = val.y;
	
	lightCount = min(lightCount, MAX_LIGHTS_PER_PIXEL);

	fAmbientColor  = vec4(0,0,0,0);
	fDiffuseColor  = vec4(0,0,0,0);
	fSpecularColor = vec4(0,0,0,0);

	for(int i = 0; i < lightCount ; ++i)
    {
		vec4 fCurrentAmbientColor  = vec4(0,0,0,0);
		vec4 fCurrentDiffuseColor  = vec4(0,0,0,0);
		vec4 fCurrentSpecularColor = vec4(0,0,0,0);

		texcoords.x   = (lightOffset + i)/4;
		int component = (lightOffset + i)%4;

		int lightIndex = texelFetchBuffer(lightIndexListTBO, texcoords.x)[component];

		ComputeLighting(lightIndex
                      , cDiffuseColor, cSpecularColor, fSpecularExponent
                      , vPosition, vNormal, vEyePosition
					  , fCurrentAmbientColor, fCurrentDiffuseColor, fCurrentSpecularColor);

		fAmbientColor  += fCurrentAmbientColor;
		fDiffuseColor  += fCurrentDiffuseColor;
		fSpecularColor += fCurrentSpecularColor;
	}
}

vec4 compute_forward_plus_lighting(vec4 cDiffuseColor, vec4 cSpecularColor, float fSpecularExponent
								 , vec3 vPosition, vec3 vNormal, vec3 vEyePosition
								 , vec4 vPositionProj)
{

	vec4 fAmbientColor  = vec4(0,0,0,0);
	vec4 fDiffuseColor  = vec4(0,0,0,0);
	vec4 fSpecularColor = vec4(0,0,0,0);

	compute_forward_plus_lighting(cDiffuseColor, cSpecularColor, fSpecularExponent
								, vPosition, vNormal, vEyePosition
								, vPositionProj
								, fAmbientColor, fDiffuseColor, fSpecularColor);

	return fAmbientColor + fDiffuseColor + fSpecularColor;
}