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

#include <osg/ref_ptr>
#include <osg/LightSource>
#include <osg/Material>
#include <osg/Group>

#include <osgDB/XmlParser>

#include <osgShadow/ShadowedScene>
#include <osgShadow/MinimalShadowMap>

#include <map>
#include <sstream>

namespace igplugins
{

class SimpleLightingPlugin : public igplugincore::Plugin
{
public:
    SimpleLightingPlugin()
        :_cloudsShadowsTextureSlot(6)
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

    virtual void init(igplugincore::PluginContext& context)
    {
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
            "varying vec3 normal;                                                   \n"
            "varying vec3 eyeVec;                                                   \n"
            "varying vec3 lightDirs[8];                                             \n"
            "uniform mat4 osg_ViewMatrixInverse;									\n"
            "uniform vec3 cameraPos;        										\n"
            "uniform bool environmentalEnabled;                                     \n"
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
            "	if (!environmentalEnabled) return;              					\n"
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
            "   environmentalMapping();                                         \n"
            "}                                                                  \n"
        );


        osg::Shader* mainFragmentShader = new osg::Shader( osg::Shader::FRAGMENT,
            "#version 120                                                           \n"
            "uniform sampler2D ambientOcclusionTexture;                             \n"
            "uniform bool ambientOcclusionEnabled;                                  \n"
            "uniform samplerCube environmentalMapTexture;                           \n"
            "																		\n"
            "uniform bool environmentalEnabled;                                     \n"
            "uniform bool shadowingEnabled;                                         \n"
            "uniform bool lightingEnabled;                                          \n"
            "uniform float environmentalFactor;                                     \n"
            "uniform float ambientOcclusionFactor;                                  \n"
            "uniform float shadowsFactor;                                           \n"
            "                                                                       \n"
            "varying vec3 normal;                                                   \n"
            "varying vec3 eyeVec;                                                   \n"
            "varying vec3 lightDirs[8];                                             \n"
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
            "                                                                       \n"
            "	float lambertTerm = max(dot(N,L),0.0);                              \n"
            "                                                                       \n"
            "	//if(lambertTerm > 0.0)                                             \n"
            "	{                                                                   \n"
            "		final_color += gl_LightSource[0].diffuse *                      \n"
            "		               gl_FrontMaterial.diffuse *                       \n"
            "					   lambertTerm;                                     \n"
            "                                                                       \n"
            "		vec3 E = normalize(eyeVec);                                     \n"
            "		vec3 R = reflect(-L, N);                                        \n"
            "		float specular = pow( max(dot(R, E), 0.0),                      \n"
            "		                 gl_FrontMaterial.shininess );                  \n"
            "		final_color +=  gl_LightSource[0].specular *                    \n"
            "                               gl_FrontMaterial.specular *				\n"
            "					   specular;                                        \n"
            "	}                                                                   \n"
            "                                                                       \n"
            "	color += final_color;                                               \n"
            "}                                                                      \n"
            "                                                                       \n"
            "void computeColorForLightSource(int i, inout vec4 color)				\n"
            "{                                                                      \n"
            "   if (!lightsEnabled[i]) return;                                      \n"
            "                                                                       \n"
            "	vec4 final_color = vec4(0.0,0.0,0.0,1.0);                           \n"
            //"   (gl_FrontLightModelProduct.sceneColor * gl_FrontMaterial.ambient) +	\n"
            //"   (gl_LightSource[i].ambient * gl_FrontMaterial.ambient);               \n"
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
            "	computeAmbientColor(clr);                                           \n"
            "                                                                       \n"
            "	color *= clr;                                                       \n"
            "                                                                       \n"
            "	computeFogColor(color);                                             \n"
            "}                                                                      \n"
            "void computeAmbientOcclusion( inout vec4 color )                       \n"
            "{                                                                      \n"
            "   if (!ambientOcclusionEnabled) return;                               \n"
            "   vec4 aocolor = texture2D(ambientOcclusionTexture,gl_TexCoord[0].xy);\n"
            "   aocolor.rgb *= ambientOcclusionFactor;                              \n"
            "   color.rgb *= aocolor.rgb;                                           \n"
            "}                                                                      \n"
            "void computeEnvironmentalMap( inout vec4 color )                       \n"
            "{																		\n"
            "	if (!environmentalEnabled) return;                                  \n"
            "   vec3 v = gl_TexCoord[4].xzy;                                        \n"
            "   v.y *= -1.0;                                                        \n"
            "	vec3 cube_color =													\n"
            "		textureCube(environmentalMapTexture, v).rgb;                    \n"
            "																		\n"
            "   vec3 mixed_color =                                                  \n"
            "       mix(cube_color, color.rgb, 1.0-environmentalFactor).rgb;        \n"
            "	color.rgb *= mixed_color;//vec4(mixed_color , color.a);                                \n"
            "}																		\n"
            "void main()                                                            \n"
            "{                                                                      \n"
            "   vec4 color = texture2D( baseTexture, gl_TexCoord[0].xy );           \n"
            "   float shadow = DynamicShadow();                                     \n"
            "   if (shadowingEnabled) color.rgb = mix( color.rgb * (1.0-shadowsFactor), color.rgb, shadow );\n"
            "   if (lightingEnabled) lighting(color);                               \n"
            "   computeEnvironmentalMap(color);                                     \n"
            "   computeAmbientOcclusion(color);                                     \n"
            "   gl_FragColor = color;                                               \n"
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

                osg::Uniform* u = new osg::Uniform(osg::Uniform::BOOL,"lightsEnabled", 8);
                osg::Uniform::Callback* cb = new UpdateLightsEnabledUniformCallback(context.getImageGenerator());
                u->setUpdateCallback(cb);
                ss->addUniform(u);

                float shadowsFactor = igcore::Configuration::instance()->getConfig("Shadows-Factor",0.5);
                ss->addUniform(new osg::Uniform("shadowsFactor",shadowsFactor));

                ss->addUniform(new osg::Uniform("ambientOcclusionEnabled",(bool)false),osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE);
                ss->addUniform(new osg::Uniform("environmentalEnabled",(bool)false),osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE);
                ss->addUniform(new osg::Uniform("shadowingEnabled",(bool)true),osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE);
                ss->addUniform(new osg::Uniform("lightingEnabled",(bool)true),osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE);

                unsigned int defaultDiffuseSlot = igcore::Configuration::instance()->getConfig("Default-diffuse-texture-slot",0);
                ss->addUniform(new osg::Uniform("baseTexture",(int)defaultDiffuseSlot),osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE);

            }
        }

    }

    virtual void clean(igplugincore::PluginContext& context)
    {
        context.getImageGenerator()->setLightImplementationCallback(0);
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
            light->getLight()->setLightNum(id);
            light->getLight()->setAmbient(definition._ambient);
            light->getLight()->setDiffuse(definition._diffuse*definition._brightness);
            light->getLight()->setSpecular(definition._specular);
            light->getLight()->setConstantAttenuation(1.f/definition._constantAttenuation);
            light->getLight()->setSpotCutoff(definition._spotCutoff);
            light->getLight()->setPosition(osg::Vec4(0,0,0,1));
            light->getLight()->setDirection(osg::Vec3(0,1,-0));
            light->setStateSetModes(*_ig->getViewer()->getView(0)->getSceneData()->getOrCreateStateSet(),osg::StateAttribute::ON);

            _lights[id] = light;

            return light;
        }

        virtual void updateLight(unsigned int id, const igcore::LightAttributes& definition)
        {
            LightsMapIterator itr = _lights.find(id);
            if ( itr != _lights.end())
            {
                osg::LightSource* light = itr->second;

                if (definition._dirtyMask & igcore::LightAttributes::AMBIENT)
                    light->getLight()->setAmbient(definition._ambient);

                if (definition._dirtyMask & igcore::LightAttributes::DIFFUSE && definition._dirtyMask & igcore::LightAttributes::BRIGHTNESS)
                    light->getLight()->setDiffuse(definition._diffuse*definition._brightness);

                if (definition._dirtyMask & igcore::LightAttributes::SPECULAR)
                    light->getLight()->setSpecular(definition._specular);

                if (definition._dirtyMask & igcore::LightAttributes::CONSTANTATTENUATION)
                    light->getLight()->setConstantAttenuation(1.f/definition._constantAttenuation);

                if (definition._dirtyMask & igcore::LightAttributes::SPOTCUTOFF)
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
