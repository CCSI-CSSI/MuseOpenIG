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
// note: This is experimental, and it is valid
// with the datanase that is provided by ComPro
// for the OpenIG Demo. Will be revisited soonest
// to make it available as standard plugin to
// OpenIG. Nick

#include <Core-PluginBase/Plugin.h>
#include <Core-PluginBase/PluginContext.h>
#include <Core-PluginBase/PluginHost.h>

#include <Core-Base/Types.h>
#include <Core-Base/IDPool.h>
#include <Core-Base/Mathematics.h>
#include <Core-Base/Commands.h>
#include <Core-Base/FileSystem.h>
#include <Core-Base/Configuration.h>

#include <Core-Utils/TextureCache.h>

#include <Core-OpenIG/RenderBins.h>
#include <Core-OpenIG/Engine.h>

#include <osg/Node>
#include <osg/ValueObject>
#include <osg/Texture2D>

#include <osgSim/LightPointNode>
#include <osgSim/MultiSwitch>

#include <osgDB/XmlParser>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>

#include <OpenThreads/Mutex>
#include <OpenThreads/ScopedLock>
#include <OpenThreads/Thread>

#include <iostream>
#include <sstream>
#include <ctime>

#include <boost/thread.hpp>
#include <boost/filesystem.hpp>

using namespace osg;


namespace OpenIG { namespace Plugins {
  
  static const std::string tilesWildcardMatchAll = ".*";
  
  // This is for debugging/statistics purpose only
  struct LightPointNodeStats
  {
    std::string	name;
    size_t		numOfLightPoints;
    LightPointNodeStats() : numOfLightPoints(0) {}
  };
  typedef std::vector<LightPointNodeStats>	LightPointsStats;
  
  class LightsControlPlugin : public OpenIG::PluginBase::Plugin
  {
  public:
    
    LightsControlPlugin()
    : _ig(0)
    , _xmlThreadIsRunning(false)
    , _xmlThreadRunningCondition(false)
    , _xmlLastCheckedTime(0)
    {
      std::string resourcePath = OpenIG::Base::FileSystem::path(OpenIG::Base::FileSystem::Resources, "../resources");
      _textureCache.addPath(resourcePath);
    }
    
    virtual std::string getName() { return "LightsControl"; }
    
    virtual std::string getDescription() { return "Replace the lightpoints from a visual database with real lights and manages their paging"; }
    
    virtual std::string getVersion() { return "2.0.0"; }
    
    virtual std::string getAuthor() { return "ComPro, Nick"; }
    
    
    // We manage the MultiSwitches by a command. Obviously
    // a runway model can have many versions and from the
    // OpenFlight world these are being converted into
    // MultiSwitches by the osg flt plugin
    typedef std::map< std::string, std::vector<unsigned> >				MultiSwitches;
    MultiSwitches _multiSwitches;
    
    // This was introduced to set the active switch set
    // for a MultiSwitch and it has priority over the
    // setting the active children in the ValueList
    typedef std::map< std::string, int >								ActiveSwitchSets;
    ActiveSwitchSets _activeSwtichSets;
    
    // We traverse the active/inactive child of the MultiSwitches and enable/disable
    // lights accordingly. We use NodeVisitor for that
    struct UpdateMSLightsNodeVisitor : public osg::NodeVisitor
    {
      UpdateMSLightsNodeVisitor(bool enable, OpenIG::Base::ImageGenerator* ig)
      : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN), _enable(enable), _ig(ig) {}
      
      virtual void apply(osg::Node& node)
      {
        osg::ref_ptr<osgSim::LightPointNode> lpn = dynamic_cast<osgSim::LightPointNode*>(&node);
        if (lpn.valid())
        {
          bool always_on = false;
          lpn->getUserValue("always_on", always_on);
          
          std::string ids;
          if (lpn->getUserValue("Real-Lights-IDs", ids) && !ids.empty())
          {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(OpenIG::Plugins::LightsControlPlugin::lightMutex);
            
            OpenIG::Base::StringUtils::Tokens tokens = OpenIG::Base::StringUtils::instance()->tokenize(ids, ";");
            OpenIG::Base::StringUtils::TokensIterator titr = tokens.begin();
            for (; titr != tokens.end(); ++titr)
            {
              const std::string& id = *titr;
              
              unsigned int lightId = atoi(id.c_str());
              if (lightId)
              {
                _ig->enableLight(lightId, always_on ? true : _enable);
              }
            }
          }
          for (size_t i = 0; i < lpn->getNumLightPoints(); ++i)
          {
            osgSim::LightPoint& lp = lpn->getLightPoint(i);
            lp._on = _enable;
          }
          
        }
        else
          traverse(node);
      }
    protected:
      bool					_enable;
      OpenIG::Base::ImageGenerator*	_ig;
    };
    
  public:
    // Time of day. We use this to read in from the lighting.xml
    // to turn on/off lights based on time
    struct TOD
    {
    protected:
      unsigned	_hour;
      unsigned	_minutes;
      bool		_set;
    public:
      TOD() : _hour(0), _minutes(0), _set(false) {}
      
      unsigned& hour() { _set = true;  return _hour; }
      unsigned& minutes() { _set = true; return _minutes; }
      bool& set() { return _set; }
    };
    
    // This command is to modify the brightness of lights
    // by a multiplier based on the light name from the XML
    class LightPointsBrightnessCommand : public OpenIG::Base::Commands::Command
    {
    public:
      LightPointsBrightnessCommand(OpenIG::Plugins::LightsControlPlugin* plugin)
      : _plugin(plugin)
      {
      }
      
      virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
      {
        if (tokens.size() == 2)
        {					
          std::string name = tokens.at(0);
          float		multiplier = atof(tokens.at(1).c_str());
          
          OpenIG::Plugins::LightsControlPlugin::LightPointBrightness::iterator itr = OpenIG::Plugins::LightsControlPlugin::lightPointBrightness.find(name);
          if (itr != OpenIG::Plugins::LightsControlPlugin::lightPointBrightness.end())
          {
            OpenIG::Plugins::LightsControlPlugin::lightPointBrightness[name] = multiplier;
            _plugin->updateLightPointsBrighntess(name);
          }
          
          return 0;
        }
        
        return -1;
      }
      
      virtual const std::string getUsage() const
      {
        return "lightpointsname multiplier";
      }
      virtual const std::string getArgumentsFormat() const
      {
        return "S:D";
      }
      virtual const std::string getDescription() const
      {
        return  "modifies the light points brightness for XML registered lights\n"
        "     lightpointsname - the light points name (per the XML)\n"
        "     multiplier      - factor to multiply the light points brighntess";
      }
    protected:
      OpenIG::Plugins::LightsControlPlugin* _plugin;
    };
    
    // This is the custom command. On execution it will
    // launch a NodeVisitor to find the MultiSwicth nodes
    // based on their pattern and change the Active Switch
    // Set based on the ValueLists
    class MultiSwitchExtendedCommand : public OpenIG::Base::Commands::Command
    {
    public:
      
      struct UpdateMultiSwitchSetActiveSwitchSetNodeVisitor : public osg::NodeVisitor
      {
        UpdateMultiSwitchSetActiveSwitchSetNodeVisitor(const std::string& name, int index, ActiveSwitchSets& mss, OpenIG::Base::ImageGenerator* ig, bool enabledByTOD)
        : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
        , _msName(name), _index(index), _mss(mss), _ig(ig), _enabledByTOD(enabledByTOD)
        {
          _mss[_msName] = _index;
        }
        
        virtual void apply(osg::Node& node)
        {
          osg::ref_ptr<osgSim::MultiSwitch> ms = dynamic_cast<osgSim::MultiSwitch*>(&node);
          if (ms.valid())
          {
            if (ms->getName().substr(0, osg::minimum(ms->getName().length(), _msName.length())) == _msName)
            {
              if (_index > -1 && _index < (int)ms->getSwitchSetList().size())
              {
                osgSim::MultiSwitch::ValueList vl = ms->getValueList(_index);
                
                for (size_t i = 0; i < vl.size(); ++i)
                {
                  bool active = vl.at(i);
                  
                  UpdateMSLightsNodeVisitor nv(_enabledByTOD ? active : false, _ig);
                  ms->getChild(i)->accept(nv);
                }
                
                ms->setActiveSwitchSet(_index);
                
                osg::notify(osg::NOTICE) << "LightsControl: MSExtended " << _msName << " active switch set set to: " << _index << " { ";
                for (size_t i = 0; i < vl.size(); ++i)
                {
                  osg::notify(osg::NOTICE) << vl.at(i) << " ";
                }
                osg::notify(osg::NOTICE) << "}" << std::endl;
              }
              
              return;
            }
          }
          
          traverse(node);
        }
      protected:
        const std::string						_msName;
        int										_index;
        ActiveSwitchSets&						_mss;
        OpenIG::Base::ImageGenerator*			_ig;
        bool									_enabledByTOD;
      };
      
      MultiSwitchExtendedCommand(OpenIG::Base::ImageGenerator* ig, ActiveSwitchSets& mss)
      : _ig(ig), _mss(mss), _lightsEnabled(true)
      {
      }
      
      virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
      {
        if (tokens.size() == 2)
        {
          std::string     name = tokens.at(0);
          int				index = atoi(tokens.at(1).c_str());
          
          if (LightsControlPlugin::tod.set() && LightsControlPlugin::onTOD.set() && LightsControlPlugin::offTOD.set())
          {
            _lightsEnabled =
            LightsControlPlugin::tod.hour() >= LightsControlPlugin::onTOD.hour() ||
            LightsControlPlugin::tod.hour() <= LightsControlPlugin::offTOD.hour();
          }
          
          UpdateMultiSwitchSetActiveSwitchSetNodeVisitor nv(name, index, _mss, _ig, _lightsEnabled);
          _ig->getScene()->accept(nv);
          
          return 0;
        }
        
        return -1;
      }
      
      virtual const std::string getUsage() const
      {
        return "multiswitch_namepattern index";
      }
      virtual const std::string getArgumentsFormat() const
      {
        return "S:I";
      }
      virtual const std::string getDescription() const
      {
        return  "find the multiswitch in the database based on a name pattern and set the active switch set\n"
        "     multiswitch_namepattern - part or whole name of the MultiSwitch\n"
        "     index- index of the switchset to set it as active";
      }
      
    protected:
      OpenIG::Base::ImageGenerator*	_ig;
      ActiveSwitchSets&				_mss;
      bool							_lightsEnabled;
    };
    
    // This is the custom command. On execution it will
    // launch a NodeVisitor to find the MultiSwicth nodes
    // based on their pattern and change the child
    class MultiSwitchCommand : public OpenIG::Base::Commands::Command
    {
    public:
      MultiSwitchCommand(OpenIG::Base::ImageGenerator* ig, MultiSwitches& mss)
      : _ig(ig), _mss(mss), _lightsEnabled(true)
      {
      }
      
      // The NodeVisitor to find MultiSwitches and select
      // the proper child
      struct UpdateMultiSwitchNodeVisitor : public osg::NodeVisitor
      {
        UpdateMultiSwitchNodeVisitor(const std::string& name, const std::vector<unsigned>& childs, MultiSwitches& mss, OpenIG::Base::ImageGenerator* ig, bool enabledByTOD)
        : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
        , _msName(name), _msChild(childs), _mss(mss), _ig(ig), _enabledByTOD(enabledByTOD)
        {
          // we update here the plugin map
          _mss[_msName] = _msChild;
        }
        
        virtual void apply(osg::Node& node)
        {
          osg::ref_ptr<osgSim::MultiSwitch> ms = dynamic_cast<osgSim::MultiSwitch*>(&node);
          if (ms.valid())
          {
            if (ms->getName().substr(0, osg::minimum(ms->getName().length(), _msName.length())) == _msName)
            {
              {
                osgSim::MultiSwitch::ValueList vl;
                for (size_t i = 0; i < ms->getNumChildren(); ++i)
                {
                  bool childEnabled = false;
                  for (size_t j = 0; j < _msChild.size(); ++j)
                  {
                    if (_msChild.at(j) == i)
                    {
                      childEnabled = true;
                      break;
                    }
                  }
                  vl.push_back(childEnabled);
                  
                  UpdateMSLightsNodeVisitor nv((_enabledByTOD ? childEnabled : false), _ig);
                  ms->getChild(i)->accept(nv);
                }
                ms->setValueList(0, vl);
                ms->setActiveSwitchSet(0);
                
                // probably we print out something
                osg::notify(osg::NOTICE) << "LightsControl: MS " << _msName << " set to ";
                for (size_t i = 0; i < vl.size(); ++i)
                {
                  osg::notify(osg::NOTICE) << vl.at(i) << " ";
                }
                osg::notify(osg::NOTICE) << std::endl;
                
                return;
              }
            }
          }
          traverse(node);
        }
      protected:
        const std::string						_msName;
        const std::vector<unsigned>				_msChild;
        MultiSwitches&							_mss;
        OpenIG::Base::ImageGenerator*			_ig;
        bool									_enabledByTOD;
      };
      
      virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
      {
        if (tokens.size() >= 1)
        {
          std::string     name = tokens.at(0);
          
          std::vector<unsigned> childs;
          for (size_t i = 1; i < tokens.size(); ++i)
          {
            unsigned int    child = atoi(tokens.at(i).c_str());
            childs.push_back(child);
          }
          
          if (LightsControlPlugin::tod.set() && LightsControlPlugin::onTOD.set() && LightsControlPlugin::offTOD.set())
          {
            _lightsEnabled =
            LightsControlPlugin::tod.hour() >= LightsControlPlugin::onTOD.hour() ||
            LightsControlPlugin::tod.hour() <= LightsControlPlugin::offTOD.hour();
          }
          
          UpdateMultiSwitchNodeVisitor nv(name, childs, _mss, _ig, _lightsEnabled);
          _ig->getScene()->accept(nv);
          
          return 0;
        }
        
        return -1;
      }
      
      virtual const std::string getUsage() const
      {
        return "multiswitch_namepattern [optional:child1] [optional:child2] ...";
      }
      virtual const std::string getArgumentsFormat() const
      {
        return "S:S";
      }
      virtual const std::string getDescription() const
      {
        return  "find the multiswitch in the database based on a name pattern and select the child\n"
        "     multiswitch_namepattern - part or whole name of the MultiSwitch\n"
        "     child1, child2 ... - index to make it on. If no optional childs added they are all\n"
        "     going to be set to off";
      }
      
    protected:
      OpenIG::Base::ImageGenerator*     _ig;
      MultiSwitches&				_mss;
      bool						_lightsEnabled;
    };
    
    // We save the reference to the IG for later use here
    virtual void init(OpenIG::PluginBase::PluginContext& context)
    {
      // Some defaults here
      LightsControlPlugin::tod.hour() = 4;
      LightsControlPlugin::onTOD.hour() = 17;
      LightsControlPlugin::offTOD.hour() = 6;
      
      // Save for later
      _ig = context.getImageGenerator();
      
      // Our custom command for multiswictes
      OpenIG::Base::Commands::instance()->addCommand("ms", new MultiSwitchCommand(_ig,_multiSwitches));
      OpenIG::Base::Commands::instance()->addCommand("mse", new MultiSwitchExtendedCommand(_ig, _activeSwtichSets));
      OpenIG::Base::Commands::instance()->addCommand("lpb", new LightPointsBrightnessCommand(this));
      
      // Let find if we have foward+ available
      // if not, no sprites either, till fixed
      OpenIG::Plugins::LightsControlPlugin::forwardPlusPluginAvailable = false;
      
      // Look up for the F+ plugin
      OpenIG::Engine* openIG = dynamic_cast<OpenIG::Engine*>(_ig);
      if (openIG)
      {
        const OpenIG::PluginBase::PluginHost::PluginsMap& plugins = openIG->getPlugins();
        OpenIG::PluginBase::PluginHost::PluginsMap::const_iterator itr = plugins.begin();
        for (; itr != plugins.end(); ++itr)
        {
          if (itr->second->getName() == "ForwardPlusLighting")
          {
            OpenIG::Plugins::LightsControlPlugin::forwardPlusPluginAvailable = true;
            break;
          }
        }
      }
      
    }
    
    // More on multiswitches. If the ms command is used in
    // configuration file, then we have to adjust the new
    // paged in tiles containing MS based on the settings
    // of the command. The command is saving the state of
    // a named MS in a map
    virtual void databaseReadInVisitorBeforeTraverse(osg::Node& node, const osgDB::Options*)
    {
      osg::ref_ptr<osgSim::MultiSwitch> ms = dynamic_cast<osgSim::MultiSwitch*>(&node);
      if (ms.valid())
      {
        bool processed = false;
        if (ms->getUserValue("processed", processed) && processed) return;
        
        processed = true;
        ms->setUserValue("processed", processed);
        
        bool todEnabled = true;
        if (LightsControlPlugin::tod.set() && LightsControlPlugin::offTOD.set() && LightsControlPlugin::onTOD.set())
        {
          todEnabled =
          LightsControlPlugin::tod.hour() >= LightsControlPlugin::onTOD.hour() ||
          LightsControlPlugin::tod.hour() <= LightsControlPlugin::offTOD.hour();
        }
        
        
        for (ActiveSwitchSets::iterator itr = _activeSwtichSets.begin();
             itr != _activeSwtichSets.end();
        ++itr)
             {
               const std::string& name = itr->first;
               if (ms->getName().substr(0, osg::minimum(name.length(), ms->getName().length())) == name)
               {
                 int activeSwitchSet = -1;
                 ActiveSwitchSets::iterator aitr = _activeSwtichSets.find(name);
                 if (aitr != _activeSwtichSets.end())
                 {
                   activeSwitchSet = aitr->second;
                 }
                 else
                 {
                   _activeSwtichSets[name] = -1;
                 }
                 if (activeSwitchSet != -1)
                 {
                   osgSim::MultiSwitch::ValueList vl = ms->getValueList(activeSwitchSet);
                   
                   for (size_t i = 0; i < vl.size(); ++i)
                   {
                     bool active = vl.at(i);
                     
                     UpdateMSLightsNodeVisitor nv(todEnabled ? active : false, _ig);
                     ms->getChild(i)->accept(nv);
                   }
                   
                   ms->setActiveSwitchSet(activeSwitchSet);
                   
                   osg::notify(osg::NOTICE) << "LightsControl: MSExtended " << name << " active switch set set to: " << activeSwitchSet << " { ";
                   for (size_t i = 0; i < vl.size(); ++i)
                   {
                     osg::notify(osg::NOTICE) << vl.at(i) << " ";
                   }
                   osg::notify(osg::NOTICE) << "}" << std::endl;
                 }
               }
             }
             
             // now we go through the map and look for some
             // names we have processed already
             for (MultiSwitches::iterator itr = _multiSwitches.begin();
                  itr != _multiSwitches.end();
             ++itr)
                  {
                    bool handled = false;
                    if (ms->getUserValue("handled", handled) && handled)
                    {
                      continue;
                    }
                    
                    const std::string& name = itr->first;
                    if (ms->getName().substr(0, osg::minimum(name.length(), ms->getName().length())) == name)
                    {
                      std::vector<unsigned>& childs = itr->second;
                      osgSim::MultiSwitch::ValueList vl;
                      for (size_t i = 0; i < ms->getNumChildren(); ++i)
                      {
                        bool childEnabled = false;
                        for (size_t j = 0; j < childs.size(); ++j)
                        {
                          if (i == childs.at(j))
                          {
                            childEnabled = true;
                            break;
                          }
                        }
                        vl.push_back(childEnabled);
                        
                        UpdateMSLightsNodeVisitor nv(todEnabled ? childEnabled : false, _ig);
                        ms->getChild(i)->accept(nv);
                      }
                      
                      ms->setValueList(0, vl);
                      ms->setActiveSwitchSet(0);
                      
                      osg::notify(osg::NOTICE) << "LightsControl: MS " << name << "(" << (long long)ms.get() << ")" << " set to: ";
                      for (size_t i = 0; i < vl.size(); ++i)
                      {
                        osg::notify(osg::NOTICE) << vl.at(i) << " ";
                      }
                      osg::notify(osg::NOTICE) << std::endl;
                      
                      ms->setUserValue("handled", (bool)true);
                      
                      break;
                    }
                  }
      }
    }
    
    
    // Referenced pointer type for LightPointNodes
    typedef osg::ref_ptr<osgSim::LightPointNode>					LightPointNodePointer;
    
    // Observer pointer type for PagedLODs
    typedef osg::observer_ptr<osg::PagedLOD>						PagedLODObserverPointer;
    
    // We map PagedLOD to list of LightPointNodes
    typedef std::vector<LightPointNodePointer>						LightPointNodeList;
    typedef std::map<PagedLODObserverPointer, LightPointNodeList>	PagedLODWithLightPointNodeListMap;
    
    // Then we have here a node visitor. This NodeVistor when a tile is loaded
    // will traverse all the tile and will populate the map we keep, it will
    // fill the list of LPNs for a specific PagedLOD. Later when we page out
    // this tile, we release the imagegenerator::lights for reuse by new tiles
    struct FindPagedLODsNodeVisitor: public osg::NodeVisitor
    {
      // We go look in the tile for LPNs
      struct FindLightPointNodesNodeVistor : public osg::NodeVisitor
      {
        FindLightPointNodesNodeVistor(osg::PagedLOD* plod, PagedLODWithLightPointNodeListMap& map, OpenThreads::Mutex& mutex)
        : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN), _plod(plod), _map(map), _mutex(mutex) {}
        
        virtual void apply(osg::Node& node)
        {
          // In quad tree structure we can have nested
          // PagedLODs. We want only these LPNs that are
          // attached to the given PagedLOD. The nested
          // ones will have their own
          osg::ref_ptr<osg::PagedLOD> plod = dynamic_cast<osg::PagedLOD*>(&node);
          if (plod.valid()) return;
          
          LightPointNodePointer lpn = dynamic_cast<osgSim::LightPointNode*>(&node);
          if (lpn.valid())
          {
            if (_plod.valid())
            {
              OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
              LightPointNodeList& list = _map[_plod];
              
              NameBasedLPMap::iterator itr = _nbmap.find(lpn->getName());
              if (itr == _nbmap.end())
              {
                list.push_back(lpn);
                _nbmap[lpn->getName()] = lpn;
              }
            }
          }
          
          traverse(node);
        }
        
        PagedLODObserverPointer				_plod;
        PagedLODWithLightPointNodeListMap&	_map;
        OpenThreads::Mutex&					_mutex;
        
        typedef std::map<std::string, LightPointNodePointer >	NameBasedLPMap;
        NameBasedLPMap						_nbmap;
      };
      // This NodeVistitor will update the MultiSwitches lighs
      // based on the active switch
      struct UpdateLightsInMultiSwitchNodeVisitor : public osg::NodeVisitor
      {
        UpdateLightsInMultiSwitchNodeVisitor(OpenIG::Base::ImageGenerator* ig)
        : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN), _ig(ig)
        {
          _todEnabled = true;
          if (LightsControlPlugin::tod.set() && LightsControlPlugin::offTOD.set() && LightsControlPlugin::onTOD.set())
          {
            _todEnabled =
            LightsControlPlugin::tod.hour() >= LightsControlPlugin::onTOD.hour() ||
            LightsControlPlugin::tod.hour() <= LightsControlPlugin::offTOD.hour();
          }
        }
        
        virtual void apply(osg::Node& node)
        {
          osg::ref_ptr<osgSim::MultiSwitch> ms = dynamic_cast<osgSim::MultiSwitch*>(&node);
          if (ms.valid())
          {
            unsigned activeSwitchSet = 0;// ms->getActiveSwitchSet();
            const osgSim::MultiSwitch::SwitchSetList& ssl = ms->getSwitchSetList();
            if (activeSwitchSet < ssl.size())
            {
              const osgSim::MultiSwitch::ValueList& vl = ms->getValueList(activeSwitchSet);
              for (size_t i = 0; i < vl.size(); ++i)
              {
                UpdateMSLightsNodeVisitor nv(_todEnabled ? vl.at(i) : false, _ig);
                ms->getChild(i)->accept(nv);
              }
            }
          }
          traverse(node);
        }
        
      protected:
        OpenIG::Base::ImageGenerator* _ig;
        bool					_todEnabled;
      };
      
      // This will be used for blinking sequences
      struct BlinkUpdateCallback : public osg::NodeCallback
      {
        BlinkUpdateCallback(OpenIG::Base::ImageGenerator* ig)
        : _imageGenerator(ig), _simulationTime(0.0), _simulationTimeInterval(0.0) {}
        
        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
        {
          LightPointNodeObserverList::iterator itr = _lpns.begin();
          for (; itr != _lpns.end(); ++itr)
          {
            LightPointNodeObserverPointer& lpn = *itr;
            if (!lpn.valid()) continue;
            
            if (_simulationTime == 0.0)
            {
              _simulationTime = nv->getFrameStamp()->getSimulationTime();
              _simulationTimeInterval = 0.0;
            }
            double time = nv->getFrameStamp()->getSimulationTime();
            
            _simulationTimeInterval = osg::clampAbove(time - _simulationTime, 0.0);
            _simulationTime = time;
            
            std::string ids;
            if (lpn->getUserValue("Real-Lights-IDs", ids) && !ids.empty())
            {
              OpenIG::Base::StringUtils::Tokens tokens = OpenIG::Base::StringUtils::instance()->tokenize(ids, ";");
              for (size_t i = 0; i < tokens.size(); ++i)
              {
                unsigned int id = atoi(tokens.at(i).c_str());
                if (id == 0) continue;
                if (i >= lpn->getNumLightPoints()) break;
                if (!lpn->getLightPoint(i)._blinkSequence.valid()) continue;
                
                osg::Vec4 bs = lpn->getLightPoint(i)._blinkSequence->color(
                  _simulationTime,
                  _simulationTimeInterval);
                
                osg::Vec4 color = lpn->getLightPoint(i)._color;
                
                color[0] *= bs[0];
                color[1] *= bs[1];
                color[2] *= bs[2];
                color[3] *= bs[3];
                
                OpenIG::Base::LightAttributes la;
                la.diffuse = color;
                la.brightness = 3;
                la.dirtyMask = OpenIG::Base::LightAttributes::DIFFUSE;
                
                _imageGenerator->updateLightAttributes(id, la);
              }
            }
          }
        }
        
        void addLightPointNode(LightPointNodePointer& lpn)
        {
          _lpns.push_back(lpn);
        }
        
      protected:
        OpenIG::Base::ImageGenerator* _imageGenerator;
        double                  _simulationTime;
        double                  _simulationTimeInterval;
        
        typedef osg::observer_ptr<osgSim::LightPointNode>	LightPointNodeObserverPointer;
        typedef std::vector<LightPointNodeObserverPointer>	LightPointNodeObserverList;
        LightPointNodeObserverList _lpns;
      };
      
      // Now we need an UpdateCallback for the PagedLOD that will
      // monitor the paging.
      struct PagedLODPagingObserverNodeCallback : public osg::NodeCallback
      {
        PagedLODPagingObserverNodeCallback(PagedLODWithLightPointNodeListMap& map,OpenThreads::Mutex& mutex,OpenIG::Base::ImageGenerator* ig)
        : _map(map), _mutex(mutex), _numChildrenCheck(0), _ig(ig)
        {
        }
        
        bool isFPlusLight(LightPointNodePointer& lpn)
        {
          LightsControlPlugin::LightPointDefinitions::iterator itr = LightsControlPlugin::definitions.begin();
          for (; itr != LightsControlPlugin::definitions.end(); ++itr)
          {
            if (lpn->getName().substr(0, osg::minimum(lpn->getName().length(), itr->second.name.length())) == itr->second.name)
            {
              bool fplus = false;
              lpn->getUserValue("f+", fplus);
              
              return fplus;
            }
          }
          return false;
        }
        
        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
        {
          osg::ref_ptr<osg::PagedLOD> plod = dynamic_cast<osg::PagedLOD*>(node);
          if (!plod.valid()) return;
          
          osg::ref_ptr<osg::NodeCallback> callback = dynamic_cast<osg::NodeCallback*>(plod->getUserData());
          if (callback.valid())
          {
            callback->operator()(plod, nv);
          }
          
          // Some change happened. Either it is being paged in or out
          if (_numChildrenCheck != plod->getNumChildren())
          {
            // if we have new children in,
            // we traverse them and collect all the lightpoints
            if (plod->getNumChildren())
            {
              // Probably we reset these before new light search
              LightPointNodeList& lights = _map[plod];
              lights.clear();
              
              // Let find the lights
              FindLightPointNodesNodeVistor nv(plod, _map, _mutex);
              
              // Just a hack to avoid parsing
              // nested PagedLODs.
              for (size_t i = 0; i < plod->getNumChildren(); ++i)
              {
                plod->getChild(i)->accept(nv);
              }
              
              OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
              
              // We record for debug purposes how many
              // light have been created
              std::vector<unsigned> lightsAllocated;
              
              // Record the non=F+ lights as well
              std::vector<unsigned> nonFPlusLights;
              
              LightPointsStats lpnStats;
              
              // Here we convert the osgSim::LightPointNodes from the visual database
              // into imagegenerator lights whatever their implementations is - obviously
              // might come from a Lighting plugin like Forward+ so they become real
              // physics based lights
              PagedLODWithLightPointNodeListMap::iterator lpn_iter = _map.find(plod);
              if (lpn_iter != _map.end() && lpn_iter->second.size())
              {
                LightPointNodeList& lights = lpn_iter->second;
                LightPointNodeList::iterator light_iter = lights.begin();
                for (; light_iter != lights.end(); ++light_iter)
                {
                  LightPointNodePointer& lpn = *light_iter;
                  if (!lpn.valid()) continue;
                  
                  // Do our stats
                  LightPointNodeStats stats;
                  stats.name = lpn->getName();
                  stats.numOfLightPoints = lpn->getNumLightPoints();
                  
                  lpnStats.push_back(stats);
                  
                  if (!isFPlusLight(lpn))
                  {
                    nonFPlusLights.push_back(lpn->getNumLightPoints());
                    continue;
                  }
                  
                  osg::NodePath np;
                  np.push_back(lpn.get());
                  
                  osg::ref_ptr<osg::Group> parent = lpn->getNumParents() ? lpn->getParent(0) : 0;
                  while (parent)
                  {
                    np.insert(np.begin(), parent);
                    parent = parent->getNumParents() ? parent->getParent(0) : 0;
                  }
                  
                  osg::Matrixd wmx = osg::computeLocalToWorld(np);
                  
                  
                  bool hasBlinkingLights = false;
                  
                  std::ostringstream oss;
                  for (size_t i = 0; i < lpn->getNumLightPoints(); ++i)
                  {
                    osgSim::LightPoint& lp = lpn->getLightPoint(i);
                    
                    unsigned int id = 0;
                    OpenIG::Base::LightType lightType = OpenIG::Base::LT_UNKNOWN;
                    if (OpenIG::Base::IDPool::instance()->getNextId("Real-Lights", id))
                    {
                      osg::Matrixd mx;
                      
                      if (lp._sector.valid())
                      {
                        osg::ref_ptr<osgSim::DirectionalSector> ds = dynamic_cast<osgSim::DirectionalSector*>(lp._sector.get());
                        if (ds.valid())
                        {
                          osg::Vec3 direction = ds->getDirection();
                          
                          osg::Quat q;
                          q.makeRotate(osg::Vec3(0, 1, 0), direction);
                          
                          osg::Vec3d hpr = OpenIG::Base::Math::instance()->fromQuat(q);
                          
                          // NOTE: For geocentric database please consider
                          // the proper directions
                          mx = OpenIG::Base::Math::instance()->toMatrix(
                            lp._position.x(),
                                                                        lp._position.y(),
                                                                        lp._position.z(),
                                                                        osg::RadiansToDegrees(hpr.x()), 0, 0);
                          
                          lightType = OpenIG::Base::LT_SPOT;
                        }
                      }
                      // no sector valid, so probably we make them generic to
                      // point down to the terrain
                      else
                      {
                        mx = osg::Matrixd::translate(lp._position);
                        lightType = OpenIG::Base::LT_POINT;
                      }
                      
                      assert(lightType!=OpenIG::Base::LT_UNKNOWN);
                      
                      osg::Matrixd final = mx * wmx;
                      
                      float brightness = 1.f;
                      float range = 15.f;
                      
                      lpn->getUserValue("brightness", brightness);
                      lpn->getUserValue("fEndRange",range);
                      
                      double lodRange = OpenIG::Base::Configuration::instance()->getConfig("ForwardPlusLightsDefaultLODRange",1000.0);
                      
                      OpenIG::Base::LightAttributes la;
                      la.diffuse = lp._color;
                      la.ambient = osg::Vec4(0, 0, 0, 1);
                      la.specular = osg::Vec4(0, 0, 0, 1);
                      la.constantAttenuation = 50;
                      la.brightness = brightness;
                      la.spotCutoff = 20;
                      la.realLightLOD = lodRange;
                      la.fStartRange = 0.0;
                      la.fEndRange = range;
                      la.fSpotInnerAngle = 10;
                      la.fSpotOuterAngle = 120;
                      la.lightType = lightType;
                      la.enabled = lp._on;
                      la.dataVariance = osg::Object::STATIC;
                      la.dirtyMask = OpenIG::Base::LightAttributes::ALL;
                      
                      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(OpenIG::Plugins::LightsControlPlugin::lightMutex);
                      
                      _ig->addLight(id, la, final);
                      lightsAllocated.push_back(id);
                      
                      oss << id << ";";
                      
                      _ig->updateLightAttributes(id, la);
                      
                      if (lp._blinkSequence.valid())
                        hasBlinkingLights = true;
                    }
                    else
                    {
                      //osg::notify(osg::NOTICE) << "LightsControl: unable to get a light ID" << std::endl;
                    }
                  }
                  
                  
                  // here we keep track of what lids IDs are used
                  // so later we release them when a tile keeping
                  // these lights is being paged
                  std::string value = oss.str();
                  lpn->setUserValue("Real-Lights-IDs", value);
                  
                  if (hasBlinkingLights)
                  {
                    osg::ref_ptr<BlinkUpdateCallback> callback = dynamic_cast<BlinkUpdateCallback*>(plod->getUserData());
                    if (!callback.valid())
                    {
                      callback = new BlinkUpdateCallback(_ig);
                      plod->setUserData(callback);
                    }
                    callback->addLightPointNode(lpn);
                  }
                }
              }
              
              if (lightsAllocated.size())
              {
                UpdateLightsInMultiSwitchNodeVisitor nv(_ig);
                plod->accept(nv);
                
                osg::notify(osg::NOTICE) << "LightsControl: lights allocated: " << lightsAllocated.size() << std::endl;
              }
              
              if (nonFPlusLights.size())
              {
                size_t num = 0;
                for (size_t i = 0; i < nonFPlusLights.size(); ++i)
                  num += nonFPlusLights.at(i);
                
                osg::notify(osg::NOTICE) << "LightsControl: non F+ lights allocated: " << num << std::endl;
              }
              
              // Print out the stats
              if (lpnStats.size())
              {
                osg::notify(osg::INFO) << "LightsControl: LightPointNode stats ========" << std::endl;
                LightPointsStats::iterator lpnIter = lpnStats.begin();
                for (; lpnIter != lpnStats.end(); ++lpnIter)
                {
                  LightPointNodeStats& stats = *lpnIter;
                  osg::notify(osg::INFO) << stats.name << ": " << stats.numOfLightPoints << std::endl;
                }
                osg::notify(osg::INFO) << "============================================" << std::endl;
              }
            }
            // No children, they are out
            else
            {
              // We sync now with the Database Pager
              OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
              
              // We fill here ids of lights that are paged out
              // for reuse
              std::vector<unsigned int> reuseLightIds;
              
              // Go through all the PagedLODs and if they are expiered,
              // you recall we observe them, then release the lights
              // for reuse by new paged in tiles, and erase them from the map
              PagedLODObserverPointer ptr(plod);
              PagedLODWithLightPointNodeListMap::iterator itr = _map.find(ptr);
              if (itr != _map.end())
              {
                const PagedLODObserverPointer& plod = itr->first;
                LightPointNodeList& lights = itr->second;
                
                // If this is not valid then the PagedLOD we
                // are observing has been paged out, so we
                // clean stuff here
                if (plod.valid())
                {
                  LightPointNodeList::iterator litr = lights.begin();
                  for (; litr != lights.end(); ++litr)
                  {
                    LightPointNodePointer& lpn = *litr;
                    
                    // A bit of paranoia
                    if (!lpn.valid()) continue;
                    
                    // We have encoded the IDs used for
                    // imagegenerator lights in User Value,
                    // see above
                    std::string ids;
                    if (lpn->getUserValue("Real-Lights-IDs", ids) && !ids.empty())
                    {
                      OpenIG::Base::StringUtils::Tokens tokens = OpenIG::Base::StringUtils::instance()->tokenize(ids, ";");
                      OpenIG::Base::StringUtils::TokensIterator titr = tokens.begin();
                      for (; titr != tokens.end(); ++titr)
                      {
                        const std::string& id = *titr;
                        
                        unsigned int lightId = atoi(id.c_str());
                        if (lightId)
                        {
                          reuseLightIds.push_back(lightId);
                          _ig->removeLight(lightId);
                        }
                      }
                    }
                  }
                }
                lights.clear();
              }
              
              // Now if we have released some lights
              // let reuse their ids
              if (reuseLightIds.size())
              {
                OpenIG::Base::IDPool::instance()->setAvailableIds("Real-Lights", reuseLightIds);
                // Print out for debug
                osg::notify(osg::NOTICE) << "LightsControl: " << reuseLightIds.size() << " LPNs paged out" << std::endl;
              }
              
              // clean the callback stored in the user data
              plod->setUserData(0);
            }
            
            _numChildrenCheck = plod->getNumChildren();
          }
        }
        
      protected:
        PagedLODWithLightPointNodeListMap&	_map;
        OpenThreads::Mutex&					_mutex;
        unsigned							_numChildrenCheck;
        OpenIG::Base::ImageGenerator*		_ig;
      };
      
      FindPagedLODsNodeVisitor(PagedLODWithLightPointNodeListMap& map,OpenThreads::Mutex& mutex,OpenIG::Base::ImageGenerator* ig)
      : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN), _map(map), _mutex(mutex), _ig(ig)
      {
      }
      
      // We have a stack of PLODs. When we
      // hit a PagedLOD we add it to the stack,
      // traverse it, and remove from the stack.
      // Basic operation to have the actual PagedLOD
      // parent on top of the stack
      // If we hit a osgSim::LightPointNode we add it
      // to our list (and map) so we keep track of them
      virtual void apply(osg::Node& node)
      {
        osg::ref_ptr<osg::PagedLOD> plod = dynamic_cast<osg::PagedLOD*>(&node);
        if (plod.valid())
        {
          // install the observer callback
          plod->setUpdateCallback(new PagedLODPagingObserverNodeCallback(_map, _mutex, _ig));
          
          LightPointNodeList list;
          
          _mutex.lock();
          PagedLODObserverPointer ptr(plod);
          _map[ptr] = list;
          _mutex.unlock();
        }
        
        traverse(node);
      }
      
      PagedLODWithLightPointNodeListMap&		_map;
      OpenThreads::Mutex&						_mutex;
      OpenIG::Base::ImageGenerator*					_ig;
    };
    
    // Okay. Now we want to know what tiles are to be parsed for lights
    // points. They are defined in the XML with the tag <Tile>. You
    // cand use wild card. If not found, then we simple remove the
    // lightpoints. The reason for this is that we want to keep the
    // lighting system optimized from unoptimized databases. The following
    // node visitor will remove all unwanted lights from the loaded tile
    struct RemoveLightsNodeVisitor : public osg::NodeVisitor
    {
      RemoveLightsNodeVisitor()
      : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
      {
        
      }
      
      virtual void apply(osg::Node& node)
      {
        osg::ref_ptr<osgSim::LightPointNode> lpn = dynamic_cast<osgSim::LightPointNode*>(&node);
        if (lpn.valid())
        {
          osgSim::LightPointNode::LightPointList& lps = lpn->getLightPointList();
          lps.clear();
        }
        
        traverse(node);
      }
    };
    
    // Here we want to combine light point nodes for faster rendering based
    // on their name. We use this visitor in the databaseRead hook
    struct CombineLightPointNodesVisitor : public osg::NodeVisitor
    {
      CombineLightPointNodesVisitor() : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}
      
      virtual void apply(osg::Node& node)
      {
        osgSim::LightPointNode* lpn = dynamic_cast<osgSim::LightPointNode*>(&node);
        if (lpn)
        {
          // What is the XML name pattern for this lpn?
          const std::string xmlLightName = getLpnNameBasedOnXMLDefinition(lpn);
          
          if (!xmlLightName.empty())
          {
            // Now we have the name, we find or create a new lpn with
            // this XML name that will contain all the lpns but grouped
            osg::ref_ptr<osgSim::LightPointNode> matchedLpn = _lpns[xmlLightName];
            if (!matchedLpn.valid())
            {
              _lpns[xmlLightName] = matchedLpn = new osgSim::LightPointNode(*lpn, osg::CopyOp::DEEP_COPY_ALL);
              matchedLpn->setName(xmlLightName);
              
              // If we work with Triton, then
              // dont render these lights in the
              // Height map
              matchedLpn->setNodeMask(0x4);
              matchedLpn->setCullingActive(true);
            }
            else
            {
              // We add the light points to our
              // 'grouped' lpn
              for (size_t i = 0; i < lpn->getNumLightPoints(); ++i)
              {
                osgSim::LightPoint& lp = lpn->getLightPoint(i);
                matchedLpn->addLightPoint(lp);
              }
            }
            
            // And we record the processed lpn for further removal
            _lpnsToBeGrouped.push_back(lpn);
          }
        }
        traverse(node);
      }
      
      void groupLightPointNodes()
      {
        typedef std::map< std::string, osg::Node::ParentList >			Parents;
        Parents	parents;
        
        // Ok, here we go through all the parents of the 'old' lpns to be
        // grouped and we remove the individual lpns from them and we keep
        // track of these parents, they can be switches and we attach to them
        // our 'new' grouped lpns
        for (LightPointNodesToBeGrouped::iterator itr = _lpnsToBeGrouped.begin();
             itr != _lpnsToBeGrouped.end();
        ++itr)
             {
               osg::ref_ptr<osgSim::LightPointNode> lpn = *itr;
               lpn->setCullingActive(true);
               
               const std::string xmlName = getLpnNameBasedOnXMLDefinition(lpn);
               if (!xmlName.empty())
               {
                 osg::Node::ParentList& lpnParents = parents[xmlName];
                 
                 osg::Node::ParentList oldParents = lpn->getParents();
                 for (osg::Node::ParentList::iterator pitr = oldParents.begin();
                      pitr != oldParents.end();
                 ++pitr)
                      {
                        // Remove it from the original parent
                        osg::ref_ptr<osg::Group> parent = *pitr;
                        if (parent.valid())
                        {
                          parent->removeChild(lpn);
                          
                          // and record the parent for the new
                          // attachment to for the grouped lpn
                          lpnParents.push_back(parent);
                        }
                      }
               }
             }
             
             // now, we go through all the new grouped lpns
             // and attach them to the original parents
             for (LightPointNodes::iterator itr = _lpns.begin();
                  itr != _lpns.end();
             ++itr)
                  {
                    const std::string		name = itr->first;
                    osgSim::LightPointNode*	lpn = itr->second;
                    
                    lpn->setCullingActive(true);
                    
                    // Get the parent list
                    osg::Node::ParentList& lpnParents = parents[name];
                    if (lpnParents.size())
                    {
                      #if 1
                      osg::Group* parent = lpnParents.at(0);
                      parent->removeChild(lpn);
                      parent->addChild(lpn);
                      #else
                      for (osg::Node::ParentList::iterator pitr = lpnParents.begin();
                      pitr != lpnParents.end();
                      ++pitr)
                      {
                      // Here we add the grouped lpns and we are done
                      osg::Group* parent = *pitr;
                      parent->removeChild(lpn);
                      parent->addChild(lpn);
                    }
                    #endif
                    }
                  }
      }
    protected:
      typedef std::map< std::string, osg::ref_ptr<osgSim::LightPointNode> >		LightPointNodes;
      LightPointNodes					_lpns;
      
      typedef std::vector< osg::ref_ptr<osgSim::LightPointNode> >					LightPointNodesToBeGrouped;
      LightPointNodesToBeGrouped		_lpnsToBeGrouped;
      
      const std::string getLpnNameBasedOnXMLDefinition(osgSim::LightPointNode* lpn)
      {
        LightsControlPlugin::LightPointDefinitions::iterator itr = LightsControlPlugin::definitions.begin();
        for (; itr != LightsControlPlugin::definitions.end(); ++itr)
        {
          const std::string& name = itr->first;
          if (lpn->getName().substr(0, osg::minimum(lpn->getName().length(), name.length())) == name)
          {
            // Ok .. Here we have to be carefull. If we have lp nodes
            // attached to parents with names same as some other names
            // let attach the pointer of their parent so we make sure
            // they are attached to the same parent
            std::ostringstream oss;
            oss << name;
            if (lpn->getParents().size() != 0)
            {
              oss << "_" << (long long)lpn->getParent(0);
            }
            return oss.str();
          }
        }
        return "";
      }
    };
    
    // Here when we read in a tile, we check for xml config file, if exists, we read it,
    // and we store the lights definitions coming from there. Further, we traverse the
    // tile to find all PagedLODs and the light point nodes coming from the database and
    // we store this into PagedLOD observer_ptr map. Later in update, we check if a PagedLOD
    // was paged out, and we release those imagegenerator light implementations attached to them
    // Same is done in the PagedLOD update callbacks
    virtual void databaseRead(const std::string& fileName, osg::Node* node, const osgDB::Options*)
    {
      // We read the lights control config xml if exists for these lights
      const std::string xmlFileName = fileName + ".lighting.xml";
      if (osgDB::fileExists(xmlFileName))
      {
        readXML(_xmlFile = xmlFileName);
        
        // we expect one config file per
        // visual database so we launch
        // a observer thread to monitor
        // changes on this file. If there
        // will be a need of muitiple files
        // like per tile, or multiple databases
        // then consider making these in a vector
        _xmlThreadRunningCondition = true;
        _xmlFileObserverThread = boost::shared_ptr<boost::thread>(new boost::thread(&OpenIG::Plugins::LightsControlPlugin::xmlFileObserverThread, this));
        
      }
      else
        // no top level XML? then match all tiles for light points
      {
      }
      
      // We filter here tiles for light points management. If the tile
      // does not match the pattern from the XML (see tag <Tile>), we
      // remove all the lps from there to not overload the Lighting System
      // NOTE: future versions dealing with only light points should be
      // considered for grouping the lps and not creating real lights from these
      // light points, for maximum peformance
      std::string simpleFileName(osgDB::getSimpleFileName(fileName));
      
      // this will use regex expressions to match a tile but it makes
      // the paging way too slow
      // if (OpenIG::Base::FileSystem::match(_tiles,simpleFileName))
      // we use tilemap instead and let makes them all valid by default
      bool validTile = _validTiles.empty() ? true : _validTiles[simpleFileName];
      if (!validTile)
      {
        RemoveLightsNodeVisitor remove;
        node->accept(remove);
        
        FindPagedLODsNodeVisitor nv(_plodLightPointNodes, _mutex, _ig);
        node->accept(nv);
      }
      else
      {
        // We combine all the light points here
        // with common attributes for performance
        // gain. NOTE: reconsider this to use
        // different plugin hooks to speed up the paging
        // since we have now multiple vistits of the node
        // once readed
        if (OpenIG::Base::Configuration::instance()->getConfig("GroupLightsBasedOnXMLDefinition", "no") == "yes")
        {
          CombineLightPointNodesVisitor nv;
          node->accept(nv);
          nv.groupLightPointNodes();
        }
        
        FindPagedLODsNodeVisitor nv(_plodLightPointNodes, _mutex, _ig);
        node->accept(nv);
        
        // add the PATH to the texture cache
        _textureCache.addPath(osgDB::getFilePath(fileName));
        
        // We have some XML Definitions for the lights, so
        // let update the new lights with them. Later, there
        // is a thread that monitors changes in this XML
        // configuration and on change it update the whole scene
        // again
        updateLightPointNodesBasedOnXMLDefinitions(node);
        
        // we update lights based on the time of day as well
        updateLightPointNodesBasedOnTimeOfDay(node);
      }
    }
    
    // We iterate here over the map and those expired
    // PagedLODs we remove them
    // Also here we look for Time of Day change since the
    // lights can be turned on/off from the XML file, and
    // also to check for XML file change
    virtual void update(OpenIG::PluginBase::PluginContext& context)
    {
      PagedLODWithLightPointNodeListMap::iterator itr = _plodLightPointNodes.begin();
      while (itr != _plodLightPointNodes.end())
      {
        const PagedLODObserverPointer& plod = itr->first;
        if (!plod.valid())
        {
          // Ok. Here we have to clean stuff again. Reaching this point
          // we probably have proper quad tree database with nested
          // PagedLODs
          // We sync now with the Database Pager
          OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
          
          // We fill here ids of lights that are paged out
          // for reuse
          std::vector<unsigned int> reuseLightIds;
          
          // Go through all the PagedLODs and if they are expiered,
          // you recall we observe them, then release the lights
          // for reuse by new paged in tiles, and erase them from the map
          if (itr->second.size())
          {
            LightPointNodeList& lights = itr->second;
            
            // If this is not valid then the PagedLOD we
            // are observing has been paged out, so we
            // clean stuff here
            if (!plod.valid())
            {
              LightPointNodeList::iterator litr = lights.begin();
              for (; litr != lights.end(); ++litr)
              {
                LightPointNodePointer& lpn = *litr;
                
                // A bit of paranoia
                if (!lpn.valid()) continue;
                
                // We have encoded the IDs used for
                // imagegenerator lights in User Value,
                // see above
                std::string ids;
                if (lpn->getUserValue("Real-Lights-IDs", ids) && !ids.empty())
                {
                  OpenIG::Base::StringUtils::Tokens tokens = OpenIG::Base::StringUtils::instance()->tokenize(ids, ";");
                  OpenIG::Base::StringUtils::TokensIterator titr = tokens.begin();
                  for (; titr != tokens.end(); ++titr)
                  {
                    const std::string& id = *titr;
                    
                    unsigned int lightId = atoi(id.c_str());
                    if (lightId)
                    {
                      reuseLightIds.push_back(lightId);
                      _ig->removeLight(lightId);
                    }
                  }
                }
              }
            }
          }
          
          // Now if we have released some lights
          // let reuse their ids
          if (reuseLightIds.size())
          {
            OpenIG::Base::IDPool::instance()->setAvailableIds("Real-Lights", reuseLightIds);
            // Print out for debug
            osg::notify(osg::NOTICE) << "LightsControl: " << reuseLightIds.size() << " LPNs paged out" << std::endl;
          }
          
          // Remove this non valid PagedLOD
          PagedLODWithLightPointNodeListMap::iterator saved_itr = itr;
          ++saved_itr;
          
          _plodLightPointNodes.erase(itr);
          
          itr = saved_itr;
        }
        else
          ++itr;
      }
      
      // Here we check if the XML has changed. If so, reload and update
      _xmlAccessMutex.lock();
      if (_xmlLastCheckedTime == 0)
      {
        _xmlLastCheckedTime = _xmlLastWriteTime;
      }
      if (_xmlLastCheckedTime != _xmlLastWriteTime)
      {
        osg::notify(osg::NOTICE) << "LightsControl: XML updated: " << _xmlFile << std::endl;
        readXML(_xmlFile);
        updateLightPointNodesBasedOnXMLDefinitions(context.getImageGenerator()->getScene());
        updateLightPointNodesBasedOnTimeOfDay(context.getImageGenerator()->getScene());
        
        _xmlLastCheckedTime = _xmlLastWriteTime;
      }
      _xmlAccessMutex.unlock();
      
      // Save the current time
      osg::ref_ptr<osg::Referenced> todRef = context.getAttribute("TOD");
      OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::TimeOfDayAttributes> *todAttr = dynamic_cast<OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::TimeOfDayAttributes> *>(todRef.get());
      if (todAttr)
      {
        // NOTE: we have to set some default here
        LightsControlPlugin::tod.hour()    = todAttr->getValue().getHour();
        LightsControlPlugin::tod.minutes() = todAttr->getValue().getMinutes();
        
        updateLightPointNodesBasedOnTimeOfDay(context.getImageGenerator()->getScene());
      }
      
      // Set a Multiswitch Active Switchset
      osg::ref_ptr<osg::Referenced> msRef = context.getAttribute("MultiSwitch");
      OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::MultiSwitchAttributes> *msAttr = dynamic_cast<OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::MultiSwitchAttributes> *>(msRef.get());
      if (msAttr)
      {
        // NOTE: we have to set some default here
        LightsControlPlugin::msName  = msAttr->getValue().getMultiSwitchName();
        LightsControlPlugin::msIndex = msAttr->getValue().getMultiSwitchIndex();
      }
    }
    
    virtual void entityAdded(OpenIG::PluginBase::PluginContext& context, unsigned int id, osg::Node& node, const std::string&)
    {
      if (id == 0)
      {
        node.setNodeMask(0x1);
      }
    }
    
    virtual void clean(OpenIG::PluginBase::PluginContext&)
    {
      OpenIG::Base::Commands::instance()->removeCommand("ms");
      
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
      
      PagedLODWithLightPointNodeListMap::iterator itr = _plodLightPointNodes.begin();
      for (; itr != _plodLightPointNodes.end(); ++itr)
      {
        const PagedLODObserverPointer& plod = itr->first;
        if (plod.valid())
        {
          plod->setUpdateCallback(0);
          plod->setUserData(0);
        }
      }
      _plodLightPointNodes.clear();
      
      if (_xmlFileObserverThread.get())
      {
        _xmlThreadRunningCondition = false;
        while (_xmlThreadIsRunning);
        _xmlFileObserverThread->join();
      }
      _xmlFileObserverThread.reset();
    }
    
  protected:
    OpenIG::Base::ImageGenerator*				_ig;
    PagedLODWithLightPointNodeListMap			_plodLightPointNodes;
    OpenThreads::Mutex							_mutex;
    std::string									_xmlFile;
    OpenIG::Base::StringUtils::StringList		_tiles;
    
    typedef std::map<std::string, bool >		ValidTiles;
    ValidTiles									_validTiles;
    
  public:
    struct LightPointDefinition
    {
      std::string name;
      bool		always_on;
      float		minPixelSize;
      float		minPixelSizeMultiplierForSprites;
      float		maxPixelSize;
      float		radius;
      float		intensity;
      float		brightness;
      float		range;
      bool		sprites;
      std::string	texture;
      bool		fplus;
    };
    
    typedef std::map< std::string, LightPointDefinition >	LightPointDefinitions;
    static LightPointDefinitions definitions;
    
    static bool	forwardPlusPluginAvailable;
    
  protected:
    void readXML(const std::string&  xmlFile)
    {
      _tiles.clear();
      _validTiles.clear();
      
      osgDB::XmlNode* root = osgDB::readXmlFile(xmlFile);
      if (!root)
      {
        osg::notify(osg::NOTICE) << "LightsControl: XML NULL root: " << xmlFile << std::endl;
        return;
      }
      if (!root->children.size())
      {
        osg::notify(osg::NOTICE) << "LightsControl: XML root with no children: " << xmlFile << std::endl;
        return;
      }
      
      typedef std::multimap< std::string, osgDB::XmlNode::Properties >		TagProperties;
      TagProperties	tags;
      
      typedef std::multimap< std::string, std::string>						TagValues;
      TagValues	values;
      
      for (osgDB::XmlNode::Children::iterator itr = root->children.at(0)->children.begin();
           itr != root->children.at(0)->children.end();
      ++itr)
           {
             osgDB::XmlNode* child = *itr;
             tags.insert(std::pair<std::string, osgDB::XmlNode::Properties>(child->name, child->properties));
             values.insert(std::pair<std::string, std::string>(child->name, child->contents));
           }
           
           for (TagProperties::iterator itr = tags.begin();
                itr != tags.end();
           ++itr)
                {
                  typedef std::map< std::string, std::string>	Properties;
                  Properties properties;
                  
                  for (osgDB::XmlNode::Properties::iterator pitr = itr->second.begin();
                       pitr != itr->second.end();
                  ++pitr)
                       {
                         properties[pitr->first] = pitr->second;
                       }
                       
                       if (itr->first == "LightPointNode")
                       {
                         LightPointDefinition def;
                         def.name								= properties["name"];
                         def.always_on							= properties["always_on"] == "true";
                         def.minPixelSize						= osg::maximum(atof(properties["minPixelSize"].c_str()),1.0);
                         def.minPixelSizeMultiplierForSprites	= osg::maximum(atof(properties["minPixelSizeMultiplierForSprites"].c_str()),1.0);
                         def.maxPixelSize						= osg::maximum(atof(properties["maxPixelSize"].c_str()),1.0);
                         def.intensity							= atof(properties["intensity"].c_str());
                         def.radius								= atof(properties["radius"].c_str());
                         def.brightness							= atof(properties["brightness"].c_str());
                         def.range								= atof(properties["range"].c_str());
                         def.sprites								= properties["sprites"] == "true";
                         def.texture								= properties["texture"];
                         def.fplus								= properties["fplus"] == "true";
                         
                         OpenIG::Plugins::LightsControlPlugin::LightPointBrightness::iterator l_itr = OpenIG::Plugins::LightsControlPlugin::lightPointBrightness.find(def.name);
                         if (l_itr == OpenIG::Plugins::LightsControlPlugin::lightPointBrightness.end())
                         {
                           OpenIG::Plugins::LightsControlPlugin::lightPointBrightness[def.name] = 1.f;
                         }
                         
                         definitions[def.name] = def;					
                       }
                       else
                         if (itr->first == "TimeofDay")
                         {
                           LightsControlPlugin::onTOD.hour() = atoi(properties["on"].c_str());
                           LightsControlPlugin::offTOD.hour() = atoi(properties["off"].c_str());
                         }
                         
                         for (TagValues::iterator itr = values.begin();
                              itr != values.end();
                         ++itr)
                              {
                                if (itr->first == "Tile")
                                {
                                  std::string fileName = itr->second;
                                  std::string path = osgDB::getFilePath(fileName);
                                  if (path.empty()) path = osgDB::getFilePath(_xmlFile);
                                  
                                  _tiles.push_back(path + "/" + fileName);
                                  _validTiles[fileName] = true;
                                }
                              }
                              if (_tiles.empty()) _tiles.push_back(tilesWildcardMatchAll);
                }
    }
    
    typedef std::map< std::size_t, osg::ref_ptr<osg::StateSet> > SpriteStateSetCache;
    SpriteStateSetCache _spriteStateSetCache;
    
    osg::ref_ptr<osg::Program> _spriteProgram;
    osg::ref_ptr<osg::Program> _lightPointProgram;
    
    
    // We will use simple cache for images
    // used for LPN sprites. Already created
    // as textures
    
    TextureCache	_textureCache;
    
  public:
    
    // A node visitor that will update the lights brightness
    struct UpdateLightPointBrightnessNodeVisitor : public osg::NodeVisitor
    {
      UpdateLightPointBrightnessNodeVisitor(OpenIG::Base::ImageGenerator* ig, LightPointDefinitions& defs, const std::string& name)
      : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
      , _ig(ig)
      , _defs(defs)
      , _name(name)
      {
      }
      
      virtual void apply(osg::Node& node)
      {		
        osg::ref_ptr<osgSim::LightPointNode> lpn = dynamic_cast<osgSim::LightPointNode*>(&node);
        if (lpn.valid())
        {									
          if (lpn->getName().substr(0, osg::minimum(lpn->getName().length(), _name.length())) == _name)
          {
            LightPointDefinitions::iterator itr = _defs.find(_name);
            if (itr != _defs.end())
            {
              LightPointDefinition& def = itr->second;
              
              OpenIG::Plugins::LightsControlPlugin::LightPointBrightness::iterator l_itr = OpenIG::Plugins::LightsControlPlugin::lightPointBrightness.find(_name);
              if (l_itr != OpenIG::Plugins::LightsControlPlugin::lightPointBrightness.end())
              {
                float multiplier = l_itr->second;
                lpn->setUserValue("brightness", def.brightness * multiplier);
                
                std::vector<osg::Vec4> color;
                for (size_t i = 0; i < lpn->getNumLightPoints(); ++i)
                {
                  osgSim::LightPoint& lp = lpn->getLightPoint(i);
                  lp._color.a() = OpenIG::Plugins::LightsControlPlugin::lightPointBrightness[_name];
                  color.push_back(lp._color);
                }
                
                std::string ids;
                if (lpn->getUserValue("Real-Lights-IDs", ids) && !ids.empty())
                {									
                  
                  OpenThreads::ScopedLock<OpenThreads::Mutex> lock(OpenIG::Plugins::LightsControlPlugin::lightMutex);
                  
                  OpenIG::Base::StringUtils::Tokens tokens = OpenIG::Base::StringUtils::instance()->tokenize(ids, ";");
                  OpenIG::Base::StringUtils::TokensIterator titr = tokens.begin();
                  size_t index = 0;
                  for (; titr != tokens.end(); ++titr)
                  {
                    const std::string& id = *titr;
                    
                    unsigned int lightId = atoi(id.c_str());
                    if (lightId)
                    {
                      OpenIG::Base::LightAttributes la;
                      la.diffuse = color.at(index++);
                      la.brightness = def.brightness * OpenIG::Plugins::LightsControlPlugin::lightPointBrightness[_name];											
                      la.dirtyMask = OpenIG::Base::LightAttributes::DIFFUSE;
                      
                      _ig->updateLightAttributes(lightId, la);
                      
                    }
                  }
                }														
              }
            }
          }				
        }
        else
          traverse(node);
      }
      
    protected:
      LightPointDefinitions&			_defs;
      std::string						_name;
      OpenIG::Base::ImageGenerator*	_ig;
    };
    
    void updateLightPointsBrighntess(const std::string& name)
    {
      osg::Node* scene = _ig->getScene();
      if (!scene) return;
      
      UpdateLightPointBrightnessNodeVisitor nv(_ig,definitions, name);
      scene->accept(nv);
    }
    
  protected:
    
    // A node visitor that will update a lightpointnode
    // based on it's XML definition from a config file
    struct UpdateLightPointNodeNodeVisitor : public osg::NodeVisitor
    {
      UpdateLightPointNodeNodeVisitor(LightPointDefinitions& defs, TextureCache& textureCache
      , SpriteStateSetCache& spriteStateSetCache, osg::ref_ptr<osg::Program>& spriteProgram
      , osg::ref_ptr<osg::Program>& lightPointProgram
      , OpenIG::Base::ImageGenerator& ig)
      : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
      , _defs(defs), _textureCache(textureCache)
      , _spriteStateSetCache(spriteStateSetCache)
      , _spriteProgram(spriteProgram)
      , _lightPointProgram(lightPointProgram), _ig(ig)
      {
      }
      
      void updateLight(osgSim::LightPointNode* lpn)
      {
        LightPointDefinitions::iterator itr = _defs.begin();
        for (; itr != _defs.end(); ++itr)
        {
          LightPointDefinition& def = itr->second;
          const std::string name = itr->first;
          
          // This is our light definition based on the name
          if (lpn->getName().substr(0, osg::minimum(lpn->getName().length(), name.length())) == name)
          {
            std::vector<osg::Vec4> color;
            for (size_t i = 0; i < lpn->getNumLightPoints(); ++i)
            {
              osgSim::LightPoint& lp = lpn->getLightPoint(i);
              lp._radius = def.radius;
              lp._intensity = def.intensity;
              // always use additive blending
              lp._blendingMode = osgSim::LightPoint::ADDITIVE;
              
              color.push_back(lp._color);
            }
            lpn->setMinPixelSize(def.minPixelSize);
            lpn->setMaxPixelSize(def.maxPixelSize);
            lpn->setPointSprite(def.sprites);
            lpn->setUserValue("always_on", def.always_on);
            lpn->setUserValue("brightness", def.brightness * OpenIG::Plugins::LightsControlPlugin::lightPointBrightness[name]);
            lpn->setUserValue("fEndRange", def.range);
            
            bool fplusLight = false;
            lpn->getUserValue("f+", fplusLight);
            lpn->setUserValue("f+", def.fplus);
            
            std::string ids;
            if (lpn->getUserValue("Real-Lights-IDs", ids) && !ids.empty())
            {
              OpenThreads::ScopedLock<OpenThreads::Mutex> lock(OpenIG::Plugins::LightsControlPlugin::lightMutex);
              
              OpenIG::Base::StringUtils::Tokens tokens = OpenIG::Base::StringUtils::instance()->tokenize(ids, ";");
              OpenIG::Base::StringUtils::TokensIterator titr = tokens.begin();
              size_t index = 0;
              for (; titr != tokens.end(); ++titr)
              {
                const std::string& id = *titr;
                
                unsigned int lightId = atoi(id.c_str());
                if (lightId)
                {
                  OpenIG::Base::LightAttributes la;
                  la.diffuse = color.at(index++);
                  la.brightness = def.brightness * OpenIG::Plugins::LightsControlPlugin::lightPointBrightness[name];
                  la.fEndRange = def.range;
                  la.fStartRange = 0.f;
                  la.dirtyMask = OpenIG::Base::LightAttributes::DIFFUSE|OpenIG::Base::LightAttributes::RANGES;
                  
                  if (!def.fplus && fplusLight)
                    _ig.removeLight(lightId);
                  else
                    if (def.fplus && fplusLight)
                      _ig.updateLightAttributes(lightId, la);
                    
                }
              }
            }
            
            setUpSpriteStateSet(lpn, def);
            
            break;
          }
        }
      }
      
      virtual void apply(osg::Node& node)
      {
        osg::ref_ptr<osgSim::LightPointNode> lpn = dynamic_cast<osgSim::LightPointNode*>(&node);
        if (lpn.valid())
        {
          updateLight(lpn.get());
        }
        else
          traverse(node);
      }
      
      LightPointDefinitions&	_defs;
      TextureCache&			_textureCache;
      SpriteStateSetCache&    _spriteStateSetCache;
      osg::ref_ptr<osg::Program>& _spriteProgram;
      osg::ref_ptr<osg::Program>& _lightPointProgram;
      
      OpenIG::Base::ImageGenerator& _ig;
      
      size_t getStateSetID(const LightPointDefinition& def)
      {
        std::size_t seed =0;
        boost::hash_combine(seed, def.texture);
        boost::hash_combine(seed, def.minPixelSize);
        boost::hash_combine(seed, def.maxPixelSize);
        boost::hash_combine(seed, def.radius);
        return seed;
      }
      
      
      void setUpLightPointStateSet(osgSim::LightPointNode* lpn, LightPointDefinition& def)
      {
        if (_lightPointProgram.valid()==false)
        {
          osg::notify(osg::NOTICE) <<"Empty light point program!"<<std::endl;
          return;
        }
        
        osg::StateSet* stateSet = new osg::StateSet;
        stateSet->merge(*lpn->getOrCreateStateSet());
        lpn->setStateSet(stateSet);
        
        stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF|osg::StateAttribute::PROTECTED|osg::StateAttribute::OVERRIDE);
        
        stateSet->setAttributeAndModes(_lightPointProgram, osg::StateAttribute::ON|osg::StateAttribute::PROTECTED|osg::StateAttribute::OVERRIDE);
        
        stateSet->setTextureAttributeAndModes(0,0);
      }
      
      static bool useLogZDepthBuffer(void)
      {
        std::string strLogZDepthBuffer = OpenIG::Base::Configuration::instance()->getConfig("LogZDepthBuffer","yes");
        if (strLogZDepthBuffer.compare(0, 3, "yes") == 0)
          return true;
        else
          return false;
      }
      
      void setUpSpriteStateSet(osgSim::LightPointNode* lpn, LightPointDefinition& def)
      {
        // Not sure why this was looking for the F+ lights - I know there was a reason
        // but suddenly other plugins like SimpleLighting can not have sprites with this
        //if (def.sprites == false || !OpenIG::Plugins::LightsControlPlugin::forwardPlusPluginAvailable)
        if (def.sprites == false)
        {
          setUpLightPointStateSet(lpn, def);
          return;
        }
        
        if (def.texture.empty())
        {
          setUpLightPointStateSet(lpn, def);
          return;
        }
        
        if (_spriteProgram.valid()==false)
        {
          setUpLightPointStateSet(lpn, def);
          return;
        }
        
        //osg::notify(osg::NOTICE)<<"Lights Control: Setup Sprite texture!!"<<std::endl;
        Texture2DPointer texture = _textureCache.get(def.texture);
        if (texture.valid()==false)
        {
          setUpLightPointStateSet(lpn, def);
          //osg::notify(osg::NOTICE)<<"Lights Control: Setup Sprite texture FAILED!!!!!!!"<<std::endl;
          return;
        }
        else
        {
          //osg::notify(osg::NOTICE) << "Lights Control: Setup Sprite texture PASSED!!!!!!!" << std::endl;
        }
        
        
        std::size_t stateSetID = getStateSetID(def);
        
        SpriteStateSetCache::const_iterator it = _spriteStateSetCache.find(stateSetID);
        if (it!=_spriteStateSetCache.end())
        {
          #if 0
          // This is wrong. No idea who added it, me (Nick) or Poojan
          // but it causes crashes on large paged databases with lots of
          // lights. Anyhow, this uniform is set below in the StateSet
          // that is cached and being used here
          static const float radiusMultiplier = 0.25f;
          osg::Vec4f vSpriteDimensions(def.minPixelSize*def.minPixelSizeMultiplierForSprites, def.maxPixelSize, def.radius*radiusMultiplier, 0);
          it->second->addUniform(new osg::Uniform("spriteDimensions", vSpriteDimensions), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
          #endif
          
          lpn->setStateSet(it->second);
          //osg::notify(osg::NOTICE)<<"Lights Control: Setup exisiting lp: "<<lpn->getName() << "'s stateset" << std::endl;
          return;
        }
        
        osg::StateSet* stateSet = new osg::StateSet();
        stateSet->setRenderBinDetails(GROUND_SPRITE_LIGHT_POINTS_RENDER_BIN, "DepthSortedBin");
        
        // Turn off our lighting. We will use our own shader, primarily because we use logarithmic depth buffer,
        // but also because we could have an optional sprite texture
        stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF|osg::StateAttribute::PROTECTED|osg::StateAttribute::OVERRIDE);
        
        osg::StateAttribute::OverrideValue val = osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE;
        
        stateSet->setTextureAttributeAndModes(0,texture,val);
        stateSet->addUniform(new osg::Uniform("spriteTexture",0), val);
        
        if (useLogZDepthBuffer())
        {
          stateSet->setDefine("USE_LOG_DEPTH_BUFFER");
        }
        
        static const float radiusMultiplier = 0.25f;
        osg::Vec4f vSpriteDimensions(def.minPixelSize*def.minPixelSizeMultiplierForSprites, def.maxPixelSize, def.radius*radiusMultiplier, 0);
        stateSet->addUniform(new osg::Uniform("spriteDimensions", vSpriteDimensions), val);
        
        stateSet->setDefine("MODULATE_WITH_VERTEX_COLOR");
        stateSet->setDefine("WEIGHTED_HOT_SPOT");
        
        stateSet->setAttributeAndModes(_spriteProgram, val);
        
        _spriteStateSetCache.insert(std::make_pair(stateSetID, stateSet));
        
        // Override the state set because the LightPoint node uses an internal singleton stateset
        lpn->setStateSet(stateSet);
        //osg::notify(osg::NOTICE)<<"Lights Control: Setup NEW lp: "<<lpn->getName() << "'s stateset" << std::endl;
      }
    };
    
    void setUpSpriteStateSetProgram()
    {
      static bool checkedProgram = false;
      if (checkedProgram)
      {
        return;
      }
      checkedProgram = true;
      
      
      #if defined(_WIN32)
      std::string resourcesPath = OpenIG::Base::FileSystem::path(OpenIG::Base::FileSystem::Resources, "../resources");
      #else
      std::string resourcesPath = OpenIG::Base::FileSystem::path(OpenIG::Base::FileSystem::Resources, "../../openig/resources");
      #endif
      
      std::string strVS = OpenIG::Base::FileSystem::readFileIntoString(resourcesPath + "/shaders/sprite_bb_vs.glsl");
      std::string strGS = OpenIG::Base::FileSystem::readFileIntoString(resourcesPath + "/shaders/sprite_bb_gs.glsl");
      std::string strPS = OpenIG::Base::FileSystem::readFileIntoString(resourcesPath + "/shaders/sprite_bb_ps.glsl");
      if (strVS!=""&&strPS!=""&&strGS!="")
      {
        osg::notify(osg::NOTICE)<<"Lights Control: Successfully read in sprite programs (vs, gs, ps)"<<std::endl;
        _spriteProgram = new osg::Program;
        bool statusV = _spriteProgram->addShader(new osg::Shader(osg::Shader::VERTEX  , strVS));
        if(statusV)
          osg::notify(osg::NOTICE)<<"Lights Control: added sprite VERTEX shader!!"<<std::endl;
        else
          osg::notify(osg::NOTICE)<<"Lights Control: FAILED to add sprite VERTEX shader!!"<<std::endl;
        bool statusG = _spriteProgram->addShader(new osg::Shader(osg::Shader::GEOMETRY, strGS));
        if(statusG)
          osg::notify(osg::NOTICE)<<"Lights Control: added GEOMETRY shader!!"<<std::endl;
        else
          osg::notify(osg::NOTICE)<<"Lights Control: FAILED to add sprite GEOMETRY shader!!"<<std::endl;
        bool statusF = _spriteProgram->addShader(new osg::Shader(osg::Shader::FRAGMENT, strPS));
        if(statusF)
          osg::notify(osg::NOTICE)<<"Lights Control: added FRAGMENT shader!!"<<std::endl;
        else
          osg::notify(osg::NOTICE)<<"Lights Control: FAILED to add sprite FRAGMENT shader!!"<<std::endl;
      }
      else
      {
        osg::notify(osg::NOTICE)<<"####Lights Control: Error: could not load sprite programs (vs, gs, ps)####"<<std::endl;
      }
    }
    
    void setUpLightPointStateSetProgram()
    {
      static bool checkedProgram = false;
      if (checkedProgram)
      {
        return;
      }
      checkedProgram = true;
      
      #if defined(_WIN32)
      std::string resourcesPath = OpenIG::Base::FileSystem::path(OpenIG::Base::FileSystem::Resources, "../resources");
      #else
      std::string resourcesPath = OpenIG::Base::FileSystem::path(OpenIG::Base::FileSystem::Resources, "../../openig/resources");
      #endif
      osg::notify(osg::NOTICE)<<"Lights Control: setUpLightPointStateSetProgram resourcesPath: " << resourcesPath << std::endl;
      
      
      std::string strVS = OpenIG::Base::FileSystem::readFileIntoString(resourcesPath+"/shaders/lightpoint_vs.glsl");
      std::string strPS = OpenIG::Base::FileSystem::readFileIntoString(resourcesPath+"/shaders/lightpoint_ps.glsl");
      
      if (strVS!=""&&strPS!="")
      {
        _lightPointProgram = new osg::Program;
        _lightPointProgram->addShader(new osg::Shader(osg::Shader::VERTEX  , strVS));
        _lightPointProgram->addShader(new osg::Shader(osg::Shader::FRAGMENT, strPS));
        osg::notify(osg::NOTICE)<<"Lights Control: Loaded light point (fallback) programs"<<std::endl;
      }
      else
      {
        osg::notify(osg::NOTICE)<<"Lights Control: Error: could not load light point (fallback) programs"<<std::endl;
      }
    }
    
    void updateLightPointNodesBasedOnXMLDefinitions(osg::Node* node = 0)
    {
      if (!definitions.size()) return;
      
      setUpSpriteStateSetProgram();
      setUpLightPointStateSetProgram();
      UpdateLightPointNodeNodeVisitor nv(definitions, _textureCache, _spriteStateSetCache, _spriteProgram, _lightPointProgram, *_ig);
      
      if (node)
      {
        node->accept(nv);
      }
    }
    
    boost::shared_ptr<boost::thread>	_xmlFileObserverThread;
    volatile bool						_xmlThreadIsRunning;
    volatile bool						_xmlThreadRunningCondition;
    boost::mutex						_xmlAccessMutex;
    std::time_t							_xmlLastWriteTime;
    std::time_t							_xmlLastCheckedTime;
    
  public:
    static OpenThreads::Mutex	lightMutex;
  protected:
    
    // Thread function to check the
    // XML config file if changed
    void xmlFileObserverThread()
    {
      _xmlThreadIsRunning = true;
      while (_xmlThreadRunningCondition)
      {
        try
        {
          _xmlAccessMutex.lock();
          _xmlLastWriteTime = OpenIG::Base::FileSystem::lastWriteTime(_xmlFile);
          _xmlAccessMutex.unlock();
        }
        catch (const std::exception& e)
        {
          osg::notify(osg::NOTICE) << "Lights control: File montoring exception: " << e.what() << std::endl;
          break;
        }
        
        OpenThreads::Thread::microSleep(10000);
      }
      _xmlThreadIsRunning = false;
    }
    
  public:
    static TOD	tod;
    static TOD	onTOD;
    static TOD	offTOD;
    std::string msName;
    int         msIndex;
    
    typedef std::map< std::string, float >			LightPointBrightness;
    static LightPointBrightness						lightPointBrightness;
    
  protected:
    // Nodevisitor that will go through
    // LPNs and turn them on/off based
    // on the time of day and the XML
    // config
    struct AdjustLightPointNodesNodeVisitor : public osg::NodeVisitor
    {
      AdjustLightPointNodesNodeVisitor(OpenIG::Base::ImageGenerator* ig)
      : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN), _ig(ig), _msActiveChild(true)
      {
        _lightsEnabled =
        LightsControlPlugin::tod.hour() >= LightsControlPlugin::onTOD.hour() ||
        LightsControlPlugin::tod.hour() <= LightsControlPlugin::offTOD.hour();
      }
      
      void enableDisableLight(osgSim::LightPointNode* lpn)
      {
        bool always_on = true;
        if (lpn->getUserValue("always_on", always_on))
        {
          // Manage turn them on/off, the
          // lights implementations by the ig
          std::string ids;
          if (lpn->getUserValue("Real-Lights-IDs", ids) && !ids.empty())
          {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(OpenIG::Plugins::LightsControlPlugin::lightMutex);
            
            OpenIG::Base::StringUtils::Tokens tokens = OpenIG::Base::StringUtils::instance()->tokenize(ids, ";");
            OpenIG::Base::StringUtils::TokensIterator titr = tokens.begin();
            for (; titr != tokens.end(); ++titr)
            {
              const std::string& id = *titr;
              
              unsigned int lightId = atoi(id.c_str());
              if (lightId)
              {
                bool on = (always_on && _msActiveChild) ? true : _lightsEnabled;
                _ig->enableLight(lightId, on);
              }
            }
          }
          // And the light points
          bool on = (always_on && _msActiveChild) ? true : _lightsEnabled;
          for (size_t i = 0; i < lpn->getNumLightPoints(); ++i)
          {
            osgSim::LightPoint& lp = lpn->getLightPoint(i);
            lp._on = on;
          }
          
        }
      }
      
      virtual void apply(osg::Node& node)
      {
        osg::ref_ptr<osgSim::MultiSwitch> ms = dynamic_cast<osgSim::MultiSwitch*>(&node);
        if (ms.valid())
        {
          // NOTE: This will not work with nested MultiSwiches
          // In that case re-write to use stack
          bool enabled = _lightsEnabled;
          
          unsigned int activeSwitchSet = ms->getActiveSwitchSet();
          const osgSim::MultiSwitch::SwitchSetList& ssl = ms->getSwitchSetList();
          if (activeSwitchSet < ssl.size())
          {
            const osgSim::MultiSwitch::ValueList& vl = ms->getValueList(activeSwitchSet);
            for (size_t i = 0; i < vl.size(); ++i)
            {
              _msActiveChild = vl.at(i);
              _lightsEnabled = (vl.at(i)) ? enabled : false;
              ms->getChild(i)->accept(*this);
            }
          }
          
          _lightsEnabled = enabled;
          _msActiveChild = true;
        }
        else
        {
          osg::ref_ptr<osgSim::LightPointNode> lpn = dynamic_cast<osgSim::LightPointNode*>(&node);
          if (lpn.valid())
          {
            enableDisableLight(lpn.get());
          }
          else
            traverse(node);
        }
      }
      
    protected:
      OpenIG::Base::ImageGenerator* _ig;
      bool					_lightsEnabled;
      bool					_msActiveChild;
    };
    
    void updateLightPointNodesBasedOnTimeOfDay(osg::Node* node)
    {
      if (!LightsControlPlugin::tod.set() ||
        !LightsControlPlugin::onTOD.set() ||
        !LightsControlPlugin::offTOD.set()
      )
      {
        return;
      }
      
      OpenThreads::ScopedLock<OpenThreads::Mutex>	lock(_mutex);
      
      AdjustLightPointNodesNodeVisitor nv(_ig);
      if (node) node->accept(nv);
    }
  };
} // namespace
} // namespace

OpenIG::Plugins::LightsControlPlugin::TOD						OpenIG::Plugins::LightsControlPlugin::tod;
OpenIG::Plugins::LightsControlPlugin::TOD						OpenIG::Plugins::LightsControlPlugin::onTOD;
OpenIG::Plugins::LightsControlPlugin::TOD						OpenIG::Plugins::LightsControlPlugin::offTOD;
OpenThreads::Mutex												OpenIG::Plugins::LightsControlPlugin::lightMutex;
OpenIG::Plugins::LightsControlPlugin::LightPointDefinitions		OpenIG::Plugins::LightsControlPlugin::definitions;
OpenIG::Plugins::LightsControlPlugin::LightPointBrightness		OpenIG::Plugins::LightsControlPlugin::lightPointBrightness;
bool															OpenIG::Plugins::LightsControlPlugin::forwardPlusPluginAvailable = false;

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
  return new OpenIG::Plugins::LightsControlPlugin;
}

extern "C" EXPORT void DeletePlugin(OpenIG::PluginBase::Plugin* plugin)
{
  osg::ref_ptr<OpenIG::PluginBase::Plugin> p(plugin);
}
