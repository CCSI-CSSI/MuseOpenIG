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
#ifndef VEHICLE_H
#define VEHICLE_H

#if defined(OPENIG_SDK)
	#include <OpenIG-Bullet/export.h>
	#include <OpenIG-Bullet/configreader.h>

	#include <OpenIG-Base/mathematics.h>
	#include <OpenIG-Base/imagegenerator.h>
#else
	#include <Library-Bullet/export.h>
	#include <Library-Bullet/configreader.h>

	#include <Core-Base/mathematics.h>
	#include <Core-Base/imagegenerator.h>
#endif

#include <btBulletDynamicsCommon.h>
#include <BulletDynamics/Vehicle/btRaycastVehicle.h>

#include <osg/MatrixTransform>


namespace OpenIG {
	namespace Library {
		namespace Bullet {

			class IGLIBBULLET_EXPORT Vehicle : public btRaycastVehicle
			{
			public:
				Vehicle()
					: btRaycastVehicle(btRaycastVehicle::btVehicleTuning(), 0, 0) {}

				Vehicle(const Vehicle& vehicle)
					: btRaycastVehicle(*dynamic_cast<const btRaycastVehicle*>(&vehicle)) {}

				Vehicle(VehicleData* vd, OpenIG::Base::ImageGenerator* ig, btRigidBody* chassis, btVehicleRaycaster* raycaster);

				void				update(void);
				inline osg::Vec3	getPos(void) { return _pos; }
				void				setEngineForce(float mult);
				void				stop(void);
				void                clearBrakes(void);
				void				setSteering(bool left, bool update = true);
				void                setSteering(float steer);
				inline osg::Vec3	getAtt() { return _att; }
				void				reset(void);
				int					getID(void) { return vehicle_data->id; }
				inline float		getSpeed(void) { return _speed; }
				void                setBrakes(float brakes);
				inline float        getEngineForce(void) { return gEngineForce; }
				void                setInitialPosition(osg::Vec3 pos);

			private:
				float												gEngineForce;
				float												gBreakingForce;
				float												gVehicleSteering;
				float												wheelWidth;
				btVector3											wheelDirectionCS0;
				btVector3											wheelAxleCS;
				btScalar											suspensionRestLength;
				btRaycastVehicle::btVehicleTuning					m_tuning;
				std::vector< osg::ref_ptr<osg::MatrixTransform> >	wheel_mat;
				osg::Vec3											_pos;
				osg::Vec3											_att;
				btTransform											init_tr;
				OpenIG::Base::ImageGenerator*								_ig;
				VehicleData*										vehicle_data;
				float												_speed;

				void setWheel(std::string tmodel, btVector3 pos);

			};
		}
	}
}

#endif // VEHICLE_H
