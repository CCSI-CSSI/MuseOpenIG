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
#include <Core-Base/Commands.h>
#include <Core-Base/Types.h>
#include <Core-Base/Mathematics.h>

#include "fgmersivelib.h"

//#if !defined (_WIN32)
//	#include <cstshare/cstshareobject.h>
//#endif

#include <osg/ref_ptr>
#include <osg/Vec3>
#include <osg/Camera>
#include <osg/Callback>

#include <osgDB/XmlParser>

#include <iostream>

namespace OpenIG {
    namespace Plugins {

        class MersivePlugin : public PluginBase::Plugin
        {
        public:
            MersivePlugin()
            {
            }

            class MersiveChannel
            {
            public:
                int         channel_screen_number;
                std::string channel_name;
                float       channel_width;
                float       channel_height;
                std::string channel_ip;
            };
            typedef std::vector<MersiveChannel*> _tmersiveChannel;
            _tmersiveChannel             *_vmersiveChannel;

            virtual std::string getName() { return "Mersive"; }

            virtual std::string getDescription() { return "Integration sample of OpenIG and the Mersive Warper SDK"; }

            virtual std::string getVersion() { return "2.0.0"; }

            virtual std::string getAuthor() { return "ComPro, CGR"; }

            class MChannelCommand : public OpenIG::Base::Commands::Command
            {
            public:
                MChannelCommand(OpenIG::Base::ImageGenerator* ig, _tmersiveChannel *tmersiveChannel)
                    : _ig(ig), vmersiveChannel(tmersiveChannel) {}

                virtual const std::string getUsage() const
                {
                    return "mersive-channel-name";
                }

                virtual const std::string getArgumentsFormat() const
                {
                    return	"D";
                }

                virtual const std::string getDescription() const
                {
                    return  "add a new Mersvie channel\n"
                        "     mersive-channel-name -- as defined in your Mersive Servers Setup/Configuration/Alignment Configuration";
                }

                virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
                {
                    std::string _channel_name = tokens.at(0);
                    osg::notify(osg::NOTICE) << "Mersive: tokens.at(0): " <<_channel_name << std::endl;

                    if (tokens.size() == 1)
                    {
                        std::vector<MersiveChannel*>::iterator m_it;

                        for (m_it = vmersiveChannel->begin(); m_it != vmersiveChannel->end(); ++m_it)
                        {
//                            bool test = ( _channel_name.compare(0,((*m_it)->channel_name.size()),(*m_it)->channel_name) == 0);
//                            osg::notify(osg::NOTICE) << "Mersive: COMPARE: " << (test ? "TRUE" : "FALSE") << std::endl;
//                            osg::notify(osg::NOTICE) << "Mersive: sizeof _channel_name: " << _channel_name.size() << std::endl;
//                            osg::notify(osg::NOTICE) << "Mersive: sizeof m_it->channel_name: " << (*m_it)->channel_name.size() << std::endl;

                            if( (_channel_name.compare(0,((*m_it)->channel_name.size()),(*m_it)->channel_name)) == 0 )
                            {
                                MersiveOsgWarperCallback *m_warperCallback = new MersiveOsgWarperCallback( (*m_it)->channel_name,
                                                                                                           (*m_it)->channel_width,
                                                                                                           (*m_it)->channel_height,
                                                                                                           (*m_it)->channel_ip,
                                                                                                                         NULL);


                                _ig->getViewer()->getView(0)->getCamera()->setPostDrawCallback(m_warperCallback);
#if 1
                                if((*m_it)->channel_screen_number>0)
                                    _ig->getViewer()->getView(0)->setUpViewOnSingleScreen((*m_it)->channel_screen_number);
#else

                                //This is not apparently configured/installed with OSG for some reason even though the above call is deprecated....
                                //Needs to be investigated should the above call be removed...
                                _ig->getViewer()->getView(0)->apply(new osgViewer::SingleScreen());
#endif
#if 0
                                osg::notify(osg::NOTICE) << "Mersive: added channel: screen: " << (*m_it)->channel_screen_number << std::endl;
                                osg::notify(osg::NOTICE) << "Mersive: added channel:   name: " << (*m_it)->channel_name << std::endl;
                                osg::notify(osg::NOTICE) << "Mersive: added channel:  width: " << (*m_it)->channel_width << std::endl;
                                osg::notify(osg::NOTICE) << "Mersive: added channel: height: " << (*m_it)->channel_height << std::endl;
                                osg::notify(osg::NOTICE) << "Mersive: added channel: svr ip: " << (*m_it)->channel_ip << std::endl;
#endif
                            }
                        }
                        return 0;
                    }
                    osg::notify(osg::NOTICE) << "Mersive: unable to add channel: " << _channel_name << std::endl;
                    osg::notify(osg::NOTICE) << "Possibly it is not in the libOpenIG-Plugin-Mersive.XX.xml datafile!!!!" << std::endl;
                    return -1;
                }
            protected:
                OpenIG::Base::ImageGenerator* _ig;
                _tmersiveChannel *vmersiveChannel;
            };

            virtual void config(const std::string& xmlFileName)
            {
                _vmersiveChannel = new _tmersiveChannel;

                osg::notify(osg::NOTICE) << "Mersive: parsing xml file: " << xmlFileName << std::endl;

                osgDB::XmlNode* root = osgDB::readXmlFile(xmlFileName);
                if (!root || !root->children.size() || root->children.at(0)->name != "MersiveChannelSettings") return;

                osgDB::XmlNode::Children::iterator itr = root->children.at(0)->children.begin();
                for (; itr != root->children.at(0)->children.end(); ++itr)
                {
                    osgDB::XmlNode* child = *itr;
                    mchannel = new MersiveChannel;

                    //<Channel name="otw-c-0" width="1680" height="1050" serverip="10.5.63.109"/>
                    if (child->name == "Channel")
                    {
                        osgDB::XmlNode::Properties::iterator pitr = child->properties.begin();
                        for (; pitr != child->properties.end(); ++pitr)
                        {

                            if (pitr->first == "screen")
                            {
                                mchannel->channel_screen_number = atoi(pitr->second.c_str());
                            }
                            if (pitr->first == "name")
                            {
                                mchannel->channel_name = pitr->second;
                            }

                            if (pitr->first == "width")
                            {
                                mchannel->channel_width = atof(pitr->second.c_str());
                            }

                            if (pitr->first == "height")
                            {
                                mchannel->channel_height = atof(pitr->second.c_str());
                            }

                            if (pitr->first == "serverip")
                            {
                                mchannel->channel_ip = pitr->second;
                            }
                        }
                    }
#if 0
                    osg::notify(osg::NOTICE) << "Mersive: parsed channel:   name: " << mchannel->channel_name << std::endl;
                    osg::notify(osg::NOTICE) << "Mersive: parsed channel: screen: " << mchannel->channel_screen_number << std::endl;
                    osg::notify(osg::NOTICE) << "Mersive: parsed channel:  width: " << mchannel->channel_width << std::endl;
                    osg::notify(osg::NOTICE) << "Mersive: parsed channel: height: " << mchannel->channel_height << std::endl;
                    osg::notify(osg::NOTICE) << "Mersive: parsed channel: svr ip: " << mchannel->channel_ip << std::endl;
#endif
                    _vmersiveChannel->push_back(mchannel);
                }

            }

            virtual void init(PluginBase::PluginContext& context)
            {
                OpenIG::Base::Commands::instance()->addCommand("mchannel", new MChannelCommand(context.getImageGenerator(), _vmersiveChannel));
            }


        protected:
            std::string                      _xmlFileName;
            std::string                 _xml_channel_name;
            MersiveChannel*                      mchannel;

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
    return new OpenIG::Plugins::MersivePlugin;
}

extern "C" EXPORT void DeletePlugin(OpenIG::PluginBase::Plugin* plugin)
{
    osg::ref_ptr<OpenIG::PluginBase::Plugin> p(plugin);
}
