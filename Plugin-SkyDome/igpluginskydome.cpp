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
#include <Core-PluginBase/plugin.h>
#include <Core-PluginBase/plugincontext.h>

#include <Core-Base/imagegenerator.h>
#include <Core-Base/attributes.h>
#include <Core-Base/mathematics.h>
#include <Core-Base/commands.h>

#include <iostream>
#include <sstream>

#include <osg/ref_ptr>
#include <osg/FrontFace>
#include <osg/Material>
#include <osg/ClearNode>
#include <osg/Depth>

#include <osgDB/XmlParser>
#include <osgDB/ReadFile>

#include <osgViewer/CompositeViewer>

#include <osgParticle/PrecipitationEffect>

#include <Core-OpenIG/renderbins.h>

namespace OpenIG {
	namespace Plugins {

		class SkyDomePlugin : public OpenIG::PluginBase::Plugin
		{
		public:

			SkyDomePlugin() {}

			virtual std::string getName() { return "SkyDome"; }

			virtual std::string getDescription() { return "Simple skydome implementation"; }

			virtual std::string getVersion() { return "1.0.0"; }

			virtual std::string getAuthor() { return "ComPro, Nick - not really, the code was taken from the old MUSE Visual System"; }

			virtual void config(const std::string& fileName)
			{
				osgDB::XmlNode* root = osgDB::readXmlFile(fileName);
				if (root == 0)
				{
					osg::notify(osg::NOTICE) << "SkyDome: failed to read XML file: " << fileName << std::endl;
					return;
				}

				if (root->children.size() == 0)
				{
					osg::notify(osg::NOTICE) << "SkyDome: empty XML file: " << fileName << std::endl;
					return;
				}

				osgDB::XmlNode* config = root->children.at(0);
				if (config->name != "OpenIG-Plugin-Config")
				{
					osg::notify(osg::NOTICE) << "SkyDome: <OpenIG-Plugin-Config> tag missing in " << fileName << std::endl;
					return;
				}

				osgDB::XmlNode::Children::iterator itr = config->children.begin();
				for (; itr != config->children.end(); ++itr)
				{
					osgDB::XmlNode* child = *itr;
					if (child->name == "SkyDome-Model")
					{
						_modelFileName = child->contents;
					}
				}
			}

		protected:
			std::string     _modelFileName;

			virtual void clean(OpenIG::PluginBase::PluginContext& context)
			{
				OpenIG::Base::Commands::instance()->removeCommand("fog");
				OpenIG::Base::Commands::instance()->removeCommand("rain");
				OpenIG::Base::Commands::instance()->removeCommand("snow");

				if (context.getImageGenerator()->getScene() && context.getImageGenerator()->getScene()->asGroup())
				{
					context.getImageGenerator()->getScene()->asGroup()->removeChild(_sky);
					_sky = NULL;

					context.getImageGenerator()->getViewer()->getView(0)->getSceneData()->asGroup()->removeChild(_precipitation);
					_precipitation = NULL;
				}
			}


			virtual void init(OpenIG::PluginBase::PluginContext& context)
			{

				OpenIG::Base::Commands::instance()->addCommand("fog", new SetFogCommand(context.getImageGenerator()));
				OpenIG::Base::Commands::instance()->addCommand("rain", new RainCommand(context.getImageGenerator()));
				OpenIG::Base::Commands::instance()->addCommand("snow", new SnowCommand(context.getImageGenerator()));
#if 1
				osg::ref_ptr<osgViewer::CompositeViewer> viewer = context.getImageGenerator()->getViewer();
				if (!viewer.valid()) return;
				if (!viewer->getNumViews()) return;
				if (!viewer->getView(0)->getSceneData()) return;

				osg::Node* sky = createSkyDome(context.getImageGenerator());
				if (sky)
				{
					osg::Shader* mainVertexShader = new osg::Shader(osg::Shader::VERTEX,
						"#version 120                                                       \n"
						"varying vec3 eyeVec;                                               \n"
#ifdef FRAGMENT_LEVEL_DEPTH
						"varying float flogz;                                               \n"
#endif
						"                                                                   \n"
						"uniform float Fcoef;							    	     		\n"

						"void main()                                                        \n"
						"{                                                                  \n"
						"   gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;         \n"
						"   if (Fcoef > 0) {                                                \n"
						"       gl_Position.z = (log2(max(1e-6, 1.0 + gl_Position.w)) * Fcoef - 1.0) * gl_Position.w;\n"
#ifdef FRAGMENT_LEVEL_DEPTH
						"       flogz = 1.0 + gl_Position.w;						        \n"
#endif
						"   }                                                               \n"
						"   gl_TexCoord[0] = gl_TextureMatrix[0] *gl_MultiTexCoord0;        \n"
						"                                                                   \n"
						"   eyeVec = -vec3(gl_ModelViewMatrix * gl_Vertex);                 \n"
						"                                                                   \n"
						"}                                                                  \n"
						);

					osg::Shader* mainFragmentShader = new osg::Shader(osg::Shader::FRAGMENT,
						"#version 120                                                           \n"
						"#extension GL_ARB_texture_rectangle : enable                           \n"
						"varying vec3 eyeVec;                                                   \n"
						"uniform sampler2D baseTexture;                                         \n"
#ifdef FRAGMENT_LEVEL_DEPTH
						"varying float flogz;                                                   \n"
						"uniform float Fcoef;                                                   \n"
#endif
						"void computeFogColor(inout vec4 color)                                 \n"
						"{                                                                      \n"
						"   float fogExp = gl_Fog.density * length(eyeVec);                     \n"
						"   float fogFactor = exp(-(fogExp * fogExp));                          \n"
						"   fogFactor = clamp(fogFactor, 0.0, 1.0);                             \n"
						"   vec4 clr = color;                                                   \n"
						"   color = mix(gl_Fog.color, color, fogFactor);                        \n"
						"   color.a = clr.a;                                                    \n"
						"}                                                                      \n"
						"void main()                                                            \n"
						"{                                                                      \n"
						"   vec4 color = texture2D( baseTexture, gl_TexCoord[0].xy );           \n"
						"	computeFogColor(color);                                             \n"
						"   gl_FragColor = color * gl_LightSource[0].diffuse;                   \n"
#ifdef FRAGMENT_LEVEL_DEPTH
						"   gl_FragDepth = log2(flogz) * Fcoef * 0.5;                           \n"
#endif
						"}                                                                      \n"
						);

					osg::Program* program = new osg::Program;
					program->addShader(mainVertexShader);
					program->addShader(mainFragmentShader);

					osg::StateSet* ss = sky->getOrCreateStateSet();
					ss->setAttributeAndModes(program, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
					ss->addUniform(new osg::Uniform("baseTexture", 0));
					ss->setRenderBinDetails(SKY_RENDER_BIN, "RenderBin");
					ss->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0.0, 1.0, false));

					context.getImageGenerator()->getScene()->asGroup()->addChild(sky);
				}
#else
				context.getImageGenerator()->getScene()->asGroup()->addChild(createSkyDome(context.getImageGenerator()));
#endif
			}

			virtual void update(OpenIG::PluginBase::PluginContext& context)
			{
				osgViewer::CompositeViewer* viewer = context.getImageGenerator()->getViewer();
				if (!viewer) return;
				if (viewer->getNumViews() == 0) return;

				osg::ref_ptr<osg::Camera> camera = viewer->getView(0)->getCamera();

				osg::Vec3d eye = camera->getInverseViewMatrix().getTrans();
				double radius = osg::minimum((double)context.getImageGenerator()->getScene()->getBound().radius(), 92600.0);
				radius = osg::maximum(radius, 92600.0);
				osg::Vec3d scale(radius, radius, radius);

				if (_sky.valid())
				{
					_sky->setEye(eye);
					_sky->setScale(scale);

					{
						osg::ref_ptr<osg::Referenced> ref = context.getAttribute("TOD");
						OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::TimeOfDayAttributes> *attr = dynamic_cast<OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::TimeOfDayAttributes> *>(ref.get());
						if (attr)
						{
							_sky->setSunPosition((float)attr->getValue().getHour());

							if (!_precipitation.valid())
							{
								_precipitation = new osgParticle::PrecipitationEffect;
								context.getImageGenerator()->getViewer()->getView(0)->getSceneData()->asGroup()->addChild(_precipitation);

								_precipitation->snow(0);
								_precipitation->rain(0);

								_precipitation->setFog(context.getImageGenerator()->getFog());
							}

							_precipitation->getFog()->setColor(_sky->getSun()->getDiffuse());

						}
					}
			{
				osg::ref_ptr<osg::Referenced> ref = context.getAttribute("Rain");
				OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::RainSnowAttributes> *attr = dynamic_cast<OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::RainSnowAttributes> *>(ref.get());
				if (attr)
				{
					if (!_precipitation.valid())
					{
						_precipitation = new osgParticle::PrecipitationEffect;
						context.getImageGenerator()->getViewer()->getView(0)->getSceneData()->asGroup()->addChild(_precipitation);

						_precipitation->snow(0);
						_precipitation->rain(0);
					}

					_precipitation->rain(attr->getValue().getFactor());

				}

				{
					osg::ref_ptr<osg::Referenced> ref = context.getAttribute("Fog");
					OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::FogAttributes> *attr = dynamic_cast<OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::FogAttributes> *>(ref.get());
					if (attr)
					{
						if (!_precipitation.valid())
						{
							_precipitation = new osgParticle::PrecipitationEffect;
							context.getImageGenerator()->getViewer()->getView(0)->getSceneData()->asGroup()->addChild(_precipitation);

							_precipitation->snow(0);
							_precipitation->rain(0);

							_precipitation->setFog(context.getImageGenerator()->getFog());
						}

						_precipitation->getFog()->setColor(_sky->getSun()->getDiffuse());
					}
				}

				{
					osg::ref_ptr<osg::Referenced> ref = context.getAttribute("Wind");
					OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::WindAttributes> *attr = dynamic_cast<OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::WindAttributes> *>(ref.get());
					if (attr)
					{
						if (!_precipitation.valid())
						{
							_precipitation = new osgParticle::PrecipitationEffect;
							context.getImageGenerator()->getViewer()->getView(0)->getSceneData()->asGroup()->addChild(_precipitation);

							_precipitation->snow(0);
							_precipitation->rain(0);

							_precipitation->setFog(context.getImageGenerator()->getFog());
						}

						float direction = attr->getValue().direction + 90.0;
						float speed = attr->getValue().speed;

						osg::Matrixd mx = OpenIG::Base::Math::instance()->toMatrix(0, 0, 0, direction, 0, 0);
						osg::Quat q = mx.getRotate();

						osg::Vec3 v(0, 1, 0);
						v = q * v;
						v.normalize();

						v *= speed;

						_precipitation->setWind(v);
					}
				}
			}
			{
				osg::ref_ptr<osg::Referenced> ref = context.getAttribute("Snow");
				OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::RainSnowAttributes> *attr = dynamic_cast<OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::RainSnowAttributes> *>(ref.get());
				if (attr)
				{
					if (!_precipitation.valid())
					{
						_precipitation = new osgParticle::PrecipitationEffect;
						context.getImageGenerator()->getViewer()->getView(0)->getSceneData()->asGroup()->addChild(_precipitation);

						_precipitation->snow(0);
						_precipitation->rain(0);

						_precipitation->setFog(context.getImageGenerator()->getFog());
					}

					_precipitation->snow(attr->getValue().getFactor());


				}
			}
				}
			}

			// Code taken from muse/libs/OtwOsg
			// slightly modified to fit into OpenIg framework
			class Sky : public osg::Group
			{
			public:
				Sky(OpenIG::Base::ImageGenerator* ig, const std::string& fileName, float ceiling = 18520.0f, float visibility = 92600)
					: _ig(ig)
				{
					sun_ = ig->getSunOrMoonLight()->getLight();

					osg::ref_ptr<osg::StateSet> skystate = new osg::StateSet;
					skystate->setGlobalDefaults();
					setStateSet(skystate);
					skystate->setMode(GL_NORMALIZE, osg::StateAttribute::ON);
					//skystate->setAttributeAndModes(new osg::FrontFace(osg::FrontFace::COUNTER_CLOCKWISE), osg::StateAttribute::ON);

					_dome = osgDB::readNodeFile(fileName);

					if (!_dome.valid())
					{
						osg::notify(osg::NOTICE) << "SkyDome: failed to load sky model : " << fileName << std::endl;
					}
#if 0
					_domeScale = new osg::MatrixTransform;
					_domeScale->setMatrix( osg::Matrix::scale( visibility, visibility, ceiling ) );
					//_domeScale->addChild( _dome );

					_domeTranslate = new osg::MatrixTransform;
					_domeTranslate->addChild( _dome );

					_domeScale->addChild(_domeTranslate);

					addChild( _domeScale );
#else
					_domeMxt = new osg::MatrixTransform;
					_domeMxt->setMatrix(osg::Matrix::scale(visibility, visibility, ceiling));
					_domeMxt->addChild(_dome);

					addChild(_domeMxt);
#endif

#if 1
					//osg::Material* material = dynamic_cast<osg::Material*>(skystate->getAttribute(osg::StateAttribute::MATERIAL));

					//if (!material)
					//{
					osg::Material* material = new osg::Material;
					skystate->setAttribute(material, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);
					//}
					//    _domeScale->setMatrix( osg::Matrix::scale( VISIBILITY_MAX, VISIBILITY_MAX, VISIBILITY_MAX/5.0 ) );
					//material->setColorMode( osg::Material::SPECULAR);
					material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0, 1.0, 1.0, 1.0));
					material->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0, 1.0, 1.0, 1.0));
					material->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0.0, 0.0, 0.0, 1.0));
					material->setShininess(osg::Material::FRONT_AND_BACK, 10.0);

					skystate->setAttributeAndModes(new osg::Program, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
#endif
					sky_light = _ig->getSunOrMoonLight()->getLight();
					//sky_light->setLightNum(3);
					sky_light->setAmbient(osg::Vec4(0.0, 0.0, 0.0, 1.0));
					sky_light->setSpecular(osg::Vec4(1.0, 1.0, 1.0, 1.0));
					sky_light->setDiffuse(osg::Vec4(1.0, 1.0, 1.0, 1.0));
					//sky_light->setPosition( osg::Vec4( 1.0,1.0,1.0,1.0 ) );
#if 0
					osg::LightSource *lightS1 = _ig->getSunOrMoonLight();//new osg::LightSource;
					//lightS1->setLight(sky_light);
					lightS1->setLocalStateSetModes(osg::StateAttribute::ON);
					lightS1->setStateSetModes(*skystate, osg::StateAttribute::ON);
					addChild(lightS1);
#endif
				}


				void setEye(osg::Vec3 eye)
				{
					//_domeTranslate->setMatrix( osg::Matrix::translate( eye.x(), eye.y(), 0.0 ) );

					osg::Vec3d s = _domeMxt->getMatrix().getScale();

					_domeMxt->setMatrix(osg::Matrixd::scale(s)*osg::Matrixd::translate(eye));
				}

				void setScale(osg::Vec3 scale)
				{
					//_domeScale->setMatrix( osg::Matrix::scale( scale ) );

					osg::Vec3d t = _domeMxt->getMatrix().getTrans();

					_domeMxt->setMatrix(osg::Matrixd::scale(scale) * osg::Matrixd::translate(t));
				}

				void setSun()
				{
					setSunPosition(12.0f);
				}

				void setSunPosition(float hours)
				{
					float intFactor;
					float angle;
					osg::Vec4                     sunAmbient_;
					osg::Vec4                     sunDiffuse_;
					osg::Vec4                     sunPosition_;

					float timeOfDay = hours;

					angle = timeOfDay * 2.0 * osg::PI / 24.0;

					if (hours <= 6.0 || hours >= 18.0)
					{
						intFactor = 0.0;
					}
					else
					{
						if (hours < 12.0)
						{
							intFactor = sqrt(hours / 6.0 - 1.0)*1.7;
						}
						else
						{
							intFactor = sqrt(-hours / 6.0 + 3.0)*1.7;
						}
					}

					if (intFactor > 1.0) intFactor = 1.0;

					float colorFactor;

					if (intFactor > 0.32)
						colorFactor = 1.0;
					else
						colorFactor = 1.0 + (0.32 - intFactor) * 50.0f;//_visibility;


					//Cosmetic changes
					if (hours >= 14.0f && 22.0f >= hours)
					{
						if (hours == 14.0f)
						{
							sunAmbient_.set(0.6, 0.6, 0.6, 1.0);
							sunDiffuse_.set(0.6, 0.6, 0.6, 1.0);

						}
						else if (hours == 15.0f)
						{
							sunAmbient_.set(0.4, 0.4, 0.2, 1.0);
							sunDiffuse_.set(0.4, 0.4, 0.3, 1.0);

						}
						else if (hours == 16.0f)
						{
							sunAmbient_.set(0.2, 0.2, 0.105, 1.0);
							sunDiffuse_.set(0.2, 0.2, 0.105, 1.0);

						}
						else if (hours == 17.0f)
						{
							sunAmbient_.set(0.1, 0.1, 0.05, 1.0);
							sunDiffuse_.set(0.1, 0.1, 0.05, 1.0);


							sky_light->setDiffuse(osg::Vec4(0.0, 0.0, 0.45, 1.0));
							sky_light->setAmbient(osg::Vec4(1.0, 0.54, 0.0, 1.0));
						}
						else if (hours >= 18.0f && hours <= 20.0f)
						{
							sunAmbient_.set(0.1, 0.1, 0.05, 1.0);
							sunDiffuse_.set(0.1, 0.1, 0.05, 1.0);


							sky_light->setDiffuse(osg::Vec4(1.0, 0.54, 0.0, 1.0));
							sky_light->setAmbient(osg::Vec4(1.0, 0.54, 0.0, 1.0));
						}
						else if (hours >= 21.0f && hours <= 22.0f)
						{
							sunAmbient_.set(0.03, 0.03, 0.03, 1.0);
							sunDiffuse_.set(0.03, 0.03, 0.03, 1.0);


							sky_light->setDiffuse(osg::Vec4(0.0, 0.0, 0.45, 1.0));
							sky_light->setAmbient(osg::Vec4(0.0, 0.0, 0.45, 1.0));
						}

						if (hours >= 14.0 && hours <= 16.0f)
						{
							sky_light->setDiffuse(osg::Vec4(0.6*intFactor, 0.6*intFactor, 0.6*intFactor, 1.0));
							sky_light->setAmbient(osg::Vec4(0.9*intFactor*colorFactor, 0.9*intFactor, 0.8*intFactor, 1.0));

							sunAmbient_.set(0.6*intFactor, 0.6*intFactor, 0.6*intFactor, 1.0);
							sunDiffuse_.set(0.9*intFactor*colorFactor, 0.9*intFactor, 0.8*intFactor, 1.0);
						}

						sun_->setAmbient(sunAmbient_);
						sun_->setDiffuse(sunDiffuse_);

					}
					else if (hours == 06.0f)
					{
						sky_light->setDiffuse(osg::Vec4(1.0, 0.54, 0.0, 1.0));
						sky_light->setAmbient(osg::Vec4(1.0, 0.54, 0.0, 1.0));

						sunAmbient_.set(0.1, 0.1, 0.05, 1.0);
						sunDiffuse_.set(0.1, 0.1, 0.05, 1.0);
					}
					else if (hours >= 23.0f && hours <= 05.0f)
					{
						sky_light->setDiffuse(osg::Vec4(0.0, 0.0, 0.0, 1.0));
						sky_light->setAmbient(osg::Vec4(0.0, 0.0, 0.0, 1.0));

						sunAmbient_.set(0.0, 0.0, 0.0, 1.0);
						sunDiffuse_.set(0.0, 0.0, 0.0, 1.0);

					}
					else
					{
						sunAmbient_.set(0.6*intFactor, 0.6*intFactor, 0.6*intFactor, 1.0);
						sunDiffuse_.set(0.9*intFactor*colorFactor, 0.9*intFactor, 0.8*intFactor, 1.0);


						sky_light->setDiffuse(osg::Vec4(0.6*intFactor, 0.6*intFactor, 0.6*intFactor, 1.0));
						sky_light->setAmbient(osg::Vec4(0.9*intFactor*colorFactor, 0.9*intFactor, 0.8*intFactor, 1.0));
					}

					sun_->setAmbient(sunAmbient_);
					sun_->setDiffuse(sunDiffuse_);


					if (hours >= 19.0f)
						sunPosition_.set(sin(angle), 0.0, -cos(angle), 0.0);
					else //Black Horizon Fix
						sunPosition_.set(sin(angle)*0.1, 0.0, -cos(angle), 0.0);
					sun_->setPosition(sunPosition_);

					sunPosition_.z() *= -1.0;
					setTOD(sunPosition_);
				}

				void setTOD(osg::Vec4 sunpos)
				{
					sunpos.normalize();
					sky_light->setPosition(osg::Vec4(-sunpos.x(), -sunpos.y(), -sunpos.z(), 0.0));
					//sky_light->setPosition( osg::Vec4(0.0,0.0,-1.0, 0.0) );
				}

				osg::ref_ptr<osg::Light> getSkyLight(void)    { return sky_light; }
				osg::ref_ptr<osg::Light> getSun(void)         { return sun_; }

			private:
				osg::ref_ptr<osg::Light>            sky_light;
				osg::ref_ptr<osg::Light>            sun_;
				osg::ref_ptr<osg::MatrixTransform> _domeScale;
				osg::ref_ptr<osg::MatrixTransform> _domeTranslate;
				osg::ref_ptr<osg::Node>             _dome;
				OpenIG::Base::ImageGenerator*             _ig;
				osg::ref_ptr<osg::MatrixTransform>	_domeMxt;

			};

			osg::Node* createSkyDome(OpenIG::Base::ImageGenerator* ig)
			{
				_sky = new Sky(ig, _modelFileName);
				//_sky->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

				//osg::ClearNode* cn = new osg::ClearNode();
				//cn->addChild(_sky);
				return _sky;;
			}

			osg::ref_ptr<Sky>                               _sky;
			osg::ref_ptr<osgParticle::PrecipitationEffect>  _precipitation;

			class SetFogCommand : public OpenIG::Base::Commands::Command
			{
			public:
				SetFogCommand(OpenIG::Base::ImageGenerator* ig)
					: _ig(ig) {}

				virtual const std::string getUsage() const
				{
					return "visibility";
				}

				virtual const std::string getArgumentsFormat() const
				{
					return "D";
				}

				virtual const std::string getDescription() const
				{
					return  "sets the visibility of the scene by using fog\n"
						"     visibility - in meteres, the distance of the fog";
				}

				virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
				{
					if (tokens.size() == 1)
					{
						double visibility = atof(tokens.at(0).c_str());

						_ig->setFog(visibility);

						return 0;
					}
					return -1;
				}

			protected:
				OpenIG::Base::ImageGenerator* _ig;
			};

			class RainCommand : public OpenIG::Base::Commands::Command
			{
			public:
				RainCommand(OpenIG::Base::ImageGenerator* ig)
					: _ig(ig) {}

				virtual const std::string getUsage() const
				{
					return "rainfactor";
				}

				virtual const std::string getArgumentsFormat() const
				{
					return "D";
				}

				virtual const std::string getDescription() const
				{
					return  "adds rain to the scene\n"
						"     rainfactor - from 0.0-1.0, 0 no rain, 1 heavy rain";
				}

				virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
				{
					if (tokens.size() == 1)
					{
						double factor = atof(tokens.at(0).c_str());

						_ig->setRain(factor);

						return 0;
					}

					return -1;
				}
			protected:
				OpenIG::Base::ImageGenerator* _ig;
			};

			class SnowCommand : public OpenIG::Base::Commands::Command
			{
			public:
				SnowCommand(OpenIG::Base::ImageGenerator* ig)
					: _ig(ig) {}

				virtual const std::string getUsage() const
				{
					return "snowfactor";
				}

				virtual const std::string getArgumentsFormat() const
				{
					return "D";
				}

				virtual const std::string getDescription() const
				{
					return  "adds snow to the scene\n"
						"     snowfactor - from 0.0-1.0. 0 no snow, 1 heavy snow";
				}

				virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
				{
					if (tokens.size() == 1)
					{
						double factor = atof(tokens.at(0).c_str());

						_ig->setSnow(factor);

						return 0;
					}

					return -1;
				}
			protected:
				OpenIG::Base::ImageGenerator* _ig;
			};
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
    return new OpenIG::Plugins::SkyDomePlugin;
}

extern "C" EXPORT void DeletePlugin(OpenIG::PluginBase::Plugin* plugin)
{
    osg::ref_ptr<OpenIG::PluginBase::Plugin> p(plugin);
}
