#version 120
#pragma import_defines (USE_LOG_DEPTH_BUFFER)

varying vec3 eyeVec;

#ifdef USE_LOG_DEPTH_BUFFER
varying float flogz;
uniform float Fcoef;
#endif

uniform bool ViewOptions_EO;
uniform bool ViewOptions_IR;

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

	if (ViewOptions_EO || ViewOptions_IR)
	{
		//uses NTSC conversion weights
		float gray = dot(color.rgb, vec3(0.299, 0.587, 0.114));
		gl_FragColor = vec4(gray, gray, gray, color.a);
	}	
	else
	{
		gl_FragColor = color;
	}
#ifdef USE_LOG_DEPTH_BUFFER
	gl_FragDepth = log2(flogz) * Fcoef * 0.5;
#endif
}		