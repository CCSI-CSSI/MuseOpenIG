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
            #define USE_LOG_DEPTH_BUFFER

            const char *VertSource = {
#ifdef USE_LOG_DEPTH_BUFFER
                "uniform float Fcoef;																		\n"
#endif
				"varying vec3 eyeVec;																		\n"
				"void main()																				\n"
                "{																							\n"
				"   eyeVec = -vec3(gl_ModelViewMatrix * gl_Vertex);											\n"
                "   gl_FrontColor = gl_Color;																\n"
                "   gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
#ifdef USE_LOG_DEPTH_BUFFER
                "   gl_Position.z = (log2(max(1e-6, 1.0 + gl_Position.w)) * Fcoef - 1.0) * gl_Position.w;	\n"
#endif
                "}																							\n"
            };


            const char *FragSource = {
				"varying vec3 eyeVec;                                                   \n"
				"void computeFogColor(inout vec4 color)                                 \n"
				"{                                                                      \n"
				"   float fogExp = gl_Fog.density * length(eyeVec);                     \n"
				"   float fogFactor = exp(-(fogExp * fogExp));                          \n"
				"   fogFactor = clamp(fogFactor, 0.0, 1.0);                             \n"
				"   vec4 clr = color;                                                   \n"
				"   color = mix(gl_Fog.color, color, fogFactor);                        \n"
				"   color.a = clr.a;                                                    \n"
				"}                                                                      \n"
				"void main()															\n"
				"{																		\n"
				"	vec4 color = gl_Color;												\n"
				"	computeFogColor(color);												\n"
                "	gl_FragColor = color;												\n"
				"}																		\n"
            };

            osg::ref_ptr<osgSim::LightPointNode> lpn = dynamic_cast<osgSim::LightPointNode*>(&node);
            if (lpn.valid())
            {
                osg::ref_ptr<LightPointNode> mylpn = new LightPointNode(*lpn.get());
                mylpn->setStateSet(lpn->getOrCreateStateSet());


                osg::ref_ptr<osg::Program> pointProgram = new osg::Program;
                pointProgram->addShader(new osg::Shader(osg::Shader::VERTEX, VertSource));
                pointProgram->addShader(new osg::Shader(osg::Shader::FRAGMENT, FragSource));

                osg::StateSet *stateSet = lpn->getOrCreateStateSet();
                stateSet->setAttributeAndModes(pointProgram.get(), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);

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

	class FindAndReplaceLightPointNodesNodeVisitor2 : public osg::NodeVisitor
	{
	public:
		FindAndReplaceLightPointNodesNodeVisitor2()
			: osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
		{

		}

		virtual void apply(osg::Node& node)
		{
			osg::ref_ptr<osgSim::LightPointNode> lpn = dynamic_cast<osgSim::LightPointNode*>(&node);
			if (lpn.valid())
			{
				lps.push_back(lpn);
			}

			traverse(node);
		}

		std::vector< osg::ref_ptr<osgSim::LightPointNode> >		lps;
	};

    virtual void databaseRead(const std::string&, osg::Node* node, const osgDB::Options*)
    {
#if 1
        FindAndReplaceLightPointNodesNodeVisitor nv;
        node->accept(nv);
#else
		FindAndReplaceLightPointNodesNodeVisitor2 nv;
		node->accept(nv);

		if (nv.lps.size())
		{
			osg::ref_ptr<LightPointNode> mylpn = new LightPointNode();
			mylpn->setStateSet(nv.lps.at(0)->getOrCreateStateSet());
			mylpn->getOrCreateStateSet()->setAttributeAndModes(new osg::Program, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);

			std::vector< osg::ref_ptr<osgSim::LightPointNode> >::iterator itr = nv.lps.begin();
			for (; itr != nv.lps.end(); ++itr)
			{
				osgSim::LightPointNode* lp = *itr;

				osgSim::LightPointNode::LightPointList& lps = lp->getLightPointList();

				mylpn->getLightPointList().insert(mylpn->getLightPointList().end(), lps.begin(), lps.end());

				const osg::Node::ParentList& parents = lp->getParents();
				osg::Node::ParentList::const_iterator pitr = parents.begin();
				for (; pitr != parents.end(); ++pitr)
				{
					(**pitr).removeChild(lp);
				}
			}

			node->asGroup()->addChild(mylpn);

		}
#endif
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
