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
#include <Core-Base/Animation.h>
#include <Core-Base/StringUtils.h>
#include <Core-Base/Commands.h>

#include <string>

#include <osgDB/FileNameUtils>

#include <Library-Bullet/BulletManager.h>

namespace OpenIG {
    namespace Plugins {

        class BulletPlugin : public OpenIG::PluginBase::Plugin
        {
        public:

            BulletPlugin() {}

            virtual std::string getName() { return "Bullet"; }

            virtual std::string getDescription() { return "Implements bullet3 and osgBullet"; }

            virtual std::string getVersion() { return "1.0.0"; }

            virtual std::string getAuthor() { return "ComPro, Nick & Roni"; }

            virtual void entityAdded(OpenIG::PluginBase::PluginContext&, unsigned int id, osg::Node& entity, const std::string& fileName)
            {
                if (id == 0) // we default 0 for terrain
                {
                    OpenIG::Library::Bullet::BulletManager::instance()->setupTerrain(entity);
                }
                else
                    if (id < 1000)
                    {
                        OpenIG::Base::ImageGenerator::Entity entityMXT = dynamic_cast<osg::MatrixTransform*>(&entity);
                        OpenIG::Library::Bullet::BulletManager::instance()->setupVehicle(id, entityMXT, fileName);
                    }
            }

            virtual void update(OpenIG::PluginBase::PluginContext& context)
            {
                if (OpenIG::Library::Bullet::BulletManager::instance()->getIsFreezed()) return;
#if 1
                bool bulletSteeringCommand = false;
                if (context.getOrCreateValueObject()->getUserValue("BulletSteeringCommand", bulletSteeringCommand) && bulletSteeringCommand)
                {
                    unsigned int vehicleID = 0;
                    if (context.getOrCreateValueObject()->getUserValue("vehicleID", vehicleID) && vehicleID)
                    {
                        if (OpenIG::Library::Bullet::BulletManager::instance()->getVehicle(vehicleID) != 0)
                        {
                            bool left = false;
                            if (context.getOrCreateValueObject()->getUserValue("left", left))
                            {
                                OpenIG::Library::Bullet::BulletManager::instance()->getVehicle(vehicleID)->setSteering(left);
                            }
                            else
                            {
                                OpenIG::Library::Bullet::BulletManager::instance()->getVehicle(vehicleID)->setSteering(false, false);
                            }
                        }
                        else
                        {
                            osg::notify(osg::NOTICE) << "Bullet: NULL vehicle with an ID: " << vehicleID << std::endl;
                        }
                    }
                }
                bool bulletEngineCommand = false;
                if (context.getOrCreateValueObject()->getUserValue("BulletEngineCommand", bulletEngineCommand) && bulletEngineCommand)
                {
                    unsigned int vehicleID = 0;
                    if (context.getOrCreateValueObject()->getUserValue("vehicleID", vehicleID) && vehicleID)
                    {
                        if (OpenIG::Library::Bullet::BulletManager::instance()->getVehicle(vehicleID) != 0)
                        {
                            bool engine = false;
                            if (context.getOrCreateValueObject()->getUserValue("engine", engine))
                            {
                                OpenIG::Library::Bullet::BulletManager::instance()->getVehicle(vehicleID)->setEngineForce(engine ? 1 : -1);
                            }
                            else
                            {
                                OpenIG::Library::Bullet::BulletManager::instance()->getVehicle(vehicleID)->setEngineForce(0);
                            }
                        }
                        else
                        {
                            osg::notify(osg::NOTICE) << "Bullet: NULL vehicle with an ID: " << vehicleID << std::endl;
                        }
                    }
                }

                OpenIG::Library::Bullet::BulletManager::instance()->update(context.getImageGenerator()->getViewer()->getView(0)->getFrameStamp()->getSimulationTime());
#endif
            }

            class BulletCommand : public OpenIG::Base::Commands::Command
            {
            public:
                virtual const std::string getUsage() const
                {
                    return "freeze/update";
                }

                virtual const std::string getArgumentsFormat() const
                {
                    return "{freeze;update}";
                }

                virtual const std::string getDescription() const
                {
                    return  "controls bullet to update entities or not\n"
                        "     freeze/update - one of these. defult is update";
                }

                virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
                {
                    if (tokens.size() == 1)
                    {
                        if (tokens.at(0).compare(0, 6, "update") == 0)
                        {
                            OpenIG::Library::Bullet::BulletManager::instance()->setFreeze(false);
                            osg::notify(osg::NOTICE) << "Bullet: update running" << std::endl;
                        }
                        else
                            if (tokens.at(0).compare(0, 6, "freeze") == 0)
                            {
                                OpenIG::Library::Bullet::BulletManager::instance()->setFreeze(true);
                                osg::notify(osg::NOTICE) << "Bullet: update frozen" << std::endl;
                            }
                    }

                    return 0;
                }
            };

            virtual void init(OpenIG::PluginBase::PluginContext& context)
            {
                OpenIG::Library::Bullet::BulletManager::instance()->init(context.getImageGenerator());

                OpenIG::Base::Commands::instance()->addCommand("bullet", new BulletCommand);
            }
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
    return new OpenIG::Plugins::BulletPlugin;
}

extern "C" EXPORT void DeletePlugin(OpenIG::PluginBase::Plugin* plugin)
{
    osg::ref_ptr<OpenIG::PluginBase::Plugin> p(plugin);
}
