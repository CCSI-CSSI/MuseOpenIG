#ifndef TRITON_DRAWABLE_H
#define TRITON_DRAWABLE_H

#include <Triton.h>

#include<osg/Drawable>
#include<osg/TextureCubeMap>
#include<osg/Texture2D>

#include <OpenIG/openig.h>

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
    TritonDrawable( const std::string& resourcePath, const std::string& userName, const std::string& licence, bool geocentric=false, osg::TextureCubeMap * cubeMap = NULL );

    void setHeightMapCamera(osg::Camera *heightMapcamera) { _heightMapcamera = heightMapcamera; }
    void setHeightMap(osg::Texture2D *heightMap) { _heightMap = heightMap; }
    void setBelowWaterVisibiliy(double visibility) { _belowWaterVisibility = visibility; }
	void setIG(openig::OpenIG* ig) { _openIG = ig;  }
	void setEnvironmentalMap(int id);
	void setEnvironmentalMapping(bool value) { _environmentalMapping = value; }
	void setPlanarReflection(osg::Texture2D* texture, osg::RefMatrix* mx);
	void setPlanarReflectionBlend(float value);

    Triton::Environment * getEnvironment(void) { return _environment;}

    virtual bool isSameKindAs(const Object* obj) const {
        return dynamic_cast<const TritonDrawable*>(obj)!=NULL;
    }
    virtual Object* cloneType() const {
        return new TritonDrawable("","","");
    }
    virtual Object* clone(const osg::CopyOp& copyop) const {
        return new TritonDrawable("","","");
    }

    virtual void drawImplementation(osg::RenderInfo& renderInfo) const;

protected:

    void Setup( void );

public:
    void Cleanup( void );

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

	openig::OpenIG*					_openIG;
	bool							_environmentalMapping;

	osg::ref_ptr< osg::Texture2D >       _planarReflectionMap;
	osg::ref_ptr< osg::RefMatrix >       _planarReflectionProjection;
	float								 _planarReflectionBlend;
};


#endif
