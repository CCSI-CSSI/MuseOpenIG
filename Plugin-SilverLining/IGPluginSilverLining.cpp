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
//#*    Please direct any questions or comments to the OpenIG Forums
//#*    Email address: openig@compro.net
//#*
//#*
//#*****************************************************************************
#include <Core-PluginBase/Plugin.h>
#include <Core-PluginBase/PluginContext.h>

#include <Core-Base/ImageGenerator.h>
#include <Core-Base/Types.h>
#include <Core-Base/Mathematics.h>
#include <Core-Base/Commands.h>
#include <Core-Base/FileSystem.h>

#include <Core-OpenIG/Engine.h>
#include <Core-OpenIG/RenderBins.h>

#include <osg/ref_ptr>
#include <osg/ValueObject>

#include <osgDB/XmlParser>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include <SilverLining.h>

#include <boost/thread.hpp>
#include <boost/filesystem.hpp>

#include "AtmosphereReference.h"
#include "CloudsDrawable.h"
#include "SkyDrawable.h"



namespace OpenIG {
    namespace Plugins {

        class SilverLiningPlugin : public OpenIG::PluginBase::Plugin
        {
        public:
            SilverLiningPlugin()
                : _geocentric(false)
                , _forwardPlusEnabled(false)
                , _logZEnabled(false)
                , _lightBrightness_enable(true)
                , _lightBrightness_day(1.f)
                , _lightBrightness_night(1.f)
                , _skyboxSize(100000)
                , _longitude(42)
                , _latitude(12)
                , _tz(0)
                , _skyboxSizeSet(true)
                , _sunMoonBrightness_day(1.f)
                , _sunMoonBrightness_night(1.f)
                , _updateEnvMapDynamically(false)
            {

            }

            virtual std::string getName() { return "SilverLining"; }

            virtual std::string getDescription() { return "Integration of Sundog's SilverLining Atmopshere model"; }

            virtual std::string getVersion() { return "1.0.0"; }

            virtual std::string getAuthor() { return "ComPro, Nick"; }

            void setDynamicEnvMapUpdate(bool update)
            {
                _updateEnvMapDynamically = update;
            }

            class AdditionalAmbientCommand : public OpenIG::Base::Commands::Command
            {
            public:
                AdditionalAmbientCommand() {}

                virtual const std::string getUsage() const
                {
                    return "r g b";
                }

                virtual const std::string getArgumentsFormat() const
                {
                    return "D:D:D";
                }

                virtual const std::string getDescription() const
                {
                    return  "Set/Add the additional r g b values to the current ambient light\n"
                            "    r g b - the colors as floating point numbers";
                }

                virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
                {
                    if (tokens.size() == 3)
                    {
                        osg::Vec3 color;

                        color.x() = atof(tokens.at(0).c_str());
                        color.y() = atof(tokens.at(1).c_str());
                        color.z() = atof(tokens.at(2).c_str());

                        SkyDrawable::setAdditionalAmbientLightConstant(color);
                    }

                    return 0;
                }
            };

            class AdditionalDiffuseCommand : public OpenIG::Base::Commands::Command
            {
            public:
                AdditionalDiffuseCommand() {}

                virtual const std::string getUsage() const
                {
                    return "r g b";
                }

                virtual const std::string getArgumentsFormat() const
                {
                    return "D:D:D";
                }

                virtual const std::string getDescription() const
                {
                    return  "set the additional r g b to the diffuse light\n"
                        "    r g b - the colors";
                }

                virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
                {
                    if (tokens.size() == 3)
                    {
                        osg::Vec3 color;

                        color.x() = atof(tokens.at(0).c_str());
                        color.y() = atof(tokens.at(1).c_str());
                        color.z() = atof(tokens.at(2).c_str());

                        SkyDrawable::setAdditionalDiffuseLightConstant(color);
                    }

                    return 0;
                }
            };

            class EnableDisableForwardPlusCommand : public OpenIG::Base::Commands::Command
            {
            public:
                EnableDisableForwardPlusCommand() {}

                virtual const std::string getUsage() const
                {
                    return "enable/disable";
                }

                virtual const std::string getArgumentsFormat() const
                {
                    return "{enable;disable}";
                }

                virtual const std::string getDescription() const
                {
                    return  "enable/disable forward plus for silverlining\n"
                        "    enable/disable - the switch";
                }

                virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
                {
                    if (tokens.size() == 1)
                    {
                        bool enable = tokens.at(0) == "enable";
                        CloudsDrawable::setEnableForwardPlus(enable);
                    }

                    return 0;
                }
            };

            class GammaCommand : public OpenIG::Base::Commands::Command
            {
            public:
                GammaCommand() {}

                virtual const std::string getUsage() const
                {
                    return "gamma";
                }

                virtual const std::string getArgumentsFormat() const
                {
                    return "D";
                }

                virtual const std::string getDescription() const
                {
                    return  "set gamma for silverlining\n"
                        "    gamma - the gamma value";
                }

                virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
                {
                    if (tokens.size() == 1)
                    {
                        double gamma = atof(tokens.at(0).c_str());
                        SkyDrawable::setGamma(gamma, true);
                    }

                    return 0;
                }
            };

            class LocationCommand : public OpenIG::Base::Commands::Command
            {
            public:
                LocationCommand() {}

                virtual const std::string getUsage() const
                {
                    return "latitude longitude";
                }

                virtual const std::string getArgumentsFormat() const
                {
                    return "D:D";
                }

                virtual const std::string getDescription() const
                {
                    return  "set the geographics location\n"
                        "    latitude - in degrees\n"
                        "    longitude - in degrees";
                }

                virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
                {
                    if (tokens.size() == 2)
                    {
                        double lat = atof(tokens.at(0).c_str());
                        double lon = atof(tokens.at(1).c_str());

                        SkyDrawable::setLocation(lat, lon);
                    }

                    return 0;
                }
            };

            class UTCCommand : public OpenIG::Base::Commands::Command
            {
            public:
                UTCCommand() {}

                virtual const std::string getUsage() const
                {
                    return "hour minutes";
                }

                virtual const std::string getArgumentsFormat() const
                {
                    return "I:I";
                }

                virtual const std::string getDescription() const
                {
                    return  "set the UTC time\n"
                        "    hour - hour\n"
                        "    minutes - minutes";
                }

                virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
                {
                    if (tokens.size() == 2)
                    {
                        int hour = atoi(tokens.at(0).c_str());
                        int minutes = atoi(tokens.at(1).c_str());

                        SkyDrawable::setUTC(hour,minutes);
                    }

                    return 0;
                }
            };

            class SilverLiningCommand : public OpenIG::Base::Commands::Command
            {
            public:
                SilverLiningCommand(SilverLiningPlugin* plugin)
                    : _plugin(plugin)
                {
                }
                virtual const std::string getUsage() const
                {
                    return "mode dynamic_env_map_update [optional:gamma]";
                }

                virtual const std::string getArgumentsFormat() const
                {
                    return "{geocentric;flat}:dynamic;onchange}:D";
                }

                virtual const std::string getDescription() const
                {
                    return  "configures the SilverLining plugin\n"
                        "    mode - one of these:\n"
                        "    geocentric - if in geocentric mode\n"
                        "    flat       - if in flat earth mode\n"
                        "    dynamic_env_map_update - 'dynamic' to update the environmental\n"
                        "                             map on each frame, 'onchange' only on clouds change\n";
                        "	 gamma     - [optional] SilverLining gamma";
                }

                virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
                {
                    if (tokens.size() >= 1 && _plugin != 0)
                    {
                        std::string mode = tokens.at(0);
                        //ensure its lowercase for the string compare....
                        std::transform(mode.begin(), mode.end(), mode.begin(), ::tolower);
                        if (mode.compare(0, 10, "geocentric") == 0)
                            _plugin->setGeocentric(true);
                        else if (mode.compare(0, 4, "flat") == 0)
                            _plugin->setGeocentric(false);

                        if (tokens.size() >= 2)
                        {
                            _plugin->setDynamicEnvMapUpdate(tokens.at(1) == "dynamic");
                        }

                        if (tokens.size() >= 3)
                        {
                            SkyDrawable::setGamma(atof(tokens.at(1).c_str()),true);
                        }

                        return 0;
                    }
                    return -1;
                }

            protected:
                SilverLiningPlugin* _plugin;
            };

            class SetSkyboxSizeCommand : public OpenIG::Base::Commands::Command
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

                virtual const std::string getArgumentsFormat() const
                {
                    return "D";
                }

                virtual const std::string getDescription() const
                {
                    return  "configures the SilverLining skybox size\n"
                        "    skyboxsize - any value that encompasses your scene size\n"
                        "                 it should be just a bit larger than the farclip\n"
                        "                 value that you are using on your scene: ie: 100000.01\n";
                }

                virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
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

            class SetSilverLiningParamsCommand : public OpenIG::Base::Commands::Command
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

                virtual const std::string getArgumentsFormat() const
                {
                    return "D:D";
                }

                virtual const std::string getDescription() const
                {
                    return  "configures some SilverLining parameters\n"
                        "    cloudsBaseLength - the base length of the newely created cloud layers\n"
                        "    cloudsBaseWidth - the base width of the newely created cloud layers\n";

                }

                virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
                {
                    if (tokens.size() == 2 && _plugin != 0)
                    {
                        double cloudsBaseLength = atof(tokens.at(0).c_str());
                        double cloudsBaseWidth = atof(tokens.at(1).c_str());

                        SkyDrawable::SilverLiningParams params;
                        params.cloudsBaseLength = cloudsBaseLength;
                        params.cloudsBaseWidth = cloudsBaseWidth;

                        int mask = OpenIG::Plugins::SkyDrawable::SilverLiningParams::CLOUDS_BASE_WIDTH | OpenIG::Plugins::SkyDrawable::SilverLiningParams::CLOUDS_BASE_LENGTH;
                        _plugin->_skyDrawable->setSilverLiningParams(params, mask);

                        return 0;
                    }
                    return -1;
                }

            protected:
                SilverLiningPlugin* _plugin;
            };

            virtual void config(const std::string& fileName)
            {
                osgDB::XmlNode* root = osgDB::readXmlFile(fileName);
                if (root == 0) return;

                if (root->children.size() == 0) return;

                osgDB::XmlNode* config = root->children.at(0);
                if (config->name != "OpenIG-Plugin-Config") return;

                bool useGamma = false;

                osgDB::XmlNode::Children::iterator itr = config->children.begin();
                for (; itr != config->children.end(); ++itr)
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
                        _skyboxSize = std::strtod(child->contents.c_str(), NULL);
                        _skyboxSizeSet = false;
                    }
                    if (child->name == "SilverLining-ForwardPlusEnabled")
                    {
                        _forwardPlusEnabled = child->contents == "yes";
                    }
                    if (child->name == "SilverLining-LogZEnabled")
                    {
                        _logZEnabled = child->contents == "yes";
                    }
                    if (child->name == "SilverLining-ForwardPlusRange")
                    {
                        CloudsDrawable::setForwardPlusRange(atof(child->contents.c_str()));
                    }
                    if (child->name == "SilverLining-UserDefinedGamma")
                    {
                        useGamma = child->contents == "yes";
                    }
                    if (child->name == "SilverLining-Gamma")
                    {
                        SkyDrawable::setGamma(atof(child->contents.c_str()),useGamma);
                    }
                    if (child->name == "SilverLining-Latitude")
                    {
                        _latitude = std::strtod(child->contents.c_str(), NULL);
                    }
                    if (child->name == "SilverLining-Longitude")
                    {
                        _longitude = std::strtod(child->contents.c_str(), NULL);
                    }
                    if (child->name == "SilverLining-Timezone")
                    {
                        _tz = atoi(child->contents.c_str());
                    }
                    if (child->name == "SilverLining-SkyModel")
                    {
                        SkyDrawable::setSkyModel(child->contents);
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

            virtual void init(OpenIG::PluginBase::PluginContext& context)
            {
                OpenIG::Base::Commands::instance()->addCommand("addclouds", new AddCloudLayerCommand(context.getImageGenerator()));
                OpenIG::Base::Commands::instance()->addCommand("createcloudfile", new CreateCloudLayerFileCommand(context.getImageGenerator()));
                OpenIG::Base::Commands::instance()->addCommand("loadcloudfile", new LoadCloudLayerFileCommand(context.getImageGenerator()));
                OpenIG::Base::Commands::instance()->addCommand("updateclouds", new UpdateCloudLayerCommand(context.getImageGenerator()));
                OpenIG::Base::Commands::instance()->addCommand("enableclouds", new EnableCloudLayerCommand(context.getImageGenerator()));
                OpenIG::Base::Commands::instance()->addCommand("removeclouds", new RemoveCloudLayerCommand(context.getImageGenerator()));
                OpenIG::Base::Commands::instance()->addCommand("removecloudfile", new RemoveCloudLayerFileCommand(context.getImageGenerator()));
                OpenIG::Base::Commands::instance()->addCommand("removeallclouds", new RemoveAllCloudLayersCommand(context.getImageGenerator()));
                OpenIG::Base::Commands::instance()->addCommand("fog", new SetFogCommand(context.getImageGenerator()));
                OpenIG::Base::Commands::instance()->addCommand("rain", new RainCommand(context.getImageGenerator()));
                OpenIG::Base::Commands::instance()->addCommand("snow", new SnowCommand(context.getImageGenerator()));
                OpenIG::Base::Commands::instance()->addCommand("date", new DateCommand(context.getImageGenerator()));
                OpenIG::Base::Commands::instance()->addCommand("location", new LocationCommand);
                OpenIG::Base::Commands::instance()->addCommand("utc", new UTCCommand);
                OpenIG::Base::Commands::instance()->addCommand("silverlining", new SilverLiningCommand(this));
                OpenIG::Base::Commands::instance()->addCommand("setskyboxsize", new SetSkyboxSizeCommand(this));
                OpenIG::Base::Commands::instance()->addCommand("setsilverliningparams", new SetSilverLiningParamsCommand(this));
                OpenIG::Base::Commands::instance()->addCommand("ambient+", new AdditionalAmbientCommand);
                OpenIG::Base::Commands::instance()->addCommand("diffuse+", new AdditionalDiffuseCommand);
                OpenIG::Base::Commands::instance()->addCommand("f+", new EnableDisableForwardPlusCommand);
                OpenIG::Base::Commands::instance()->addCommand("gamma", new GammaCommand);

                initSilverLining(context.getImageGenerator());

                OpenIG::Base::AtmosphereAttributes atmosphere((void*)ar);
                context.addAttribute("SLAtmosphere", new OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::AtmosphereAttributes>(atmosphere));
            }

            virtual void update(OpenIG::PluginBase::PluginContext& context)
            {
                //initSilverLining(context.getImageGenerator());

// For DEBUG only
#if 0
                static int h = 0;
                static int m = 0;
                context.getImageGenerator()->setTimeOfDay(h, m);

                std::cout << "H:" << h << ":" << m << std::endl;
                m += 5;
                if (m == 60)
                {
                    m = 0;
                    h++;
                }
                if (h > 23) h = 0;
#endif
                // Here we check if the XML has changed. If so, reload and update
                _xmlAccessMutex.lock();
                if (_xmlLastCheckedTime != _xmlLastWriteTime)
                {
                    osg::notify(osg::NOTICE) << "SilverLining: XML updated: " << _xmlFileName << std::endl;
                    readXML(_xmlFileName);
                    _xmlLastCheckedTime = _xmlLastWriteTime;
                }
                _xmlAccessMutex.unlock();

                {
                    if (!_skyboxSizeSet)
                        setSkyboxSize();
                }
                {
                    osg::ref_ptr<osg::Referenced> ref = context.getAttribute("Date");
                    OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::DateAttributes> *attr = dynamic_cast<OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::DateAttributes> *>(ref.get());
                    if (attr && _skyDrawable.valid())
                    {
                        _skyDrawable->setDate(attr->getValue().getMonth(),attr->getValue().getDay(),attr->getValue().getYear());
                    }
                }
                {
                    osg::ref_ptr<osg::Referenced> ref = context.getAttribute("Rain");
                    OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::RainSnowAttributes> *attr = dynamic_cast<OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::RainSnowAttributes> *>(ref.get());
                    if (attr && _skyDrawable.valid())
                    {
                        _skyDrawable->setRain(attr->getValue().getFactor());
                    }
                }
                {
                    osg::ref_ptr<osg::Referenced> ref = context.getAttribute("Snow");
                    OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::RainSnowAttributes> *attr = dynamic_cast<OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::RainSnowAttributes> *>(ref.get());
                    if (attr && _skyDrawable.valid())
                    {
                        _skyDrawable->setSnow(attr->getValue().getFactor());
                    }
                }
                {
                    osg::ref_ptr<osg::Referenced> ref = context.getAttribute("Fog");
                    OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::FogAttributes> *attr = dynamic_cast<OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::FogAttributes> *>(ref.get());
                    if (attr && _skyDrawable.valid())
                    {
                        _skyDrawable->setVisibility(attr->getValue().getVisibility());
                    }
                }
                {
                    osg::ref_ptr<osg::Referenced> ref = context.getAttribute("Wind");
                    OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::WindAttributes> *attr = dynamic_cast<OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::WindAttributes> *>(ref.get());
                    if (attr && _skyDrawable.valid())
                    {
                        _skyDrawable->setWind(attr->getValue().speed, attr->getValue().direction);
                    }
                }

                {
                    osg::ref_ptr<osg::Referenced> ref = context.getAttribute("TOD");
                    OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::TimeOfDayAttributes> *attr = dynamic_cast<OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::TimeOfDayAttributes> *>(ref.get());
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

            OpenIG::PluginBase::PluginContext::AttributeMapIterator itr = context.getAttributes().begin();
            for (; itr != context.getAttributes().end(); ++itr)
            {
                osg::ref_ptr<osg::Referenced> ref = itr->second;
                OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::CLoudLayerAttributes> *attr = dynamic_cast<OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::CLoudLayerAttributes> *>(ref.get());
                osg::notify(osg::NOTICE) << "IGPluinSilverLining incoming CLoudLayerAttribute: " << itr->first << std::endl;

                // This is cleaner way of dealing with
                // PluginContext attributes but the Mac
                // compiler doesn't like it. It works ok
                // on Linux though
                // osg::ref_ptr<OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::CLoudLayerAttributes> > attr = itr->second;

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
                            osg::notify(osg::NOTICE) << "IGPluinSilverLining cloud layer is dirty, id: " << attr->getValue().getId() << std::endl;

                            switch (attr->getValue().getAddFlag())
                            {
                            case true:
                                osg::notify(osg::NOTICE) << "IGPluinSilverLining add cloud layer: " << attr->getValue().getId() << std::endl;
                                _skyDrawable->addCloudLayer(
                                    attr->getValue().getId(),
                                    attr->getValue().getType(),
                                    attr->getValue().getAltitude(),
                                    attr->getValue().getThickness(),
                                    attr->getValue().getDensity(),
                                    attr->getValue().getEnable());
                                _cloudsDrawable->setEnvironmentMapDirty(true);
                                break;
                            }

                            switch (attr->getValue().getRemoveFlag())
                            {
                            case true:
                                osg::notify(osg::NOTICE) << "IGPluinSilverLining remove cloud layer: " << attr->getValue().getId() << std::endl;
                                _skyDrawable->removeCloudLayer(attr->getValue().getId());
                                _cloudsDrawable->setEnvironmentMapDirty(true);
                                break;
                            }

                            if (!attr->getValue().getAddFlag() && !attr->getValue().getRemoveFlag())
                            {
                                osg::notify(osg::NOTICE) << "IGPluinSilverLining update cloud layer: " << attr->getValue().getId() << std::endl;
                                _skyDrawable->updateCloudLayer(
                                    attr->getValue().getId(),
                                    attr->getValue().getAltitude(),
                                    attr->getValue().getThickness(),
                                    attr->getValue().getDensity());
                                _cloudsDrawable->setEnvironmentMapDirty(true);
                            }
                        }
                    }

                    OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::CLoudLayerAttributes> *enableAttr = dynamic_cast<OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::CLoudLayerAttributes> *>(ref.get());
                    if(enableAttr && itr->first == "EnableCloudLayer" && _skyDrawable.valid())
                    {
                        osg::notify(osg::NOTICE) << "slpluin::update -- EnableCloudLayer(" << enableAttr->getValue().getEnableFlag()<< ")" << std::endl;

                        _skyDrawable->enableCloudLayer(
                                    enableAttr->getValue().getId(),
                                    enableAttr->getValue().getEnableFlag());
                    }

                    OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::CLoudLayerFileAttributes> *fileattr = dynamic_cast<OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::CLoudLayerFileAttributes> *>(ref.get());
                    if(fileattr && itr->first == "CloudLayerFile" && _skyDrawable.valid())
                    {
                        osg::notify(osg::NOTICE) << "slpluin::update -- CloudLayerFile( " << fileattr->getValue().getId();
                        if(fileattr->getValue().getFilename().size())
                            osg::notify(osg::NOTICE) << ", file: " << fileattr->getValue().getFilename() << ")" << std::endl;
                        else
                            osg::notify(osg::NOTICE) << ")" << std::endl;

                        //Are we going to create a new cloud layer file
                        if(fileattr->getValue().getAddFlag())
                        {
                            _skyDrawable->addCloudLayerFile(
                                fileattr->getValue().getId(),
                                fileattr->getValue().getType(),
                                fileattr->getValue().getAltitude(),
                                fileattr->getValue().getThickness(),
                                fileattr->getValue().getDensity(),
                                fileattr->getValue().getEnable(),
                                fileattr->getValue().getFilename());

                        }
                        //Are we going to remove a cloud layer file
                        else if(fileattr->getValue().getRemoveFlag())
                        {
                            _skyDrawable->removeCloudLayerFile(fileattr->getValue().getId());
                        }
                        //otherwise we are just going to load a previously created and saved cloud layer file...
                        else
                        {
                            _skyDrawable->loadCloudLayer(
                                        fileattr->getValue().getId(),
                                        fileattr->getValue().getType(),
                                        fileattr->getValue().getFilename());
                            _cloudsDrawable->setEnvironmentMapDirty(true);
                        }
                    }

                }

                {
                    //Make sure we can access our imagegenerator object within our skyDrawable class...
                    OpenIG::Engine* ig = dynamic_cast<OpenIG::Engine*>(context.getImageGenerator());
                    if (ig && this->_skyDrawable)
                        this->_skyDrawable->setIG(ig);
                }

                // Instantiate an Atmosphere and associate it with this camera. If you have multiple cameras
                // in multiple contexts, be sure to instantiate seperate Atmosphere objects for each.
                // Remember to delete this object at shutdown.
                if (!ar) ar = new AtmosphereReference;
                if (!ar->atmosphere)
                {
                    SilverLining::Atmosphere *atm = new SilverLining::Atmosphere(_userName.c_str(), _key.c_str());
                    ar->atmosphere = atm;
                }

                osgViewer::ViewerBase::Views views;
                context.getImageGenerator()->getViewer()->getViews(views);

                osgViewer::ViewerBase::Views::iterator vitr = views.begin();
                for (; vitr != views.end(); ++vitr)
                {
                    osgViewer::View* view = *vitr;

                    bool openIGScene = false;
                    if (view->getUserValue("OpenIG-Scene", openIGScene) && openIGScene)
                    {
                        if (!view->getCamera()->getUserData())
                            view->getCamera()->setUserData(ar);
                    }
                }

                if (_updateEnvMapDynamically)
                {
                    CloudsDrawable::setEnvironmentMapDirty(true);
                }
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

                OpenIG::Base::Commands::instance()->removeCommand("silverlining");
                OpenIG::Base::Commands::instance()->removeCommand("setskyboxsize");
                OpenIG::Base::Commands::instance()->removeCommand("setsilverliningparams");
                OpenIG::Base::Commands::instance()->removeCommand("addclouds");
                OpenIG::Base::Commands::instance()->removeCommand("updateclouds");
                OpenIG::Base::Commands::instance()->removeCommand("removeclouds");
                OpenIG::Base::Commands::instance()->removeCommand("removeallclouds");
                OpenIG::Base::Commands::instance()->removeCommand("fog");
                OpenIG::Base::Commands::instance()->removeCommand("rain");
                OpenIG::Base::Commands::instance()->removeCommand("snow");

                if (_cloudsGeode->getNumParents() != 0)
                {
                    _skyGeode->getParent(0)->removeChild(_skyGeode);
                    _cloudsGeode->getParent(0)->removeChild(_cloudsGeode);
                }

                _skyGeode = NULL;
                _cloudsGeode = NULL;

                _skyDrawable = NULL;
                _cloudsDrawable = NULL;

                osgViewer::CompositeViewer* viewer = context.getImageGenerator()->getViewer();

                osgViewer::ViewerBase::Views views;
                viewer->getViews(views);

                osgViewer::ViewerBase::Views::iterator itr = views.begin();
                for (; itr != views.end(); ++itr)
                {
                    osgViewer::View* view = *itr;

                    bool openIGScene = false;
                    if (view->getUserValue("OpenIG-Scene", openIGScene) && openIGScene)
                    {
                        AtmosphereReference *ar = dynamic_cast<AtmosphereReference*>(view->getCamera()->getUserData());
                        if (ar && ar->atmosphere)
                        {
                            if (ar->referenceCount() == 1)
                            {
                                delete ar->atmosphere;
                            }
                        }
                        view->getCamera()->setUserData(0);
                    }
                }
            }

            void readXML(const std::string& xmlFileName)
            {
                osg::notify(osg::NOTICE) << "SilverLining: parsing xml file: " << xmlFileName << std::endl;

                osgDB::XmlNode* root = osgDB::readXmlFile(xmlFileName);
                if (!root || !root->children.size() || root->children.at(0)->name != "OsgNodeSettings") return;

                osgDB::XmlNode::Children::iterator itr = root->children.at(0)->children.begin();
                for (; itr != root->children.at(0)->children.end(); ++itr)
                {
                    osgDB::XmlNode* child = *itr;

                    //<SunMoonBrightness day="1.0" night="0.5"/>
                    if (child->name == "SunMoonBrightness")
                    {
                        osgDB::XmlNode::Properties::iterator pitr = child->properties.begin();
                        for (; pitr != child->properties.end(); ++pitr)
                        {
                            if (pitr->first == "day")
                            {
                                _sunMoonBrightness_day = atof(pitr->second.c_str());
                            }
                            if (pitr->first == "night")
                            {
                                _sunMoonBrightness_night = atof(pitr->second.c_str());
                            }
                        }
                    }
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
                        _xmlLastWriteTime = _xmlLastWriteTime = OpenIG::Base::FileSystem::lastWriteTime(_xmlFileName);
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

                _xmlFileObserverThread = boost::shared_ptr<boost::thread>(new boost::thread(&OpenIG::Plugins::SilverLiningPlugin::xmlFileObserverThread, this));


                readXML(_xmlFileName = xmlFile);
            }


        protected:
            std::string                     _userName;
            std::string                     _key;
            std::string                     _path;
            osg::ref_ptr<SkyDrawable>       _skyDrawable;
            osg::ref_ptr<CloudsDrawable>    _cloudsDrawable;
            AtmosphereReference             *ar;
            bool							_geocentric;
            bool							_forwardPlusEnabled;
            bool							_logZEnabled;
            bool							_lightBrightness_enable;
            float							_lightBrightness_day;
            float							_lightBrightness_night;
            double                          _skyboxSize;
            double                          _latitude;
            double                          _longitude;
            int                             _tz;
            bool                            _skyboxSizeSet;
            std::string						_xmlFileName;
            float							_sunMoonBrightness_day;
            float							_sunMoonBrightness_night;
            bool							_updateEnvMapDynamically;
            osg::ref_ptr<osg::Geode>		_skyGeode;
            osg::ref_ptr<osg::Geode>		_cloudsGeode;

            boost::shared_ptr<boost::thread>	_xmlFileObserverThread;
            volatile bool						_xmlThreadIsRunning;
            volatile bool						_xmlThreadRunningCondition;
            boost::mutex						_xmlAccessMutex;
            std::time_t							_xmlLastWriteTime;
            std::time_t							_xmlLastCheckedTime;


            const EnvMapUpdater* initSilverLining(OpenIG::Base::ImageGenerator* ig)
            {
                if (_cloudsDrawable.valid() && _skyDrawable.valid())
                {
                    if (_cloudsGeode->getNumParents() == 0 && ig->getScene() && ig->getScene()->asGroup())
                    {
                        ig->getScene()->asGroup()->addChild(_skyGeode);
                        ig->getScene()->asGroup()->addChild(_cloudsGeode);

                        _skyDrawable->setLight(ig->getSunOrMoonLight());
                        _skyDrawable->setFog(ig->getFog());
                    }
                    return _cloudsDrawable.get();
                }

                if (ig == 0) return 0;

                osgViewer::CompositeViewer* viewer = ig->getViewer();

                osg::LightSource* sunOrMoonLight = ig->getSunOrMoonLight();
                osg::Fog* fog = ig->getFog();

                if (viewer == 0)
                    return 0;

                if (viewer->getNumViews() == 0)
                    return 0;

                // Instantiate an Atmosphere and associate it with this camera. If you have multiple cameras
                // in multiple contexts, be sure to instantiate seperate Atmosphere objects for each.
                // Remember to delete this object at shutdown.
                SilverLining::Atmosphere *atm = new SilverLining::Atmosphere(_userName.c_str(), _key.c_str());

                ar = new AtmosphereReference;
                ar->atmosphere = atm;

                osgViewer::ViewerBase::Views views;
                viewer->getViews(views);

                osgViewer::ViewerBase::Views::iterator itr = views.begin();
                for (; itr != views.end(); ++itr)
                {
                    osgViewer::View* view = *itr;

                    bool openIGScene = false;
                    if (view->getUserValue("OpenIG-Scene", openIGScene) && openIGScene)
                    {
                        view->getCamera()->setUserData(ar);
                        // No need for OSG to clear the color buffer, the sky will fill it for you.
                        view->getCamera()->setClearMask(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                        // configure the near/far so we don't clip things that are up close
                        view->getCamera()->setNearFarRatio(0.00002);
                    }

                }

                // Add the sky (calls Atmosphere::DrawSky and handles initialization once you're in
                // the rendering thread)
                _skyGeode = new osg::Geode;
                _skyDrawable = new SkyDrawable(_path, viewer, sunOrMoonLight ? sunOrMoonLight->getLight() : 0, fog, _geocentric);

                // ***IMPORTANT!**** Check that the path to the resources folder for SilverLining in SkyDrawable.cpp
                // SkyDrawable::initializeSilverLining matches with where you installed SilverLining.

                _skyGeode->addDrawable(_skyDrawable);
                _skyGeode->setCullingActive(false); // The skybox is always visible.

                _skyGeode->getOrCreateStateSet()->setRenderBinDetails(SKY_RENDER_BIN, "RenderBin");
                //skyGeode->getOrCreateStateSet()->setAttributeAndModes( new osg::Depth( osg::Depth::LEQUAL, 0.0, 1.0, false ) );

                // Add the clouds (note, you need this even if you don't have clouds in your scene - it calls
                // Atmosphere::DrawObjects() which also draws precipitation, lens flare, etc.)
                _cloudsGeode = new osg::Geode;
                _cloudsDrawable = new CloudsDrawable(viewer, ig, _forwardPlusEnabled, _logZEnabled);
                _cloudsGeode->addDrawable(_cloudsDrawable);
                _cloudsGeode->getOrCreateStateSet()->setRenderBinDetails(CLOUDS_RENDER_BIN, "DepthSortedBin");
                _cloudsGeode->setCullingActive(false);

                // Add our sky and clouds into the scene.
                if (ig->getScene() && ig->getScene()->asGroup())
                {
                    ig->getScene()->asGroup()->addChild(_skyGeode);
                    ig->getScene()->asGroup()->addChild(_cloudsGeode);
                }

                _cloudsDrawable->setLightingBrightness(
                    _lightBrightness_enable,
                    _lightBrightness_day,
                    _lightBrightness_night
                    );

                if (_skyDrawable.valid())
                {
                    _skyboxSizeSet = true;
                    _skyDrawable->setSkyboxSize(_skyboxSize);
                    _skyDrawable->setLocation(_latitude, _longitude);
                    _skyDrawable->setTimezone(_tz);
                }

                return _cloudsDrawable.get();
            }

            class AddCloudLayerCommand : public OpenIG::Base::Commands::Command
            {
            public:
                AddCloudLayerCommand(OpenIG::Base::ImageGenerator* ig)
                    : _ig(ig) {}

                virtual const std::string getUsage() const
                {
                    return "id type altitude thickness density";
                }

                virtual const std::string getArgumentsFormat() const
                {
                    return	"I:{CIRROCUMULUS;CIRRUS_FIBRATUS;STRATUS;CUMULUS_MEDIOCRIS;CUMULUS_CONGESTUS;CUMULUS_CONGESTUS_HI_RES;"
                        "CUMULONIMBUS_CAPPILATUS;STRATOCUMULUS;STRATOCUMULUS_PARTICLES;TOWERING_CUMULUS;SANDSTORM}:D:D:D";
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
                        "          8 - STRATOCUMULUS_PARTICLES\n"
                        "          9 - TOWERING_CUMULUS\n"
                        "         10 - SANDSTORM\n"
                        "     altitude  - in feet, the base altitude of the layer\n"
                        "     thickness - in feet, the thickness of the layer, \n"
                        "                 or random offset of base altitude for clouds, depending on cloud type\n"
                        "     density - from 0.0-1.0, 1.0 most dense\n"
                        "     enable    - This is an OPTIONAL flag, default is set to true, but it can be set to false\n"
                        "                 in cases where you want the layer created but not visible when its loaded into,\n"
                        "                 the SL atmosphere.  When it is finally enabled it will show immediately\n"
                        "                 with no delay to seed it or for the atmosphere to load it.  Good for changing cloud\n"
                        "                 types when do delay is expected, like the A320 I/ITSEC demo for instance\n";
                }

                virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
                {
                    if (tokens.size() == 6)
                    {
                        bool enable = true;
                        unsigned int id = atoi(tokens.at(0).c_str());
                        int type = atoi(tokens.at(1).c_str());
                        double altitude = atof(tokens.at(2).c_str());// * Base::Math::instance()->M_PER_FT;
                        double thickness = atof(tokens.at(3).c_str());// * Base::Math::instance()->M_PER_FT;
                        double density = atof(tokens.at(4).c_str());
                        std::string _sEnable = tokens.at(5);
                        if (_sEnable.compare(0, 5, "false") == 0)
                            enable = false;

                        _ig->addCloudLayer(id, type, altitude, thickness, density, enable);

                        return 0;
                    }
                    else if (tokens.size() == 5)
                    {
                        unsigned int id = atoi(tokens.at(0).c_str());
                        int type = atoi(tokens.at(1).c_str());
                        double altitude = atof(tokens.at(2).c_str());// * Base::Math::instance()->M_PER_FT;
                        double thickness = atof(tokens.at(3).c_str());// * Base::Math::instance()->M_PER_FT;
                        double density = atof(tokens.at(4).c_str());

                        _ig->addCloudLayer(id, type, altitude, thickness, density);

                        return 0;
                    }
                    return -1;
                }

            protected:
                OpenIG::Base::ImageGenerator* _ig;
            };

            class CreateCloudLayerFileCommand : public OpenIG::Base::Commands::Command
            {
            public:
                CreateCloudLayerFileCommand(OpenIG::Base::ImageGenerator* ig)
                    : _ig(ig) {}

                virtual const std::string getUsage() const
                {
                    return "id type altitude thickness density filename";
                }

                virtual const std::string getArgumentsFormat() const
                {
                    return	"I:{CIRROCUMULUS;CIRRUS_FIBRATUS;STRATUS;CUMULUS_MEDIOCRIS;CUMULUS_CONGESTUS;CUMULUS_CONGESTUS_HI_RES;"
                        "CUMULONIMBUS_CAPPILATUS;STRATOCUMULUS;STRATOCUMULUS_PARTICLES;TOWERING_CUMULUS;SANDSTORM}:D:D:D:S";
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
                        "          8 - STRATOCUMULUS_PARTICLES\n"
                        "          9 - TOWERING_CUMULUS\n"
                        "         10 - SANDSTORM\n"
                        "     altitude  - in feet, the base altitude of the layer\n"
                        "     thickness - in feet, the thickness of the layer, \n"
                        "                 or random offset of base altitude for clouds, depending on cloud type\n"
                        "     density   - from 0.0-1.0, 1.0 most dense\n"
                        "     enable    - This is an OPTIONAL flag, default is set to true, but it can be set to false\n"
                        "                 in cases where you want the layer created but not visible when its loaded into,\n"
                        "                 the SL atmosphere.  When it is finally enabled it will show immediately\n"
                        "                 with no delay to seed it or for the atmosphere to load it.  Good for changing cloud\n"
                        "                 types when do delay is expected, like the A320 I/ITSEC demo for instance\n"
                        "     filename  - full path where the newly created cloud layer file will be saved";
                }

                virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
                {
                    if (tokens.size() == 7)
                    {
                        bool enable = true;
                        unsigned int id = atoi(tokens.at(0).c_str());
                        int type = atoi(tokens.at(1).c_str());
                        double altitude = atof(tokens.at(2).c_str());// * Base::Math::instance()->M_PER_FT;
                        double thickness = atof(tokens.at(3).c_str());// * Base::Math::instance()->M_PER_FT;
                        double density = atof(tokens.at(4).c_str());
                        std::string _sEnable = tokens.at(5);
                        if (_sEnable.compare(0, 5, "false") == 0)
                            enable = false;
                        std::string filename = tokens.at(6);

                        _ig->createCloudLayerFile(id, type, altitude, thickness, density, enable, filename);

                        return 0;
                    }
                    else if (tokens.size() == 6)
                    {
                        unsigned int id = atoi(tokens.at(0).c_str());
                        int type = atoi(tokens.at(1).c_str());
                        double altitude = atof(tokens.at(2).c_str());// * Base::Math::instance()->M_PER_FT;
                        double thickness = atof(tokens.at(3).c_str());// * Base::Math::instance()->M_PER_FT;
                        double density = atof(tokens.at(4).c_str());
                        std::string filename = tokens.at(5);

                        bool enable = true;
                        _ig->createCloudLayerFile(id, type, altitude, thickness, density, enable, filename);

                        return 0;
                    }
                    return -1;
                }

            protected:
                OpenIG::Base::ImageGenerator* _ig;
            };

            class LoadCloudLayerFileCommand : public OpenIG::Base::Commands::Command
            {
            public:
                LoadCloudLayerFileCommand(OpenIG::Base::ImageGenerator* ig)
                    : _ig(ig) {}

                virtual const std::string getUsage() const
                {
                    return "id type filename";
                }

                virtual const std::string getArgumentsFormat() const
                {
                    return	"I:{CIRROCUMULUS;CIRRUS_FIBRATUS;STRATUS;CUMULUS_MEDIOCRIS;CUMULUS_CONGESTUS;CUMULUS_CONGESTUS_HI_RES;"
                            "CUMULONIMBUS_CAPPILATUS;STRATOCUMULUS;STRATOCUMULUS_PARTICLES;TOWERING_CUMULUS;SANDSTORM}:S";
                }

                virtual const std::string getDescription() const
                {
                    return  "load a previously saved cloud layer file -- based on Sundog's SilverLining plugin\n"
                        "       id - the id of the cloud latyer across the scene\n"
                        " filename - the complete path to the filename of the cloudlayer file to load\n"
                        "     type - of layer loaded, from the available cloud types from SilverLining, from 0 to 9 respoding to:\n"
                        "          0 - CIRROCUMULUS\n"
                        "          1 - CIRRUS_FIBRATUS\n"
                        "          2 - STRATUS\n"
                        "          3 - CUMULUS_MEDIOCRIS\n"
                        "          4 - CUMULUS_CONGESTUS\n"
                        "          5 - CUMULUS_CONGESTUS_HI_RES\n"
                        "          6 - CUMULONIMBUS_CAPPILATUS\n"
                        "          7 - STRATOCUMULUS\n"
                        "          8 - STRATOCUMULUS_PARTICLES\n"
                        "          9 - TOWERING_CUMULUS\n"
                        "         10 - SANDSTORM\n";
                }

                virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
                {
                    if (tokens.size() == 3)
                    {
                        unsigned int id      = atoi(tokens.at(0).c_str());
                        int type             = atoi(tokens.at(1).c_str());
                        std::string filename = tokens.at(2).c_str();

                        _ig->loadCloudLayerFile(id, type, filename);

                        return 0;
                    }
                    return -1;
                }

            protected:
                OpenIG::Base::ImageGenerator* _ig;
            };

            class UpdateCloudLayerCommand : public OpenIG::Base::Commands::Command
            {
            public:
                UpdateCloudLayerCommand(OpenIG::Base::ImageGenerator* ig)
                    : _ig(ig) {}

                virtual const std::string getUsage() const
                {
                    return "id altitude thickness density";
                }

                virtual const std::string getArgumentsFormat() const
                {
                    return	"I:{CIRROCUMULUS;CIRRUS_FIBRATUS;STRATUS;CUMULUS_MEDIOCRIS;CUMULUS_CONGESTUS;CUMULUS_CONGESTUS_HI_RES;"
                        "CUMULONIMBUS_CAPPILATUS;STRATOCUMULUS;TOWERING_CUMULUS;SANDSTORM}:D:D:D";
                }

                virtual const std::string getDescription() const
                {
                    return  "updates clouds layer settings -- based on Sundog's SilverLining plugin\n"
                        "     id - the id of the cloud latyer across the scene\n"
                        "     altitude  - in feet, the altitude of the layer\n"
                        "     thickness - in feet, the thickness of the layer\n"
                        "     density - from 0.0-1.0, 1.0 most dense\n";
                }

                virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
                {
                    if (tokens.size() == 4)
                    {
                        unsigned int id = atoi(tokens.at(0).c_str());
                        double altitude = atof(tokens.at(1).c_str());
                        double thickness = atof(tokens.at(2).c_str());
                        double density = atof(tokens.at(3).c_str());

                        _ig->updateCloudLayer(id, altitude, thickness, density);

                        return 0;
                    }
                    return -1;
                }

            protected:
                OpenIG::Base::ImageGenerator* _ig;
            };

            class EnableCloudLayerCommand : public OpenIG::Base::Commands::Command
            {
            public:
                EnableCloudLayerCommand(OpenIG::Base::ImageGenerator* ig)
                    : _ig(ig) {}

                virtual const std::string getUsage() const
                {
                    return "id enable";
                }

                virtual const std::string getArgumentsFormat() const
                {
                    return	"I:S";
                }

                virtual const std::string getDescription() const
                {
                    return  "enables the cloud layer in the scene using the cloud layer id\n"
                        "     id - the id of the cloud layer\n"
                        " enable - This is an OPTIONAL flag, default is set to true, but it can be set to false\n"
                        "          in cases where you want the layer created but not visible when its loaded into,\n"
                        "          the SL atmosphere.  When it is finally enabled it will show immediately\n"
                        "          with no delay to seed it or for the atmosphere to load it.  Good for changing cloud\n"
                        "          types when do delay is expected, like the A320 I/ITSEC demo for instance\n";
                }

                virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
                {
                    if (tokens.size() == 2)
                    {
                        bool enable = true;
                        unsigned int id = atoi(tokens.at(0).c_str());
                        std::string _senable = tokens.at(1);

                        if (_senable.compare(0,5, "false") == 0)
                            enable = false;

                        _ig->enableCloudLayer(id, enable);

                        return 0;
                    }
                    return -1;
                }

            protected:
                OpenIG::Base::ImageGenerator* _ig;
            };

            class RemoveCloudLayerCommand : public OpenIG::Base::Commands::Command
            {
            public:
                RemoveCloudLayerCommand(OpenIG::Base::ImageGenerator* ig)
                    : _ig(ig) {}

                virtual const std::string getUsage() const
                {
                    return "id";
                }

                virtual const std::string getArgumentsFormat() const
                {
                    return	"I";
                }

                virtual const std::string getDescription() const
                {
                    return  "removes the cloud layer from the scene by a given cloud layer id\n"
                        "     id - the id of the cloud layer";
                }

                virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
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
                OpenIG::Base::ImageGenerator* _ig;
            };

            class RemoveCloudLayerFileCommand : public OpenIG::Base::Commands::Command
            {
            public:
                RemoveCloudLayerFileCommand(OpenIG::Base::ImageGenerator* ig)
                    : _ig(ig) {}

                virtual const std::string getUsage() const
                {
                    return "id";
                }

                virtual const std::string getArgumentsFormat() const
                {
                    return	"I";
                }

                virtual const std::string getDescription() const
                {
                    return  "Removes the cloud layer file from the scene as well\n"
                            "as the associated saved cloud file, by a given cloud layer id\n\n"
                            "id - the id of the cloud layer";
                }

                virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
                {
                    if (tokens.size() == 1)
                    {
                        unsigned int id = atoi(tokens.at(0).c_str());

                        _ig->removeCloudLayerFile(id);

                        return 0;
                    }
                    return -1;
                }

            protected:
                OpenIG::Base::ImageGenerator* _ig;
            };

            class RemoveAllCloudLayersCommand : public OpenIG::Base::Commands::Command
            {
            public:
                RemoveAllCloudLayersCommand(OpenIG::Base::ImageGenerator* ig)
                    : _ig(ig) {}

                virtual const std::string getUsage() const
                {
                    return "(no attributes)";
                }

                virtual const std::string getArgumentsFormat() const
                {
                    return	"";
                }


                virtual const std::string getDescription() const
                {
                    return "removes all the cloud layers from the scene";
                }

                virtual int exec(const OpenIG::Base::StringUtils::Tokens&)
                {
                    _ig->removeAllCloudlayers();

                    return 0;
                }

            protected:
                OpenIG::Base::ImageGenerator* _ig;
            };

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
                    return	"D";
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
                    return	"D";
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
                    return	"D";
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

            class DateCommand : public OpenIG::Base::Commands::Command
            {
            public:
                DateCommand(OpenIG::Base::ImageGenerator* ig)
                    : _ig(ig) {}

                virtual const std::string getUsage() const
                {
                    return "simulation date";
                }

                virtual const std::string getArgumentsFormat() const
                {
                    return	"I:I:I";
                }

                virtual const std::string getDescription() const
                {
                    return  "set simulation date\n"
                        "    month day year"
                        "    date 05 06 2016";
                }

                virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
                {
                    if (tokens.size() == 3)
                    {
                        int month = atoi(tokens.at(0).c_str());
                        int day = atoi(tokens.at(1).c_str());
                        int year = atoi(tokens.at(2).c_str());

                        osg::notify(osg::NOTICE) << "DateCommand  year: " << year << std::endl;
                        osg::notify(osg::NOTICE) << "DateCommand month: " << month << std::endl;
                        osg::notify(osg::NOTICE) << "DateCommand   day: " << day << std::endl;

                        _ig->setDate(month, day, year);

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
    return new OpenIG::Plugins::SilverLiningPlugin;
}

extern "C" EXPORT void DeletePlugin(OpenIG::PluginBase::Plugin* plugin)
{
    osg::ref_ptr<OpenIG::PluginBase::Plugin> p(plugin);
}
