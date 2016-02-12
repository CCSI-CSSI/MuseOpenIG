#version 130

#pragma import_defines (USE_LOG_DEPTH_BUFFER)

#ifdef USE_LOG_DEPTH_BUFFER
uniform float Fcoef;
out float flogz;
#endif

out vec3 eyeVec;

void main()
{
    eyeVec = -vec3(gl_ModelViewMatrix * gl_Vertex);
    gl_FrontColor = gl_Color;
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

#ifdef USE_LOG_DEPTH_BUFFER
	gl_Position.z = (log2(max(1e-6, 1.0 + gl_Position.w)) * Fcoef - 1.0) * gl_Position.w;
    flogz = 1.0 + gl_Position.w;
#endif
}
