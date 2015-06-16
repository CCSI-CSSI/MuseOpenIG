// Copyright (c) 2008-2012 Sundog Software, LLC. All rights reserved worldwide.

#pragma once

#include <IgCore/imagegenerator.h>

#include <IgPluginCore/plugincontext.h>

#include <osg/Drawable>
#include <osgViewer/View>
#include <SilverLining.h>
#include <OpenThreads/Mutex>

namespace igplugins
{


// The update callback is just used to mark our bounds dirty each frame
struct SilverLiningCloudsUpdateCallback : public osg::Drawable::UpdateCallback
{
    SilverLiningCloudsUpdateCallback() {}

    virtual void update(osg::NodeVisitor*, osg::Drawable* drawable);
};

// We also hook in with a bounding box callback to tell OSG how big our cloud volumes are
struct SilverLiningCloudsComputeBoundingBoxCallback : public osg::Drawable::ComputeBoundingBoxCallback
{
    SilverLiningCloudsComputeBoundingBoxCallback() : camera(0) {}

    virtual osg::BoundingBox computeBound(const osg::Drawable&) const;

    osg::Camera *camera;
};

// Define an interface for a class that gives us an enviroment map.
class EnvMapUpdater
{
public:
    EnvMapUpdater() : _envMapID(0) {}

    virtual GLint getEnvironmentMap() const {
        return _envMapID;
    }

    virtual void setEnvironmentMap(GLint id) {
        _envMapID = id;
    }

protected:
    GLint _envMapID;

};

// The CloudsDrawable does the actual drawing of the clouds.
class CloudsDrawable : public osg::Drawable, public EnvMapUpdater
{
public:
    CloudsDrawable();
    CloudsDrawable(osgViewer::View* view, igcore::ImageGenerator* ig);

    virtual bool isSameKindAs(const Object* obj) const {
        return dynamic_cast<const CloudsDrawable*>(obj)!=NULL;
    }
    virtual Object* cloneType() const {
        return new CloudsDrawable();
    }
    virtual Object* clone(const osg::CopyOp& copyop) const {
        return new CloudsDrawable();
    }

    virtual void drawImplementation(osg::RenderInfo& renderInfo) const;

    void setEnvironmentMapDirty( bool dirty )
    {
        _envMapDirty = dirty;
    }

    void setPluginContext(igplugincore::PluginContext* context)
    {
        _pluginContext = context;
    }

protected:
    void initialize();

    osgViewer::View*                _view;
    mutable OpenThreads::Mutex      _mutex;
    igcore::ImageGenerator*         _ig;
    bool                            _envMapDirty;
    igplugincore::PluginContext*    _pluginContext;


};

} // namespace
