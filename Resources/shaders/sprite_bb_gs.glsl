

#version 430 compatibility
#extension GL_EXT_geometry_shader4 : enable

#pragma import_defines(USE_LOG_DEPTH_BUFFER)

layout (points) in;
layout (triangle_strip) out;
layout (max_vertices = 6) out;

#ifdef USE_LOG_DEPTH_BUFFER
uniform float Fcoef;
out float flogz;

void doLogZBufferXForm()
{
    gl_Position.z = (log2(max(1e-6, 1.0 + gl_Position.w)) * Fcoef - 1.0) * gl_Position.w;
    flogz = 1.0 + gl_Position.w;
}
#else
void doLogZBufferXForm()
{
}
#endif

uniform vec4 spriteDimensions;
uniform ivec4 vTilingParams;

out vec2 vTexCoords;
const vec2 t0 = vec2(0,0);
const vec2 t1 = vec2(1,0);
const vec2 t2 = vec2(1,1);
const vec2 t3 = vec2(0,1);

in vec4 vsoutput_Color[];
out vec4 gsoutput_Color;
out vec3 gsoutput_eveVec;

//vec2 Project(in vec4 v, float width, float height)
//{
//    vec4 vNormalized = gl_ModelViewProjectionMatrix*v;
//    vNormalized.w = abs(vNormalized.w);
//
//    vNormalized.x = clamp(vNormalized.x, -vNormalized.w, vNormalized.w);
//
//    vNormalized.x /= vNormalized.w;
//
//    vNormalized.x = vNormalized.x*0.5 + 0.5;
//
//    vNormalized.x *= width;
//}

//void clampToMinMaxPixelSize(inout vec4 v0, inout vec4 v1, inout vec4 v2, inout vec4 v3
//    , float minPixelSize, float maxPixelSize
//    , float viewport_width, float viewportHeight)
//{
//
//}

vec2 rotate(vec2 coords, float theta)
{
    coords = coords-0.5;

    float x = coords.x*cos(theta) - coords.y*sin(theta);
    float y = coords.x*sin(theta) + coords.y*cos(theta);
    
    return vec2(x,y) + 0.5;
}

vec4 project(vec4 vPosition)
{
	vec4 position = gl_ModelViewProjectionMatrix*vPosition;
	position.w = abs(position.w);

	position.x = clamp(position.x, -position.w,position.w);
    position.y = clamp(position.y, -position.w,position.w);

	position /= position.w;

	return position*0.5 + 0.5;
}

vec2 worldSizeToPixelSize(vec4 v0, vec4 v1)
{
	v0 = project(v0);
	v1 = project(v1);
	return vec2(abs(v0.x-v1.x)*vTilingParams.z, abs(v0.y-v1.y)*vTilingParams.w);
}

vec4 adjustOffset(vec4 vc, vec2 offset)
{
	vec4 v = vc;
	v.xy = v.xy/v.w;
	v.xy = v.xy + offset;
	v.xy = v.xy*v.w;
	return v;
}

void main()
{
    vec4 vCenter = gl_in[0].gl_Position;

	gsoutput_eveVec = -vec3(gl_ModelViewMatrix * vCenter);

    float minPixelSize = spriteDimensions.x;
    float maxPixelSize = spriteDimensions.y;
    float width        = spriteDimensions.z;

	bool adjustSize = false;
    vec2 actualSize = worldSizeToPixelSize(vCenter + vec4(-width, -width, 0, 0), vCenter + vec4( width, width, 0, 0));
    if (actualSize.y<minPixelSize||actualSize.y<minPixelSize) 
    {
		adjustSize = true;
    }
	float adjustedWidth  = minPixelSize/vTilingParams.z;
	float adjustedHeight = minPixelSize/vTilingParams.w;
	
	vec4 v0, v1, v2, v3;
	
	if (adjustSize==false)
	{
		v0 = gl_ModelViewProjectionMatrix*(vCenter + vec4(-width, -width, 0, 0));
		v1 = gl_ModelViewProjectionMatrix*(vCenter + vec4( width, -width, 0, 0));
		v2 = gl_ModelViewProjectionMatrix*(vCenter + vec4( width,  width, 0, 0));
		v3 = gl_ModelViewProjectionMatrix*(vCenter + vec4(-width,  width, 0, 0));
	}
	else
	{
		vec4 vc = gl_ModelViewProjectionMatrix*vCenter;

		v0 = adjustOffset(vc, vec2(-adjustedWidth,-adjustedHeight));
		v1 = adjustOffset(vc, vec2(+adjustedWidth,-adjustedHeight));
		v2 = adjustOffset(vc, vec2(+adjustedWidth,+adjustedHeight));
		v3 = adjustOffset(vc, vec2(-adjustedWidth,+adjustedHeight));
	}

	float theta = 0;
	if (adjustSize==false)
	{
		theta = float(vCenter.x+vCenter.y+vCenter.z+vsoutput_Color[0].x*2+vsoutput_Color[0].y*3+vsoutput_Color[0].z*5);
		theta = mod(theta, 360);
		theta = (theta*3.14)/180;
	}
	else
	{
		theta = 0;
	}

    // Triangle 1
	gl_Position = v0;
	doLogZBufferXForm();
    gsoutput_Color = vsoutput_Color[0];
    vTexCoords = rotate(t0, theta);
    EmitVertex();

	gl_Position = v1;
	doLogZBufferXForm();
	gsoutput_Color = vsoutput_Color[0];
    vTexCoords = rotate(t1, theta);
    EmitVertex();

	gl_Position = v2;
	doLogZBufferXForm();
	gsoutput_Color = vsoutput_Color[0];
    vTexCoords = rotate(t2, theta);
    EmitVertex();

    EndPrimitive();

    // Triangle 2
	gl_Position = v0;
	doLogZBufferXForm();
	gsoutput_Color = vsoutput_Color[0];
    vTexCoords = rotate(t0, theta);
    EmitVertex();

	gl_Position = v2;
	doLogZBufferXForm();
	gsoutput_Color = vsoutput_Color[0];
    vTexCoords = rotate(t2, theta);
    EmitVertex();

	gl_Position = v3;
	doLogZBufferXForm();
	gsoutput_Color = vsoutput_Color[0];
    vTexCoords = rotate(t3, theta);
    EmitVertex();

    EndPrimitive();
}