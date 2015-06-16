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
#include <IgPluginCore/plugin.h>
#include <IgPluginCore/plugincontext.h>
#include <IgCore/imagegenerator.h>
#include <IgCore/attributes.h>

#include <osg/ref_ptr>
#include <osg/ValueObject>

#include <osgDB/XmlParser>

#include <SilverLining.h>

#include "AtmosphereReference.h"
#include "CloudsDrawable.h"
#include "SkyDrawable.h"

#define SKY_RENDER_BIN -10
#define OCEAN_RENDER_BIN 7
#define MODELS_RENDER_BIN 8
#define CLOUDS_RENDER_BIN 24

namespace igplugins
{

class SilverLiningPlugin : public igplugincore::Plugin
{
public:
    virtual std::string getName() { return "SilverLining"; }

    virtual std::string getDescription() { return "Integration of Sundog's SilverLining Atmopshere model"; }

    virtual std::string getVersion() { return "1.0.0"; }

    virtual std::string getAuthor() { return "ComPro, Nick"; }

    virtual void config(const std::string& fileName)
    {
        osgDB::XmlNode* root = osgDB::readXmlFile(fileName);
        if (root == 0) return;

        if (root->children.size() == 0) return;

        osgDB::XmlNode* config = root->children.at(0);
        if (config->name != "OpenIG-Plugin-Config") return;

        osgDB::XmlNode::Children::iterator itr = config->children.begin();
        for ( ; itr != config->children.end(); ++itr)
        {
            osgDB::XmlNode* child = *itr;
            if (child->name == "SilverLining-License-UserName")
            {
                _userName = child->contents;
            }
            if (child->name == "SilverLining-License-Key")
            {
               _key = child->contents;
            }
            if (child->name == "SilverLining-Resource-Path")
            {
                _path = child->contents;
            }
        }
    }

    virtual void init(igplugincore::PluginContext& context)
    {
        initSilverLining(context.getImageGenerator());
    }

    virtual void update(igplugincore::PluginContext& context)
    {
        {
            osg::ref_ptr<osg::Referenced> ref = context.getAttribute("Rain");
            igplugincore::PluginContext::Attribute<igcore::RainSnowAttributes> *attr = dynamic_cast<igplugincore::PluginContext::Attribute<igcore::RainSnowAttributes> *>(ref.get());
            if (attr && _skyDrawable.valid())
            {
                _skyDrawable->setRain(attr->getValue().getFactor());
            }
        }
        {
            osg::ref_ptr<osg::Referenced> ref = context.getAttribute("Snow");
            igplugincore::PluginContext::Attribute<igcore::RainSnowAttributes> *attr = dynamic_cast<igplugincore::PluginContext::Attribute<igcore::RainSnowAttributes> *>(ref.get());
            if (attr && _skyDrawable.valid())
            {
                _skyDrawable->setSnow(attr->getValue().getFactor());
            }
        }
        {
            osg::ref_ptr<osg::Referenced> ref = context.getAttribute("Fog");
            igplugincore::PluginContext::Attribute<igcore::FogAttributes> *attr = dynamic_cast<igplugincore::PluginContext::Attribute<igcore::FogAttributes> *>(ref.get());
            if (attr && _skyDrawable.valid())
            {
                _skyDrawable->setVisibility(attr->getValue().getVisibility());
            }
        }
        {
            osg::ref_ptr<osg::Referenced> ref = context.getAttribute("Wind");
            igplugincore::PluginContext::Attribute<igcore::WindAttributes> *attr = dynamic_cast<igplugincore::PluginContext::Attribute<igcore::WindAttributes> *>(ref.get());
            if (attr && _skyDrawable.valid())
            {
                _skyDrawable->setWind(attr->getValue()._speed,attr->getValue()._direction);
            }
        }

        {
            osg::ref_ptr<osg::Referenced> ref = context.getAttribute("TOD");
            igplugincore::PluginContext::Attribute<igcore::TimeOfDayAttributes> *attr = dynamic_cast<igplugincore::PluginContext::Attribute<igcore::TimeOfDayAttributes> *>(ref.get());
            if (attr && _skyDrawable.valid())
            {
                _skyDrawable->setTimeOfDay(
                    attr->getValue().getHour(),
                    attr->getValue().getMinutes());

                _cloudsDrawable->setEnvironmentMapDirty(true);
                _cloudsDrawable->setPluginContext(&context);
            }

        }

        igplugincore::PluginContext::AttributeMapIterator itr = context.getAttributes().begin();
        for ( ; itr != context.getAttributes().end(); ++itr)
        {
            osg::ref_ptr<osg::Referenced> ref = itr->second;
            igplugincore::PluginContext::Attribute<igcore::CLoudLayerAttributes> *attr = dynamic_cast<igplugincore::PluginContext::Attribute<igcore::CLoudLayerAttributes> *>(ref.get());

            // This is cleaner way of dealing with
            // PluginContext attributes but the Mac
            // compiler doesn't like it. It works ok
            // on Linux though
            // osg::ref_ptr<igplugincore::PluginContext::Attribute<igcore::CLoudLayerAttributes> > attr = itr->second;

            if (itr->first == "RemoveAllCloudLayers" && _skyDrawable.valid())
            {
                _skyDrawable->removeAllCloudLayers();
            }
            else
            if (attr && itr->first == "CloudLayer" && _skyDrawable.valid())
            {
                if (attr->getValue().isDirty())
                {
                    switch (attr->getValue().getAddFlag())
                    {
                    case true:
                        _skyDrawable->addCloudLayer(
                             attr->getValue().getId(),
                             attr->getValue().getType(),
                             attr->getValue().getAltitude(),
                             attr->getValue().getThickness(),
                             attr->getValue().getDensity());
                        break;
                    }

                    switch (attr->getValue().getRemoveFlag())
                    {
                    case true:
                        _skyDrawable->removeCloudLayer(attr->getValue().getId());
                        //osg::notify(osg::NOTICE) << "remove cloud layer: " << attr->getValue().getId() << std::endl;
                        break;
                    }

                    if (!attr->getValue().getAddFlag() && !attr->getValue().getRemoveFlag())
                    {
                        _skyDrawable->updateCloudLayer(
                            attr->getValue().getId(),
                            attr->getValue().getAltitude(),
                           attr->getValue().getThickness(),
                            attr->getValue().getDensity());
                    }
                }
            }

        }
    }

	virtual void clean(igplugincore::PluginContext& context)
	{
		osgViewer::CompositeViewer* viewer = context.getImageGenerator()->getViewer();

		//std::cout << "SilverLining Atmosphere deleting ..." << std::endl;
		AtmosphereReference *ar = dynamic_cast<AtmosphereReference*>(viewer->getView(0)->getCamera()->getUserData());
		if (ar && ar->atmosphere)
		{
			delete ar->atmosphere;
			//std::cout << "SilverLining Atmosphere deleted" << std::endl;
		}

	}

protected:
    std::string                     _userName;
    std::string                     _key;
    std::string                     _path;
    osg::ref_ptr<SkyDrawable>       _skyDrawable;
    osg::ref_ptr<CloudsDrawable>    _cloudsDrawable;

    const EnvMapUpdater* initSilverLining(igcore::ImageGenerator* ig)
    {
        if (_cloudsDrawable.valid() && _skyDrawable.valid()) return _cloudsDrawable.get();
        if (ig == 0) return 0;

        osgViewer::CompositeViewer* viewer = ig->getViewer();

        osg::LightSource* sunOrMoonLight = ig->getSunOrMoonLight();
        osg::Fog* fog = ig->getFog();

        if (viewer == 0)
            return 0;

        if (viewer->getNumViews()==0)
            return 0;

        // No need for OSG to clear the color buffer, the sky will fill it for you.
        viewer->getView(0)->getCamera()->setClearMask(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // configure the near/far so we don't clip things that are up close
        viewer->getView(0)->getCamera()->setNearFarRatio(0.00002);

        // Instantiate an Atmosphere and associate it with this camera. If you have multiple cameras
        // in multiple contexts, be sure to instantiate seperate Atmosphere objects for each.
        // Remember to delete this object at shutdown.
        SilverLining::Atmosphere *atm = new SilverLining::Atmosphere(_userName.c_str(), _key.c_str());

        AtmosphereReference *ar = new AtmosphereReference;
        ar->atmosphere = atm;
        viewer->getView(0)->getCamera()->setUserData(ar);

        // Add the sky (calls Atmosphere::DrawSky and handles initialization once you're in
        // the rendering thread)
        osg::Geode *skyGeode = new osg::Geode;
        _skyDrawable = new SkyDrawable(_path,viewer->getView(0),sunOrMoonLight->getLight(),fog);

        // ***IMPORTANT!**** Check that the path to the resources folder for SilverLining in SkyDrawable.cpp
        // SkyDrawable::initializeSilverLining matches with where you installed SilverLining.

        skyGeode->addDrawable(_skyDrawable);
        skyGeode->setCullingActive(false); // The skybox is always visible.

        skyGeode->getOrCreateStateSet()->setRenderBinDetails(SKY_RENDER_BIN, "RenderBin");
        //skyGeode->getOrCreateStateSet()->setAttributeAndModes( new osg::Depth( osg::Depth::LEQUAL, 0.0, 1.0, false ) );

        // Add the clouds (note, you need this even if you don't have clouds in your scene - it calls
        // Atmosphere::DrawObjects() which also draws precipitation, lens flare, etc.)
        osg::Geode *cloudsGeode = new osg::Geode;
        _cloudsDrawable = new CloudsDrawable(viewer->getView(0),ig);
        cloudsGeode->addDrawable(_cloudsDrawable);
        cloudsGeode->getOrCreateStateSet()->setRenderBinDetails(CLOUDS_RENDER_BIN, "RenderBin");
        cloudsGeode->setCullingActive(false);

        // Add our sky and clouds into the scene.

    #if 0
        viewer->getView(0)->getSceneData()->asGroup()->addChild(skyGeode);
        viewer->getView(0)->getSceneData()->asGroup()->addChild(cloudsGeode);
    #else
        ig->getScene()->asGroup()->addChild(skyGeode);
        ig->getScene()->asGroup()->addChild(cloudsGeode);
    #endif
        return _cloudsDrawable.get();
    }


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
    return new igplugins::SilverLiningPlugin;
}

extern "C" EXPORT void DeletePlugin(igplugincore::Plugin* plugin)
{
    osg::ref_ptr<igplugincore::Plugin> p(plugin);
}
