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

#include <IgCore/attributes.h>

#include <osg/Node>
#include <osg/Program>
#include <osg/Material>

#include <osgSim/LightPointNode>

#include "lightpointnode.h"

#include <iostream>

namespace igplugins
{

class LightPointsPlugin : public igplugincore::Plugin
{
public:

    LightPointsPlugin() {}

    virtual std::string getName() { return "LightPoints"; }

    virtual std::string getDescription( ) { return "Disable the lighting shader for Light Points and use FFP"; }

    virtual std::string getVersion() { return "1.0.0"; }

    virtual std::string getAuthor() { return "ComPro, Nick"; }

    class FindAndReplaceLightPointNodesNodeVisitor : public osg::NodeVisitor
    {
    public:
        FindAndReplaceLightPointNodesNodeVisitor()
            : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
        {

        }

        virtual void apply(osg::Node& node)
        {
            osg::ref_ptr<osgSim::LightPointNode> lpn = dynamic_cast<osgSim::LightPointNode*>(&node);
            if (lpn.valid())
            {
                osg::ref_ptr<LightPointNode> mylpn = new LightPointNode(*lpn.get());
                mylpn->setStateSet(lpn->getOrCreateStateSet());
                mylpn->getOrCreateStateSet()->setAttributeAndModes(new osg::Program, osg::StateAttribute::ON|osg::StateAttribute::PROTECTED|osg::StateAttribute::OVERRIDE);

                const osg::Node::ParentList& parents = node.getParents();
                osg::Node::ParentList::const_iterator itr = parents.begin();
                for ( ; itr != parents.end(); ++itr)
                {
                    (**itr).replaceChild(lpn,mylpn);
                }
            }

            traverse(node);
        }
    };

    virtual void databaseRead(const std::string&, osg::Node* node, const osgDB::Options*)
    {
        FindAndReplaceLightPointNodesNodeVisitor nv;
        node->accept(nv);
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
    return new igplugins::LightPointsPlugin;
}

extern "C" EXPORT void DeletePlugin(igplugincore::Plugin* plugin)
{
    osg::ref_ptr<igplugincore::Plugin> p(plugin);
}
