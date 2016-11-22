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
//#*    Please direct any questions or comments to the OpenIG Forums
//#*    Email address: openig@compro.net
//#*
//#*
//#*****************************************************************************
// Lighting GLSL inspired from the following article
// http://www.ozone3d.net/tutorials/glsl_lighting_phong_p3.php

#include <Core-PluginBase/Plugin.h>
#include <Core-PluginBase/PluginContext.h>

#include <Core-Base/ImageGenerator.h>
#include <Core-Base/Types.h>
#include <Core-Base/Configuration.h>
#include <Core-Base/FileSystem.h>

#include <osg/ref_ptr>
#include <osg/LightSource>
#include <osg/Material>
#include <osg/Group>

#include <osgDB/XmlParser>

#include <osgShadow/ShadowedScene>
#include <osgShadow/MinimalShadowMap>

#include <map>
#include <sstream>

namespace OpenIG {
    namespace Plugins {

        class SimpleLightingPlugin : public OpenIG::PluginBase::Plugin
        {
        public:
            SimpleLightingPlugin()
                : _cloudsShadowsTextureSlot(6)
                , _setupDone(false)
            {

            }

            virtual std::string getName() { return "SimpleLighting"; }

            virtual std::string getDescription() { return "Implements simple lighting by using shaders and OpenGL, limited to 7 (+sun) lights"; }

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

            virtual void init(OpenIG::PluginBase::PluginContext& context)
            {
                setup(context);
            }

            virtual void update(OpenIG::PluginBase::PluginContext& context)
            {
                if (!_setupDone) setup(context);
            }

            virtual void setup(OpenIG::PluginBase::PluginContext& context)
            {
                _lightImplementationCallback = new SimpleLightImplementationCallback(context.getImageGenerator());
                context.getImageGenerator()->setLightImplementationCallback(_lightImplementationCallback);

                std::ostringstream ossfs;
                ossfs << "#define SELF_SHADOW_STAGE  1                                                                   \n";
                ossfs << "#define CLOUDS_SHADOW_STAGE " << _cloudsShadowsTextureSlot << "                                \n";
                ossfs << "uniform sampler2DShadow    shadowTexture;                                                      \n";
                ossfs << "uniform sampler2D          cloudShadowTexture;                                                 \n";
                ossfs << "float DynamicShadow()                                                                          \n";
                ossfs << "{                                                                                              \n";
                ossfs << "   float selfShadow = shadow2DProj( shadowTexture, gl_TexCoord[SELF_SHADOW_STAGE] ).r;         \n";
                ossfs << "   float cloudsShadow = texture2D( cloudShadowTexture, gl_TexCoord[CLOUDS_SHADOW_STAGE].xy ).r;\n";
                ossfs << "   return selfShadow * cloudsShadow;                                                     \n";
                ossfs << "}                                                                                              \n";
                osg::ref_ptr<osg::Shader> shadowFragmentShader = new osg::Shader(osg::Shader::FRAGMENT, ossfs.str());

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
                osg::ref_ptr<osg::Shader> shadowVertexShader = new osg::Shader(osg::Shader::VERTEX, ossvs.str());

#if defined(_WIN32)
            std::string resourcesPath = OpenIG::Base::FileSystem::path(OpenIG::Base::FileSystem::Resources, "../resources");
#else
            std::string resourcesPath = OpenIG::Base::FileSystem::path(OpenIG::Base::FileSystem::Resources, "../../openig/resources");
#endif

                std::string shaderSource = OpenIG::Base::FileSystem::readFileIntoString(resourcesPath + "/shaders/simplelighting_vs.glsl");
                osg::ref_ptr<osg::Shader> mainVertexShader = new osg::Shader(osg::Shader::VERTEX, shaderSource);
                mainVertexShader->setName("SimpleLighting: main vertex shader");

                shaderSource = OpenIG::Base::FileSystem::readFileIntoString(resourcesPath + "/shaders/simplelighting_ps.glsl");
                osg::ref_ptr<osg::Shader> mainFragmentShader = new osg::Shader(osg::Shader::FRAGMENT, shaderSource);
                mainVertexShader->setName("SimpleLighting: main fragment shader");

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

                        osgViewer::ViewerBase::Views views;
                        context.getImageGenerator()->getViewer()->getViews(views);

                        osgViewer::ViewerBase::Views::iterator itr = views.begin();
                        for (; itr != views.end(); ++itr)
                        {
                            osgViewer::View* view = *itr;

                            bool openIGScene = false;
                            if (view->getUserValue("OpenIG-Scene", openIGScene) && openIGScene)
                            {
                                bool viewSetupWithSimplePlugin = false;
                                if (view->getUserValue("OpenIG-SimpleLightingSetup", viewSetupWithSimplePlugin) && viewSetupWithSimplePlugin)
                                {
                                    continue;
                                }
                                view->setUserValue("OpenIG-SimpleLightingSetup", (bool)true);

                                osg::StateSet* ss = context.getImageGenerator()->getScene()->getOrCreateStateSet();// view->getSceneData()->getOrCreateStateSet();

                                if (!_sceneMaterial.valid())
                                {
                                    osg::Material* sceneMaterial = new osg::Material;
                                    sceneMaterial->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(0.7, 0.7, 0.7, 1.0));
                                    sceneMaterial->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(0.6, 0.6, 0.6, 1.0));
                                    sceneMaterial->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0.6, 0.6, 0.6, 1.0));
                                    sceneMaterial->setShininess(osg::Material::FRONT_AND_BACK, 60);

                                    ss->setAttributeAndModes(sceneMaterial, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
                                }
                                else
                                {
                                    ss->setAttributeAndModes(_sceneMaterial, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
                                }

                                osg::Uniform* u = new osg::Uniform(osg::Uniform::BOOL, "lightsEnabled", 8);
                                osg::Uniform::Callback* cb = new UpdateLightsEnabledUniformCallback(context.getImageGenerator());
                                u->setUpdateCallback(cb);
                                ss->addUniform(u);

                                float shadowsFactor = OpenIG::Base::Configuration::instance()->getConfig("Shadows-Factor", 0.5);
                                ss->addUniform(new osg::Uniform("shadowsFactor", shadowsFactor));

                                std::string strLogZDepthBuffer = OpenIG::Base::Configuration::instance()->getConfig("LogZDepthBuffer", "yes");
                                if (strLogZDepthBuffer.compare(0, 3, "yes") == 0)
                                {
                                    ss->setDefine("USE_LOG_DEPTH_BUFFER");
                                }

                                ss->setDefine("SHADOWING");
                                ss->setDefine("ENVIRONMENTAL_FACTOR", "0");

                                unsigned int defaultDiffuseSlot = OpenIG::Base::Configuration::instance()->getConfig("Default-diffuse-texture-slot", 0);
                                ss->addUniform(new osg::Uniform("baseTexture", (int)defaultDiffuseSlot), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

                                break;
                            }
                        }

                    }
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

            class SimpleLightImplementationCallback : public OpenIG::Base::LightImplementationCallback
            {
            public:
                SimpleLightImplementationCallback(OpenIG::Base::ImageGenerator* ig)
                    : _ig(ig)
                {

                }

                void setInitialOSGLightParameters(osg::Light* light, const OpenIG::Base::LightAttributes& definition, const osg::Vec4d& pos, const osg::Vec3f& dir)
                {
                    if (light == 0)
                    {
                        return;
                    }

                    light->setAmbient(definition.ambient);
                    light->setDiffuse(definition.diffuse*definition.brightness);
                    light->setSpecular(definition.specular);
                    light->setConstantAttenuation(1.f / definition.constantAttenuation);
                    light->setSpotCutoff(definition.spotCutoff);
                    light->setPosition(pos);
                    light->setDirection(dir);
                    light->setUserValue("realLightLOD", (double)definition.realLightLOD);
                    light->setUserValue("fStartRange", (double)definition.fStartRange);
                    light->setUserValue("fEndRange", (double)definition.fEndRange);
                    light->setUserValue("fSpotInnerAngle", (double)definition.fSpotInnerAngle);
                    light->setUserValue("fSpotOuterAngle", (double)definition.fSpotOuterAngle);
                }

                void updateOSGLightParameters(osg::Light* light, const OpenIG::Base::LightAttributes& definition)
                {
                    if (light == 0)
                    {
                        return;
                    }

                    if (definition.dirtyMask & OpenIG::Base::LightAttributes::AMBIENT)
                        light->setAmbient(definition.ambient);

                    if (definition.dirtyMask & OpenIG::Base::LightAttributes::DIFFUSE)
                    {
                        light->setDiffuse(definition.diffuse*definition.brightness);
                    }
                    if (definition.dirtyMask & OpenIG::Base::LightAttributes::BRIGHTNESS)
                    {
                        light->setDiffuse(definition.diffuse*definition.brightness);
                    }
                    if (definition.dirtyMask & OpenIG::Base::LightAttributes::SPECULAR)
                        light->setSpecular(definition.specular);

                    if (definition.dirtyMask & OpenIG::Base::LightAttributes::CONSTANTATTENUATION)
                        light->setConstantAttenuation(1.f / definition.constantAttenuation);

                    if (definition.dirtyMask & OpenIG::Base::LightAttributes::SPOTCUTOFF)
                        light->setSpotCutoff(definition.spotCutoff);

                    if (definition.dirtyMask & OpenIG::Base::LightAttributes::REALLIGHTLOD)
                    {
                        light->setUserValue("realLightLOD", (double)definition.realLightLOD);
                    }
                    if (definition.dirtyMask & OpenIG::Base::LightAttributes::RANGES)
                    {
                        light->setUserValue("fStartRange", (double)definition.fStartRange);
                        light->setUserValue("fEndRange", (double)definition.fEndRange);
                    }
                    if (definition.dirtyMask & OpenIG::Base::LightAttributes::ANGLES)
                    {
                        light->setUserValue("fSpotInnerAngle", (double)definition.fSpotInnerAngle);
                        light->setUserValue("fSpotOuterAngle", (double)definition.fSpotOuterAngle);
                    }
                    if ((definition.dirtyMask & OpenIG::Base::LightAttributes::ENABLED) == OpenIG::Base::LightAttributes::ENABLED)
                    {
                        light->setUserValue("enabled", definition.enabled);
                    }
                }


                virtual osg::Referenced* createLight(
                    unsigned int id,
                    const OpenIG::Base::LightAttributes& definition,
                    osg::Group*)
                {
                    if (_lights.size() < 8) return NULL;

                    osg::LightSource* light = new osg::LightSource;
                    light->getLight()->setLightNum(id);
                    setInitialOSGLightParameters(light->getLight(), definition, osg::Vec4d(0, 0, 0, 1), osg::Vec3f(0, 1, 0));

                    light->getLight()->setUserValue("id", (unsigned int)id);
                    light->getLight()->setUserValue("enabled", definition.enabled);

                    light->setDataVariance(definition.dataVariance);
                    light->setCullingActive(true);

                    osg::ref_ptr<osg::StateAttribute> attr = light->getOrCreateStateSet()->getAttribute(osg::StateAttribute::LIGHT);
                    if (attr.valid())
                    {
                        light->getOrCreateStateSet()->removeAttribute(attr);
                        osg::notify(osg::NOTICE) << "Light attr removed" << std::endl;
                    }

                    _lights[id] = light;

                    return light;
                }

                virtual void updateLight(unsigned int id, const OpenIG::Base::LightAttributes& definition)
                {

                    LightsMapIterator itr = _lights.find(id);
                    if (itr != _lights.end())
                    {
                        osg::LightSource* lightSource = itr->second;
                        osg::Light* light = lightSource->getLight();

                        updateOSGLightParameters(light, definition);
                    }
                }

            protected:
                OpenIG::Base::ImageGenerator*	_ig;

                typedef std::map<unsigned int, osg::ref_ptr<osg::LightSource> >                 LightsMap;
                typedef std::map<unsigned int, osg::ref_ptr<osg::LightSource> >::iterator       LightsMapIterator;

                LightsMap						_lights;
            };

            osg::ref_ptr<OpenIG::Base::LightImplementationCallback>			_lightImplementationCallback;
            osg::ref_ptr<osg::Material>										_sceneMaterial;
            int																_cloudsShadowsTextureSlot;
            bool															_setupDone;
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
    return new OpenIG::Plugins::SimpleLightingPlugin;
}

extern "C" EXPORT void DeletePlugin(OpenIG::PluginBase::Plugin* plugin)
{
    osg::ref_ptr<OpenIG::PluginBase::Plugin> p(plugin);
}
