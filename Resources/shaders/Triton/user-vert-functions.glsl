// Latest test against Triton SDK Version 3.91 February 10, 2018

varying vec3 eyeVec;

float user_get_depth( in vec3 worldPos )
{
    return 1000.0;
}

#ifdef DOUBLE_PRECISION

dvec3 user_horizon(in dvec3 intersect)
{
    return intersect;
}

#else

vec3 user_horizon(in vec3 intersect)
{
    return intersect;
}

#endif

vec4 log_z_vs(in vec4 position);

/* You may use this hook to set any varying parameters you need for the user-functions.glsl fragment program.
   provided are the ocean vertex in world, eye, and projected coordinates. */
void user_intercept(in vec3 worldPosition, in vec3 localPosition, in vec4 eyePosition, in vec4 projectedPosition)
{
    vec3 vVertex =  eyePosition.xyz;//vec3(gl_ModelViewMatrix * gl_Vertex);
    eyeVec = -vVertex;
}

// Provides a point to override the final value of gl_Position.
// Useful for implementing logarithmic depth buffers etc.
vec4 overridePosition(in vec4 position)
{
	return log_z_vs(position);
}
