#version 430 compatibility
#extension GL_ARB_geometry_shader4 : enable

in vec4 inUV;
in vec3 inScale;

out vec4 vsoutput_UV;
out vec3 vsoutput_Scale;

void main()
{
   vsoutput_Scale = inScale;
   vsoutput_UV	  = inUV;
   gl_Position = gl_Vertex;
}