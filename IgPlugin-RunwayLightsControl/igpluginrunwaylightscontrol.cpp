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
#include <IgCore/commands.h>
#include <IgCore/stringutils.h>

#include <IgPluginCore/plugin.h>
#include <IgPluginCore/plugincontext.h>


#include <osgDB/ReadFile>
#include <osgDB/XmlParser>
#include <osgDB/FileUtils>

#include <osgSim/LightPointNode>

#include <osg/Texture2D>
#include <osg/ValueObject>

namespace igplugins
{

class RunwayLightsControlPlugin;

class RunwayControlCommand : public igcore::Commands::Command
{
public:
    RunwayControlCommand( RunwayLightsControlPlugin* plugin, igcore::ImageGenerator* ig )
        : _plugin(plugin)
        , _ig(ig)
    {

    }

    virtual int exec(const igcore::StringUtils::Tokens& tokens);

    virtual const std::string getUsage() const
    {
        return "id command";
    }
    virtual const std::string getDescription() const
    {
        return  "applies command on database runways by their xml defition files\n"
                "     id - the id of the entity across the scene containing the runways\n"
                "     command - one of these: update";
    }

protected:
    RunwayLightsControlPlugin*  _plugin;
    igcore::ImageGenerator*     _ig;

    RunwayControlCommand() {}
};


class RunwayLightsControlPlugin : public igplugincore::Plugin
{

friend class RunwayControlCommand;

public:

    RunwayLightsControlPlugin()
        : _timeOfDay_on(19)
        , _timeOfDay_off(7)
        , _landingLightBrightness_enable(true)
        , _landingLightBrightness_day(0.05f)
        , _landingLightBrightness_night(5.f)
        , _currentTimeOfDay_hour(0)
        , _ig(0)
    {

    }

    virtual std::string getName() { return "RunwayLightsControl"; }

    virtual std::string getDescription( ) { return "Controls the runway lights on airport models in the database from a XML file and custom commands"; }

    virtual std::string getVersion() { return "1.0.0"; }

    virtual std::string getAuthor() { return "ComPro, Nick"; }

    virtual void databaseRead(const std::string& fileName, osg::Node*, const osgDB::Options*)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

        std::string xmlFile = fileName + ".lighting.xml";
        if (!osgDB::fileExists(xmlFile))
        {
            osg::notify(osg::NOTICE) << "Runway control: xml file does not exists: " << xmlFile << std::endl;
            return;
        }

        osgDB::XmlNode* root = osgDB::readXmlFile(xmlFile);
        if (!root)
        {
            osg::notify(osg::NOTICE) << "Runway control: NULL root: " << xmlFile << std::endl;
            return;
        }
        if (!root->children.size())
        {
            osg::notify(osg::NOTICE) << "Runway control: root with no children: " << xmlFile << std::endl;
            return;
        }
        if (root->children.at(0)->name != "OsgNodeSettings")
        {
            osg::notify(osg::NOTICE) << "Runway control: OsgNodeSettings tag not found: " << xmlFile << std::endl;
            return;
        }

        _currentFileName = fileName;
        osg::notify(osg::NOTICE) << "Runway control: current file: " << _currentFileName << std::endl;

        LightPointSystemDefinitions& lpsdefs = _lpdefs[fileName];

        osgDB::XmlNode::Children::iterator itr = root->children.at(0)->children.begin();
        for ( ; itr != root->children.at(0)->children.end(); ++itr )
        {
            osgDB::XmlNode* child = *itr;

            // <TimeofDay on="19.0" off="7.0" />
            if (child->name == "TimeofDay")
            {
                osgDB::XmlNode::Properties::iterator pitr = child->properties.begin();
                for ( ; pitr != child->properties.end(); ++pitr )
                {
                    if (pitr->first == "on")
                    {
                        _timeOfDay_on = atoi(pitr->second.c_str());
                    }
                    if (pitr->first == "off")
                    {
                        _timeOfDay_off = atoi(pitr->second.c_str());
                    }
                }
            }
            //<LandingLightBrightness  enable="true" day="0.05" night="5"/>
            if (child->name == "LandingLightBrightness")
            {
                osgDB::XmlNode::Properties::iterator pitr = child->properties.begin();
                for ( ; pitr != child->properties.end(); ++pitr )
                {
                    if (pitr->first == "enable")
                    {
                        _landingLightBrightness_enable = pitr->second == "true";
                    }
                    if (pitr->first == "day")
                    {
                        _landingLightBrightness_day = atof(pitr->second.c_str());
                    }
                    if (pitr->first == "night")
                    {
                        _landingLightBrightness_night = atof(pitr->second.c_str());
                    }
                }
            }
            //<LightBrightnessOnClouds day="0.01" night=".1" />
            if (child->name == "LightBrightnessOnClouds")
            {
                osgDB::XmlNode::Properties::iterator pitr = child->properties.begin();
                for ( ; pitr != child->properties.end(); ++pitr )
                {
                    if (pitr->first == "")
                    {

                    }
                }
            }
            //<LightBrightnessOnWater  day="0.01" night=".1" />
            if (child->name == "LightBrightnessOnWater")
            {
                osgDB::XmlNode::Properties::iterator pitr = child->properties.begin();
                for ( ; pitr != child->properties.end(); ++pitr )
                {
                    if (pitr->first == "")
                    {

                    }
                }
            }
            //<MultiSwitchNames ActiveRunway="ACTIVE_"/>
            if (child->name == "MultiSwitchNames")
            {
                osgDB::XmlNode::Properties::iterator pitr = child->properties.begin();
                for ( ; pitr != child->properties.end(); ++pitr )
                {
                    if (pitr->first == "")
                    {

                    }
                }
            }
            //<LightPointNode name="lp"
            //                always_on="false"
            //                minPixelSize="1"
            //                maxPixelSize="1"
            //                intensity="1"
            //                radius="5"
            //                sprites="false"
            //                texture="Images/particle.rgb"/>
            if (child->name == "LightPointNode")
            {
                LightPointSystemDefinition  def;
                std::string                 name;

                osgDB::XmlNode::Properties::iterator pitr = child->properties.begin();
                for ( ; pitr != child->properties.end(); ++pitr )
                {
                    if (pitr->first == "name")
                    {
                        name = pitr->second;
                    }
                    if (pitr->first == "always_on")
                    {
                        def._always_on = pitr->second == "true";
                    }
                    if (pitr->first == "minPixelSize")
                    {
                        def._minPixelSize = atof(pitr->second.c_str());
                    }
                    if (pitr->first == "maxPixelSize")
                    {
                        def._maxPixelSize = atof(pitr->second.c_str());
                    }
                    if (pitr->first == "intensity")
                    {
                        def._intensity = atof(pitr->second.c_str());
                    }
                    if (pitr->first == "radius")
                    {
                        def._radius = atof(pitr->second.c_str());
                    }
                    if (pitr->first == "sprites")
                    {
                        def._sprites = pitr->second == "true";
                    }
                    if (pitr->first == "texture")
                    {
                        def._texture = pitr->second;

                        osg::ref_ptr<osg::Texture2D> texture;
#if 1

                        SpriteTextureCache::iterator itr = _textureCache.find(def._texture);
                        if (itr != _textureCache.end())
                            texture = itr->second;
                        else
                        {
                            texture = new osg::Texture2D;
                            _textureCache[def._texture] = texture;

                            osg::ref_ptr<osg::Image> image;

                            SpriteImageCache::iterator iitr = _imageCache.find(def._texture);
                            if (iitr != _imageCache.end())
                                image = iitr->second;
                            else
                            {
                                image = osgDB::readImageFile(def._texture);

                                if (!image.valid()) osg::notify(osg::NOTICE) << "Runway control: WARNING, sprite image not found: " << def._texture << std::endl;
                                _imageCache[def._texture] = image;
                            }
                            texture->setImage(image);
                        }
#endif
                        def._spriteTexture = texture;

                    }
                }

                lpsdefs[name] = def;
            }

        }
    }

    void setupLightPointNode( osgSim::LightPointNode& lpn, const std::string& fileName )
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

        LightPointSystemDefinitions& defs = _lpdefs[fileName];

        LightPointSystemDefinitions::iterator itr = defs.begin();
        for ( ; itr != defs.end(); ++itr )
        {
            LightPointSystemDefinition& def = itr->second;
            std::string                 name = itr->first;

            if (lpn.getName().substr(0,osg::minimum(name.size(),lpn.getName().size())) == name )
            {
                lpn.setMinPixelSize(def._minPixelSize);
                lpn.setMaxPixelSize(def._maxPixelSize);
                lpn.setPointSprite(def._sprites);
                if (def._sprites)
                {
                    lpn.getOrCreateStateSet()->setTextureAttributeAndModes(0,def._spriteTexture,
                        osg::StateAttribute::ON|osg::StateAttribute::PROTECTED);
                }
                else
                {
                    lpn.getOrCreateStateSet()->setTextureAttributeAndModes(0,0);
                }

                bool lightsOn = true;
                if (_currentTimeOfDay_hour < _timeOfDay_on && _currentTimeOfDay_hour > _timeOfDay_off)
                {
                    lightsOn = false;
                }

                for (size_t i = 0; i < lpn.getNumLightPoints(); ++i)
                {
                    osgSim::LightPoint& lp = lpn.getLightPoint(i);
                    lp._intensity = def._intensity;
                    lp._radius = def._radius;
                    lp._on = def._always_on ? true : lightsOn;
                }

                std::string ids;
                lpn.getUserValue("Real-Lights-IDs",ids);
                if (!ids.empty() && _ig)
                {
                    igcore::StringUtils::Tokens tokens = igcore::StringUtils::instance()->tokenize(ids,";");
                    igcore::StringUtils::Tokens::iterator itr = tokens.begin();
                    for ( ; itr != tokens.end(); ++itr )
                    {
                        unsigned int id = atoi((*itr).c_str());
                        if (id)
                        {
                            _ig->enableLight(id,def._always_on ? true : lightsOn);
                        }
                    }
                }
            }
        }
    }

    virtual void databaseReadInVisitorBeforeTraverse(osg::Node& node, const osgDB::Options*)
    {
        osg::observer_ptr<osgSim::LightPointNode> lpn = dynamic_cast<osgSim::LightPointNode*>(&node);
        if (lpn.valid())
        {
            LPNodes& lpnodes = _lpnodesMap[_currentFileName];
            lpnodes.push_back(lpn);

            setupLightPointNode(*lpn,_currentFileName);

            //osg::notify(osg::NOTICE) << "Runway control: LightPointNode found: " << lpn->getName() << std::endl;
        }
    }

    virtual void preFrame(igplugincore::PluginContext& context, double)
    {
        osg::ref_ptr<osg::Referenced> ref = context.getAttribute("TOD");
        igplugincore::PluginContext::Attribute<igcore::TimeOfDayAttributes> *attr = dynamic_cast<igplugincore::PluginContext::Attribute<igcore::TimeOfDayAttributes> *>(ref.get());
        if (attr)
        {
            _currentTimeOfDay_hour = attr->getValue().getHour();

            osg::notify(osg::NOTICE) << "Runway control: hour set to: " << _currentTimeOfDay_hour << std::endl;

            LPNodesMap::iterator itr = _lpnodesMap.begin();
            for ( ; itr != _lpnodesMap.end(); ++itr )
            {
                osg::notify(osg::NOTICE) << "Runway control: adjusting light points for : " << itr->first << std::endl;

                LPNodes& lpnodes = itr->second;

                osg::notify(osg::NOTICE) << "\t adjusting num of lights: " << lpnodes.size() << std::endl;

                for (size_t i = 0; i < lpnodes.size(); ++i )
                {
                    osg::observer_ptr<osgSim::LightPointNode>& lpn = lpnodes.at(i);

                    //osg::notify(osg::NOTICE) << "\t " << i << ":" << (lpn.valid() ? lpn->getName() : "NULL") << std::endl;

                    if ( lpn.valid() )
                    {
                        bool lightsOn = true;
                        if (_currentTimeOfDay_hour < _timeOfDay_on && _currentTimeOfDay_hour > _timeOfDay_off)
                        {
                            lightsOn = false;
                        }

                        LightPointSystemDefinitions& defs = _lpdefs[_currentFileName];

                        LightPointSystemDefinitions::iterator itr = defs.begin();
                        for ( ; itr != defs.end(); ++itr )
                        {
                            LightPointSystemDefinition& def = itr->second;
                            std::string                 name = itr->first;

                            if (lpn->getName().substr(0,osg::minimum(name.size(),lpn->getName().size())) == name )
                            {
                                if (def._always_on)
                                {
                                    lightsOn = true;
                                }
                            }
                        }

                        for (size_t j = 0; j < lpn->getNumLightPoints(); ++j )
                        {
                            osgSim::LightPoint& lp = lpn->getLightPoint(j);
                            lp._on = lightsOn;

                            //osg::notify(osg::NOTICE) << "Runway Control: " << lpn->getName() << ":" << j << std::endl;
                        }

                        std::string ids;
                        lpn->getUserValue("Real-Lights-IDs",ids);
                        if (!ids.empty() && _ig)
                        {
                            igcore::StringUtils::Tokens tokens = igcore::StringUtils::instance()->tokenize(ids,";");
                            igcore::StringUtils::Tokens::iterator itr = tokens.begin();
                            for ( ; itr != tokens.end(); ++itr )
                            {
                                unsigned int id = atoi((*itr).c_str());
                                if (id)
                                {
                                    _ig->enableLight(id,lightsOn);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    virtual void init(igplugincore::PluginContext& context)
    {
        igcore::Commands::instance()->addCommand("runway", new RunwayControlCommand(this,context.getImageGenerator()));

        _ig = context.getImageGenerator();
    }

protected:
    struct LightPointSystemDefinition
    {
        bool                            _always_on;
        float                           _minPixelSize;
        float                           _maxPixelSize;
        float                           _intensity;
        float                           _radius;
        bool                            _sprites;
        std::string                     _texture;
        osg::ref_ptr<osg::Texture2D>    _spriteTexture;

        LightPointSystemDefinition()
            : _always_on(true)
            , _minPixelSize(1.f)
            , _maxPixelSize(1.f)
            , _intensity(1.f)
            , _radius(1.f)
            , _sprites(false)
        {

        }
    };

    typedef std::map< std::string, LightPointSystemDefinition >             LightPointSystemDefinitions;
    typedef std::vector< osg::observer_ptr<osgSim::LightPointNode> >        LPNodes;
    typedef std::map< std::string, LPNodes >                                LPNodesMap;
    typedef std::map< std::string, LightPointSystemDefinitions >            LPDefsPerFile;
    typedef std::map< std::string, osg::ref_ptr<osg::Image> >               SpriteImageCache;
    typedef std::map< std::string, osg::ref_ptr<osg::Texture2D> >           SpriteTextureCache;

    LPDefsPerFile           _lpdefs;
    LPNodesMap              _lpnodesMap;
    SpriteImageCache        _imageCache;
    SpriteTextureCache      _textureCache;
    unsigned int            _timeOfDay_on;
    unsigned int            _timeOfDay_off;
    bool                    _landingLightBrightness_enable;
    float                   _landingLightBrightness_day;
    float                   _landingLightBrightness_night;
    std::string             _currentFileName;
    unsigned int            _currentTimeOfDay_hour;
    OpenThreads::Mutex      _mutex;
    igcore::ImageGenerator* _ig;
};

int RunwayControlCommand::exec(const igcore::StringUtils::Tokens& tokens)
{
    if (tokens.size()==2 && _ig && _plugin)
    {
        unsigned int    id = atoi(tokens.at(0).c_str());
        std::string     command = tokens.at(1);

        if (command == "update")
        {
            if (_ig->getEntityMap().count(id))
            {
                igcore::ImageGenerator::Entity entity = _ig->getEntityMap()[id];

                std::string name;
                entity->getUserValue("fileName",name);

                _plugin->databaseRead(name,0,0);

                RunwayLightsControlPlugin::LPNodesMap::iterator itr = _plugin->_lpnodesMap.find(name);
                if ( itr != _plugin->_lpnodesMap.end())
                {
                    osg::notify(osg::NOTICE) << "Runway control command: updating " << name << std::endl;

                    RunwayLightsControlPlugin::LPNodes& lpnodes = itr->second;
                    for (size_t i = 0; i < lpnodes.size(); ++i)
                    {
                        osg::observer_ptr<osgSim::LightPointNode>& lpn = lpnodes.at(i);
                        if (lpn.valid())
                        {
                            _plugin->setupLightPointNode(*lpn,name);
                            //osg::notify(osg::NOTICE) << "Runway control command: updating light " << lpn->getName() << std::endl;
                        }
                    }
                }

                return 0;
            }
        }
    }

    return -1;
}

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
    return new igplugins::RunwayLightsControlPlugin;
}

extern "C" EXPORT void DeletePlugin(igplugincore::Plugin* plugin)
{
    osg::ref_ptr<igplugincore::Plugin> p(plugin);
}
