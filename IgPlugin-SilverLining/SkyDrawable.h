// Copyright (c) 2008-2012 Sundog Software, LLC. All rights reserved worldwide.

#pragma once

#include <osg/Drawable>
#include <osg/Light>
#include <osg/Fog>
#include <osgViewer/View>
#include <osg/TexGen>
#include <SilverLining.h>

namespace igplugins
{

class AtmosphereReference;
class SkyDrawable;

// Grab the camera and projection matrices for SilverLining. Note we do not use Atmosphere::CullObjects(), since it will
// interfere with the environment cube maps generated from the render thread.
struct SilverLiningCullCallback : public osg::Drawable::CullCallback
{
    SilverLiningCullCallback() : atmosphere(0) {}

    virtual bool cull(osg::NodeVisitor* nv, osg::Drawable* drawable, osg::RenderInfo* renderInfo) const 
    { 
        if (atmosphere)  {
            osgUtil::CullVisitor* cv = static_cast<osgUtil::CullVisitor*>( nv );
            atmosphere->SetCameraMatrix( cv->getModelViewMatrix()->ptr() );
            atmosphere->SetProjectionMatrix( cv->getProjectionMatrix()->ptr() );
        }
        return false; 
    }

    SilverLining::Atmosphere *atmosphere;
};

// Our update callback just marks our bounds dirty each frame (since they move with the camera.)
struct SilverLiningUpdateCallback : public osg::Drawable::UpdateCallback
{
    SilverLiningUpdateCallback() : camera(0) {}

    virtual void update(osg::NodeVisitor*, osg::Drawable* drawable);

    osg::Camera *camera;
};

// We also hook in with a bounding box callback to tell OSG how big our skybox is, plus the
// atmospheric limb if applicable.
struct SilverLiningSkyComputeBoundingBoxCallback : public osg::Drawable::ComputeBoundingBoxCallback
{
    SilverLiningSkyComputeBoundingBoxCallback() : camera(0) {}

    virtual osg::BoundingBox computeBound(const osg::Drawable&) const;

    osg::Camera *camera;
};

// The SkyDrawable wraps SilverLining to handle updates, culling, and drawing of the skybox - and works together with a CloudsDrawable
// to draw the clouds toward the end of the scene.


class SkyDrawable : public osg::Drawable
{
public:
	SkyDrawable();
    SkyDrawable(const std::string& path, osgViewer::View* view, osg::Light* light, osg::Fog* fog, bool geocentric = false);

    virtual bool isSameKindAs(const Object* obj) const {
        return dynamic_cast<const SkyDrawable*>(obj)!=NULL;
    }
    virtual Object* cloneType() const {
        return new SkyDrawable();
    }
    virtual Object* clone(const osg::CopyOp& copyop) const {
        return new SkyDrawable();
    }

    void            setSkyboxSize(double size) {_skyboxSize = size;}
    double          getSkyboxSize() const {return _skyboxSize;}
    virtual void    drawImplementation(osg::RenderInfo& renderInfo) const;
    void            setTimeOfDay(unsigned int hour, unsigned int minutes);
    void            setVisibility(double visibility);
    void            setRain(double factor);
    void            setSnow(double factor);
    void            setWind(float speed, float direction);
    void            addCloudLayer(int id, int type, double altitude, double thickness, double density);
    void            removeCloudLayer(int id);
    void            updateCloudLayer(int id, double altitude, double thickness, double density);
    void            removeAllCloudLayers();
	void			setGeocentric(bool geocentric);	

protected:
	void setLighting(SilverLining::Atmosphere *atm) const;
	void setShadow(SilverLining::Atmosphere *atm, osg::RenderInfo & renderInfo );
    void initializeSilverLining(AtmosphereReference *ar) const;
    void initializeDrawable();
	void initializeShadow();

    osg::ref_ptr<osgViewer::View>   _view;
    double                          _skyboxSize;
    osg::ref_ptr<osg::Light>        _light;
    osg::ref_ptr<osg::Fog>          _fog;
    std::string                     _path;

    unsigned int                    _todHour;
    unsigned int                    _todMinutes;
    bool                            _todDirty;

    float                           _windSpeed;
    float                           _windDirection;
    bool                            _windDirty;
    int                             _windVolumeHandle;

    osg::ref_ptr< osg::TexGen >         _cloudShadowTexgen;
    osg::ref_ptr< osg::Texture2D >      _cloudShadowTextureWhiteSubstitute;
    int                                 _cloudShadowTextureStage;
    int                                 _cloudShadowTexgenStage;
    osg::ref_ptr< osg::Uniform >        _cloudShadowCoordMatrixUniform;
    osg::ref_ptr< osg::Texture2D >      _texture;
    void*                               _shadowTexHandle;
    SilverLining::Matrix4               _lightMVP;
    SilverLining::Matrix4               _worldToShadowMapTexCoord;
    bool                                _cloudShadowsEnabled;
    bool                                _init_shadows_once;
    bool                                _needsShadowUpdate;
    bool                                _cloudReflections;

    SilverLiningCullCallback *                  cullCallback;
    SilverLiningUpdateCallback *                updateCallback;
    SilverLiningSkyComputeBoundingBoxCallback * computeBoundingBoxCallback;

    double                              _rainFactor;
    double                              _snowFactor;
    bool                                _removeAllCloudLayers;
    bool                                _enableCloudShadows;
	bool								_geocentric;

    struct CloudLayerInfo
    {
        int     _id;
        int     _type;
        int     _handle;
        double  _altitude;
        double  _density;
        double _thickness;
        bool    _dirty;
        bool    _needReseed;

        CloudLayerInfo()
            : _type(-1)
            , _handle(-1)
            , _altitude(0.0)
            , _density(0.0)
            , _thickness(0.0)
            , _id(-1)
            , _dirty(false)
            , _needReseed(false)
        {

        }
    };
    typedef std::map< int, CloudLayerInfo >                     CloudLayers;
    typedef std::map< int, CloudLayerInfo >::iterator           CloudLayersIterator;
    typedef std::map< int, CloudLayerInfo >::const_iterator     CloudLayersConstIterator;

    CloudLayers         _clouds;

    typedef std::vector< CloudLayerInfo >                       CloudLayersQueue;
    typedef std::vector< CloudLayerInfo >::iterator             CloudLayersQueueIterator;
    typedef std::vector< CloudLayerInfo >::const_iterator       CloudLayersQueueConstIterator;

    CloudLayersQueue    _cloudsQueueToAdd;
    CloudLayersQueue    _cloudsQueueToRemove;

    void addClouds(SilverLining::Atmosphere *atmosphere, const osg::Vec3d& position);
    void removeClouds(SilverLining::Atmosphere *atmosphere);
    void updateClouds(SilverLining::Atmosphere *atmosphere);

};

} // namespace
