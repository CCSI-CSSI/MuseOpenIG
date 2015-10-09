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
#ifndef BULLETMANAGER_H
#define BULLETMANAGER_H

#include <string>

#include <btBulletDynamicsCommon.h>

#include <osg/Vec3>
#include <osg/Vec4>

#include <osgbCollision/GLDebugDrawer.h>

#include <IgCore/mathematics.h>

#include <IgLib-Bullet/configreader.h>
#include <IgLib-Bullet/vehicle.h>
#include <IgLib-Bullet/export.h>

namespace iglib
{

	class IGLIBBULLET_EXPORT BulletManager
	{
	protected:
		BulletManager();
		~BulletManager();

	public:
		static BulletManager*			instance();

		void							init(igcore::ImageGenerator* ig, bool debug = false);
		void							clean();

		Vehicle*						getVehicle(int id);
		osgbCollision::GLDebugDrawer*	getDebugDrawer(void) { return dbgDraw; }
		void							setDebug(void);
		void							resetScene(void);
		void							update(const double sim_time);
		inline bool						getInitialContact(void) { return initial_contact; }

		void							setupTerrain(osg::Node& entity);
		void							setupVehicle(unsigned int id, igcore::ImageGenerator::Entity& entity, const std::string& fileName);
        inline void                     setFreeze(bool on)          { _freeze=on; }
		inline bool						getIsFreezed() const		{ return _freeze;  }

	private:
		void					initPhysics(void);
		void					createVehicle(VehicleData* vd, osg::ref_ptr< osg::Node > nodeDB, osg::Vec4 r);
		void					verifyContact(void);
		osg::MatrixTransform*	createOSGBox(osg::Vec3 size, std::string texture = "");

		btDynamicsWorld*				dynamicsWorld;
		osgbCollision::GLDebugDrawer*	dbgDraw;
		igcore::ImageGenerator*			_ig;
		bool							initial_contact;
		std::vector<Vehicle*>			vehicle_list;
		bool							debug_mode;
        bool                            _freeze;
	};

} // namespace

#endif // BULLETMANAGER_H
