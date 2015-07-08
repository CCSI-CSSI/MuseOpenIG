//#******************************************************************************
//#*
//#*      Copyright (C) 2015  Compro Computer Services
//#*      http://openig.compro.net
//#*
//#*      Source available at: https://github.com/CCSI-CSSI/MuseOpenIG
//#*
//#*      This software is released under the LGPL.
//#*
//#*   This software is free software; you can redistribute it and/or modify
//#*   it under the terms of the GNU Lesser General Public License as published
//#*   by the Free Software Foundation; either version 2.1 of the License, or
//#*   (at your option) any later version.
//#*
//#*   This software is distributed in the hope that it will be useful,
//#*   but WITHOUT ANY WARRANTY; without even the implied warranty of
//#*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
//#*   the GNU Lesser General Public License for more details.
//#*
//#*   You should have received a copy of the GNU Lesser General Public License
//#*   along with this library; if not, write to the Free Software
//#*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//#*
//#*****************************************************************************
// Lighting GLSL inspired from the following article
// http://www.ozone3d.net/tutorials/glsl_lighting_phong_p3.php

#include <IgPluginCore/plugin.h>
#include <IgPluginCore/plugincontext.h>

#include <IgCore/imagegenerator.h>
#include <IgCore/attributes.h>
#include <IgCore/configuration.h>
#include <IgCore/commands.h>

#include <osg/ref_ptr>
#include <osg/LightSource>
#include <osg/Material>
#include <osg/Group>

#include <osgDB/XmlParser>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include <osgShadow/ShadowedScene>
#include <osgShadow/MinimalShadowMap>
#include <osg/ValueObject>

#include <map>
#include <sstream>

namespace igplugins
{

class SimpleLightingPlugin : public igplugincore::Plugin
{
public:
    SimpleLightingPlugin()
        : _cloudsShadowsTextureSlot(6)
		, _lightBrightness_enable(true)
		, _lightBrightness_day(1.f)
		, _lightBrightness_night(1.f)
		, _todHour(12)
	{

    }

    virtual std::string getName() { return "SimpleLighting"; }

    virtual std::string getDescription() { return "Implements simple lighting by using shaders and OpenGL, limited to 7 (+sun) lights"; }

    virtual std::string getVersion() { return "1.0.0"; }

    virtual std::string getAuthor() { return "ComPro, Nick"; }

    virtual void config(const std::string& fileName)
    {
        _cloudsShadowsTextureSlot = igcore::Configuration::instance()->getConfig("Clouds-Shadows-Texture-Slot",6);

        osgDB::XmlNode* root = osgDB::readXmlFile(fileName);
        if (root == 0) return;

        if (root->children.size() == 0) return;

        osgDB::XmlNode* config = root->children.at(0);
        if (config->name != "OpenIG-Plugin-Config") return;

        osgDB::XmlNode::Children::iterator itr = config->children.begin();
        for ( ; itr != config->children.end(); ++itr)
        {
            osgDB::XmlNode* child = *itr;

            if (child->name == "Material")
            {
                _sceneMaterial = new osg::Material;

                osgDB::XmlNode::Children::iterator citr = child->children.begin();
                for ( ; citr != child->children.end(); ++citr)
                {
                    if ((**citr).name == "Ambient")
                    {
                        igcore::StringUtils::Tokens tokens = igcore::StringUtils::instance()->tokenize((**citr).contents);
                        if (tokens.size() == 4)
                        {
                            float r = atof(tokens.at(0).c_str());
                            float g = atof(tokens.at(1).c_str());
                            float b = atof(tokens.at(2).c_str());
                            float a = atof(tokens.at(3).c_str());

                            if (_sceneMaterial.valid()) _sceneMaterial->setAmbient(osg::Material::FRONT_AND_BACK,osg::Vec4(r,g,b,a));
                        }
                    }
                    if ((**citr).name == "Diffuse")
                    {
                        igcore::StringUtils::Tokens tokens = igcore::StringUtils::instance()->tokenize((**citr).contents);
                        if (tokens.size() == 4)
                        {
                            float r = atof(tokens.at(0).c_str());
                            float g = atof(tokens.at(1).c_str());
                            float b = atof(tokens.at(2).c_str());
                            float a = atof(tokens.at(3).c_str());

                            if (_sceneMaterial.valid()) _sceneMaterial->setDiffuse(osg::Material::FRONT_AND_BACK,osg::Vec4(r,g,b,a));
                        }
                    }
                    if ((**citr).name == "Specular")
                    {
                        igcore::StringUtils::Tokens tokens = igcore::StringUtils::instance()->tokenize((**citr).contents);
                        if (tokens.size() == 4)
                        {
                            float r = atof(tokens.at(0).c_str());
                            float g = atof(tokens.at(1).c_str());
                            float b = atof(tokens.at(2).c_str());
                            float a = atof(tokens.at(3).c_str());

                            if (_sceneMaterial.valid()) _sceneMaterial->setSpecular(osg::Material::FRONT_AND_BACK,osg::Vec4(r,g,b,a));
                        }
                    }
                    if ((**citr).name == "Shininess")
                    {
                        if (_sceneMaterial.valid()) _sceneMaterial->setShininess(osg::Material::FRONT_AND_BACK,atof((**citr).contents.c_str()));
                    }
                }
            }
        }
    }

    class UpdateCameraPosUniformCallback : public osg::Uniform::Callback
    {
    public:
        UpdateCameraPosUniformCallback(igcore::ImageGenerator* ig)
            : _ig(ig)
        {

        }

        virtual void operator () (osg::Uniform* u, osg::NodeVisitor* )
        {
            osg::Vec3d eye;
            osg::Vec3d center;
            osg::Vec3d up;

            _ig->getViewer()->getView(0)->getCamera()->getViewMatrixAsLookAt(eye,center,up);

            u->set(eye);
        }

    protected:
        igcore::ImageGenerator*     _ig;
    };
	

	class UpdateTODBasedLightingUniformCallback : public osg::Uniform::Callback
	{
	public:
		UpdateTODBasedLightingUniformCallback(bool& enabled, float& onDay, float& onNight, unsigned int& tod)
			: _enabled(enabled)
			, _onDay(onDay)
			, _onNight(onNight)
			, _tod(tod)
		{

		}

		virtual void operator () (osg::Uniform* u, osg::NodeVisitor*)
		{
			if (_enabled)
			{
				float factor = _tod > 4 && _tod < 19 ? _onDay : _onNight;
				u->set(factor);
			}
			else
			{
				u->set(1.f);
			}
		}

	protected:
		bool&			_enabled;
		float&			_onDay;
		float&			_onNight;
		unsigned int&	_tod;
	};

	void updateFromXML(const std::string& fileName)
	{
		std::string xmlFile = fileName;
		if (xmlFile.empty())
		{
			xmlFile = _currentXMLFile;
		}

		if (!osgDB::fileExists(xmlFile))
		{
			osg::notify(osg::NOTICE) << "SimpleLighting: xml file does not exists: " << xmlFile << std::endl;
			return;
		}

		osgDB::XmlNode* root = osgDB::readXmlFile(xmlFile);
		if (!root)
		{
			osg::notify(osg::NOTICE) << "SimpleLighting: NULL root : " << xmlFile << std::endl;
			return;
		}
		if (!root->children.size())
		{
			osg::notify(osg::NOTICE) << "SimpleLighting: root with no children: " << xmlFile << std::endl;
			return;
		}
		if (root->children.at(0)->name != "OsgNodeSettings")
		{
			osg::notify(osg::NOTICE) << "SimpleLighting: OsgNodeSettings tag not found: " << xmlFile << std::endl;
			return;
		}

		osg::notify(osg::NOTICE) << "SimpleLighting: current file: " << xmlFile << std::endl;

		_currentXMLFile = xmlFile;

		osgDB::XmlNode::Children::iterator itr = root->children.at(0)->children.begin();
		for (; itr != root->children.at(0)->children.end(); ++itr)
		{
			osgDB::XmlNode* child = *itr;

			//<LandingLightBrightness  enable="true" day="0.05" night="5"/>
			if (child->name == "LandingLightBrightness")
			{
				osgDB::XmlNode::Properties::iterator pitr = child->properties.begin();
				for (; pitr != child->properties.end(); ++pitr)
				{
					if (pitr->first == "enable")
					{
						_lightBrightness_enable = pitr->second == "true";
					}
					if (pitr->first == "day")
					{
						_lightBrightness_day = atof(pitr->second.c_str());
					}
					if (pitr->first == "night")
					{
						_lightBrightness_night = atof(pitr->second.c_str());
					}
				}
			}
		}
	}

	class UpdateFromXMLCommand : public igcore::Commands::Command
	{
	public:
		UpdateFromXMLCommand(SimpleLightingPlugin* plugin)
			: _plugin(plugin)
		{

		}

		virtual int exec(const igcore::StringUtils::Tokens& tokens)
		{
			if (tokens.size() == 1)
			{
				std::string command = tokens.at(0);
				if (command == "update")
				{
					_plugin->updateFromXML("");
				}
				return 0;
			}
			return -1;
		}

		virtual const std::string getUsage() const
		{
			return "command";
		}
		virtual const std::string getDescription() const
		{
			return  "updates the lighting factor based on XML definition\n"				
				"        command - one of these: update";
		}

	protected:
		SimpleLightingPlugin*  _plugin;
	};

    virtual void init(igplugincore::PluginContext& context)
    {
		igcore::Commands::instance()->addCommand("lighting", new UpdateFromXMLCommand(this));

        _lightImplementationCallback = new SimpleLightImplementationCallback(context.getImageGenerator());
        context.getImageGenerator()->setLightImplementationCallback(_lightImplementationCallback);

        std::ostringstream ossvs;

        ossvs << "#define SELF_SHADOW_STAGE 1                                                                \n";
        ossvs << "#define CLOUDS_SHADOW_STAGE " << _cloudsShadowsTextureSlot << "                            \n";
        ossvs << "uniform mat4 cloudShadowCoordMatrix;                                                       \n";
        ossvs << "void DynamicShadow( in vec4 ecPosition )                                                   \n";
        ossvs << "{                                                                                          \n";
        ossvs << "    // generate coords for shadow mapping                                                  \n";
        ossvs << "    gl_TexCoord[SELF_SHADOW_STAGE].s = dot( ecPosition, gl_EyePlaneS[SELF_SHADOW_STAGE] ); \n";
        ossvs << "    gl_TexCoord[SELF_SHADOW_STAGE].t = dot( ecPosition, gl_EyePlaneT[SELF_SHADOW_STAGE] ); \n";
        ossvs << "    gl_TexCoord[SELF_SHADOW_STAGE].p = dot( ecPosition, gl_EyePlaneR[SELF_SHADOW_STAGE] ); \n";
        ossvs << "    gl_TexCoord[SELF_SHADOW_STAGE].q = dot( ecPosition, gl_EyePlaneQ[SELF_SHADOW_STAGE] ); \n";
        ossvs << "    gl_TexCoord[CLOUDS_SHADOW_STAGE] = cloudShadowCoordMatrix * ecPosition;                \n";
        ossvs << "}                                                                                          \n";

        osg::Shader* shadowVertexShader = new osg::Shader( osg::Shader::VERTEX, ossvs.str());

        osg::Shader* mainVertexShader = new osg::Shader( osg::Shader::VERTEX,
            "#version 120                                                           \n"
            "#pragma import_defines(SIMPLELIGHTING,SHADOWING,ENVIRONMENTAL,AO)      \n"
            "varying vec3 normal;                                                   \n"
            "varying vec3 eyeVec;                                                   \n"
            "varying vec3 lightDirs[8];                                             \n"
			"varying vec3 vertex_light_position;									\n"
            "uniform mat4 osg_ViewMatrixInverse;									\n"
            "uniform vec3 cameraPos;        										\n"            
            "mat3 getLinearPart( mat4 m )											\n"
            "{																		\n"
            "	mat3 result;														\n"
            "																		\n"
            "	result[0][0] = m[0][0];												\n"
            "	result[0][1] = m[0][1];												\n"
            "	result[0][2] = m[0][2];												\n"
            "																		\n"
            "	result[1][0] = m[1][0];												\n"
            "	result[1][1] = m[1][1];												\n"
            "	result[1][2] = m[1][2];												\n"
            "																		\n"
            "	result[2][0] = m[2][0];												\n"
            "	result[2][1] = m[2][1];												\n"
            "	result[2][2] = m[2][2];												\n"
            "																		\n"
            "	return result;														\n"
            "}																		\n"
            "																		\n"
            "void environmentalMapping()											\n"
            "{																		\n"            
            "	mat4 modelWorld4x4 = osg_ViewMatrixInverse * gl_ModelViewMatrix;	\n"
            "																		\n"
            "	mat3 modelWorld3x3 = getLinearPart( modelWorld4x4 );				\n"
            "																		\n"
            "	vec4 worldPos = modelWorld4x4 *  gl_Vertex;							\n"
            "																		\n"
            "	vec3 N = normalize( modelWorld3x3 * gl_Normal );					\n"
            "																		\n"
            "	vec3 E = normalize( worldPos.xyz - cameraPos.xyz );					\n"
            "																		\n"
            "	gl_TexCoord[4].xyz = reflect( E, N );                               \n"
            "}																		\n"
            "                                                                   \n"
            "void DynamicShadow( in vec4 ecPosition );                          \n"
            "                                                                   \n"
            "void main()                                                        \n"
            "{                                                                  \n"
            "   gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;         \n"
            "   normal = normalize( gl_NormalMatrix * gl_Normal );              \n"
            "   gl_TexCoord[0] = gl_TextureMatrix[0] *gl_MultiTexCoord0;        \n"
            "   vec3 vVertex = vec3(gl_ModelViewMatrix * gl_Vertex);            \n"          
            "   lightDirs[0] = gl_LightSource[0].position.xyz;                  \n"
            "   lightDirs[1] = vec3(gl_LightSource[1].position.xyz - vVertex);  \n"
            "   lightDirs[2] = vec3(gl_LightSource[2].position.xyz - vVertex);  \n"
            "   lightDirs[3] = vec3(gl_LightSource[3].position.xyz - vVertex);  \n"
            "   lightDirs[4] = vec3(gl_LightSource[4].position.xyz - vVertex);  \n"
            "   lightDirs[5] = vec3(gl_LightSource[5].position.xyz - vVertex);  \n"
            "   lightDirs[6] = vec3(gl_LightSource[6].position.xyz - vVertex);  \n"
            "   lightDirs[7] = vec3(gl_LightSource[7].position.xyz - vVertex);  \n"
            "   eyeVec = -vVertex;                                              \n"
            "   vec4  ecPos  = gl_ModelViewMatrix * gl_Vertex;                  \n"
            "   DynamicShadow( ecPos );                                         \n"
            "#if defined(ENVIRONMENTAL)                                         \n"
            "   environmentalMapping();                                         \n"
            "#endif                                                             \n"
			" vertex_light_position = normalize(gl_LightSource[0].position.xyz);\n"
            "}                                                                  \n"
        );


		osg::Shader* mainFragmentShader = new osg::Shader(osg::Shader::FRAGMENT,
			"#version 120                                                           \n"
			"#pragma import_defines(SIMPLELIGHTING,SHADOWING,ENVIRONMENTAL,AO,ENVIRONMENTAL_FACTOR,TEXTURING)\n"
			"uniform sampler2D ambientOcclusionTexture;                             \n"
			"uniform samplerCube environmentalMapTexture;                           \n"
			"																		\n"
			"uniform float ambientOcclusionFactor;                                  \n"
			"uniform float shadowsFactor;                                           \n"
			"																		\n"
			"uniform float	todBasedLightBrightness;								\n"
			"uniform bool	todBasedLightBrightnessEnabled;							\n"
			"                                                                       \n"
			"varying vec3 normal;                                                   \n"
			"varying vec3 eyeVec;                                                   \n"
			"varying vec3 lightDirs[8];                                             \n"
			"varying vec3 vertex_light_position;									\n"			
			"                                                                       \n"
			"uniform bool lightsEnabled[8];                                         \n"
			"                                                                       \n"
			"const float cos_outer_cone_angle = 0.4; // 36 degrees                  \n"
			"                                                                       \n"
			"float DynamicShadow();                                                  \n"
			"                                                                       \n"
			"uniform sampler2D baseTexture;                                         \n"
			"                                                                       \n"
			"void computeFogColor(inout vec4 color)                                 \n"
			"{                                                                      \n"
			"    if (gl_FragCoord.w > 0.0)                                          \n"
			"    {                                                                  \n"
			"        const float LOG2 = 1.442695;									\n"
			"        float z = gl_FragCoord.z / gl_FragCoord.w;                     \n"
			"        float fogFactor = exp2( -gl_Fog.density *						\n"
			"            gl_Fog.density *											\n"
			"            z *														\n"
			"            z *														\n"
			"            LOG2 );													\n"
			"        fogFactor = clamp(fogFactor, 0.0, 1.0);                        \n"
			"                                                                       \n"
			"        vec4 clr = color;                                              \n"
			"        color = mix(gl_Fog.color, color, fogFactor );                  \n"
			"        color.a = clr.a;                                               \n"
			"    }                                                                  \n"
			"}                                                                      \n"
			"void computeAmbientColor(inout vec4 color)                             \n"
			"{                                                                      \n"
			"	vec4 final_color =                                                  \n"
			"   (gl_FrontLightModelProduct.sceneColor * gl_FrontMaterial.ambient) + \n"
			"	(gl_LightSource[0].ambient * gl_FrontMaterial.ambient);             \n"
			"                                                                       \n"
			"	vec3 N = normalize(normal);                                         \n"
			"	vec3 L = normalize(gl_LightSource[0].position.xyz);                 \n"
			"	vec3 D = normalize(gl_LightSource[0].spotDirection);				\n"
			"	float cos_cur_angle = dot(-L, D);									\n"
			"	float cos_inner_cone_angle = gl_LightSource[0].spotCosCutoff;		\n"
			"	float cos_inner_minus_outer_angle =									\n"
			"			cos_inner_cone_angle - cos_outer_cone_angle;				\n"
			"	float spot = 0.0;													\n"
			"	spot = clamp((cos_cur_angle - cos_outer_cone_angle) /				\n"
			"					cos_inner_minus_outer_angle, 0.0, 1.0);				\n"
			"                                                                       \n"
			"	float lambertTerm = max(dot(N,L),0.0);                              \n"
			"                                                                       \n"
			"	float specular = 0.0;												\n"
			"	vec4 specular_color = vec4(0.0,0.0,0.0,0.0);						\n"			
			"	{                                                                   \n"
			"		final_color += gl_LightSource[0].diffuse *                      \n"
			"		               gl_FrontMaterial.diffuse *                       \n"
			"					   lambertTerm;										\n"
			"                                                                       \n"
			"		vec3 E = normalize(eyeVec);                                     \n"
			"		vec3 R = reflect(-L, N);                                        \n"
			"		specular = pow( max(dot(R, E), 0.0),							\n"
			"		                 gl_FrontMaterial.shininess );                  \n"
			"		specular_color =  gl_LightSource[0].specular *                  \n"
			"                               gl_FrontMaterial.specular *				\n"
			"					   specular;										\n"
			"		final_color += specular_color;									\n"
			"	}                                                                   \n"
			"                                                                       \n"			
			"	vec4 ambient_color = gl_FrontMaterial.ambient * gl_LightSource[0].ambient + gl_LightModel.ambient * gl_FrontMaterial.ambient; \n"
			"	vec4 diffuse_color = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse; \n"
			"	float diffuse_value = max(dot(normal, vertex_light_position), 0.0);	\n"
			"#if defined(TEXTURING)													\n"
			"	color += final_color;												\n"
			"#else																	\n"
			"	color += ambient_color * 0.5 + diffuse_color * diffuse_value + specular_color; \n"
			"#endif																	\n"
            "}                                                                      \n"
            "                                                                       \n"
            "void computeColorForLightSource(int i, inout vec4 color)				\n"
            "{                                                                      \n"
            "   if (!lightsEnabled[i]) return;                                      \n"
            "                                                                       \n"
			"	vec4 final_color = vec4(0.0,0.0,0.0,1.0);                           \n"
			"#if !defined(TEXTURING)												\n"
            "	final_color =														\n"	
            "		(gl_FrontLightModelProduct.sceneColor * gl_FrontMaterial.ambient) +	\n"
            "		(gl_LightSource[i].ambient * gl_FrontMaterial.ambient);             \n"
			"#endif																	\n"
            "                                                                       \n"
            "		float distSqr = dot(lightDirs[i],lightDirs[i]);                 \n"
            "		float invRadius = gl_LightSource[i].constantAttenuation;		\n"
            "		float att = clamp(1.0-invRadius * sqrt(distSqr), 0.0, 1.0);		\n"
            "		vec3 L = lightDirs[i] * inversesqrt(distSqr);                   \n"
            "		vec3 D = normalize(gl_LightSource[i].spotDirection);			\n"
            "                                                                       \n"
            "		float cos_cur_angle = dot(-L, D);                               \n"
            "                                                                       \n"
            "		float cos_inner_cone_angle = gl_LightSource[i].spotCosCutoff;	\n"
            "                                                                       \n"
            "		float cos_inner_minus_outer_angle =                             \n"
            "			cos_inner_cone_angle - cos_outer_cone_angle;                \n"
            "                                                                       \n"
            "		float spot = 0.0;                                               \n"
            "		spot = clamp((cos_cur_angle - cos_outer_cone_angle) /			\n"
            "			cos_inner_minus_outer_angle, 0.0, 1.0);                     \n"
            "                                                                       \n"
            "		vec3 N = normalize(normal);                                     \n"
            "                                                                       \n"
            "		float lambertTerm = max( dot(N,L), 0.0);                        \n"
            "		if(lambertTerm > 0.0)                                           \n"
            "		{                                                               \n"
			"			float todBasedFactor = todBasedLightBrightnessEnabled ?		\n"
			"				todBasedLightBrightness : 1.0;							\n"
            "			final_color += gl_LightSource[i].diffuse * todBasedFactor * \n"
            "				gl_FrontMaterial.diffuse *                              \n"
            "				lambertTerm * spot * att;                               \n"
            "                                                                       \n"
            "			vec3 E = normalize(eyeVec);                                 \n"
            "			vec3 R = reflect(-L, N);                                    \n"
            "                                                                       \n"
            "			float specular = pow( max(dot(R, E), 0.0),                  \n"
            "				gl_FrontMaterial.shininess );                           \n"
            "                                                                       \n"
            "			final_color += gl_LightSource[i].specular *                 \n"
            "				gl_FrontMaterial.specular *                             \n"
            "				specular * spot * att;                                  \n"
            "		}                                                               \n"
            "                                                                       \n"
            "                                                                       \n"
            "		color += final_color;                                           \n"
            "}                                                                      \n"
            "                                                                       \n"
            "void lighting( inout vec4 color )                                      \n"
            "{                                                                      \n"
            "	vec4 clr = vec4(0.0);                                               \n"
            "                                                                       \n"
            "	for (int i = 1; i < 8; i++)                                         \n"
            "	{                                                                   \n"
            "		computeColorForLightSource(i,clr);                              \n"
            "	}                                                                   \n"            
            "                                                                       \n"
			"#if defined(TEXTURING)													\n"			
			"	computeAmbientColor(clr);                                           \n"
            "	color *= clr;                                                       \n"
			"#else																	\n"
			"	computeAmbientColor(clr);                                           \n"
			"	color = clr;														\n"
			"#endif                                                                 \n"			
            "	computeFogColor(color);                                             \n"
            "}                                                                      \n"
            "void computeAmbientOcclusion( inout vec4 color )                       \n"
            "{                                                                      \n"            
            "   vec4 aocolor = texture2D(ambientOcclusionTexture,gl_TexCoord[0].xy);\n"
            "   aocolor.rgb *= ambientOcclusionFactor;                              \n"
            "   color.rgb *= aocolor.rgb;                                           \n"
            "}                                                                      \n"
            "void computeEnvironmentalMap( inout vec4 color )                       \n"
            "{																		\n"            
            "   vec3 v = gl_TexCoord[4].xzy;                                        \n"
            "   v.y *= -1.0;                                                        \n"
            "	vec3 cube_color =													\n"
            "		textureCube(environmentalMapTexture, v).rgb;                    \n"
            "																		\n"
            "   vec3 mixed_color =                                                  \n"
            "       mix(cube_color, color.rgb, 1.0-ENVIRONMENTAL_FACTOR).rgb;        \n"
            "	color.rgb *= mixed_color;//vec4(mixed_color , color.a);             \n"
            "}																		\n"
            "void main()                                                            \n"
            "{                                                                      \n"
			"   vec4 color = vec4(0.0,0.0,0.0,1.0);									\n"
			"#if defined(TEXTURING)													\n"
			"   color = texture2D( baseTexture, gl_TexCoord[0].xy );				\n"
			"#endif																	\n"														
			"#if !defined(TEXTURING)												\n"
			"#if defined(SIMPLELIGHTING)                                            \n"
			"   lighting(color);                                                    \n"
			"#endif                                                                 \n"
            "   float shadow = DynamicShadow();                                     \n"
            "#if defined(SHADOWING)                                                 \n"
            "   color.rgb = mix( color.rgb * (1.0-shadowsFactor), color.rgb, shadow );\n"
            "#endif                                                                 \n"            
			"#else																	\n"
			"   float shadow = DynamicShadow();                                     \n"
			"#if defined(SHADOWING)                                                 \n"
			"   color.rgb = mix( color.rgb * (1.0-shadowsFactor), color.rgb, shadow );\n"
			"#endif                                                                 \n"
			"#if defined(SIMPLELIGHTING)                                            \n"
			"   lighting(color);                                                    \n"
			"#endif                                                                 \n"
			"#endif																	\n"
            "#if defined(ENVIRONMENTAL)                                             \n"
            "   computeEnvironmentalMap(color);                                     \n"
            "#endif                                                                 \n"
            "#if defined(AO)                                                        \n"
            "   computeAmbientOcclusion(color);                                     \n"
            "#endif                                                                 \n"
            "   gl_FragColor =  color;												\n"
            "}                                                                      \n"
        );

        std::ostringstream ossfs;

        ossfs << "#define SELF_SHADOW_STAGE  1                                                                   \n";
        ossfs << "#define CLOUDS_SHADOW_STAGE " << _cloudsShadowsTextureSlot << "                                \n";
        ossfs << "uniform sampler2DShadow    shadowTexture;                                                      \n";
        ossfs << "uniform sampler2D          cloudShadowTexture;                                                 \n";
        ossfs << "float DynamicShadow()                                                                          \n";
        ossfs << "{                                                                                              \n";
        ossfs << "   float selfShadow = shadow2DProj( shadowTexture, gl_TexCoord[SELF_SHADOW_STAGE] ).r;         \n";
        ossfs << "   float cloudsShadow = texture2D( cloudShadowTexture, gl_TexCoord[CLOUDS_SHADOW_STAGE].xy ).r;\n";
        ossfs << "   return selfShadow * cloudsShadow;                                                           \n";
        ossfs << "}                                                                                              \n";

        osg::Shader* shadowFragmentShader = new osg::Shader( osg::Shader::FRAGMENT,ossfs.str());

        osgShadow::ShadowedScene* scene = dynamic_cast<osgShadow::ShadowedScene*>(
            context.getImageGenerator()->getScene()
        );
        if (scene != 0)
        {
            osgShadow::MinimalShadowMap* msm = dynamic_cast<osgShadow::MinimalShadowMap*>(
                scene->getShadowTechnique()
            );
            if (msm != 0)
            {
                msm->setMainVertexShader(mainVertexShader);
                msm->setMainFragmentShader(mainFragmentShader);
                msm->setShadowVertexShader(shadowVertexShader);
                msm->setShadowFragmentShader(shadowFragmentShader);

                osg::StateSet* ss = context.getImageGenerator()->getViewer()->getView(0)->getSceneData()->getOrCreateStateSet();

#if 1
                if (!_sceneMaterial.valid())
                {
                    osg::Material* sceneMaterial = new osg::Material;
                    sceneMaterial->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(0.7,0.7,0.7,1.0));
                    sceneMaterial->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(0.6,0.6,0.6,1.0));
                    sceneMaterial->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0.6,0.6,0.6,1.0));
                    sceneMaterial->setShininess(osg::Material::FRONT_AND_BACK, 60);

                    ss->setAttributeAndModes(sceneMaterial, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
                }
                else
                {
                    ss->setAttributeAndModes(_sceneMaterial, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
                }
#endif
                osg::Uniform* u = new osg::Uniform(osg::Uniform::BOOL,"lightsEnabled", 8);
                osg::Uniform::Callback* cb = new UpdateLightsEnabledUniformCallback(context.getImageGenerator());
                u->setUpdateCallback(cb);
                ss->addUniform(u);

                float shadowsFactor = igcore::Configuration::instance()->getConfig("Shadows-Factor",0.5);
                ss->addUniform(new osg::Uniform("shadowsFactor",shadowsFactor));

                ss->setDefine("SIMPLELIGHTING");
                ss->setDefine("SHADOWING");
                ss->setDefine("ENVIRONMENTAL_FACTOR","0");
				ss->setDefine("TEXTURING");

                unsigned int defaultDiffuseSlot = igcore::Configuration::instance()->getConfig("Default-diffuse-texture-slot",0);
                ss->addUniform(new osg::Uniform("baseTexture",(int)defaultDiffuseSlot),osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE);

                osg::Uniform* cu = new osg::Uniform("cameraPos",osg::Vec3d());
                cu->setUpdateCallback(new UpdateCameraPosUniformCallback(context.getImageGenerator()));
				ss->addUniform(cu);	

				osg::Uniform* todBasedLightingUniform = new osg::Uniform("todBasedLightBrightness", (float)1.f);
				todBasedLightingUniform->setUpdateCallback(new UpdateTODBasedLightingUniformCallback(
					_lightBrightness_enable, _lightBrightness_day, _lightBrightness_night, _todHour)
				);
				ss->addUniform(todBasedLightingUniform);

				osg::Uniform* todBasedLightingEnabledUniform = new osg::Uniform("todBasedLightBrightnessEnabled", (bool)true);
				ss->addUniform(todBasedLightingEnabledUniform);

            }
        }

    }

    virtual void clean(igplugincore::PluginContext& context)
    {
        context.getImageGenerator()->setLightImplementationCallback(0);
    }

	virtual void update(igplugincore::PluginContext& context)
	{
		osg::ref_ptr<osg::Referenced> ref = context.getAttribute("TOD");
		igplugincore::PluginContext::Attribute<igcore::TimeOfDayAttributes> *attr = dynamic_cast<igplugincore::PluginContext::Attribute<igcore::TimeOfDayAttributes> *>(ref.get());
		if (attr)
		{
			_todHour = attr->getValue().getHour();
		}				
	}
	
	virtual void databaseRead(const std::string& fileName, osg::Node*, const osgDB::Options*)
	{
		std::string xmlFile = fileName + ".lighting.xml";
		updateFromXML(xmlFile);
	}

protected:

    class UpdateLightsEnabledUniformCallback : public osg::Uniform::Callback
    {
    public:
        UpdateLightsEnabledUniformCallback(igcore::ImageGenerator* ig)
            : _ig(ig)
        {

        }

        virtual void operator () (osg::Uniform* u, osg::NodeVisitor* )
        {
            for (size_t i = 0; i<8; ++i)
            {
                u->setElement(i,_ig->isLightEnabled(i));
            }
        }

    protected:
        igcore::ImageGenerator*     _ig;
    };

	class DummyLight : public osg::Light
	{
	public:
		DummyLight(unsigned int id, bool enabled)
			: osg::Light(id)
			, _enabled(enabled)
			, _id(id)
		{
			setUserValue("id", (unsigned int)id);
			setUserValue("enabled", (bool)enabled);
		}

		inline bool getEnabled() const
		{
			return _enabled;
		}

		virtual void apply(osg::State& state) const
		{
			if (_id >= 8) return;
			osg::Light::apply(state);
		}


	protected:
		bool            _enabled;
		unsigned int    _id;
	};

    class SimpleLightImplementationCallback : public igcore::LightImplementationCallback
    {
    public:
        SimpleLightImplementationCallback(igcore::ImageGenerator* ig)
            : _ig(ig)
        {

        }

        virtual osg::Referenced* createLight(
                unsigned int id,
                const igcore::LightAttributes& definition,
                osg::Group*)
        {
            osg::LightSource* light = new osg::LightSource;

			DummyLight* dl = new DummyLight(id, true);

            dl->setLightNum(id);
            dl->setAmbient(definition._ambient);
            dl->setDiffuse(definition._diffuse*definition._brightness);
            dl->setSpecular(definition._specular);
            dl->setConstantAttenuation(1.f/definition._constantAttenuation);
            dl->setSpotCutoff(definition._spotCutoff);
            dl->setPosition(osg::Vec4(0,0,0,1));
            dl->setDirection(osg::Vec3(0,1,-0));

			light->setLight(dl);

			if (id >= 1 && id < 8)
			{
				light->setStateSetModes(*_ig->getViewer()->getView(0)->getSceneData()->getOrCreateStateSet(), osg::StateAttribute::ON);
			}

            _lights[id] = light;

            return light;
        }

        virtual void updateLight(unsigned int id, const igcore::LightAttributes& definition)
        {
            LightsMapIterator itr = _lights.find(id);
            if ( itr != _lights.end())
            {
                osg::LightSource* light = itr->second;

				if ((definition._dirtyMask & igcore::LightAttributes::AMBIENT) == igcore::LightAttributes::AMBIENT)
                    light->getLight()->setAmbient(definition._ambient);

				if ((definition._dirtyMask & igcore::LightAttributes::DIFFUSE) == igcore::LightAttributes::DIFFUSE &&
					(definition._dirtyMask & igcore::LightAttributes::BRIGHTNESS) == igcore::LightAttributes::BRIGHTNESS)
                    light->getLight()->setDiffuse(definition._diffuse*definition._brightness);

				if ((definition._dirtyMask & igcore::LightAttributes::SPECULAR) == igcore::LightAttributes::SPECULAR)
                    light->getLight()->setSpecular(definition._specular);

				if ((definition._dirtyMask & igcore::LightAttributes::CONSTANTATTENUATION) == igcore::LightAttributes::CONSTANTATTENUATION)
                    light->getLight()->setConstantAttenuation(1.f/definition._constantAttenuation);

				if ((definition._dirtyMask & igcore::LightAttributes::SPOTCUTOFF) == igcore::LightAttributes::SPOTCUTOFF)
                    light->getLight()->setSpotCutoff(definition._spotCutoff);				

            }
        }

    protected:
        igcore::ImageGenerator*     _ig;

        typedef std::map<unsigned int, osg::ref_ptr<osg::LightSource> >                 LightsMap;
        typedef std::map<unsigned int, osg::ref_ptr<osg::LightSource> >::iterator       LightsMapIterator;

        LightsMap                   _lights;
    };

    osg::ref_ptr<igcore::LightImplementationCallback>       _lightImplementationCallback;
    osg::ref_ptr<osg::Material>                             _sceneMaterial;
    int                                                     _cloudsShadowsTextureSlot;
	bool													_lightBrightness_enable;
	float													_lightBrightness_day;
	float													_lightBrightness_night;
	unsigned int											_todHour;
	std::string												_currentXMLFile;
};

} // namespace

#if defined(_MSC_VER) || defined(__MINGW32__)
    //  Microsoft
    #define EXPORT __declspec(dllexport)
    #define IMPORT __declspec(dllimport)
#elif defined(__GNUG__)
    //  GCC
    #define EXPORT __attribute__((visibility("default")))
    #define IMPORT
#else
    //  do nothing and hope for the best?
    #define EXPORT
    #define IMPORT
    #pragma warning Unknown dynamic link import/export semantics.
#endif

extern "C" EXPORT igplugincore::Plugin* CreatePlugin()
{
    return new igplugins::SimpleLightingPlugin;
}

extern "C" EXPORT void DeletePlugin(igplugincore::Plugin* plugin)
{
    osg::ref_ptr<igplugincore::Plugin> p(plugin);
}
