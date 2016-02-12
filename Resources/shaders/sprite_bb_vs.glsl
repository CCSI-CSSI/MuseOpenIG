#version 430 compatibility

out vec3 eyeVec;
out vec4 vsoutput_Color;

void main()
{
    gl_Position = gl_Vertex;

    vsoutput_Color = gl_Color;
}