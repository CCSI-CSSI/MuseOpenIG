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
#include <IgCore/commands.h>

#include <OpenIG/openig.h>

#include <osg/ref_ptr>
#include <osg/ValueObject>

#include <osgDB/XmlParser>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include <SilverLining.h>

#include "AtmosphereReference.h"
#include "CloudsDrawable.h"
#include "SkyDrawable.h"

#define SKY_RENDER_BIN -10
#define CLOUDS_RENDER_BIN 24

namespace igplugins
{

class SilverLiningPlugin : public igplugincore::Plugin
{
public:
	SilverLiningPlugin()
		: _geocentric(false)
		, _lightBrightness_enable(true)
		, _lightBrightness_day(1.f)
		, _lightBrightness_night(1.f)
        , _skyboxSize(100000)
        , _skyboxSizeSet(true)
	{

	}

    virtual std::string getName() { return "SilverLining"; }

    virtual std::string getDescription() { return "Integration of Sundog's SilverLining Atmopshere model"; }

    virtual std::string getVersion() { return "1.0.0"; }

    virtual std::string getAuthor() { return "ComPro, Nick"; }

	class SilverLiningCommand : public igcore::Commands::Command
	{
	public:
		SilverLiningCommand(SilverLiningPlugin* plugin)
			: _plugin(plugin)
		{
		}
		virtual const std::string getUsage() const
		{
			return "mode";
		}

		virtual const std::string getDescription() const
		{
            return  "configures the SilverLining plugin\n"
                    "    mode - one of these:\n"
                    "           geocentric - if in geocentric mode\n"
                    "           flat       - if in flat earth mode\n";
		}

        virtual int exec(const igcore::StringUtils::Tokens& tokens)
        {
            if (tokens.size() == 1 && _plugin != 0)
            {
                std::string mode = tokens.at(0);
                //ensure its lowercase for the string compare....
                std::transform(mode.begin(), mode.end(), mode.begin(), ::tolower);
                if ( mode.compare(0,10,"geocentric") == 0 )
                    _plugin->setGeocentric(true);
                else if ( mode.compare(0,4,"flat") == 0 )
                    _plugin->setGeocentric(false);

                return 0;
            }
            return -1;
        }

	protected:
		SilverLiningPlugin* _plugin;
	};

    class SetSkyboxSizeCommand : public igcore::Commands::Command
    {
    public:
        SetSkyboxSizeCommand(SilverLiningPlugin* plugin)
            : _plugin(plugin)
        {
        }
        virtual const std::string getUsage() const
        {
            return "skyboxsize";
        }

        virtual const std::string getDescription() const
        {
            return  "configures the SilverLining skybox size\n"
                "    skyboxsize - any value that encompasses your scene size\n"
                "                 it should be just a bit larger than the farclip\n"
                "                 value that you are using on your scene: ie: 100000.01\n";
        }

        virtual int exec(const igcore::StringUtils::Tokens& tokens)
        {
            if (tokens.size() == 1 && _plugin != 0)
            {
                _plugin->_skyboxSize = atof(tokens.at(0).c_str());
                _plugin->setSkyboxSize();

                return 0;
            }
            return -1;
        }

    protected:
        SilverLiningPlugin* _plugin;
    };

	class SetSilverLiningParamsCommand : public igcore::Commands::Command
	{
	public:
		SetSilverLiningParamsCommand(SilverLiningPlugin* plugin)
			: _plugin(plugin)
		{
		}
		virtual const std::string getUsage() const
		{
			return "cloudsBaseLength cloudsBaseWidth";
		}

		virtual const std::string getDescription() const
		{
			return  "configures some SilverLining parameters\n"
				"    cloudsBaseLength - the base length of the newely created cloud layers\n"
				"    cloudsBaseWidth - the base width of the newely created cloud layers\n";

		}

		virtual int exec(const igcore::StringUtils::Tokens& tokens)
		{
			if (tokens.size() == 2 && _plugin != 0)
			{
				double cloudsBaseLength = atof(tokens.at(0).c_str());
				double cloudsBaseWidth = atof(tokens.at(1).c_str());
				
				SkyDrawable::SilverLiningParams params;
				params.cloudsBaseLength = cloudsBaseLength;
				params.cloudsBaseWidth = cloudsBaseWidth;

                int mask = igplugins::SkyDrawable::SilverLiningParams::CLOUDS_BASE_WIDTH | igplugins::SkyDrawable::SilverLiningParams::CLOUDS_BASE_LENGTH;
                _plugin->_skyDrawable->setSilverLiningParams(params,mask);

				return 0;
			}
			return -1;
		}

	protected:
		SilverLiningPlugin* _plugin;
	};

    virtual void config(const std::string& fileName)
    {
		igcore::Commands::instance()->addCommand("silverlining", new SilverLiningCommand(this));
        igcore::Commands::instance()->addCommand("setskyboxsize", new SetSkyboxSizeCommand(this));
		igcore::Commands::instance()->addCommand("setsilverliningparams", new SetSilverLiningParamsCommand(this));

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
			if (child->name == "SilverLining-Geocentric")
			{
				_geocentric = child->contents == "yes";
			}
            if (child->name == "SilverLining-SkyBoxSize")
            {
                _skyboxSize = std::strtod(child->contents.c_str(),NULL);
                _skyboxSizeSet = false;
            }
        }
    }

	void setGeocentric(bool geocentric)
	{
		_geocentric = geocentric;

		if (_skyDrawable.valid())
		{
			_skyDrawable->setGeocentric(_geocentric);
		}
	}
	
    void setSkyboxSize(void)
    {

        if (_skyDrawable.valid())
        {
            _skyboxSizeSet = true;
            _skyDrawable->setSkyboxSize(_skyboxSize);
            osg::notify(osg::NOTICE) << "SilverLining: setSkyboxSize() setting skyboxsize to: " << _skyboxSize << std::endl;
        }
    }

    virtual void init(igplugincore::PluginContext& context)
    {
        igcore::Commands::instance()->addCommand("addclouds",       new AddCloudLayerCommand(context.getImageGenerator()));
        igcore::Commands::instance()->addCommand("updateclouds",    new UpdateCloudLayerCommand(context.getImageGenerator()));
        igcore::Commands::instance()->addCommand("removeclouds",    new RemoveCloudLayerCommand(context.getImageGenerator()));
        igcore::Commands::instance()->addCommand("removeallclouds", new RemoveAllCloudLayersCommand(context.getImageGenerator()));
        igcore::Commands::instance()->addCommand("fog",             new SetFogCommand(context.getImageGenerator()));
        igcore::Commands::instance()->addCommand("rain",            new RainCommand(context.getImageGenerator()));
        igcore::Commands::instance()->addCommand("snow",            new SnowCommand(context.getImageGenerator()));

        initSilverLining(context.getImageGenerator());

        igcore::AtmosphereAttributes atmosphere((void*)ar);
        context.addAttribute("SLAtmosphere", new igplugincore::PluginContext::Attribute<igcore::AtmosphereAttributes>(atmosphere));
    }

    virtual void update(igplugincore::PluginContext& context)
    {
		
        {
            if(!_skyboxSizeSet)
                setSkyboxSize();
        }
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
				_cloudsDrawable->setTOD(attr->getValue().getHour());
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
				_cloudsDrawable->setEnvironmentMapDirty(true);
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
						_cloudsDrawable->setEnvironmentMapDirty(true);
                        break;
                    }

                    switch (attr->getValue().getRemoveFlag())
                    {
                    case true:
                        _skyDrawable->removeCloudLayer(attr->getValue().getId());
						_cloudsDrawable->setEnvironmentMapDirty(true);
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
						_cloudsDrawable->setEnvironmentMapDirty(true);
                    }
                }
            }

        }

		{
			openig::OpenIG* ig = dynamic_cast<openig::OpenIG*>(context.getImageGenerator());
			if (ig && this->_skyDrawable) this->_skyDrawable->setIG(ig);
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

	virtual void databaseRead(const std::string& fileName, osg::Node*, const osgDB::Options*)
	{
		std::string xmlFile = fileName + ".lighting.xml";
		if (!osgDB::fileExists(xmlFile))
		{
			osg::notify(osg::NOTICE) << "SilverLining: xml file does not exists: " << xmlFile << std::endl;
			return;
		}

		osgDB::XmlNode* root = osgDB::readXmlFile(xmlFile);
		if (!root)
		{
			osg::notify(osg::NOTICE) << "SilverLining: NULL root : " << xmlFile << std::endl;
			return;
		}
		if (!root->children.size())
		{
			osg::notify(osg::NOTICE) << "SilverLining: root with no children: " << xmlFile << std::endl;
			return;
		}
		if (root->children.at(0)->name != "OsgNodeSettings")
		{
			osg::notify(osg::NOTICE) << "SilverLining: OsgNodeSettings tag not found: " << xmlFile << std::endl;
			return;
		}

		osg::notify(osg::NOTICE) << "SilverLining: current file: " << fileName << std::endl;

		osgDB::XmlNode::Children::iterator itr = root->children.at(0)->children.begin();
		for (; itr != root->children.at(0)->children.end(); ++itr)
		{
			osgDB::XmlNode* child = *itr;

			//<LightBrightnessOnClouds day="0.01" night=".1" />
			if (child->name == "LightBrightnessOnClouds")
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

		if (_cloudsDrawable.valid())
		{
			_cloudsDrawable->setLightingBrightness(
				_lightBrightness_enable,
				_lightBrightness_day,
				_lightBrightness_night
				);
		}
	}


protected:
    std::string                     _userName;
    std::string                     _key;
    std::string                     _path;
    osg::ref_ptr<SkyDrawable>       _skyDrawable;
    osg::ref_ptr<CloudsDrawable>    _cloudsDrawable;
    AtmosphereReference             *ar;
	bool							_geocentric;
	bool							_lightBrightness_enable;
	float							_lightBrightness_day;
	float							_lightBrightness_night;
    double                          _skyboxSize;
    bool                            _skyboxSizeSet;

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

        ar = new AtmosphereReference;
        ar->atmosphere = atm;
        viewer->getView(0)->getCamera()->setUserData(ar);

        // Add the sky (calls Atmosphere::DrawSky and handles initialization once you're in
        // the rendering thread)
        osg::Geode *skyGeode = new osg::Geode;
        _skyDrawable = new SkyDrawable(_path,viewer->getView(0),sunOrMoonLight->getLight(),fog, _geocentric);

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
        ig->getScene()->asGroup()->addChild(skyGeode);
        ig->getScene()->asGroup()->addChild(cloudsGeode);
		
		_cloudsDrawable->setLightingBrightness(
			_lightBrightness_enable, 
			_lightBrightness_day, 
			_lightBrightness_night
		);

        if (_skyDrawable.valid())
        {
            _skyboxSizeSet = true;
            _skyDrawable->setSkyboxSize(_skyboxSize);
        }

        return _cloudsDrawable.get();
    }

    class AddCloudLayerCommand : public igcore::Commands::Command
    {
    public:
        AddCloudLayerCommand (igcore::ImageGenerator* ig)
            : _ig(ig) {}

        virtual const std::string getUsage() const
        {
            return "id type altitude thickness density";
        }

        virtual const std::string getDescription() const
        {
            return  "add new cloud layer -- based on Sundog's SilverLining plugin\n"
                    "     id - the id of the cloud latyer across the scene\n"
                    "     type - from the available cloud types from SilverLining, from 0 to 9 respoding to:\n"
                    "          0 - CIRROCUMULUS\n"
                    "          1 - CIRRUS_FIBRATUS\n"
                    "          2 - STRATUS\n"
                    "          3 - CUMULUS_MEDIOCRIS\n"
                    "          4 - CUMULUS_CONGESTUS\n"
                    "          5 - CUMULUS_CONGESTUS_HI_RES\n"
                    "          6 - CUMULONIMBUS_CAPPILATUS\n"
                    "          7 - STRATOCUMULUS\n"
                    "          8 - TOWERING_CUMULUS\n"
                    "          9 - SANDSTORM\n"
                    "     altitude - in meters, the altitude of the layer\n"
                    "     thickness - the thickness of the layer\n"
                    "     density - from 0.0-1.0, 1.0 most dense\n";
        }

        virtual int exec(const igcore::StringUtils::Tokens& tokens)
        {
            if (tokens.size() == 5)
            {
                unsigned int id     = atoi(tokens.at(0).c_str());
                int type            = atoi(tokens.at(1).c_str());
                double altitude     = atof(tokens.at(2).c_str());
                double thickness    = atof(tokens.at(3).c_str());
                double density      = atof(tokens.at(4).c_str());

                _ig->addCloudLayer(id,type,altitude,thickness, density);

                return 0;
            }
            return -1;
        }

    protected:
        igcore::ImageGenerator* _ig;
    };

    class UpdateCloudLayerCommand : public igcore::Commands::Command
    {
    public:
        UpdateCloudLayerCommand (igcore::ImageGenerator* ig)
            : _ig(ig) {}

        virtual const std::string getUsage() const
        {
            return "id type altitude thickness density";
        }

        virtual const std::string getDescription() const
        {
            return  "updates clouds layer settings -- based on Sundog's SilverLining plugin\n"
                    "     id - the id of the cloud latyer across the scene\n"
                    "     type - NOT Changeable!!!\n"
                    "     altitude - in meters, the altitude of the layer\n"
                    "     thickness - the thickness of the layer\n"
                    "     density - from 0.0-1.0, 1.0 most dense\n";
        }

        virtual int exec(const igcore::StringUtils::Tokens& tokens)
        {
            if (tokens.size() == 4)
            {
                unsigned int id     = atoi(tokens.at(0).c_str());
                double altitude     = atof(tokens.at(1).c_str());
                double thickness    = atof(tokens.at(2).c_str());
                double density      = atof(tokens.at(3).c_str());

                _ig->updateCloudLayer(id, altitude,thickness, density);

                return 0;
            }
            return -1;
        }

    protected:
        igcore::ImageGenerator* _ig;
    };

    class RemoveCloudLayerCommand : public igcore::Commands::Command
    {
    public:
        RemoveCloudLayerCommand(igcore::ImageGenerator* ig)
            : _ig(ig) {}

        virtual const std::string getUsage() const
        {
            return "id";
        }

        virtual const std::string getDescription() const
        {
            return  "removes the cloud layer from the scene by a given cloud layer id\n"
                    "     id - the id of the cloud layer";
        }

        virtual int exec(const igcore::StringUtils::Tokens& tokens)
        {
            if (tokens.size() == 1)
            {
                unsigned int id = atoi(tokens.at(0).c_str());

                _ig->removeCloudLayer(id);

                return 0;
            }
            return -1;
        }

    protected:
        igcore::ImageGenerator* _ig;
    };

    class RemoveAllCloudLayersCommand : public igcore::Commands::Command
    {
    public:
        RemoveAllCloudLayersCommand(igcore::ImageGenerator* ig)
            : _ig(ig) {}

        virtual const std::string getUsage() const
        {
            return "(no attributes)";
        }

        virtual const std::string getDescription() const
        {
            return "removes all the cloud layers from the scene";
        }

        virtual int exec(const igcore::StringUtils::Tokens&)
        {
            _ig->removeAllCloudlayers();

            return 0;
        }

    protected:
        igcore::ImageGenerator* _ig;
    };

    class SetFogCommand : public igcore::Commands::Command
    {
    public:
        SetFogCommand (igcore::ImageGenerator* ig)
            : _ig(ig) {}

        virtual const std::string getUsage() const
        {
            return "visibility";
        }

        virtual const std::string getDescription() const
        {
            return  "sets the visibility of the scene by using fog\n"
                    "     visibility - in meteres, the distance of the fog";
        }

        virtual int exec(const igcore::StringUtils::Tokens& tokens)
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
        igcore::ImageGenerator* _ig;
    };

    class RainCommand : public igcore::Commands::Command
    {
    public:
        RainCommand(igcore::ImageGenerator* ig)
            : _ig(ig) {}

        virtual const std::string getUsage() const
        {
            return "rainfactor";
        }

        virtual const std::string getDescription() const
        {
            return  "adds rain to the scene\n"
                    "     rainfactor - from 0.0-1.0, 0 no rain, 1 heavy rain";
        }

        virtual int exec(const igcore::StringUtils::Tokens& tokens)
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
        igcore::ImageGenerator* _ig;
    };

    class SnowCommand : public igcore::Commands::Command
    {
    public:
        SnowCommand(igcore::ImageGenerator* ig)
            : _ig(ig) {}

        virtual const std::string getUsage() const
        {
            return "snowfactor";
        }

        virtual const std::string getDescription() const
        {
            return  "adds snow to the scene\n"
                    "     snowfactor - from 0.0-1.0. 0 no snow, 1 heavy snow";
        }

        virtual int exec(const igcore::StringUtils::Tokens& tokens)
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
        igcore::ImageGenerator* _ig;
    };


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
