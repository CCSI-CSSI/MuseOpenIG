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
//#*    Please direct any questions or comments to the OpenIG Forums
//#*    Email address: openig@compro.net
//#*
//#*
//#*****************************************************************************
// Lighting GLSL inspired from the following article
// http://www.ozone3d.net/tutorials/glsl_lighting_phong_p3.php

#include <Core-PluginBase/plugin.h>
#include <Core-PluginBase/plugincontext.h>

#include <Core-Base/imagegenerator.h>
#include <Core-Base/attributes.h>
#include <Core-Base/configuration.h>

#include <osg/ref_ptr>
#include <osg/LightSource>
#include <osg/Material>
#include <osg/Group>
#include <osg/ValueObject>

#include <osgDB/XmlParser>

#include <osgShadow/ShadowedScene>
#include <osgShadow/MinimalShadowMap>
#include <osgShadow/ViewDependentShadowMap>

#include <osgEarth/VirtualProgram>
#include <osgEarth/ShaderUtils>

#include <map>
#include <sstream>

namespace OpenIG {
	namespace Plugins {

		class OSGEarthSimpleLightingPlugin : public OpenIG::PluginBase::Plugin
		{
		public:
			OSGEarthSimpleLightingPlugin()
				:_cloudsShadowsTextureSlot(6)
			{

			}

			virtual std::string getName() { return "OSGEarthSimpleLighting"; }

			virtual std::string getDescription() { return "Implements simple lighting with osgEarth by using shaders and OpenGL, limited to 7 (+sun) lights"; }

			virtual std::string getVersion() { return "1.0.0"; }

			virtual std::string getAuthor() { return "ComPro, Nick"; }

			virtual void config(const std::string& fileName)
			{
				_cloudsShadowsTextureSlot = OpenIG::Base::Configuration::instance()->getConfig("Clouds-Shadows-Texture-Slot", 6);

				osgDB::XmlNode* root = osgDB::readXmlFile(fileName);
				if (root == 0) return;

				if (root->children.size() == 0) return;

				osgDB::XmlNode* config = root->children.at(0);
				if (config->name != "OpenIG-Plugin-Config") return;

				osgDB::XmlNode::Children::iterator itr = config->children.begin();
				for (; itr != config->children.end(); ++itr)
				{
					osgDB::XmlNode* child = *itr;

					if (child->name == "Material")
					{
						_sceneMaterial = new osg::Material;

						osgDB::XmlNode::Children::iterator citr = child->children.begin();
						for (; citr != child->children.end(); ++citr)
						{
							if ((**citr).name == "Ambient")
							{
								OpenIG::Base::StringUtils::Tokens tokens = OpenIG::Base::StringUtils::instance()->tokenize((**citr).contents);
								if (tokens.size() == 4)
								{
									float r = atof(tokens.at(0).c_str());
									float g = atof(tokens.at(1).c_str());
									float b = atof(tokens.at(2).c_str());
									float a = atof(tokens.at(3).c_str());

									if (_sceneMaterial.valid()) _sceneMaterial->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(r, g, b, a));
								}
							}
							if ((**citr).name == "Diffuse")
							{
								OpenIG::Base::StringUtils::Tokens tokens = OpenIG::Base::StringUtils::instance()->tokenize((**citr).contents);
								if (tokens.size() == 4)
								{
									float r = atof(tokens.at(0).c_str());
									float g = atof(tokens.at(1).c_str());
									float b = atof(tokens.at(2).c_str());
									float a = atof(tokens.at(3).c_str());

									if (_sceneMaterial.valid()) _sceneMaterial->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(r, g, b, a));
								}
							}
							if ((**citr).name == "Specular")
							{
								OpenIG::Base::StringUtils::Tokens tokens = OpenIG::Base::StringUtils::instance()->tokenize((**citr).contents);
								if (tokens.size() == 4)
								{
									float r = atof(tokens.at(0).c_str());
									float g = atof(tokens.at(1).c_str());
									float b = atof(tokens.at(2).c_str());
									float a = atof(tokens.at(3).c_str());

									if (_sceneMaterial.valid()) _sceneMaterial->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(r, g, b, a));
								}
							}
							if ((**citr).name == "Shininess")
							{
								if (_sceneMaterial.valid()) _sceneMaterial->setShininess(osg::Material::FRONT_AND_BACK, atof((**citr).contents.c_str()));
							}
						}
					}
				}
			}

			class UpdateCameraPosUniformCallback : public osg::Uniform::Callback
			{
			public:
				UpdateCameraPosUniformCallback(OpenIG::Base::ImageGenerator* ig)
					: _ig(ig)
				{

				}

				virtual void operator () (osg::Uniform* u, osg::NodeVisitor*)
				{
					osg::Vec3d eye;
					osg::Vec3d center;
					osg::Vec3d up;

					_ig->getViewer()->getView(0)->getCamera()->getViewMatrixAsLookAt(eye, center, up);

					u->set(eye);
				}

			protected:
				OpenIG::Base::ImageGenerator*     _ig;
			};

			virtual void init(OpenIG::PluginBase::PluginContext& context)
			{
				_lightImplementationCallback = new SimpleLightImplementationCallback(context.getImageGenerator());
				context.getImageGenerator()->setLightImplementationCallback(_lightImplementationCallback);


				const char* mainVertexShaderStr =
					"#version 120                                                       \n"
					"varying vec3 normal;                                               \n"
					"varying vec3 eyeVec;                                               \n"
					"varying vec3 lightDirs[8];                                         \n"
					"#define SELF_SHADOW_STAGE 1                                                                \n"
					"#define CLOUDS_SHADOW_STAGE 6																\n"
					"uniform mat4 cloudShadowCoordMatrix;                                                       \n"
					"void DynamicShadow( in vec4 ecPosition )                                                   \n"
					"{                                                                                          \n"
					"    // generate coords for shadow mapping                                                  \n"
					"    gl_TexCoord[SELF_SHADOW_STAGE].s = dot( ecPosition, gl_EyePlaneS[SELF_SHADOW_STAGE] ); \n"
					"    gl_TexCoord[SELF_SHADOW_STAGE].t = dot( ecPosition, gl_EyePlaneT[SELF_SHADOW_STAGE] ); \n"
					"    gl_TexCoord[SELF_SHADOW_STAGE].p = dot( ecPosition, gl_EyePlaneR[SELF_SHADOW_STAGE] ); \n"
					"    gl_TexCoord[SELF_SHADOW_STAGE].q = dot( ecPosition, gl_EyePlaneQ[SELF_SHADOW_STAGE] ); \n"
					"    gl_TexCoord[CLOUDS_SHADOW_STAGE] = cloudShadowCoordMatrix * ecPosition;                \n"
					"}                                                                                          \n"
					"                                                                   \n"
					"void vert_lighting(inout vec4 VertexMODEL)                         \n"
					"{                                                                  \n"
					"   gl_Position = gl_ModelViewProjectionMatrix * VertexMODEL;       \n"
					"   normal = normalize( gl_NormalMatrix * gl_Normal );              \n"
					"   gl_TexCoord[0] = gl_TextureMatrix[0] *gl_MultiTexCoord0;        \n"
					"   vec3 vVertex = vec3(gl_ModelViewMatrix * VertexMODEL);          \n"
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
					"}                                                                  \n";

				const char* mainFragmentShaderStr =
					"#version 120                                                           \n"
					"                                                                       \n"
					"varying vec3 normal;                                                   \n"
					"varying vec3 eyeVec;                                                   \n"
					"varying vec3 lightDirs[8];                                             \n"
					"uniform bool lightsEnabled[8];                                         \n"
					"                                                                       \n"
					"const float cos_outer_cone_angle = 0.4; // 36 degrees                  \n"
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
					"																		\n"
					"		float distSqr = dot(lightDirs[0],lightDirs[0]);                 \n"
					"		float invRadius = gl_LightSource[0].constantAttenuation;		\n"
					"		float att = clamp(1.0-invRadius * sqrt(distSqr), 0.0, 1.0);		\n"
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
					"					   lambertTerm * att;								\n"
					"                                                                       \n"
					"		vec3 E = normalize(eyeVec);                                     \n"
					"		vec3 R = reflect(-L, N);                                        \n"
					"		specular = pow( max(dot(R, E), 0.0),							\n"
					"		                 gl_FrontMaterial.shininess );                  \n"
					"		specular_color =  gl_LightSource[0].specular *                  \n"
					"                               gl_FrontMaterial.specular *				\n"
					"					   specular;										\n"
					"		//final_color += specular_color;									\n"
					"	}                                                                   \n"
					"   color += final_color;												\n"
					"}                                                                      \n"
					"                                                                       \n"
					"void computeColorForLightSource(int i, inout vec4 color)				\n"
					"{                                                                      \n"
					"   if (!lightsEnabled[i]) return;                                      \n"
					"                                                                       \n"
					"	vec4 final_color = vec4(0.0,0.0,0.0,1.0);                           \n"
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
					"			final_color += gl_LightSource[i].diffuse *                  \n"
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
					"	computeAmbientColor(clr);                                           \n"
					"	color *= clr;                                                       \n"
					"	computeFogColor(color);                                             \n"
					"}                                                                      \n"
					"#define SELF_SHADOW_STAGE  1                                                                   \n"
					"#define CLOUDS_SHADOW_STAGE 6																	\n"
					"uniform sampler2DShadow    shadowTexture;                                                      \n"
					"uniform sampler2D          cloudShadowTexture;                                                 \n"
					"float DynamicShadow()                                                                          \n"
					"{                                                                                              \n"
					"   float selfShadow = shadow2DProj( shadowTexture, gl_TexCoord[SELF_SHADOW_STAGE] ).r;         \n"
					"   float cloudsShadow = texture2D( cloudShadowTexture, gl_TexCoord[CLOUDS_SHADOW_STAGE].xy ).r;\n"
					"   return selfShadow * cloudsShadow;                                                           \n"
					"}                                                                                              \n"
					"void frag_lighting(inout vec4 inout_color)                             \n"
					"{                                                                      \n"
					"   vec4 color = vec4(0.0,0.0,0.0,1.0);									\n"
					"   float shadow = DynamicShadow();                                     \n"
					"   color.rgb = mix( color.rgb * (1.0-0.5), color.rgb, shadow );		\n"
					"   color = texture2D( baseTexture, gl_TexCoord[0].xy );				\n"
					"   lighting(color);                                                    \n"
					"   inout_color =  color;												\n"
					"}                                                                      \n";

				osgShadow::ShadowedScene* scene = dynamic_cast<osgShadow::ShadowedScene*>(
					context.getImageGenerator()->getScene()
					);
				if (scene != 0)
				{
					osg::StateSet* ss = scene->getOrCreateStateSet();

					ss->setDefine("SIMPLELIGHTING");
					ss->setDefine("SHADOWING");
					ss->setDefine("ENVIRONMENTAL_FACTOR", "0");
					ss->setDefine("TEXTURING");


					osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(ss);

					vp->setFunction("frag_lighting", mainFragmentShaderStr, osgEarth::ShaderComp::LOCATION_FRAGMENT_COLORING);
					vp->setFunction("vert_lighting", mainVertexShaderStr, osgEarth::ShaderComp::LOCATION_VERTEX_MODEL);

					osg::Material* sceneMaterial = new osg::Material;
					sceneMaterial->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(0.7, 0.7, 0.7, 1.0));
					sceneMaterial->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(0.6, 0.6, 0.6, 1.0));
					sceneMaterial->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0.6, 0.6, 0.6, 1.0));
					sceneMaterial->setShininess(osg::Material::FRONT_AND_BACK, 60);

					ss->setAttributeAndModes(sceneMaterial, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);


					int ReceivesShadowTraversalMask = 0x1;
					int CastsShadowTraversalMask = 0x2;

					osgShadow::ShadowSettings* settings = scene->getShadowSettings();
					settings->setReceivesShadowTraversalMask(ReceivesShadowTraversalMask);
					settings->setCastsShadowTraversalMask(CastsShadowTraversalMask);

					unsigned int unit = 1;
					settings->setBaseShadowTextureUnit(unit);

					double n = 0.0;
					settings->setMinimumShadowMapNearFarRatio(n);
				}

			}

			virtual void clean(OpenIG::PluginBase::PluginContext& context)
			{
				context.getImageGenerator()->setLightImplementationCallback(0);
			}

		protected:

			class UpdateLightsEnabledUniformCallback : public osg::Uniform::Callback
			{
			public:
				UpdateLightsEnabledUniformCallback(OpenIG::Base::ImageGenerator* ig)
					: _ig(ig)
				{

				}

				virtual void operator () (osg::Uniform* u, osg::NodeVisitor*)
				{
					for (size_t i = 0; i < 8; ++i)
					{
						u->setElement(i, _ig->isLightEnabled(i));
					}
				}

			protected:
				OpenIG::Base::ImageGenerator*     _ig;
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

			class SimpleLightImplementationCallback : public OpenIG::Base::LightImplementationCallback
			{
			public:
				SimpleLightImplementationCallback(OpenIG::Base::ImageGenerator* ig)
					: _ig(ig)
				{

				}

				virtual osg::Referenced* createLight(
					unsigned int id,
					const OpenIG::Base::LightAttributes& definition,
					osg::Group*)
				{
					osg::LightSource* light = new osg::LightSource;

					DummyLight* dl = new DummyLight(id, true);

					dl->setLightNum(id);
					dl->setAmbient(definition.ambient);
					dl->setDiffuse(definition.diffuse*definition.brightness);
					dl->setSpecular(definition.specular);
					dl->setConstantAttenuation(1.f / definition.constantAttenuation);
					dl->setSpotCutoff(definition.spotCutoff);
					dl->setPosition(osg::Vec4(0, 0, 0, 1));
					dl->setDirection(osg::Vec3(0, 1, -0));

					light->setLight(dl);

					if (id >= 1 && id < 8)
					{
						light->setStateSetModes(*_ig->getViewer()->getView(0)->getSceneData()->getOrCreateStateSet(), osg::StateAttribute::ON);
					}

					_lights[id] = light;

					return light;
				}

				virtual void updateLight(unsigned int id, const OpenIG::Base::LightAttributes& definition)
				{
					LightsMapIterator itr = _lights.find(id);
					if (itr != _lights.end())
					{
						osg::LightSource* light = itr->second;

						if ((definition.dirtyMask & OpenIG::Base::LightAttributes::AMBIENT) == OpenIG::Base::LightAttributes::AMBIENT)
							light->getLight()->setAmbient(definition.ambient);

						if ((definition.dirtyMask & OpenIG::Base::LightAttributes::DIFFUSE) == OpenIG::Base::LightAttributes::DIFFUSE &&
							(definition.dirtyMask & OpenIG::Base::LightAttributes::BRIGHTNESS) == OpenIG::Base::LightAttributes::BRIGHTNESS)
							light->getLight()->setDiffuse(definition.diffuse*definition.brightness);

						if ((definition.dirtyMask & OpenIG::Base::LightAttributes::SPECULAR) == OpenIG::Base::LightAttributes::SPECULAR)
							light->getLight()->setSpecular(definition.specular);

						if ((definition.dirtyMask & OpenIG::Base::LightAttributes::CONSTANTATTENUATION) == OpenIG::Base::LightAttributes::CONSTANTATTENUATION)
							light->getLight()->setConstantAttenuation(1.f / definition.constantAttenuation);

						if ((definition.dirtyMask & OpenIG::Base::LightAttributes::SPOTCUTOFF) == OpenIG::Base::LightAttributes::SPOTCUTOFF)
							light->getLight()->setSpotCutoff(definition.spotCutoff);

					}
				}

			protected:
				OpenIG::Base::ImageGenerator*     _ig;

				typedef std::map<unsigned int, osg::ref_ptr<osg::LightSource> >                 LightsMap;
				typedef std::map<unsigned int, osg::ref_ptr<osg::LightSource> >::iterator       LightsMapIterator;

				LightsMap                   _lights;
			};

			osg::ref_ptr<OpenIG::Base::LightImplementationCallback>       _lightImplementationCallback;
			osg::ref_ptr<osg::Material>                             _sceneMaterial;
			int                                                     _cloudsShadowsTextureSlot;
		};
	} // namespace
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

extern "C" EXPORT OpenIG::PluginBase::Plugin* CreatePlugin()
{
	return new OpenIG::Plugins::OSGEarthSimpleLightingPlugin;
}

extern "C" EXPORT void DeletePlugin(OpenIG::PluginBase::Plugin* plugin)
{
	osg::ref_ptr<OpenIG::PluginBase::Plugin> p(plugin);
}
