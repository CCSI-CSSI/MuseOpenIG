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
#include <OpenIG-PluginBase/Plugin.h>
#include <OpenIG-PluginBase/PluginContext.h>

#include <OpenIG-Networking/Packet.h>
#include <OpenIG-Networking/Network.h>
#include <OpenIG-Networking/UDPNetwork.h>
#include <OpenIG-Networking/Buffer.h>
#include <OpenIG-Networking/Parser.h>
#include <OpenIG-Networking/Factory.h>

#include <OpenIG-Protocol/Header.h>
#include <OpenIG-Protocol/EntityState.h>
#include <OpenIG-Protocol/Camera.h>
#include <OpenIG-Protocol/TOD.h>
#include <OpenIG-Protocol/LightState.h>
#include <OpenIG-Protocol/Command.h>

#include <OpenIG-Base/Commands.h>

#include <iostream>

namespace OpenIG {
    namespace Plugins {

        struct Parser : public OpenIG::Library::Networking::Parser
        {
            Parser()
            {
                OpenIG::Library::Networking::Factory::instance()->addTemplate(new OpenIG::Library::Protocol::Header);
                OpenIG::Library::Networking::Factory::instance()->addTemplate(new OpenIG::Library::Protocol::EntityState);
                OpenIG::Library::Networking::Factory::instance()->addTemplate(new OpenIG::Library::Protocol::Camera);
                OpenIG::Library::Networking::Factory::instance()->addTemplate(new OpenIG::Library::Protocol::TOD);
                OpenIG::Library::Networking::Factory::instance()->addTemplate(new OpenIG::Library::Protocol::LightState);
                OpenIG::Library::Networking::Factory::instance()->addTemplate(new OpenIG::Library::Protocol::Command);
            }

            virtual OpenIG::Library::Networking::Packet* parse(OpenIG::Library::Networking::Buffer& buffer)
            {
                const unsigned char* opcode = buffer.fetch();

                OpenIG::Library::Networking::Packet* packet = OpenIG::Library::Networking::Factory::instance()->packet(*opcode);
                if (packet)
                {
                    packet->read(buffer);

                    OpenIG::Library::Protocol::Header* header = dynamic_cast<OpenIG::Library::Protocol::Header*>(packet);
                    if (header && header->magic != OpenIG::Library::Protocol::SWAP_BYTES_COMPARE)
                    {
                        buffer.setSwapBytes(true);
                    }
                }
                return packet;
            }

        };
        struct HeaderCallback : public OpenIG::Library::Networking::Packet::Callback
        {
            HeaderCallback(OpenIG::Base::ImageGenerator* ig)
                : imageGenerator(ig)
            {
            }

            void process(OpenIG::Library::Networking::Packet& packet)
            {
                OpenIG::Library::Protocol::Header* h = dynamic_cast<OpenIG::Library::Protocol::Header*>(&packet);
                if (h)
                {
                    if (h->masterIsDead == 1) imageGenerator->getViewer()->setDone(true);
                }
            }

            OpenIG::Base::ImageGenerator*	imageGenerator;
        };


        struct EntityStateCallback : public OpenIG::Library::Networking::Packet::Callback
        {
            EntityStateCallback(OpenIG::Base::ImageGenerator* ig)
                : imageGenerator(ig)
            {

            }

            virtual void process(OpenIG::Library::Networking::Packet& packet)
            {
                OpenIG::Library::Protocol::EntityState* es = dynamic_cast<OpenIG::Library::Protocol::EntityState*>(&packet);
                if (es)
                {
                    imageGenerator->updateEntity(es->entityID, es->mx);
                }
            }

            OpenIG::Base::ImageGenerator* imageGenerator;
        };

        struct TODCallback : public OpenIG::Library::Networking::Packet::Callback
        {
            TODCallback(OpenIG::Base::ImageGenerator* ig)
                : imageGenerator(ig)
            {

            }

            virtual void process(OpenIG::Library::Networking::Packet& packet)
            {
                OpenIG::Library::Protocol::TOD* tod = dynamic_cast<OpenIG::Library::Protocol::TOD*>(&packet);
                if (tod)
                {
                    imageGenerator->setTimeOfDay(tod->hour, tod->minutes);
                }
            }

            OpenIG::Base::ImageGenerator* imageGenerator;
        };

        struct CommandCallback : public OpenIG::Library::Networking::Packet::Callback
        {
            CommandCallback(OpenIG::Base::ImageGenerator* ig)
                : imageGenerator(ig)
            {

            }

            virtual void process(OpenIG::Library::Networking::Packet& packet)
            {
                OpenIG::Library::Protocol::Command* command = dynamic_cast<OpenIG::Library::Protocol::Command*>(&packet);
                if (command)
                {
                    OpenIG::Base::Commands::instance()->exec(command->command);
                }
            }

            OpenIG::Base::ImageGenerator* imageGenerator;
        };

        struct LightStateCallback : public OpenIG::Library::Networking::Packet::Callback
        {
            LightStateCallback(OpenIG::Base::ImageGenerator* ig)
                : imageGenerator(ig)
            {

            }

            virtual void process(OpenIG::Library::Networking::Packet& packet)
            {
                OpenIG::Library::Protocol::LightState* ls = dynamic_cast<OpenIG::Library::Protocol::LightState*>(&packet);
                if (ls)
                {
                    imageGenerator->enableLight(ls->id,ls->enabled);
                    std::cout << "Light ID: " << ls->id << ", enabled: " << ls->enabled << std::endl;
                }
            }

            OpenIG::Base::ImageGenerator* imageGenerator;
        };

        struct CameraCallback : public OpenIG::Library::Networking::Packet::Callback
        {
            CameraCallback(OpenIG::Base::ImageGenerator* ig)
                : imageGenerator(ig)
            {

            }

            virtual void process(OpenIG::Library::Networking::Packet& packet)
            {
                OpenIG::Library::Protocol::Camera* cp = dynamic_cast<OpenIG::Library::Protocol::Camera*>(&packet);
                if (cp)
                {
                    if (cp->bindToEntity)
                        imageGenerator->bindCameraUpdate(osg::Matrixd::inverse(cp->mx));
                    else
                    {
                        // And make it a view one by its inverse
                        imageGenerator->setCameraPosition(osg::Matrixd::inverse(cp->mx), true);
                    }
                }
            }

            OpenIG::Base::ImageGenerator*	imageGenerator;
        };


        class ClientPlugin : public OpenIG::PluginBase::Plugin
        {
        public:

            ClientPlugin()
                : _headingOffset(0.0)
            {
            }

            virtual std::string getName() { return "CustomPlugin"; }

            virtual std::string getDescription() { return "CustomPlugin - Description of the Plugin"; }

            virtual std::string getVersion() { return "1.0.0"; }

            virtual std::string getAuthor() { return "YourCompanyName, YourName"; }

            virtual void databaseRead(const std::string& fileName, osg::Node*, const osgDB::Options*)
            {
                std::cout << "CustomPlugin - databaseRead: " << fileName << std::endl;
            }

            virtual void databaseReadInVisitorBeforeTraverse(osg::Node&, const osgDB::Options*)
            {
                //std::cout << "CustomPlugin - databaseReadInVisitorBeforeTraverse" << std::endl;
            }

            virtual void databaseReadInVisitorAfterTraverse(osg::Node&)
            {
                //std::cout << "CustomPlugin - databaseReadInVisitorAfterTraverse" << std::endl;
            }

            virtual void config(const std::string& fileName)
            {
                std::cout << "CustomPlugin - config" << fileName << std::endl;
            }

            virtual void init(OpenIG::PluginBase::PluginContext& context)
            {
                std::cout << "CustomPlugin - init" << std::endl;

                std::string		host = "127.0.0.1";
                unsigned int	port = 8888;

                _network = boost::shared_ptr<OpenIG::Library::Networking::UDPNetwork>(new OpenIG::Library::Networking::UDPNetwork(host));
                _network->setPort(port);

                _network->addCallback((OpenIG::Library::Networking::Packet::Opcode)OPCODE_HEADER, new HeaderCallback(context.getImageGenerator()));
                _network->addCallback((OpenIG::Library::Networking::Packet::Opcode)OPCODE_ENTITYSTATE, new EntityStateCallback(context.getImageGenerator()));
                _network->addCallback((OpenIG::Library::Networking::Packet::Opcode)OPCODE_CAMERA, new CameraCallback(context.getImageGenerator()));
                _network->addCallback((OpenIG::Library::Networking::Packet::Opcode)OPCODE_TOD, new TODCallback(context.getImageGenerator()));
                _network->addCallback((OpenIG::Library::Networking::Packet::Opcode)OPCODE_LIGHTSTATE, new LightStateCallback(context.getImageGenerator()));
                _network->addCallback((OpenIG::Library::Networking::Packet::Opcode)OPCODE_COMMAND, new CommandCallback(context.getImageGenerator()));

                _network->setParser(new Parser);
            }

            virtual void update(OpenIG::PluginBase::PluginContext&)
            {
                //std::cout << "CustomPlugin - update" << std::endl;

                if (_network)
                {
                    _network->process();
                }
            }

            virtual void preFrame(OpenIG::PluginBase::PluginContext&, double)
            {
                //std::cout << "CustomPlugin - preFrame" << std::endl;
            }

            virtual void postFrame(OpenIG::PluginBase::PluginContext&, double)
            {
                //std::cout << "CustomPlugin - postFrame" << std::endl;
            }

            virtual void clean(OpenIG::PluginBase::PluginContext&)
            {
                std::cout << "CustomPlugin - clean" << std::endl;
            }

            virtual void entityAdded(OpenIG::PluginBase::PluginContext&, unsigned int, osg::Node&, const std::string& fileName)
            {
                std::cout << "CustomPlugin - entityAdded: " << fileName << std::endl;
            }

        protected:
            // The network
            boost::shared_ptr<OpenIG::Library::Networking::Network>		_network;
            // The View heading offset. Passed from the IG
            double														_headingOffset;
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
    return new OpenIG::Plugins::ClientPlugin;
}

extern "C" EXPORT void DeletePlugin(OpenIG::PluginBase::Plugin* plugin)
{
    osg::ref_ptr<OpenIG::PluginBase::Plugin> p(plugin);
}
