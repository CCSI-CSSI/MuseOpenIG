uniform float Fcoef;
varying float flogz;
varying vec3 eyeVec;
varying vec3 lightDirs[8];

float user_get_depth( in vec3 worldPos )
{
    return 1000.0;
}

/* You may use this hook to set any varying parameters you need for the user-functions.glsl fragment program.
   provided are the ocean vertex in world, eye, and projected coordinates. */
void user_intercept(in vec3 worldPosition, in vec3 localPosition, in vec4 eyePosition, in vec4 projectedPosition)
{
    vec3 vVertex =  eyePosition.xyz;//vec3(gl_ModelViewMatrix * gl_Vertex);
    
    lightDirs[0] = vec3(gl_LightSource[0].position.xyz - vVertex);
    lightDirs[1] = vec3(gl_LightSource[1].position.xyz - vVertex);
    lightDirs[2] = vec3(gl_LightSource[2].position.xyz - vVertex);
    lightDirs[3] = vec3(gl_LightSource[3].position.xyz - vVertex);
    lightDirs[4] = vec3(gl_LightSource[4].position.xyz - vVertex);
    lightDirs[5] = vec3(gl_LightSource[5].position.xyz - vVertex);
    lightDirs[6] = vec3(gl_LightSource[6].position.xyz - vVertex);
    lightDirs[7] = vec3(gl_LightSource[7].position.xyz - vVertex);

    eyeVec = -vVertex;
}

// Provides a point to override the final value of gl_Position.
// Useful for implementing logarithmic depth buffers etc.
vec4 overridePosition(in vec4 position)
{
	vec4 adjustedClipPosition = position;
	if ( Fcoef > 0.0 )
	{
		adjustedClipPosition.z = (log2(max(1e-6, position.w+1.0))*Fcoef - 1.0) * position.w;
		flogz = 1.0 + position.w;
	}
	return adjustedClipPosition;
}
