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

//#include <vld.h>

#include <Core-PluginBase/plugin.h>
#include <Core-PluginBase/plugincontext.h>

#include <Core-Base/imagegenerator.h>
#include <Core-Base/attributes.h>
#include <Core-Base/configuration.h>
#include <Core-Base/globalidgenerator.h>
#include <Core-Base/commands.h>
#include <Core-Base/stringutils.h>
#include <Core-Base/filesystem.h>

#include <Library-Graphics/LightManager.h>

#include <osg/Version>
#include <osg/ref_ptr>
#include <osg/LightSource>
#include <osg/Light>
#include <osg/Material>
#include <osg/TextureBuffer>
#include <osg/ValueObject>

#include <osgUtil/PositionalStateContainer>

#include <osgDB/XmlParser>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include <osgShadow/ShadowedScene>
#include <osgShadow/MinimalShadowMap>

#include <OpenThreads/Thread>
#include <OpenThreads/Mutex>
#include <OpenThreads/ScopedLock>

#include <osgUtil/CullVisitor>
#include <osgUtil/SceneView>

#include <osgViewer/Viewer>
#include <osgViewer/Renderer>


#include <map>
#include <iostream>
#include <sstream>
#include <ctime>

#include <boost/thread.hpp>
#include <boost/filesystem.hpp>

#include "dummylight.h"
#include "forwardpluscullvisitor.h"
#include "forwardpluslightimplementationcallback.h"
#include "osgtofputils.h"

using namespace OpenIG::Library::Graphics;

namespace OpenIG {
	namespace Plugins {

		class ForwardPlusLightingPlugin : public OpenIG::PluginBase::Plugin
		{
		public:
			ForwardPlusLightingPlugin()
				: _maxNumLightsPerPixel(200)
				, _cloudsShadowsTextureSlot(6)
				, _lightBrightness_enable(true)
				, _lightBrightness_day(1.f)
				, _lightBrightness_night(1.f)
				, _todHour(12)
				, _xmlFileObserverThreadRunning(false)
				, _xmlFileObserverThreadRunningCondition(false)
			{
			}

			virtual std::string getName() { return "ForwardPlusLighting"; }

			virtual std::string getDescription() { return "Implements scene lighting using Forward+"; }

			virtual std::string getVersion() { return "2.0.0"; }

			virtual std::string getAuthor() { return "ComPro, Poojan"; }

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
					else if (child->name == "Max-Num-Of-Lights")
					{
						_maxNumLightsPerPixel = atoi(child->contents.c_str());
					}
				}
			}

			class EffectsCommand : public OpenIG::Base::Commands::Command
			{
			public:
				EffectsCommand(OpenIG::Base::ImageGenerator* ig)
					: _ig(ig)
				{

				}

				virtual const std::string getUsage() const
				{
					return "id effect on/off";
				}

				virtual const std::string getArgumentsFormat() const
				{
					return "I:{envmapping;aomapping;lighting;shadows}:{on;off}";
				}

				virtual const std::string getDescription() const
				{
					return  "turns on/off shader effects on an entity\n"
						"     id - the id of the new entity across the scene\n"
						"     effect - one of these\n"
						"           envmapping\n"
						"           aomapping\n"
						"           lighting\n"
						"           shadows\n"
						"     on/off - turns the effect on or off";
				}

				virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
				{
					if (tokens.size() == 3)
					{
						unsigned int    id = atoi(tokens.at(0).c_str());
						std::string     effect = tokens.at(1);
						bool            on = tokens.at(2) == "on";

						if (_ig->getEntityMap().count(id) == 0) return -1;

						OpenIG::Base::ImageGenerator::Entity& entity = _ig->getEntityMap()[id];
						if (!entity.valid()) return -1;

						std::map< std::string, std::string > e2d;

						e2d["envmapping"] = "ENVIRONMENTAL";
						e2d["aomapping"] = "AO";
						e2d["shadows"] = "SHADOWING";

						std::map< std::string, std::string >::iterator itr = e2d.find(effect);
						if (itr == e2d.end()) return -1;

						OpenIG::Base::StringUtils::Tokens defines = OpenIG::Base::StringUtils::instance()->tokenize(itr->second, ";");
						for (size_t i = 0; i < defines.size(); ++i)
						{
#if OSG_VERSION_GREATER_OR_EQUAL(3,3,7)
							switch (on)
							{
							case true:
								entity->getOrCreateStateSet()->setDefine(defines.at(i),
									osg::StateAttribute::ON |
									osg::StateAttribute::PROTECTED |
									osg::StateAttribute::OVERRIDE);
								break;
							case false:
								entity->getOrCreateStateSet()->setDefine(defines.at(i),
									osg::StateAttribute::OFF |
									osg::StateAttribute::PROTECTED |
									osg::StateAttribute::OVERRIDE);
								break;
							}
#endif
						}

						return 0;
					}
					return -1;
				}

			protected:
				OpenIG::Base::ImageGenerator* _ig;
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

				if (!osgDB::fileExists(xmlFile)) return;

				osg::notify(osg::NOTICE) << "ForwardPlusLighting: processing xml file: " << xmlFile << std::endl;

				osgDB::XmlNode* root = osgDB::readXmlFile(xmlFile);
				if (!root || !root->children.size() || root->children.at(0)->name != "OsgNodeSettings") return;

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

			void replaceCullVisitor(OpenIG::PluginBase::PluginContext& context)
			{
				// We set here our new CullVisitor
				// that will find out our ShadowedScene
				// osg::Program. Nick
				osgViewer::CompositeViewer* viewer = context.getImageGenerator()->getViewer();
				osgViewer::Renderer* renderer = dynamic_cast<osgViewer::Renderer*>(viewer->getView(0)->getCamera()->getRenderer());
				if (renderer == 0)
				{
					return;
				}

				osgUtil::SceneView* sv = renderer->getSceneView(0);
				if (sv == 0)
				{
					return;
				}

#if 0
				sv->setCullVisitor(new ForwardPlusCullVisitor());
				osg::notify(osg::NOTICE) << "ForwardPlusLighting: default CullVisitor replaced in SceneView 0" << std::endl;
#endif
			}

			ForwardPlusCullVisitor* getCullVisitor(OpenIG::PluginBase::PluginContext& context)
			{
				// osg::Program. Nick
				osgViewer::CompositeViewer* viewer = context.getImageGenerator()->getViewer();
				osgViewer::Renderer* renderer = dynamic_cast<osgViewer::Renderer*>(viewer->getView(0)->getCamera()->getRenderer());
				if (renderer == 0)
				{
					return 0;
				}

				osgUtil::SceneView* sv = renderer->getSceneView(0);
				if (sv == 0)
				{
					return 0;
				}

				return dynamic_cast<ForwardPlusCullVisitor*>(sv->getCullVisitor());
			}

			void setupShaders(OpenIG::PluginBase::PluginContext& context)
			{
				std::string strSource;

				std::string resourcesPath = OpenIG::Base::FileSystem::path(OpenIG::Base::FileSystem::Resources, "../resources");

				std::string strLogZDepthBuffer = OpenIG::Base::Configuration::instance()->getConfig("LogZDepthBuffer","yes");
				if (strLogZDepthBuffer.compare(0, 3, "yes") == 0)
				{
					strLogZDepthBuffer = std::string("#define USE_LOG_DEPTH_BUFFER\n");
				}
				else
				{
					strLogZDepthBuffer="";
				}


				strSource = 
					std::string("#version 130\n")
					+ strLogZDepthBuffer
					+ OpenIG::Base::FileSystem::readFileIntoString(resourcesPath + "/shaders/forwardplus_vs.glsl");
				osg::Shader* mainVertexShader = new osg::Shader(osg::Shader::VERTEX, strSource);
				mainVertexShader->setName("ForwardPlusLighting: main vertex shader");

				std::stringstream ssMAX_LIGHTS_PER_PIXEL;ssMAX_LIGHTS_PER_PIXEL<<_maxNumLightsPerPixel;
				std::string strMAX_LIGHTS_PER_PIXEL = "#define MAX_LIGHTS_PER_PIXEL " + ssMAX_LIGHTS_PER_PIXEL.str()+"\n";

				strSource = OpenIG::Base::FileSystem::readFileIntoString(resourcesPath + "/shaders/forwardplus_preamble.glsl")
					+ strLogZDepthBuffer
					+ strMAX_LIGHTS_PER_PIXEL
					+ OpenIG::Base::FileSystem::readFileIntoString(resourcesPath + "/shaders/forwardplus_ps.glsl")
					+ OpenIG::Base::FileSystem::readFileIntoString(resourcesPath + "/shaders/lighting_math.glsl")
					+ OpenIG::Base::FileSystem::readFileIntoString(resourcesPath + "/shaders/forwardplus_math.glsl");

				osg::Shader* mainFragmentShader = new osg::Shader(osg::Shader::FRAGMENT, strSource);
				mainFragmentShader->setName("ForwardPlusLighting: main fragment shader");

				strSource = OpenIG::Base::FileSystem::readFileIntoString(resourcesPath + "/shaders/shadow_vs.glsl");
				osg::Shader* shadowVertexShader = new osg::Shader(osg::Shader::VERTEX, strSource);
				shadowVertexShader->setName("ForwardPlusLighting: shadow vertex shader");

				strSource = OpenIG::Base::FileSystem::readFileIntoString(resourcesPath + "/shaders/shadow_ps.glsl");
				osg::Shader* shadowFragmentShader = new osg::Shader(osg::Shader::FRAGMENT, strSource);
				shadowFragmentShader->setName("ForwardPlusLighting: shadow fragment shader");

				osgShadow::ShadowedScene* scene = dynamic_cast<osgShadow::ShadowedScene*>(context.getImageGenerator()->getScene());
				if (scene == 0)
				{
					return;
				}

				osgShadow::MinimalShadowMap* msm = dynamic_cast<osgShadow::MinimalShadowMap*>(
					scene->getShadowTechnique()
					);
				if (msm == 0)
				{
					return;
				}

				msm->setMainVertexShader(mainVertexShader);
				msm->setMainFragmentShader(mainFragmentShader);
				msm->setShadowVertexShader(shadowVertexShader);
				msm->setShadowFragmentShader(shadowFragmentShader);

				osg::StateSet* ss = context.getImageGenerator()->getViewer()->getView(0)->getSceneData()->getOrCreateStateSet();

#if OSG_VERSION_GREATER_OR_EQUAL(3,3,7)
				ss->setDefine("SHADOWING");
				ss->setDefine("ENVIRONMENTAL_FACTOR", "0");
#else
				osg::notify(osg::NOTICE) << "NOTE: Plugin ForwardPlusLighting built with version prior to 3.3.7." << std::endl;
				osg::notify(osg::NOTICE) << "  The shader composition will not have effect thus the" << std::endl;
				osg::notify(osg::NOTICE) << "  special rendering effects will not take place." << std::endl;
#endif
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

				float shadowsFactor = OpenIG::Base::Configuration::instance()->getConfig("Shadows-Factor", 0.5);
				ss->addUniform(new osg::Uniform("shadowsFactor", shadowsFactor));

				unsigned int defaultDiffuseSlot = OpenIG::Base::Configuration::instance()->getConfig("Default-diffuse-texture-slot", 0);
				ss->addUniform(new osg::Uniform("baseTexture", (int)defaultDiffuseSlot), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

				osg::Uniform* todBasedLightingUniform = new osg::Uniform("todBasedLightBrightness", (float)1.f);
				todBasedLightingUniform->setUpdateCallback(new UpdateTODBasedLightingUniformCallback(
					_lightBrightness_enable, _lightBrightness_day, _lightBrightness_night, _todHour)
					);
				ss->addUniform(todBasedLightingUniform);

				osg::Uniform* todBasedLightingEnabledUniform = new osg::Uniform("todBasedLightBrightnessEnabled", (bool)true);
				ss->addUniform(todBasedLightingEnabledUniform);
			}

			virtual void init(OpenIG::PluginBase::PluginContext& context)
			{
				replaceCullVisitor(context);

				OpenIG::Base::GlobalIdGenerator::instance()->initIdGroup("Real-Lights", 10000, 4000);

				_lightImplementationCallback = new ForwardPlusLightImplementationCallback(context.getImageGenerator());
				context.getImageGenerator()->setLightImplementationCallback(_lightImplementationCallback);

				setupShaders(context);

				OpenIG::Base::Commands::instance()->addCommand("effects", new EffectsCommand(context.getImageGenerator()));
			}


			virtual void update(OpenIG::PluginBase::PluginContext& context)
			{
				// PPP: TO DO
				//LightManager::instance()->updateTextureObject();

				osg::ref_ptr<osg::Referenced> ref = context.getAttribute("TOD");
				OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::TimeOfDayAttributes> *attr = dynamic_cast<OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::TimeOfDayAttributes> *>(ref.get());
				if (attr)
				{
					_todHour = attr->getValue().getHour();
				}

				// Here we check if the XML has changed. If so, reload and update
				_xmlAccessMutex.lock();
				if (_xmlLastCheckedTime != _xmlLastWriteTime)
				{
					osg::notify(osg::NOTICE) << "ForwardPlus: XML updated: " << _xmlFile << std::endl;
					updateFromXML(_xmlFile);

					_xmlLastCheckedTime = _xmlLastWriteTime;
				}
				_xmlAccessMutex.unlock();
			}

			virtual void databaseRead(const std::string& fileName, osg::Node*, const osgDB::Options*)
			{


				// We read the lights control config xml if exists for these lights
				const std::string xmlFileName = fileName + ".lighting.xml";
				if (osgDB::fileExists(xmlFileName))
				{
					updateFromXML(_xmlFile = xmlFileName);

					// we expect one config file per
					// visual database so we launch
					// a observer thread to monitor
					// changes on this file. If there
					// will be a need of muitiple files
					// like per tile, or multiple databases
					// then consider making these in a vector
					_xmlFileObserverThread = boost::shared_ptr<boost::thread>(new boost::thread(&OpenIG::Plugins::ForwardPlusLightingPlugin::xmlFileObserverThread, this));
				}
			}


			virtual void clean(OpenIG::PluginBase::PluginContext& context)
			{
				context.getImageGenerator()->setLightImplementationCallback(0);

				if (_xmlFileObserverThread.get())
				{
					_xmlFileObserverThreadRunningCondition = false;
					while (_xmlFileObserverThreadRunning);
					_xmlFileObserverThread->join();
				}
				_xmlFileObserverThread.reset();
			}

		protected:
			void xmlFileObserverThread()
			{
				_xmlFileObserverThreadRunning = true;
				_xmlFileObserverThreadRunningCondition = true;

				while (_xmlFileObserverThreadRunningCondition)
				{
					_xmlAccessMutex.lock();
					_xmlLastWriteTime = OpenIG::Base::FileSystem::lastWriteTime(_xmlFile);
					_xmlAccessMutex.unlock();

					OpenThreads::Thread::microSleep(10000);
				}

				_xmlFileObserverThreadRunning = false;
			}

			osg::ref_ptr<OpenIG::Base::LightImplementationCallback> _lightImplementationCallback;
			unsigned int                                            _maxNumLightsPerPixel;
			unsigned int                                            _cloudsShadowsTextureSlot;
			osg::ref_ptr<osg::Material>                             _sceneMaterial;
			bool													_lightBrightness_enable;
			float													_lightBrightness_day;
			float													_lightBrightness_night;
			unsigned int											_todHour;
			std::string												_currentXMLFile;
			LightManager*                                           _lightManager;
			osg::ref_ptr<osg::Program>								_program;

			boost::shared_ptr<boost::thread>	_xmlFileObserverThread;
			boost::mutex						_xmlAccessMutex;
			std::time_t							_xmlLastWriteTime;
			std::time_t							_xmlLastCheckedTime;
			std::string							_xmlFile;
			volatile bool						_xmlFileObserverThreadRunning;
			volatile bool						_xmlFileObserverThreadRunningCondition;
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
    return new OpenIG::Plugins::ForwardPlusLightingPlugin;
}

extern "C" EXPORT void DeletePlugin(OpenIG::PluginBase::Plugin* plugin)
{
    osg::ref_ptr<OpenIG::PluginBase::Plugin> p(plugin);
}
