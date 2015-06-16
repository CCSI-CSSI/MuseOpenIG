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
// note: This is experimental, and it is valid
// with the datanase that is provided by ComPro
// for the OpenIG Demo. Will be revisited soonest
// to make it available as standard plugin to
// OpenIG. Nick

#include <IgPluginCore/plugin.h>
#include <IgPluginCore/plugincontext.h>

#include <IgCore/attributes.h>
#include <IgCore/globalidgenerator.h>
#include <IgCore/mathematics.h>

#include <osg/Node>
#include <osg/ValueObject>

#include <osgSim/LightPointNode>
#include <osgSim/MultiSwitch>

#include <osgDB/FileNameUtils>

#include <iostream>
#include <sstream>

namespace igplugins
{

class RunwayLightsPlugin : public igplugincore::Plugin
{
public:

    RunwayLightsPlugin()
        : _ig(0)
    {

    }

    virtual std::string getName() { return "RunwayLights"; }

    virtual std::string getDescription( ) { return "Adds real lights computed in a shader to runways (experimental)"; }

    virtual std::string getVersion() { return "1.0.0"; }

    virtual std::string getAuthor() { return "ComPro, Nick"; }

    class FindAirportNodeVisitor : public osg::NodeVisitor
    {
    public:
        FindAirportNodeVisitor(const std::string& name)
            : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
            , _name(name)
        {

        }

        virtual void apply(osg::Node& node)
        {
#if 0
            if (node.getName().substr(0,osg::minimum(node.getName().length(),_name.length())) == _name)
            {
                osg::notify(osg::NOTICE) << "Runway Lights: node name: " << node.getName() << ", " << (node.getNumParents() ? node.getParent(0)->getName() : "") << std::endl;
            }
#endif
            if (node.getName() == _name)
            {
                _airport = &node;
            }
            else
                traverse(node);
        }

        osg::ref_ptr<osg::Node> _airport;

    protected:
        std::string _name;
    };

    class FindRunwayLightsNodeVisitor : public osg::NodeVisitor
    {
    public:
        FindRunwayLightsNodeVisitor(const std::string& namePattern)
            : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
            , _namePattern(namePattern)
        {

        }


        virtual void apply(osg::Node& node)
        {
            osg::ref_ptr<osgSim::MultiSwitch> ms = dynamic_cast<osgSim::MultiSwitch*>(&node);
            osg::ref_ptr<osgSim::LightPointNode> lpn = dynamic_cast<osgSim::LightPointNode*>(&node);

            if (lpn.valid() && lpn->getName().substr(0,osg::minimum(lpn->getName().length(),_namePattern.length())) == _namePattern)
            {
                _lightPoints.push_back(lpn);
            }
            else
            // note: temporary. Valid only
            // for the OpenIG demo database
            // and the Istrana airport
            if (ms.valid() && ms->getNumChildren()>1)
            {
                ms->getActiveSwitchSet();
                traverse(*ms->getChild(1));
            }
            else
                traverse(node);
        }

        typedef std::vector< osg::ref_ptr<osgSim::LightPointNode> >     LightPoints;
        LightPoints _lightPoints;

    protected:
        std::string _namePattern;
    };

    typedef std::vector< osg::observer_ptr<osg::PagedLOD> >     PagedLODs;
    typedef std::map< std::string, PagedLODs>                   PagedLODMap;

    class FindPagedLODsNodeVisitor : public osg::NodeVisitor
    {
    public:
        FindPagedLODsNodeVisitor()
            : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
        {

        }

        virtual void apply(osg::Node& node)
        {
            osg::observer_ptr<osg::PagedLOD> plod = dynamic_cast<osg::PagedLOD*>(&node);
            if (plod.valid())
            {
                _pagedLODs.push_back(plod);
            }
            traverse(node);
        }

        PagedLODs                       _pagedLODs;
    };

    virtual void databaseRead(const std::string& fileName, osg::Node* node, const osgDB::Options*)
    {
        if (!_ig) return;

        osg::notify(osg::NOTICE) << "Runway Lights: loading " << fileName << std::endl;

        FindPagedLODsNodeVisitor nv;
        node->accept(nv);

        PagedLODs& plods = _pagedLODs[osgDB::getSimpleFileName(fileName)];
        plods.insert(plods.begin(),nv._pagedLODs.begin(),nv._pagedLODs.end());

        osg::notify(osg::NOTICE) << "Runway Lights: PagedLODs for " << osgDB::getSimpleFileName(fileName) << std::endl;
        PagedLODs::iterator itr = plods.begin();
        for ( ; itr != plods.end(); ++itr )
        {
            osg::observer_ptr<osg::PagedLOD>& plod = *itr;
            if (plod.valid())
            {
                osg::notify(osg::NOTICE) << "\t" << plod->getFileName(0) << std::endl;
            }

            _pagedLODsNodes[plod->getFileName(0)] = plod;
        }

        // note: temporary. Valid only
        // for the OpenIG demo database
        // and the Istrana airport
        FindAirportNodeVisitor fanv("AFL_LIPS");
        node->accept(fanv);

        if (fanv._airport.valid())
        {
            osg::observer_ptr<osg::PagedLOD> plod = _pagedLODsNodes[osgDB::getSimpleFileName(fileName)];
            if (plod.valid())
            {
                plod->setUserData(fanv._airport);
            }

        }

    }

    virtual void init(igplugincore::PluginContext& context)
    {
        _ig = context.getImageGenerator();
    }

    virtual void preFrame(igplugincore::PluginContext&, double)
    {
        if (!_ig) return;

        PagedLODMap::iterator itr = _pagedLODs.begin();
        for ( ; itr != _pagedLODs.end(); ++itr )
        {
            PagedLODs& plods = itr->second;
            for (size_t p = 0; p < plods.size(); ++p)
            {
                osg::observer_ptr<osg::PagedLOD> plod = plods.at(p);

                if (plod.valid() && plod->getNumChildren()==0)
                {
                    PagedLODsLights::iterator pitr = _pagedLODsLights.find(itr->first);
                    if ( pitr == _pagedLODsLights.end() ) continue;

                    igcore::GlobalIdGenerator::instance()->setAvailableIds("Real-Lights",pitr->second);

                    for (size_t i = 0; i < pitr->second.size(); ++ i)
                    {
                        unsigned int id = pitr->second.at(i);
                        _ig->removeLight(id);
                    }

                    _pagedLODsLights.erase(pitr);

                    osg::notify(osg::NOTICE) << "Runway lights: removing lights from PagedLOD " << itr->first << std::endl;
                }
                else
                if (plod.valid() && plod->getNumChildren())
                {
                    bool processed = false;
                    if (!plod->getUserValue("processed",processed))
                    {
                        plod->setUserValue("processed",(bool)true);

                        osg::notify(osg::NOTICE) << "Runway lights: processing PagedLOD " << plod->getFileName(0) << std::endl;
                    }
                    if (processed) continue;

                    osg::observer_ptr<osg::Node> airport = dynamic_cast<osg::Node*>(plod->getUserData());
                    if (!airport.valid())
                    {
                        osg::notify(osg::NOTICE) << "\tno airport node found" << std::endl;
                        continue;
                    }

                    // note: temporary. Valid only
                    // for the OpenIG demo database
                    // and the Istrana airport
                    FindRunwayLightsNodeVisitor nv("Runway_Edge");
                    airport->accept(nv);

                    if (nv._lightPoints.size()==0)
                    {
                        osg::notify(osg::NOTICE) << "\tno light points in the airport node found" << std::endl;
                        continue;
                    }

                    LightsIDs& lids = _pagedLODsLights[plod->getFileName(0)];

                    FindRunwayLightsNodeVisitor::LightPoints::iterator litr = nv._lightPoints.begin();
                    for ( ; litr != nv._lightPoints.end(); ++litr )
                    {
                        osg::ref_ptr<osgSim::LightPointNode>& lpn = *litr;

                        osg::NodePath np;
                        np.push_back(lpn);

                        osg::ref_ptr<osg::Group> parent = lpn->getNumParents() ? lpn->getParent(0) : 0;
                        while (parent)
                        {
                            np.insert(np.begin(),parent);
                            parent = parent->getNumParents() ? parent->getParent(0) : 0;
                        }

                        osg::Matrixd wmx = osg::computeLocalToWorld(np);

                        std::ostringstream oss;
                        for (size_t i = 0; i < lpn->getNumLightPoints(); ++i )
                        {
                            osgSim::LightPoint& lp = lpn->getLightPoint(i);

                            unsigned int id  = 0;
                            if ( igcore::GlobalIdGenerator::instance()->getNextId("Real-Lights",id) )
                            {
                                lids.push_back(id);

                                osg::Matrixd mx;

                                if (lp._sector.valid())
                                {
                                    osg::ref_ptr<osgSim::DirectionalSector> ds = dynamic_cast<osgSim::DirectionalSector*>(lp._sector.get());
                                    if (ds.valid())
                                    {
                                        osg::Vec3 direction = ds->getDirection();

                                        osg::Quat q;
                                        q.makeRotate(osg::Vec3(0,1,0),direction);

                                        osg::Vec3d hpr = igcore::Math::instance()->fromQuat(q);

                                        mx = igcore::Math::instance()->toMatrix(
                                                lp._position.x(),
                                                lp._position.y(),
                                                lp._position.z(),
                                                osg::RadiansToDegrees(hpr.x()),0,0);
                                    }
                                }
                                else
                                {
                                    mx = osg::Matrixd::translate(lp._position);
                                }

                                osg::Matrixd final = mx * wmx;

                                _ig->addLight(id,final);

                                oss << id << ";";

                                igcore::LightAttributes la;
                                la._diffuse = lp._color;
                                la._ambient = osg::Vec4(0,0,0,1);
                                la._specular = osg::Vec4(0,0,0,1);
                                la._constantAttenuation = 50;
                                la._brightness = 100;
                                la._spotCutoff = 20;
                                la._dirtyMask = igcore::LightAttributes::ALL;

                                _ig->updateLightAttributes(id,la);

#if 0
                                osg::notify(osg::NOTICE) << "Runway lights: adding light with id:" << id << std::endl;
                                osg::notify(osg::NOTICE) << "\t r:" << lp._color.r() << " g:" << lp._color.g() << " b:" << lp._color.b() << std::endl;
                                osg::notify(osg::NOTICE) << "\t x:" << lp._position.x() << " y:" << lp._position.y() << " z:" << lp._position.z() << std::endl;
#endif
                            }
                            else
                            {
#if 0
                                osg::notify(osg::NOTICE) << "Runway lights: id out of available range." << std::endl;
#endif
                                break;
                            }
                        }

                        lpn->setUserValue("Real-Lights-IDs",oss.str());
                    }

                    airport->getOrCreateStateSet()->setDefine("LIGHTING");

                }
            }
        }
    }

protected:
    igcore::ImageGenerator*             _ig;
    PagedLODMap                         _pagedLODs;

    typedef std::vector< unsigned int >                                 LightsIDs;
    typedef std::map< std::string, LightsIDs >                          PagedLODsLights;
    typedef std::map< std::string, osg::observer_ptr<osg::PagedLOD> >   PagedLODsNodes;

    PagedLODsLights                     _pagedLODsLights;
    PagedLODsNodes                      _pagedLODsNodes;
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
    return new igplugins::RunwayLightsPlugin;
}

extern "C" EXPORT void DeletePlugin(igplugincore::Plugin* plugin)
{
    osg::ref_ptr<igplugincore::Plugin> p(plugin);
}
