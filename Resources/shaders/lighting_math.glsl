
vec3 GetEyeVector(vec3 vPosition, vec3 vEyePosition)
{
    return normalize(vEyePosition-vPosition);
}
vec3 GetHalfVector(vec3 vEyeVector, vec3 vLightVector)
{
    return normalize(vEyeVector+vLightVector);
}
vec3 GetLightVector(vec3 vPosition, vec3 vLightPosition)
{
    return normalize(vLightPosition-vPosition);
}

float ComputeAttenutation(float fStartRange, float fEndRange, float fBetweenVertexAndLightPos)
{
    if (fBetweenVertexAndLightPos>fEndRange)
	{
        return 0.0f;
	}
    return 1 - smoothstep(fStartRange, fEndRange, fBetweenVertexAndLightPos);
}

// theta -> inner angle
// phi   -> outer angle
// spot = 1 if rho >  cos(theta/2) or if non-spot light
// spot = 0 if rho <= cos(phi/2)
//      |    rho - cos(phi/2)     |
//      |-------------------------|^falloff
//      |cos(theta/2) - cos(phi/2)|

float ComputeSpotLightFactor(float rho, float fInnerAngle, float fOuterAngle, float fFallOff)
{
    float f_cos_theta_by_2 = cos(fInnerAngle*0.5f);
    float f_cos_phi_by_2   = cos(fOuterAngle*0.5f);

    if (rho >  f_cos_theta_by_2)
        return 1.0f;
    if (rho <= f_cos_phi_by_2)
        return 0.0f;
    return pow(((rho-f_cos_phi_by_2)/(f_cos_theta_by_2-f_cos_phi_by_2)), fFallOff);
}

// Diffuse Lighting = sum[Cd*Ld*(N.Ldir)*Atten*Spot]
vec4 ComputeDiffuseLighting(vec4 cDiffuseColor
                          , vec3 cLightDiffuseColor
                          , vec3 vNormal
                          , vec3 vLightDirection
                          , float fAttenuation
                          , float fSpot
                          , out float n_dot_l
                            )
{
	n_dot_l = dot(vNormal, vLightDirection);
    return cDiffuseColor
         * vec4(cLightDiffuseColor,1)
         * max(0,n_dot_l)
         * fAttenuation
         * fSpot;
}

// Specular Lighting = Cs * sum[Ls * (N · H)P * Atten * Spot]
vec4 ComputeSpecularLighting(vec4 cSpecularColor
                             , vec3 cLightSpecularColor
                             , vec3 vNormal
                             , vec3 vHalfVector
                             , float fAttenuation
                             , float fSpot
                             , float fSpecularExponent
                             , float n_dot_l)   
{
    return cSpecularColor
         * vec4(cLightSpecularColor,1)
         * pow(max(0,dot(vNormal, vHalfVector)), fSpecularExponent)
         * fAttenuation
         * fSpot
         * max(n_dot_l,0);
}

