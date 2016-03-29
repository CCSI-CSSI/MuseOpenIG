#version 120
#pragma import_defines(ENVIRONMENTAL_FACTOR,SHADOWING,ENVIRONMENTAL,AO,USE_LOG_DEPTH_BUFFER)
varying vec3 normal;
varying vec3 eyeVec;
varying vec3 lightDirs[8];
uniform mat4 osg_ViewMatrixInverse;
uniform mat4 osg_ViewMatrix;
uniform vec3 cameraPos;

#ifdef USE_LOG_DEPTH_BUFFER
uniform float Fcoef;
varying float flogz;
#endif

mat3 getLinearPart( mat4 m )
{
	mat3 result;
	result[0][0] = m[0][0];
	result[0][1] = m[0][1];
	result[0][2] = m[0][2];
	result[1][0] = m[1][0];
	result[1][1] = m[1][1];
	result[1][2] = m[1][2];
	result[2][0] = m[2][0];
	result[2][1] = m[2][1];
	result[2][2] = m[2][2];
	return result;
}

void environmentalMapping()
{
	mat4 modelWorld4x4 = osg_ViewMatrixInverse * gl_ModelViewMatrix;
	mat3 modelWorld3x3 = getLinearPart(modelWorld4x4);
	vec3 vNormalWorldSpace = normalize(modelWorld3x3 * gl_Normal);
	vec4 vPosEyeSpace = gl_ModelViewMatrix * gl_Vertex;
	vec3 vIncidentEyeSpace = normalize(vPosEyeSpace.xyz - vec3(0, 0, 0));
	vec3 vIncidentWorldSpace = (transpose(getLinearPart(osg_ViewMatrix))*vIncidentEyeSpace);
	vIncidentWorldSpace = normalize(vIncidentWorldSpace);
	gl_TexCoord[4].xyz = reflect(vIncidentWorldSpace, vNormalWorldSpace);
}

void DynamicShadow( in vec4 ecPosition );

void main()
{
   gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
#ifdef USE_LOG_DEPTH_BUFFER
	if (Fcoef > 0)
	{
		gl_Position.z = (log2(max(1e-6, 1.0 + gl_Position.w)) * Fcoef - 1.0) * gl_Position.w;
	}
#endif
   normal = normalize( gl_NormalMatrix * gl_Normal );
   gl_TexCoord[0] = gl_TextureMatrix[0] *gl_MultiTexCoord0;
   vec3 vVertex = vec3(gl_ModelViewMatrix * gl_Vertex);
   lightDirs[0] = gl_LightSource[0].position.xyz;
   lightDirs[1] = vec3(gl_LightSource[1].position.xyz - vVertex);
   lightDirs[2] = vec3(gl_LightSource[2].position.xyz - vVertex);
   lightDirs[3] = vec3(gl_LightSource[3].position.xyz - vVertex);
   lightDirs[4] = vec3(gl_LightSource[4].position.xyz - vVertex);
   lightDirs[5] = vec3(gl_LightSource[5].position.xyz - vVertex);
   lightDirs[6] = vec3(gl_LightSource[6].position.xyz - vVertex);
   lightDirs[7] = vec3(gl_LightSource[7].position.xyz - vVertex);
   eyeVec = -vVertex;
   vec4  ecPos  = gl_ModelViewMatrix * gl_Vertex;
   DynamicShadow( ecPos );
#ifdef ENVIRONMENTAL
   environmentalMapping();
#endif
}