#pragma import_defines(SHADOWING,ENVIRONMENTAL,AO,ENVIRONMENTAL_FACTOR)

uniform mat4 osg_ViewMatrix;
uniform mat4 osg_ViewMatrixInverse;

#if defined(USE_LOG_DEPTH_BUFFER)
uniform float Fcoef;
out float flogz;
#endif

out vec3 eyeVec;

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
	mat3 modelWorld3x3 = getLinearPart( modelWorld4x4 );

	vec3 vNormalWorldSpace = normalize( modelWorld3x3 * gl_Normal );					

	vec4 vPosEyeSpace = gl_ModelViewMatrix * gl_Vertex;
	vec3 vIncidentEyeSpace = normalize(vPosEyeSpace.xyz - vec3(0,0,0));
	vec3 vIncidentWorldSpace = (transpose(getLinearPart(osg_ViewMatrix))*vIncidentEyeSpace);
	vIncidentWorldSpace = normalize(vIncidentWorldSpace);
	gl_TexCoord[4].xyz = reflect( vIncidentWorldSpace, vNormalWorldSpace );
}

void DynamicShadow( in vec4 ecPosition );

out vec4 vPositionProj;

out vec3 vPositionEyeSpace;
out vec3 vNormalEyeSpace;

void main()                                                  
{                                                         
   gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

#if defined(USE_LOG_DEPTH_BUFFER)
   if (Fcoef > 0)
   {
		gl_Position.z = (log2(max(1e-6, 1.0 + gl_Position.w)) * Fcoef - 1.0) * gl_Position.w;
		flogz = 1.0 + gl_Position.w;
   }
#endif

   vPositionProj = gl_ModelViewProjectionMatrix * vec4(gl_Vertex.x,gl_Vertex.y,gl_Vertex.z,1);   

   gl_TexCoord[0] = gl_TextureMatrix[0] *gl_MultiTexCoord0;  
   vec3 vVertex = vec3(gl_ModelViewMatrix * gl_Vertex);

   eyeVec = -vVertex;

   vPositionEyeSpace = vec3(gl_ModelViewMatrix * gl_Vertex);
   vNormalEyeSpace   = normalize(gl_NormalMatrix * gl_Normal);

   vec4  ecPos  = gl_ModelViewMatrix * gl_Vertex;            
   DynamicShadow( ecPos );                                   
#if defined(ENVIRONMENTAL)                                   
   environmentalMapping();                                   
#endif                                                       
}                                                            