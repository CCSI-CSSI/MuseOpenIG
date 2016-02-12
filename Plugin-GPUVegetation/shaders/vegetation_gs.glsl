#version 430 compatibility              
#extension GL_ARB_geometry_shader4 : enable  
                                     

#pragma import_defines (USE_LOG_DEPTH_BUFFER)
#ifdef USE_LOG_DEPTH_BUFFER
uniform float Fcoef;
out float flogz;
#endif
                                             
in vec3 vsoutput_Scale[];
in vec4 vsoutput_UV[];

out vec3 vVertexViewSpace;
out vec3 vNormalViewSpace;
out vec4 vPositionProj;

                                             
void setupVaryings(in vec4 vVertex, in vec4 vOffset)
{
   vVertexViewSpace = vec3(gl_ModelViewMatrix * vVertex);
   vNormalViewSpace = vec3(0.0,1.0,0.0);

#ifdef USE_LOG_DEPTH_BUFFER
   gl_Position.z = (log2(max(1e-6, 1.0 + gl_Position.w)) * Fcoef - 1.0) * gl_Position.w;
   flogz = 1.0 + gl_Position.w;
#endif

   vPositionProj = gl_ModelViewProjectionMatrix * vec4((vVertex+vOffset).xyz,1);
}

void Emit( in vec4 v, in vec4 offset, in vec2 tc)   
{
   gl_Position = gl_ModelViewProjectionMatrix * (v + offset);

   gl_TexCoord[0].st = tc;
   setupVaryings(v, offset);
   EmitVertex();
}
 void main()
{
   float width_half = vsoutput_Scale[0].y/2.0;
   float height = vsoutput_Scale[0].z;

   vec4 v = gl_PositionIn[0];

   Emit(v, vec4(-width_half,0.0,0.0,0.0), vec2(vsoutput_UV[0].x,vsoutput_UV[0].y));
   Emit(v, vec4(width_half,0.0,0.0,0.0), vec2(vsoutput_UV[0].z,vsoutput_UV[0].y));
   Emit(v, vec4(-width_half,0.0,height,0.0), vec2(vsoutput_UV[0].x,vsoutput_UV[0].w));
   Emit(v, vec4(width_half,0.0, height,0.0), vec2(vsoutput_UV[0].z,vsoutput_UV[0].w));
   EndPrimitive();

   Emit(v, vec4(0.0,-width_half,0.0,0.0),  vec2(vsoutput_UV[0].x,vsoutput_UV[0].y));
   Emit(v, vec4(0.0,width_half,0.0,0.0), vec2(vsoutput_UV[0].z,vsoutput_UV[0].y));
   Emit(v, vec4(0.0,-width_half,height,0.0), vec2(vsoutput_UV[0].x,vsoutput_UV[0].w));
   Emit(v, vec4(0.0,width_half,height,0.0), vec2(vsoutput_UV[0].z,vsoutput_UV[0].w));
   EndPrimitive();
} 