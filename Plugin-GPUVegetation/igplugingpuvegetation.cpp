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
#include <Core-Base/stringutils.h>
#include <Core-Base/configuration.h>
#include <Core-Base/commands.h>
#include <Core-Base/filesystem.h>

#include <osg/ref_ptr>
#include <osg/StateSet>
#include <osg/Texture2D>
#include <osg/Group>
#include <osg/Geometry>
#include <osg/Geode>

#include <osgDB/XmlParser>
#include <osgDB/ReadFile>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include <osgViewer/CompositeViewer>

#include <osgShadow/ShadowedScene>
#include <osgShadow/MinimalShadowMap>

namespace OpenIG {
	namespace Plugins {

		class GPUVegetationPlugin : public OpenIG::PluginBase::Plugin
		{
		public:

			GPUVegetationPlugin() {}

			virtual std::string getName() { return "GPUVegetation"; }

			virtual std::string getDescription() { return "Vegetation generated on the GPU"; }

			virtual std::string getVersion() { return "1.0.0"; }

			virtual std::string getAuthor() { return "ComPro, Nick"; }

			static bool useLogZDepthBuffer(void)
			{
				std::string strLogZDepthBuffer = OpenIG::Base::Configuration::instance()->getConfig("LogZDepthBuffer","yes");
				if (strLogZDepthBuffer.compare(0, 3, "yes") == 0)
					return true;
				else
					return false;
			}

			void setUpShaders(osg::StateSet* ss)
			{
				if (!_gpuProgram.valid()) _gpuProgram = new osg::Program;
				osg::ref_ptr<osg::Program> program = _gpuProgram;

				program->setName("gpu_veg_program");

				std::string resourcesPath = OpenIG::Base::FileSystem::path(OpenIG::Base::FileSystem::Resources, "../resources");
				std::string strSource;

				useLogZDepthBuffer();

				strSource = OpenIG::Base::FileSystem::readFileIntoString(resourcesPath + "/shaders/vegetation_vs.glsl");
				osg::Shader* mainVertexShader = new osg::Shader(osg::Shader::VERTEX, strSource);

				strSource = OpenIG::Base::FileSystem::readFileIntoString(resourcesPath + "/shaders/vegetation_gs.glsl");
				osg::Shader* mainGeometryShader = new osg::Shader(osg::Shader::GEOMETRY, strSource);

				// A fair metric for vegetation
				std::string strMAX_LIGHTS_PER_PIXEL = (_forwardPlusEnabled)? "#define MAX_LIGHTS_PER_PIXEL 200\n"
					: "#define MAX_LIGHTS_PER_PIXEL 200\n";

				strSource = OpenIG::Base::FileSystem::readFileIntoString(resourcesPath + "/shaders/vegetation_ps.glsl")
					+ strMAX_LIGHTS_PER_PIXEL
					+ OpenIG::Base::FileSystem::readFileIntoString(resourcesPath + "/shaders/lighting_math.glsl")
					+ OpenIG::Base::FileSystem::readFileIntoString(resourcesPath + "/shaders/forwardplus_math.glsl");
				osg::Shader* mainFragmentShader = new osg::Shader(osg::Shader::FRAGMENT, strSource);

				program->addShader(mainVertexShader);
				program->addShader(mainFragmentShader);
				program->addShader(mainGeometryShader);

				program->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 8);
				program->setParameter(GL_GEOMETRY_INPUT_TYPE_EXT, GL_POINTS);
				program->setParameter(GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);

				program->addBindAttribLocation("inUV", 6);
				program->addBindAttribLocation("inScale", 7);

				ss->setAttributeAndModes(program, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
				if (useLogZDepthBuffer())
				{
					ss->setDefine("USE_LOG_DEPTH_BUFFER", "1");
				}
			}

			virtual void databaseRead(const std::string& fileName, osg::Node* node, const osgDB::Options* options)
			{
				// This is a bit hacky code. We really need to
				// remember the master offset if it is provided
				// for the database pager, but also have to
				// take into account loading of other non-offseted
				// entities. Let think more about this later
				if (fileName.find("master") != std::string::npos)
				{
					_options = const_cast<osgDB::Options*>(options);
					_path = osgDB::getFilePath(fileName);

					// Find the VegetationInfo.xml here and parse it
					_xmlFileName = osgDB::getFilePath(fileName) + "/VegetationInfo.xml";
					if (!osgDB::fileExists(_xmlFileName))
					{
						osg::notify(osg::NOTICE) << "GPU Vegetation: failed to read XML file: " << _xmlFileName << std::endl;
					}
					else
					{
						readXML(_xmlFileName);
					}
				}

				osg::Vec3 offset;
				if (_options.valid())
				{
					std::string offsetStr = _options.valid() ? _options->getOptionString() : "";

					OpenIG::Base::StringUtils::Tokens tokensNewOffset = OpenIG::Base::StringUtils::instance()->tokenize(offsetStr, ",");

					if (tokensNewOffset.size() == 3)
					{
						offset.x() = atof(tokensNewOffset.at(0).c_str());
						offset.y() = atof(tokensNewOffset.at(1).c_str());
						offset.z() = atof(tokensNewOffset.at(2).c_str());
					}
				}

				std::string simpleFileName = osgDB::getSimpleFileName(fileName);

				if (node && node->asGroup() && simpleFileName.substr(0, _pattern.length()) == _pattern)
				{
					osg::ref_ptr<osg::Group> root = new osg::Group;

					bool shadowed = OpenIG::Base::Configuration::instance()->getConfig("Shadowed-GPU-Vegetation", "no") == "yes";
					if (!shadowed)
					{
						root->setNodeMask(0x4);
					}

					node->asGroup()->addChild(root);

					VegetationInfoMapIterator itr = _vegetationInfo.begin();
					for (; itr != _vegetationInfo.end(); ++itr)
					{
						std::ostringstream oss;
						oss << fileName << "." << itr->first << ".vegbin";

						osg::ref_ptr<osg::Vec3Array> vxs;
						osg::ref_ptr<osg::Vec3Array> scale;

						{
							std::string treesFileName = oss.str();
							std::ifstream file;
							file.open(treesFileName.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
							if (file.is_open())
							{
								file.seekg(0, std::ios::end);
								long length = file.tellg();
								file.seekg(0, std::ios::beg);

								if (length > 0)
								{
									vxs = new osg::Vec3Array;
									vxs->resize(length / sizeof(osg::Vec3));

									file.read((char*)vxs->getDataPointer(), length);

									osg::notify(osg::NOTICE) << "GPU Vegetation: read " << length << " vegetation bytes from " << treesFileName << std::endl;
								}
								file.close();
							}
						}
					{
						std::ostringstream oss;
						oss << fileName << "." << itr->first << ".scales.vegbin";

						std::string treesFileName = oss.str();
						std::ifstream file;
						file.open(treesFileName.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
						if (file.is_open())
						{
							file.seekg(0, std::ios::end);
							long length = file.tellg();
							file.seekg(0, std::ios::beg);

							if (length > 0)
							{
								scale = new osg::Vec3Array;
								scale->resize(length / sizeof(osg::Vec3));

								file.read((char*)scale->getDataPointer(), length);

								osg::notify(osg::NOTICE) << "GPU Vegetation: read " << length << " vegetation (scales) bytes from " << treesFileName << std::endl;
							}
							file.close();
						}
					}

					if (vxs.valid() && vxs->size())
					{
						if (_options.valid())
						{
							osg::Vec3Array::iterator itr = vxs->begin();
							for (; itr != vxs->end(); ++itr)
							{
								osg::Vec3& v = *itr;
								v += offset;
							}
						}
#if 1
						osg::LOD* lod = new osg::LOD;
						lod->setName("GPU-Vegetation");
						root->addChild(lod);

						osg::Geode* geode = new osg::Geode;
						lod->addChild(geode, 0.f, _lodRange);
#else
						osg::Geode* geode = new osg::Geode;
						root->addChild(geode);
#endif
						geode->setNodeMask(0x2);

						osg::Geometry* geometry = new osg::Geometry;
						geode->addDrawable(geometry);

						geometry->setVertexArray(vxs);
						geometry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, vxs->size()));

						if (scale.valid() && scale->size())
						{
							geometry->setVertexAttribArray(7, scale);
							geometry->setVertexAttribBinding(7, osg::Geometry::BIND_PER_VERTEX);
						}
						else
						{
							osg::ref_ptr<osg::Vec3Array> scales = new osg::Vec3Array;
							scales->push_back(osg::Vec3(1.f, 1.f, 1.f));
							geometry->setVertexAttribArray(7, scale);
							geometry->setVertexAttribBinding(7, osg::Geometry::BIND_OVERALL);
						}

						osg::ref_ptr<osg::Vec4Array> uv = new osg::Vec4Array;
						uv->push_back(itr->second._uv);
						geometry->setVertexAttribArray(6, uv);
						geometry->setVertexAttribBinding(6, osg::Geometry::BIND_OVERALL);

						osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
						normals->push_back(osg::Vec3(0, 1, 0));
						geometry->setNormalArray(normals);
						geometry->setNormalBinding(osg::Geometry::BIND_OVERALL);

						if (!itr->second._ss.valid())
						{
							osg::ref_ptr<osg::StateSet> ss = new osg::StateSet;
							_vegetationInfo[itr->first]._ss = ss;

							osg::ref_ptr<osg::Texture2D> texture = itr->second._texture;

							if (!texture.valid())
							{
								osg::notify(osg::NOTICE) << "GPU Vegetation: null texture:" << itr->second._textureName << std::endl;
							}

							ss->setTextureAttributeAndModes(0, texture.get());
							ss->setMode(GL_BLEND, osg::StateAttribute::ON);
							ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

							setUpShaders(ss);
						}

						geometry->setStateSet(itr->second._ss.get());
					}
					}
				}
			}

			virtual void init(OpenIG::PluginBase::PluginContext& context)
			{
				OpenIG::Base::Commands::instance()->addCommand("gpuveg", new GPUVegetationCommand(context.getImageGenerator(), _lodRange));
			}

		protected:
			bool										_forwardPlusEnabled;
			std::string                                 _pattern;
			std::string                                 _xmlFileName;
			double                                      _lodRange;
			osg::ref_ptr<const osgDB::Options>          _options;
			osg::ref_ptr<osg::Program>                  _gpuProgram;

			struct VegetationInfo
			{
				std::string                         _textureName;
				osg::ref_ptr<osg::Texture2D>        _texture;
				osg::Vec3                           _scale;
				double                              _height;
				double                              _width;
				osg::ref_ptr<osg::StateSet>         _ss;
				size_t                              _id;
				osg::Vec4                           _uv;

				VegetationInfo()
					: _scale(osg::Vec3(1.f, 1.f, 1.f))
					, _height(0.0)
					, _width(0.0)
					, _id(0)
					, _uv(osg::Vec4(0.f, 0.f, 1.f, 1.f))
				{

				}
			};

			typedef std::map< size_t, VegetationInfo>               VegetationInfoMap;
			typedef std::map< size_t, VegetationInfo>::iterator     VegetationInfoMapIterator;

			VegetationInfoMap   _vegetationInfo;
			size_t              _vegetationInfoId;
			std::string         _path;

			void readXML(const std::string& fileName)
			{
				osg::ref_ptr<osgDB::XmlNode> root = osgDB::readXmlFile(fileName);
				if (!root.valid())
				{
					osg::notify(osg::NOTICE) << "GPU Vegetation: Failed to read the configuration xml: " << fileName << std::endl;
					return;
				}

				osg::ref_ptr<osgDB::XmlNode> config = root->children.size() ? root->children.at(0) : 0;
				if (!config.valid())
				{
					osg::notify(osg::NOTICE) << "GPU Vegetation: Expecting <OpenIG-Vegetation-Config> tag" << std::endl;
					return;
				}

				size_t id = 0;
				osgDB::XmlNode::Children::iterator itr = config->children.begin();
				for (; itr != config->children.end(); ++itr)
				{
					osg::ref_ptr<osgDB::XmlNode> child = *itr;
					if (child->name == "Vegetation-Info")
					{
						readVegetationInfo(id++, child.get());
					}
					else if (child->name == "LOD-Range")
					{
						_lodRange = atof(child->contents.c_str());
					}
					else if (child->name == "Pattern")
					{
						_pattern = child->contents;
					}
					if (child->name == "ForwardPlusEnabled")
					{
						_forwardPlusEnabled = child->contents == "yes";
					}
				}
			}

			void readVegetationInfo(size_t id, osgDB::XmlNode* node)
			{
				if (node == NULL) return;

				VegetationInfo info;

				osgDB::XmlNode::Children::iterator itr = node->children.begin();
				for (; itr != node->children.end(); ++itr)
				{
					osg::ref_ptr<osgDB::XmlNode> child = *itr;

					if (child->name == "Texture")
					{
						info._textureName = child->contents;

						osg::ref_ptr<osg::Image> image = osgDB::readImageFile(info._textureName);
						if (!image.valid())
						{
							image = osgDB::readImageFile(_path + "/" + info._textureName);
						}

						info._texture = new osg::Texture2D;
						info._texture->setImage(image);

						if (!image.valid())
						{
							osg::notify(osg::NOTICE) << "GPU Vegetation: failed to load image: " << info._textureName << std::endl;
						}
						else
						{
							osg::notify(osg::NOTICE) << "GPU Vegetation: read image: " << info._textureName << std::endl;
						}
					}
					if (child->name == "Height")
					{

					}
					if (child->name == "UV")
					{
						OpenIG::Base::StringUtils::Tokens tokens = OpenIG::Base::StringUtils::instance()->tokenize(child->contents);
						if (tokens.size() == 4)
						{
							info._uv.x() = atof(tokens.at(0).c_str());
							info._uv.y() = atof(tokens.at(1).c_str());
							info._uv.z() = atof(tokens.at(2).c_str());
							info._uv.w() = atof(tokens.at(3).c_str());
						}
					}
					if (child->name == "Width")
					{
					}
					if (child->name == "Scale")
					{
					}

				}
				info._id = id;
				_vegetationInfo[id] = info;
			}

			class GPUVegetationCommand : public OpenIG::Base::Commands::Command
			{
			public:
				GPUVegetationCommand(OpenIG::Base::ImageGenerator* ig, double& range)
					: _ig(ig)
					, _range(range)
				{

				}

				virtual const std::string getUsage() const
				{
					return "range";
				}

				virtual const std::string getArgumentsFormat() const
				{
					return "D";
				}

				virtual const std::string getDescription() const
				{
					return  "sets the range of the GPU vegetation LODs\n"
						"     range - the range in meters for the LOD nodes of the GPU vegetation\n";
				}

				class SetRangeNodeVisitor : public osg::NodeVisitor
				{
				public:
					SetRangeNodeVisitor(double range)
						: osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
						, _range(range)
					{

					}

					virtual void apply(osg::LOD& lod)
					{
						if (lod.getName() == "GPU-Vegetation")
						{
							lod.setRange(0, 0, _range);
						}
						traverse(lod);
					}

				protected:
					double  _range;
				};

				virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
				{
					if (tokens.size() == 1)
					{
						double range = atof(tokens.at(0).c_str());
						_range = range;

						SetRangeNodeVisitor nv(range);
						_ig->getScene()->accept(nv);

						return 0;
					}

					return -1;
				}
			protected:
				OpenIG::Base::ImageGenerator* _ig;
				double&                 _range;
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
	return new OpenIG::Plugins::GPUVegetationPlugin;
}

extern "C" EXPORT void DeletePlugin(OpenIG::PluginBase::Plugin* plugin)
{
	osg::ref_ptr<OpenIG::PluginBase::Plugin> p(plugin);
}
