#version 130
#pragma import_defines (USE_LOG_DEPTH_BUFFER)

in vec3 eyeVec;

#ifdef USE_LOG_DEPTH_BUFFER
in float flogz;
uniform float Fcoef;
#endif

void applyFog(inout vec4 color)
{
   float fogExp = gl_Fog.density * length(eyeVec);
   float fogFactor = exp(-(fogExp * fogExp));
   fogFactor = clamp(fogFactor, 0.0, 1.0);
   vec4 clr = color;
   color = mix(gl_Fog.color, color, fogFactor);
   color.a = clr.a;
}

void main()
{
	vec4 color = gl_Color;
	applyFog(color);
	gl_FragColor = color;
#ifdef USE_LOG_DEPTH_BUFFER
	gl_FragDepth = log2(flogz) * Fcoef * 0.5;
#endif
}		