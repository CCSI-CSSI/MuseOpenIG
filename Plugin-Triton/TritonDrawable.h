#ifndef TRITON_DRAWABLE_H
#define TRITON_DRAWABLE_H

#include <Triton.h>

#include<osg/Drawable>
#include<osg/TextureCubeMap>
#include<osg/Texture2D>

#include <Core-OpenIG/openig.h>

namespace osg
{
	class GLExtensions;
}

namespace OpenIG {
	namespace Plugins {

		struct TritonUpdateCallback : public virtual osg::Drawable::UpdateCallback {
			TritonUpdateCallback(Triton::Ocean *pOcean) : ocean(pOcean) {}

			/** Update the underlying FFT for the waves from the update thread */
			virtual void update(osg::NodeVisitor* nv, osg::Drawable*) {
				if (ocean) {
					ocean->UpdateSimulation(nv->getFrameStamp()->getSimulationTime());
				}
			}

			Triton::Ocean *ocean;
		};

		class TritonDrawable : public osg::Drawable
		{
		public:
			TritonDrawable(const std::string& resourcePath, const std::string& userName, const std::string& licence, bool geocentric = false, bool forwardPlusEnabled = true, bool logZEnabled = true, osg::TextureCubeMap * cubeMap = NULL);

			void setHeightMapCamera(osg::Camera *heightMapcamera) { _heightMapcamera = heightMapcamera; }
			void setHeightMap(osg::Texture2D *heightMap) { _heightMap = heightMap; }
			void setBelowWaterVisibiliy(double visibility) { _belowWaterVisibility = visibility; }
			void setIG(OpenIG::Engine* ig) { _openIG = ig; }
			void setEnvironmentalMap(int id);
			void setEnvironmentalMapping(bool value) { _environmentalMapping = value; }
			void setPlanarReflection(osg::Texture2D* texture, osg::RefMatrix* mx);
			void setPlanarReflectionBlend(float value);

			Triton::Environment * getEnvironment(void) { return _environment; }

			virtual bool isSameKindAs(const Object* obj) const {
				return dynamic_cast<const TritonDrawable*>(obj) != NULL;
			}
			virtual Object* cloneType() const {
				return new TritonDrawable("", "", "");
			}
			virtual Object* clone(const osg::CopyOp& copyop) const {
				return new TritonDrawable("", "", "");
			}

			virtual void drawImplementation(osg::RenderInfo& renderInfo) const;

			void cleanup(void);

			void setTOD(unsigned int hour)
			{
				_tod = hour;
			}

			void setLightingBrightness(bool enable, float onDay, float onNight)
			{
				_lightBrightness_enable = true;// we are not reading this enable;
				_lightBrightness_day = onDay;
				_lightBrightness_night = onNight;
			}

		protected:
			virtual ~TritonDrawable();
			osg::ref_ptr< osg::TextureCubeMap >  _cubeMap;

			// The three main Triton objects you need:
			Triton::ResourceLoader *_resourceLoader;
			Triton::Environment    *_environment;
			Triton::Ocean          *_ocean;
			double                  _belowWaterVisibility;

			std::string				_resourcePath;
			std::string				_userName;
			std::string				_license;
			bool                    _geocentric;


			osg::ref_ptr<osg::Camera>		_heightMapcamera;
			osg::ref_ptr<osg::Texture2D>	_heightMap;

			OpenIG::Engine*					_openIG;
			bool							_environmentalMapping;

			osg::ref_ptr< osg::Texture2D >       _planarReflectionMap;
			osg::ref_ptr< osg::RefMatrix >       _planarReflectionProjection;
			float								 _planarReflectionBlend;
			unsigned int						 _tod;

			bool							_lightBrightness_enable;
			float							_lightBrightness_day;
			float							_lightBrightness_night;


		private:
			void setUpSunOrMoonLight(void) const;
			void setUpEnvironment(void) const;
			void setUpLogZBuffer(osg::GLExtensions* ext, osg::Camera* camera) const;
			void setUpTransparency(osg::GLExtensions* ext, osg::Camera* camera) const;
			void setUpPlanarReflections(const osg::State& state) const;
			void setUpHeightMap(unsigned int contextID) const;

			void setup(osg::RenderInfo& renderInfo);

			void initializeForwardPlus(osg::RenderInfo& renderInfo, std::vector<GLint>& userShaders);
			void setUpForwardPlus(osg::GLExtensions* ext, osg::Camera* camera) const;

			bool _forwardPlusSetUpTried;
			bool _forwardPlusEnabled;

			bool _logZEnabled;
			bool _logz_tried;
			void initializeLogZDepthBuffer(osg::RenderInfo& renderInfo, std::vector<GLint>& userShaders);
		};
	}
}

#endif
