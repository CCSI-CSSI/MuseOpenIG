#version 430 compatibility

#pragma import_defines(USE_LOG_DEPTH_BUFFER,MODULATE_WITH_VERTEX_COLOR,WEIGHTED_HOT_SPOT)

#ifdef USE_LOG_DEPTH_BUFFER
varying float flogz;
uniform float Fcoef;
#endif

uniform sampler2D spriteTexture;

in vec3 gsoutput_eveVec;
in vec2 vTexCoords;
in vec4 gsoutput_Color;

void applyFog(inout vec4 color)
{   
   float fogExp = gl_Fog.density * length(gsoutput_eveVec);
   float fogFactor = exp(-(fogExp * fogExp));
   fogFactor = clamp(fogFactor, 0.0, 1.0);
   vec4 clr = color;
   color = mix(vec4(0,0,0,0), color, fogFactor);
   color.a = clr.a;
}                   
void main()
{
    vec4 decalColor = texture2D(spriteTexture, vTexCoords);

#ifdef MODULATE_WITH_VERTEX_COLOR
    decalColor.rgb *= gsoutput_Color.rgb;
#endif

#ifdef WEIGHTED_HOT_SPOT
    // As we go from 0 to max distance, color should go from 1,1,1 to decalColor
    float distFromCenter = length(vec2(0.5,0.5) - vTexCoords);
    float weight = smoothstep(0.0,0.05, distFromCenter);

    decalColor.rgb = mix(vec3(1,1,1), decalColor.rgb, weight);
#endif

    applyFog(decalColor);
	gl_FragColor = decalColor;

#ifdef USE_LOG_DEPTH_BUFFER
	gl_FragDepth = log2(flogz) * Fcoef * 0.5;
#endif
}