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
#include <Core-PluginBase/plugin.h>
#include <Core-PluginBase/plugincontext.h>

#include <Core-Base/imagegenerator.h>
#include <Core-Base/attributes.h>
#include <Core-Base/commands.h>
#include <Core-Base/filesystem.h>

#include <Core-OpenIG/renderbins.h>

#include <osg/ref_ptr>
#include <osg/ValueObject>
#include <osg/ClipNode>
#include <osg/ClipPlane>
#include <osg/FrontFace>
#include <osg/Depth>

#include <osgDB/XmlParser>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include <osgShadow/ShadowedScene>
#include <osgShadow/MinimalShadowMap>

#include <boost/thread.hpp>
#include <boost/filesystem.hpp>

#include "TritonDrawable.h"
#include "PlanarReflection.h"

#define HEIGHT_MAP_AREA 30000
#define TEXTURE_SIZE 2048
#define RAD_PER_DEG     0.0174532952 /* radians per degree (PI/180)            */

namespace OpenIG {
	namespace Plugins {

		/** This shader assumes your terrain's vertices are in world coordinates
			and Z is "up". If this isn't true in your case, you may need to
			pass additional data into this shader in order to figure out the
			height above sea level at each vertex. For example if you are in
			geocentric coordinates, you may want to pass in the distance from the
			center of the Earth to mean sea level at the camera location, and
			subtract this from the length of the vertex vector to detemine the
			height above sea level. If you have model matrices applied to your
			terrain, you may need to pass in the inverse view matrix in order to
			be able to derive world coordinates from vertex coordinates. */
		const char *HeightMapVertSource = {
			"varying float height;\n"
			"void main()\n"
			"{\n"
			"   vec4 worldVert = gl_Vertex;\n"
			"   height = worldVert.z;\n"
			"   gl_Position = gl_ModelViewProjectionMatrix * worldVert;\n"
			"   gl_ClipVertex = gl_ModelViewMatrix * worldVert;\n"
			"}\n"
		};

		const char *HeightMapFragSource = {
			"varying float height;\n"
			"void main()\n"
			"{\n"
			"   gl_FragColor = vec4(height, height, height, 1.0);\n"
			"}\n"
		};

#define USE_LOG_DEPTH_BUFFER 0

		class TritonPlugin : public OpenIG::PluginBase::Plugin
		{
		public:
			TritonPlugin()
				: _geocentric(false)
				, _forwardPlusEnabled(false)
				, _heightMapEnabled(false)
				, _heightMapHUDEnabled(false)
				, _heightMapcamera(NULL)
				, _heightMap(NULL)
				, _depthMap(NULL)
				, _belowWaterVisibility(3.5)
				, _environmentalMapping(false)
				, _ig(0)
				, _planarReflectionBlend(2.f)
			{

			}

			virtual std::string getName() { return "Triton"; }

			virtual std::string getDescription() { return "Integration of Sundog's Triton ocean model"; }

			virtual std::string getVersion() { return "1.1.0"; }

			virtual std::string getAuthor() { return "ComPro, Nick"; }

			void setPlanarReflectionBlend(float factor)
			{
				_planarReflectionBlend = factor;
				if (_tritonDrawable) _tritonDrawable->setPlanarReflectionBlend(_planarReflectionBlend);
			}

			void setEntityReflection(unsigned int id, bool on)
			{
				_reflectedGraphEntities[id] = on;

				if (_reflectedSubGraph.valid() && _ig)
				{
					OpenIG::Base::ImageGenerator::Entity& entity = _ig->getEntityMap()[id];
					switch (on)
					{
					case true:
						_reflectedSubGraph->addChild(entity);
						//std::cout << "Reflection on for " << id << std::endl;
						break;
					case false:
						_reflectedSubGraph->removeChild(entity);
						//std::cout << "Reflection on for " << id << std::endl;
						break;
					}

					if (_tritonDrawable) _tritonDrawable->setPlanarReflectionBlend(_planarReflectionBlend);
				}
			}

			class TritonCommand : public OpenIG::Base::Commands::Command
			{
			public:
				TritonCommand(TritonPlugin* plugin)
					: _plugin(plugin)
				{
				}
				virtual const std::string getUsage() const
				{
					return "mode";
				}

				virtual const std::string getArgumentsFormat() const
				{
					return	"{geocentric;flat}";
				}

				virtual const std::string getDescription() const
				{
					return  "configures the Triton plugin\n"
						"     mode - pne of these\n"
						"        geocentric - if in geocentric mode\n"
						"        flat - if in flat earth mode\n";
				}

				virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
				{
					if (tokens.size() == 1 && _plugin != 0)
					{
						std::string mode = tokens.at(0);

						//ensure its lowercase for the string compare....
						std::transform(mode.begin(), mode.end(), mode.begin(), ::tolower);
						if (mode.compare(0, 10, "geocentric") == 0)
							_plugin->setGeocentric(true);
						else if (mode.compare(0, 4, "flat") == 0)
							_plugin->setGeocentric(false);

						return 0;
					}

					return -1;
				}

			protected:
				TritonPlugin* _plugin;
			};

			class TritonReflectionCommand : public OpenIG::Base::Commands::Command
			{
			public:
				TritonReflectionCommand(TritonPlugin* plugin)
					: _plugin(plugin)
				{
				}
				virtual const std::string getUsage() const
				{
					return "id on/off [optional: blend]";
				}

				virtual const std::string getArgumentsFormat() const
				{
					return	"I:{on;off}:D";
				}

				virtual const std::string getDescription() const
				{
					return  "sets ocean reflection for an enttity\n"
						"     id - the id of the entity\n"
						"     on/off - if on the entity is reflected on the ocean, otherwise it is not\n"
						"     blend - [optional], the planar reflection blend factor, default is 2.0;";
				}

				virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
				{
					if (tokens.size() >= 2 && _plugin != 0)
					{
						unsigned int	id = atoi(tokens.at(0).c_str());
						bool			on = (tokens.at(1).compare(0, 2, "on") == 0);

						_plugin->setEntityReflection(id, on);

						if (tokens.size() == 3)
						{
							float blend = atof(tokens.at(2).c_str());
							_plugin->setPlanarReflectionBlend(blend);
						}


						return 0;
					}

					return -1;
				}

			protected:
				TritonPlugin* _plugin;
			};

			virtual void config(const std::string& fileName)
			{
				OpenIG::Base::Commands::instance()->addCommand("triton", new TritonCommand(this));
				OpenIG::Base::Commands::instance()->addCommand("tritonreflection", new TritonReflectionCommand(this));

				osgDB::XmlNode* root = osgDB::readXmlFile(fileName);
				if (root == 0) return;

				if (root->children.size() == 0) return;

				osgDB::XmlNode* config = root->children.at(0);
				if (config->name != "OpenIG-Plugin-Config") return;

				osgDB::XmlNode::Children::iterator itr = config->children.begin();
				for (; itr != config->children.end(); ++itr)
				{
					osgDB::XmlNode* child = *itr;
					if (child->name == "Triton-License-UserName")
					{
						_userName = child->contents;
					}
					if (child->name == "Triton-License-Key")
					{
						_key = child->contents;
					}
					if (child->name == "Triton-Resource-Path")
					{
						_path = child->contents;
					}
					if (child->name == "Triton-Geocentric")
					{
						_geocentric = child->contents == "yes";
					}
					if (child->name == "Triton-HeightMap")
					{
						_heightMapEnabled = child->contents == "yes";

						// temp hack to turn off heightmap
						// for geocentric databases till
						// revisited. Nick 17 Aug 2015
						if (_geocentric) _heightMapEnabled = false;
					}
					if (child->name == "Triton-HeightMapHUD")
					{
						_heightMapHUDEnabled = child->contents == "yes";
					}
					if (child->name == "Triton-BelowWaterVisibility")
					{
						_belowWaterVisibility = std::strtod(child->contents.c_str(), NULL);
					}
					if (child->name == "Triton-EnvironmentalMapping")
					{
						_environmentalMapping = child->contents == "yes";
					}
					if (child->name == "Triton-PlanarReflectionBlend")
					{
						_planarReflectionBlend = atof(child->contents.c_str());
					}
					if (child->name == "Triton-ForwardPlusEnabled")
					{
						_forwardPlusEnabled = child->contents == "yes";
					}
				}
			}

			class FollowCameraForHeightMapGenerationUpdateNodeCallback : public osg::NodeCallback
			{
			public:
				FollowCameraForHeightMapGenerationUpdateNodeCallback(osg::Camera* camera)
					: _camera(camera) {
				}
				virtual void operator()(osg::Node* node, osg::NodeVisitor* nv) {
					osg::Vec3 eye;
					osg::Vec3 center;
					osg::Vec3 up;

					_camera->getViewMatrixAsLookAt(eye, center, up);

					osg::Vec3 d = center - eye;
					d.normalize();

					osg::Vec3 newEye = eye + d*(HEIGHT_MAP_AREA);

					osg::Vec3 heightMapCameraEye = osg::Vec3(newEye.x(), newEye.y(), 2000);
					osg::Vec3 heightMapCameraUp = osg::Vec3(0, 1, 0);
					osg::Vec3 heightMapCameraCenter = osg::Vec3(newEye.x(), newEye.y(), 0);

					osg::Camera* heightMapCamera = dynamic_cast<osg::Camera*>(node);
					if (!heightMapCamera) return;

					osg::Matrixd mx = osg::Matrix::inverse(_camera->getViewMatrix());
					osg::Quat q = mx.getRotate();

					heightMapCamera->setViewMatrixAsLookAt(
						heightMapCameraEye,
						heightMapCameraCenter,
						q*heightMapCameraUp);

				}
			protected:
				osg::Camera*    _camera;
			};

			osg::Camera* createHUDForHeightMapDisplay()
			{
				osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
				if (!wsi) {
					osg::notify(osg::ALWAYS) << "Error, no WindowSystemInterface available, cannot create windows." << std::endl;
					return 0;
				}
				//        else
				//            osg::notify(osg::ALWAYS)<<"WindowSystemInterface available creating windows."<<std::endl;


				osg::Camera* hudCamera = new osg::Camera;

				unsigned int width, height;
				wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(0), width, height);
				osg::notify(osg::NOTICE) << "Windows width: " << width << ", height: " << height << std::endl;

				width = 1600;
				height = 1200;

				hudCamera->setProjectionMatrix(osg::Matrix::ortho2D(0, width, 0, height));
				hudCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
				hudCamera->setViewMatrix(osg::Matrix::identity());
				hudCamera->setClearMask(GL_DEPTH_BUFFER_BIT);
				hudCamera->setRenderOrder(osg::Camera::POST_RENDER);
				hudCamera->setAllowEventFocus(false);
				hudCamera->setCullingMode(osg::CullSettings::ENABLE_ALL_CULLING);


				osg::Vec3Array* vertices = new osg::Vec3Array;
				vertices->push_back(osg::Vec3(100, 100, -0.1));
				vertices->push_back(osg::Vec3(400, 100, -0.1));
				vertices->push_back(osg::Vec3(400, 400, -0.1));
				vertices->push_back(osg::Vec3(100, 400, -0.1));

				osg::Geometry* geometry = new osg::Geometry;
				geometry->setVertexArray(vertices);

				osg::Vec3Array* normals = new osg::Vec3Array;
				normals->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));
				geometry->setNormalArray(normals);
				geometry->setNormalBinding(osg::Geometry::BIND_OVERALL);

				osg::Vec2Array* uvs = new osg::Vec2Array;
				uvs->push_back(osg::Vec2(0, 0));
				uvs->push_back(osg::Vec2(1, 0));
				uvs->push_back(osg::Vec2(1, 1));
				uvs->push_back(osg::Vec2(0, 1));
				geometry->setTexCoordArray(0, uvs);
				geometry->getOrCreateStateSet()->setTextureAttributeAndModes(0, _heightMap);

				osg::Vec4Array* colors = new osg::Vec4Array;
				colors->push_back(osg::Vec4(1.f, 1.f, 1.f, 1.f));
				geometry->setColorArray(colors);
				geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

				geometry->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, 4));

				osg::Geode* geode = new osg::Geode;
				geode->addDrawable(geometry);

				hudCamera->addChild(geode);

				return hudCamera;
			}

			osg::Camera* createHeightMapCamera(osg::Camera* mainCamera, osg::Node* model)
			{
				_heightMap = new osg::Texture2D();
				_heightMap->setTextureSize(TEXTURE_SIZE, TEXTURE_SIZE);
#if 1
				_heightMap->setInternalFormat(GL_LUMINANCE32F_ARB);
				_heightMap->setSourceFormat(GL_LUMINANCE);
#else
				_heightMap->setInternalFormat(GL_RGBA);
#endif
				_heightMap->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
				_heightMap->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);

				_heightMap->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_EDGE);
				_heightMap->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE);

				_heightMap->setNumMipmapLevels(0);
				_heightMap->setUseHardwareMipMapGeneration(false);
				_heightMap->setResizeNonPowerOfTwoHint(false);

				_depthMap = new osg::Texture2D;
				_depthMap->setTextureSize(TEXTURE_SIZE, TEXTURE_SIZE);
				_depthMap->setSourceFormat(GL_DEPTH_COMPONENT);
				_depthMap->setSourceType(GL_FLOAT);
				_depthMap->setInternalFormat(GL_DEPTH_COMPONENT24);
				_depthMap->setNumMipmapLevels(0);
				_depthMap->setFilter(osg::Texture2D::MIN_FILTER,
					osg::Texture2D::LINEAR);
				_depthMap->setFilter(osg::Texture2D::MAG_FILTER,
					osg::Texture2D::LINEAR);
				_depthMap->setWrap(osg::Texture::WRAP_S,
					osg::Texture::CLAMP_TO_EDGE);
				_depthMap->setWrap(osg::Texture::WRAP_T,
					osg::Texture::CLAMP_TO_EDGE);
				_depthMap->setUseHardwareMipMapGeneration(false);
				_depthMap->setResizeNonPowerOfTwoHint(false);

				// Create its camera and render to it
				_heightMapcamera = new osg::Camera;
				_heightMapcamera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				_heightMapcamera->setClearColor(osg::Vec4(-1000.0, -1000.0, -1000.0, 1.0f));
				_heightMapcamera->setViewport(0, 0, TEXTURE_SIZE, TEXTURE_SIZE);
				_heightMapcamera->setRenderOrder(osg::Camera::PRE_RENDER);
				_heightMapcamera->attach(osg::Camera::COLOR_BUFFER, _heightMap);
				_heightMapcamera->attach(osg::Camera::DEPTH_BUFFER, _depthMap);
				_heightMapcamera->addChild(model);
				_heightMapcamera->setUpdateCallback(new FollowCameraForHeightMapGenerationUpdateNodeCallback(mainCamera));
				_heightMapcamera->setProjectionMatrix(osg::Matrix::ortho(-HEIGHT_MAP_AREA, HEIGHT_MAP_AREA, -HEIGHT_MAP_AREA, HEIGHT_MAP_AREA, 1, 100000));
				_heightMapcamera->setCullingActive(true);
				_heightMapcamera->setReferenceFrame(osg::Camera::ABSOLUTE_RF_INHERIT_VIEWPOINT);
				_heightMapcamera->setImplicitBufferAttachmentMask(0, 0);
				_heightMapcamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
				_heightMapcamera->getOrCreateStateSet()->setMode(GL_ALPHA_TEST,
					osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
				_heightMapcamera->setLODScale(0.5f);
				_heightMapcamera->setCullingMode(osg::CullSettings::ENABLE_ALL_CULLING);
				_heightMapcamera->setCullMask(0x2);

				osg::ref_ptr<osg::Program> heightMapProgram = new osg::Program;
				heightMapProgram->addShader(new osg::Shader(osg::Shader::VERTEX, HeightMapVertSource));
				heightMapProgram->addShader(new osg::Shader(osg::Shader::FRAGMENT, HeightMapFragSource));

				osg::StateSet *stateSet = _heightMapcamera->getOrCreateStateSet();
				stateSet->setAttributeAndModes(heightMapProgram.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);

				return _heightMapcamera.get();
			}

			void setGeocentric(bool geocentric)
			{
				_geocentric = geocentric;
			}

			virtual void init(OpenIG::PluginBase::PluginContext& context)
			{
				_ig = context.getImageGenerator();

				initTriton(context.getImageGenerator());
			}

			void readXML(const std::string& xmlFileName)
			{
				osg::notify(osg::NOTICE) << "Triton: parsing xml file: " << xmlFileName << std::endl;

				osgDB::XmlNode* root = osgDB::readXmlFile(xmlFileName);
				if (!root || !root->children.size() || root->children.at(0)->name != "OsgNodeSettings") return;

				bool lightBrightness_enable = true;
				float lightBrightness_day = 0.f;
				float lightBrightness_night = 0.f;

				osgDB::XmlNode::Children::iterator itr = root->children.at(0)->children.begin();
				for (; itr != root->children.at(0)->children.end(); ++itr)
				{
					osgDB::XmlNode* child = *itr;

					//<LightBrightnessOnWater day="0.01" night=".1" />
					if (child->name == "LightBrightnessOnWater")
					{
						osgDB::XmlNode::Properties::iterator pitr = child->properties.begin();
						for (; pitr != child->properties.end(); ++pitr)
						{
							if (pitr->first == "enable")
							{
								lightBrightness_enable = pitr->second == "true";
							}
							if (pitr->first == "day")
							{
								lightBrightness_day = atof(pitr->second.c_str());
							}
							if (pitr->first == "night")
							{
								lightBrightness_night = atof(pitr->second.c_str());
							}
						}
					}
				}	

				if (_tritonDrawable.valid())
				{
					_tritonDrawable->setLightingBrightness(
						lightBrightness_enable,
						lightBrightness_day,
						lightBrightness_night
					);
				}
			}


			virtual void update(OpenIG::PluginBase::PluginContext& context)
			{				

				if (!_tritonDrawable.valid()) return;				

				{
					OpenIG::Engine* ig = dynamic_cast<OpenIG::Engine*>(context.getImageGenerator());
					_tritonDrawable->setIG(ig);
				}
				{
					osg::ref_ptr<osg::Referenced> ref = context.getAttribute("Fog");
					OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::FogAttributes> *attr = dynamic_cast<OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::FogAttributes> *>(ref.get());
					if (attr &&  _tritonDrawable->getEnvironment())
					{
						osg::Vec3 fogColor = attr->getValue().getFogColor();

						context.getOrCreateValueObject()->getUserValue("SilverLining-Atmosphere-HorizonColor", fogColor);

						Triton::Vector3 _fogColor(fogColor.x(), fogColor.y(), fogColor.z());

						_tritonDrawable->getEnvironment()->SetAboveWaterVisibility(attr->getValue().getVisibility(), _fogColor);

					}
				}
				{
					osg::ref_ptr<osg::Referenced> ref = context.getAttribute("Wind");
					OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::WindAttributes> *attr = dynamic_cast<OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::WindAttributes> *>(ref.get());
					if (attr &&  _tritonDrawable->getEnvironment())
					{
						Triton::WindFetch wf;
						wf.SetWind(attr->getValue().speed, ((attr->getValue().direction)*RAD_PER_DEG));
						_tritonDrawable->getEnvironment()->ClearWindFetches();
						_tritonDrawable->getEnvironment()->AddWindFetch(wf);

					}
				}

				{
					osg::ref_ptr<osg::Referenced> ref = context.getAttribute("TOD");
					OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::TimeOfDayAttributes> *attr = dynamic_cast<OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::TimeOfDayAttributes> *>(ref.get());
					if (attr)
					{
						_tritonDrawable->setTOD(attr->getValue().getHour());
					}

				}

				_tritonDrawable->setBelowWaterVisibiliy(_belowWaterVisibility);
		
				// Here we check if the XML has changed. If so, reload and update
				_xmlAccessMutex.lock();
				if (_xmlLastCheckedTime != _xmlLastWriteTime)
				{
					osg::notify(osg::NOTICE) << "SilverLining: XML updated: " << _xmlFileName << std::endl;
					readXML(_xmlFileName);
					_xmlLastCheckedTime = _xmlLastWriteTime;
				}
				_xmlAccessMutex.unlock();

			}

			virtual void clean(OpenIG::PluginBase::PluginContext& context)
			{
				if (_xmlFileObserverThread.get())
				{
					_xmlThreadRunningCondition = false;
					while (_xmlThreadIsRunning);
					_xmlFileObserverThread->join();
				}
				_xmlFileObserverThread.reset();

				if (_tritonDrawable)
				{
					_tritonDrawable->cleanup();
				}
			}

			virtual void entityAdded(OpenIG::PluginBase::PluginContext& context, unsigned int entityID, osg::Node& node, const std::string& fileName)
			{
				if (entityID == 0)
				{
					osg::Group* group = new osg::Group;
					group->addChild(&node);

					if (_heightMapEnabled)
					{
						context.getImageGenerator()->getViewer()->getView(0)->getSceneData()->asGroup()->addChild(
							createHeightMapCamera(context.getImageGenerator()->getViewer()->getView(0)->getCamera(), group));

						_tritonDrawable->setHeightMapCamera(_heightMapcamera.get());
						_tritonDrawable->setHeightMap(_heightMap.get());
						if (_heightMapHUDEnabled)
							context.getImageGenerator()->getViewer()->getView(0)->getSceneData()->asGroup()->addChild(createHUDForHeightMapDisplay());
					}
				}

				if (!_reflectedSubGraph.valid())
				{
					osg::Node * worldMirrored = createMirroredWorldGraph(osg::Matrixd::identity());

					PlanarReflection* planarReflectionCamera = new PlanarReflection(context.getImageGenerator()->getViewer()->getView(0)->getCamera());
#if 0
					context.getImageGenerator()->getScene()->asGroup()->addChild(planarReflectionCamera);
#else
					context.getImageGenerator()->getViewer()->getView(0)->getSceneData()->asGroup()->addChild(planarReflectionCamera);
#endif
					planarReflectionCamera->addChild(worldMirrored);

					_tritonDrawable->setPlanarReflection(planarReflectionCamera->getTexture(), planarReflectionCamera->getTextureProjectionMatrix());
				}

				if (!osgDB::fileExists(fileName + ".xml"))
				{
					return;
				}

				osg::notify(osg::NOTICE) << "Triton: Parsing XML for " << fileName << std::endl;

				osgDB::XmlNode* root = osgDB::readXmlFile(fileName + ".xml");
				if (!root) return;
				if (!root->children.size()) return;

				osgDB::XmlNode::Children::iterator itr = root->children.at(0)->children.begin();
				for (; itr != root->children.at(0)->children.end(); ++itr)
				{
					if ((**itr).name == "Reflection")
					{
						_reflectedGraphEntities[entityID] = (**itr).contents == "yes";
						//std::cout << "Entity reflection " << entityID << "," << _reflectedGraphEntities[entityID] << std::endl;
					}
				}

				setEntityReflection(entityID, _reflectedGraphEntities[entityID]);

#if 0
				switch (_reflectedGraphEntities[entityID])
				{
				case true:
					_reflectedSubGraph->addChild(&node);
					break;
				case false:
					_reflectedSubGraph->removeChild(&node);
					break;
				}
#endif
			}

			// Thread function to check the
			// XML config file if changed
			void xmlFileObserverThread()
			{
				_xmlThreadRunningCondition = true;
				_xmlThreadIsRunning = true;

				while (_xmlThreadRunningCondition)
				{
					try
					{
						_xmlAccessMutex.lock();
						_xmlLastWriteTime = OpenIG::Base::FileSystem::lastWriteTime(_xmlFileName);
						_xmlAccessMutex.unlock();
					}
					catch (const std::exception& e)
					{
						osg::notify(osg::NOTICE) << "SilverLining: File montoring exception: " << e.what() << std::endl;
						break;
					}

					OpenThreads::Thread::microSleep(10000);
				}
				_xmlThreadIsRunning = false;
			}

			virtual void databaseRead(const std::string& fileName, osg::Node*, const osgDB::Options*)
			{
				std::string xmlFile = fileName + ".lighting.xml";
				if (!osgDB::fileExists(xmlFile)) return;

				_xmlFileObserverThread = boost::shared_ptr<boost::thread>(new boost::thread(&OpenIG::Plugins::TritonPlugin::xmlFileObserverThread, this));


				readXML(_xmlFileName = xmlFile);
			}


		protected:
			std::string                     _userName;
			std::string                     _key;
			std::string                     _path;
			bool							_geocentric;
			bool							_forwardPlusEnabled;
			osg::ref_ptr<osg::Camera>		_heightMapcamera;
			osg::ref_ptr<osg::Texture2D>	_heightMap, _depthMap;
			bool							_heightMapEnabled;
			bool                            _heightMapHUDEnabled;
			osg::ref_ptr<TritonDrawable>	_tritonDrawable;
			double                          _belowWaterVisibility;
			bool							_environmentalMapping;
			osg::ref_ptr<osg::Group>		_reflectedSubGraph;
			OpenIG::Base::ImageGenerator*			_ig;
			float							_planarReflectionBlend;

			typedef std::map<unsigned int, bool >			ReflectedGraphEntitiesMap;
			ReflectedGraphEntitiesMap		_reflectedGraphEntities;

			boost::shared_ptr<boost::thread>	_xmlFileObserverThread;
			volatile bool						_xmlThreadIsRunning;
			volatile bool						_xmlThreadRunningCondition;
			boost::mutex						_xmlAccessMutex;
			std::time_t							_xmlLastWriteTime;
			std::time_t							_xmlLastCheckedTime;
			std::string							_xmlFileName;

			osg::Node* createMirroredWorldGraph(const osg::Matrix & localToWorld)
			{
				osg::ref_ptr< osg::MatrixTransform > world = new osg::MatrixTransform;
				world->setMatrix(localToWorld);

				// Add transform inverting height coordinates
				osg::ref_ptr< osg::MatrixTransform > mirror = new osg::MatrixTransform;
				world->addChild(mirror);
				mirror->setMatrix(osg::Matrix::scale(1, 1, -1));
				// Make sure fron faces will be not culled out with changed orientation
				mirror->getOrCreateStateSet()->setAttributeAndModes(new osg::FrontFace(osg::FrontFace::CLOCKWISE));

				// Add clip node to cut off parts of the scene below the water surface
				osg::ref_ptr< osg::ClipNode > clipNode = new osg::ClipNode();
				// Set clip plane to cut off all whats below the water surface
				clipNode->addClipPlane(new osg::ClipPlane(0, osg::Plane(0, 0, 1, 0)));
				mirror->addChild(clipNode);

				_reflectedSubGraph = new osg::Group;

				osg::Shader* mainVertexShader = new osg::Shader(osg::Shader::VERTEX,
					"#version 120                                                           \n"
					"#pragma import_defines(SIMPLELIGHTING,SHADOWING,ENVIRONMENTAL,AO)      \n"
					"varying vec3 normal;                                                   \n"
					"varying vec3 vertex_light_position;									\n"
					"void main()															\n"
					"{																		\n"
					"   gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;				\n"
					"   normal = normalize( gl_NormalMatrix * gl_Normal );					\n"
					"   gl_TexCoord[0] = gl_TextureMatrix[0] *gl_MultiTexCoord0;			\n"
					"	vertex_light_position = normalize(gl_LightSource[0].position.xyz);	\n"
					"}																		\n");

				osg::Shader* mainFragmentShader = new osg::Shader(osg::Shader::FRAGMENT,
					"#version 120                                                           \n"
					"varying vec3 normal;                                                   \n"
					"varying vec3 vertex_light_position;									\n"
					"uniform sampler2D baseTexture;                                         \n"
					"void computeAmbientColor(inout vec4 color)                             \n"
					"{                                                                      \n"
					"	vec4 ambient_color = gl_LightSource[0].ambient + gl_LightModel.ambient; \n"
					"	vec4 diffuse_color = gl_LightSource[0].diffuse; \n"
					"	float diffuse_value = max(dot(normal, vertex_light_position), 0.0);	\n"
					"	color += ambient_color * 0.5 + diffuse_color * diffuse_value;		\n"
					"}                                                                      \n"
					"                                                                       \n"
					"void lighting( inout vec4 color )                                      \n"
					"{                                                                      \n"
					"	vec4 clr = vec4(0.0);                                               \n"
					"                                                                       \n"
					"	computeAmbientColor(clr);                                           \n"
					"	color *= clr;                                                       \n"
					"}																		\n"
					"void main()                                                            \n"
					"{                                                                      \n"
					"   vec4 color = vec4(0.0,0.0,0.0,1.0);									\n"
					"   color = texture2D( baseTexture, gl_TexCoord[0].xy );				\n"
					"	lighting(color);													\n"
					"   gl_FragColor =  color;												\n"
					"}                                                                      \n");

				osg::ref_ptr<osg::Program> pointProgram = new osg::Program;
				pointProgram->addShader(mainVertexShader);
				pointProgram->addShader(mainFragmentShader);
				_reflectedSubGraph->getOrCreateStateSet()->setAttributeAndModes(pointProgram, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);

				// Finally add the model
				clipNode->addChild(_reflectedSubGraph);

				return world.release();
			}


			void initTriton(OpenIG::Base::ImageGenerator* ig)
			{
				osg::ref_ptr<osg::LOD>		lod = new osg::LOD;
				osg::ref_ptr<osg::Geode>	tritonGeode = new osg::Geode;

				lod->insertChild(0, tritonGeode);
				if (_geocentric)
					lod->setRange(0, 0, osg::WGS_84_RADIUS_EQUATOR*1.001);
				else
					lod->setRange(0, 0, 50000);

				_tritonDrawable = new TritonDrawable(_path, _userName, _key, _geocentric, _forwardPlusEnabled);

				tritonGeode->addDrawable(_tritonDrawable);
				ig->getViewer()->getView(0)->getSceneData()->asGroup()->addChild(lod);

				_tritonDrawable->getOrCreateStateSet()->setRenderBinDetails(OCEAN_RENDER_BIN, "RenderBin");
				//_tritonDrawable->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

				_tritonDrawable->setEnvironmentalMapping(_environmentalMapping);
				_tritonDrawable->setPlanarReflectionBlend(_planarReflectionBlend);
			}
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
    return new OpenIG::Plugins::TritonPlugin;
}

extern "C" EXPORT void DeletePlugin(OpenIG::PluginBase::Plugin* plugin)
{
    osg::ref_ptr<OpenIG::PluginBase::Plugin> p(plugin);
}
