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
#include <IgCore/animation.h>
#include <IgCore/stringutils.h>
#include <IgCore/commands.h>

#include <string>

#include <osgDB/FileNameUtils>

#include <IgLib-Bullet/bulletmanager.h>

namespace igplugins
{

	class BulletPlugin : public igplugincore::Plugin
	{
	public:

		BulletPlugin() {}

		virtual std::string getName() { return "Bullet"; }

		virtual std::string getDescription() { return "Implements bullet3 and osgBullet"; }

		virtual std::string getVersion() { return "1.0.0"; }

		virtual std::string getAuthor() { return "ComPro, Nick & Roni"; }

		virtual void entityAdded(igplugincore::PluginContext&, unsigned int id, osg::Node& entity, const std::string& fileName)
		{
			if (id == 0) // we default 0 for terrain
			{
				iglib::BulletManager::instance()->setupTerrain(entity);
			}
			else
			if (id < 1000)
			{
				igcore::ImageGenerator::Entity entityMXT = dynamic_cast<osg::MatrixTransform*>(&entity);
				iglib::BulletManager::instance()->setupVehicle(id, entityMXT, fileName);
			}
		}

		virtual void update(igplugincore::PluginContext& context)
		{
#if 1
			bool bulletSteeringCommand = false;
			if (context.getOrCreateValueObject()->getUserValue("BulletSteeringCommand", bulletSteeringCommand) && bulletSteeringCommand)
			{
				unsigned int vehicleID = 0;
				if (context.getOrCreateValueObject()->getUserValue("vehicleID", vehicleID) && vehicleID)
				{
					if (iglib::BulletManager::instance()->getVehicle(vehicleID) != 0)
					{
						bool left = false;
						if (context.getOrCreateValueObject()->getUserValue("left", left))
						{
							iglib::BulletManager::instance()->getVehicle(vehicleID)->setSteering(left);
						}
						else
						{
							iglib::BulletManager::instance()->getVehicle(vehicleID)->setSteering(false, false);
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
					if (iglib::BulletManager::instance()->getVehicle(vehicleID) != 0)
					{
						bool engine = false;
						if (context.getOrCreateValueObject()->getUserValue("engine", engine))
						{
							iglib::BulletManager::instance()->getVehicle(vehicleID)->setEngineForce(engine ? 1 : -1);
						}
						else
						{
							iglib::BulletManager::instance()->getVehicle(vehicleID)->setEngineForce(0);
						}
					}
					else
					{
						osg::notify(osg::NOTICE) << "Bullet: NULL vehicle with an ID: " << vehicleID << std::endl;
					}
				}
			}			

			iglib::BulletManager::instance()->update(context.getImageGenerator()->getViewer()->getView(0)->getFrameStamp()->getSimulationTime());
#endif
		}

		virtual void init(igplugincore::PluginContext& context)
		{
			iglib::BulletManager::instance()->init(context.getImageGenerator());
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
	return new igplugins::BulletPlugin;
}

extern "C" EXPORT void DeletePlugin(igplugincore::Plugin* plugin)
{
	osg::ref_ptr<igplugincore::Plugin> p(plugin);
}
