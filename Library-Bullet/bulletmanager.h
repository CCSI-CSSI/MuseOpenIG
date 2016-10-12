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
//#*****************************************************************************
//#*	author    Roni Zanoli <openig@compro.net>
//#*	copyright(c)Compro Computer Services, Inc.
//#*
//#*    Please direct any questions or comments to the OpenIG Forums
//#*    Email address: openig@compro.net
//#*

#ifndef BULLETMANAGER_H
#define BULLETMANAGER_H

#include <string>

#include <btBulletDynamicsCommon.h>

#include <osg/Vec3>
#include <osg/Vec4>

#include <osgbCollision/GLDebugDrawer.h>

#if defined(OPENIG_SDK)
	#include <OpenIG-Base/mathematics.h>

	#include <OpenIG-Bullet/configreader.h>
	#include <OpenIG-Bullet/vehicle.h>
	#include <OpenIG-Bullet/export.h>
#else
	#include <Core-Base/mathematics.h>

	#include <Library-Bullet/configreader.h>
	#include <Library-Bullet/vehicle.h>
	#include <Library-Bullet/export.h>
#endif

namespace OpenIG {
	namespace Library {
		namespace Bullet {


			class IGLIBBULLET_EXPORT BulletManager
			{
			protected:
				BulletManager();
				~BulletManager();

			public:
				static BulletManager*			instance();

				void							init(OpenIG::Base::ImageGenerator* ig, bool debug = false);
				void							clean();

				Vehicle*						getVehicle(int id);
				osgbCollision::GLDebugDrawer*	getDebugDrawer(void) { return dbgDraw; }
				void							setDebug(void);
				void							resetScene(void);
				void							update(const double sim_time);
				inline bool						getInitialContact(void) { return initial_contact; }

				void							setupTerrain(osg::Node& entity);
				void							setupVehicle(unsigned int id, OpenIG::Base::ImageGenerator::Entity& entity, const std::string& fileName);
				inline void                     setFreeze(bool on)          { _freeze = on; }
				inline bool						getIsFreezed() const		{ return _freeze; }

			private:
				void					initPhysics(void);
				void					createVehicle(VehicleData* vd, osg::ref_ptr< osg::Node > nodeDB, osg::Vec4 r);
				void					verifyContact(void);
				osg::MatrixTransform*	createOSGBox(osg::Vec3 size, std::string texture = "");

				btDynamicsWorld*				dynamicsWorld;
				osgbCollision::GLDebugDrawer*	dbgDraw;
				OpenIG::Base::ImageGenerator*			_ig;
				bool							initial_contact;
				std::vector<Vehicle*>			vehicle_list;
				bool							debug_mode;
				bool                            _freeze;
			};
		}
	}
} // namespace

#endif // BULLETMANAGER_H
