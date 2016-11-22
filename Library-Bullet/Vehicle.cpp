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
//#*	author    Roni Zanoli <openig@compro.net>
//#*	copyright(c)Compro Computer Services, Inc.
//#*
//#*    Please direct any questions or comments to the OpenIG Forums
//#*    Email address: openig@compro.net
//#*
//#*
//#*    Please direct any questions or comments to the OpenIG Forums
//#*    Email address: openig@compro.net
//#*

#include <Library-Bullet/Vehicle.h>

#include <osgbCollision/Utils.h>

#include <iostream>

#include <Core-Base/Mathematics.h>

using namespace OpenIG::Library::Bullet;

Vehicle::Vehicle( VehicleData* vd,
                  OpenIG::Base::ImageGenerator* ig,
                  btRigidBody* chassis,
                  btVehicleRaycaster* raycaster ) :
    _ig(ig),
    vehicle_data(vd),
    btRaycastVehicle(m_tuning, chassis, raycaster)
{
    gEngineForce = 0.0f;
    gBreakingForce = 0.0f;
    gVehicleSteering = 0.0f;
    _speed = 0.0f;

    wheelDirectionCS0.setValue(vd->wheel_direction_CS0.x(),
                               vd->wheel_direction_CS0.y(),
                               vd->wheel_direction_CS0.z());

    wheelAxleCS.setValue(vd->wheel_axle_CS.x(),vd->wheel_axle_CS.y(),vd->wheel_axle_CS.z());

    // Set up the initial position so we place the vehicle in its default position
    init_tr.setIdentity();
    init_tr.setOrigin( btVector3(vehicle_data->pos.x(), vehicle_data->pos.y(), vehicle_data->pos.z() ) );

    setWheel( vehicle_data->tire_model,
              btVector3( vehicle_data->pos.x(), vehicle_data->pos.y(), vehicle_data->pos.z() ) );

    //choose coordinate system
    setCoordinateSystem( vehicle_data->right_index, vehicle_data->up_index, vehicle_data->forward_index );
}

void
Vehicle::
setWheel( std::string tmodel, btVector3 pos )
{
    bool isFrontWheel=true;

    btVector3 connectionPointCS0( vehicle_data->front_wheel_pos.x(), vehicle_data->front_wheel_pos.y(), vehicle_data->front_wheel_pos.z() );
    addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,suspensionRestLength,vehicle_data->wheel_radius,m_tuning,isFrontWheel);

    connectionPointCS0 = btVector3( -vehicle_data->front_wheel_pos.x(), vehicle_data->front_wheel_pos.y(), vehicle_data->front_wheel_pos.z() );
    addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,suspensionRestLength,vehicle_data->wheel_radius,m_tuning,isFrontWheel);

    connectionPointCS0 = btVector3( -vehicle_data->rear_wheel_pos.x(), -vehicle_data->rear_wheel_pos.y(), vehicle_data->rear_wheel_pos.z() );
    isFrontWheel = false;
    addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,suspensionRestLength,vehicle_data->wheel_radius,m_tuning,isFrontWheel);

    connectionPointCS0 = btVector3( vehicle_data->rear_wheel_pos.x(), -vehicle_data->rear_wheel_pos.y(), vehicle_data->rear_wheel_pos.z() );
    addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,suspensionRestLength,vehicle_data->wheel_radius,m_tuning,isFrontWheel);

    int id = vehicle_data->id * 1000;

    for (int i=0;i<getNumWheels();i++)
    {
        btWheelInfo& wheel = getWheelInfo(i);
        wheel.m_suspensionStiffness = vehicle_data->suspension_stiffness;
        wheel.m_wheelsDampingRelaxation = vehicle_data->suspension_damping;
        wheel.m_wheelsDampingCompression = vehicle_data->suspension_compression;
        wheel.m_suspensionRestLength1 = vehicle_data->suspension_rest_length;
        wheel.m_frictionSlip = vehicle_data->wheel_friction;
        wheel.m_rollInfluence = vehicle_data->roll_influence;

        btTransform btt = getWheelTransformWS( i );

        osg::Vec3 pos = osgbCollision::asOsgVec3(btt.getOrigin());

        btt.setOrigin( btVector3(pos.x(), pos.y(), pos.z() ) );

        osg::Matrix m = osgbCollision::asOsgMatrix( btt );

        osg::Matrixd mx_tire1;
        mx_tire1.makeTranslate( pos );

        if( i == 0 || i == 3 )
            m.preMult( osg::Matrix::rotate( osg::DegreesToRadians(270.), 0., 0., 1. ) );
        else
            m.preMult( osg::Matrix::rotate( osg::DegreesToRadians(90.), 0., 0., 1. ) );

        _ig->addEntity(++id, tmodel, m);
    }
}

void
Vehicle::
setEngineForce( float mult )
{
    gEngineForce = vehicle_data->engine_force*mult;
    gBreakingForce = 0;
}

void
Vehicle::
stop( void )
{
    gEngineForce = 0;
    gBreakingForce = vehicle_data->brakes;
}

void
Vehicle::
clearBrakes( void )
{
    gBreakingForce = 0;
}

void
Vehicle::
setSteering( bool left, bool update )
{
    if (update)
    {
        if (left)
        {
            gVehicleSteering += vehicle_data->steering_increment;

            if (gVehicleSteering > vehicle_data->steering_clamp)
                gVehicleSteering = vehicle_data->steering_clamp;
        }
        else
        {
            gVehicleSteering -= vehicle_data->steering_increment;

            if (gVehicleSteering < -vehicle_data->steering_clamp)
                gVehicleSteering = -vehicle_data->steering_clamp;
        }
    }
}

void
Vehicle::
setSteering( float steer )
{
    gVehicleSteering = steer;
}

void
Vehicle::
setBrakes(float brakes)
{
    gBreakingForce = brakes;
}

void
Vehicle::
reset( void )
{
    // reset all vehicle variables
    btQuaternion rot;
    rot.setRotation(btVector3(0, 0, 1), osg::DegreesToRadians(vehicle_data->heading));
    init_tr.setRotation( rot );

    gVehicleSteering = 0.f;
    getRigidBody()->setCenterOfMassTransform(init_tr);
    getRigidBody()->setLinearVelocity(btVector3(0,0,0));
    getRigidBody()->setAngularVelocity(btVector3(0,0,0));
    resetSuspension();

    update();
}

void
Vehicle::
setInitialPosition( osg::Vec3 pos )
{
    init_tr.setIdentity();
    init_tr.setOrigin( btVector3(pos.x(), pos.y(), pos.z() ) );
}

void
Vehicle::
update( void )
{
    btTransform chassisWorldTrans = getChassisWorldTransform();

    _pos = osgbCollision::asOsgVec3( chassisWorldTrans.getOrigin() );

    // apply force and brakes to the vehicle wheels
    int wheelIndex = 2;
    applyEngineForce(gEngineForce,wheelIndex);
    setBrake(gBreakingForce,wheelIndex);
    wheelIndex = 3;
    applyEngineForce(gEngineForce,wheelIndex);
    setBrake(gBreakingForce,wheelIndex);

    // Apply steering
    wheelIndex = 0;
    setSteeringValue(gVehicleSteering,wheelIndex);
    wheelIndex = 1;
    setSteeringValue(gVehicleSteering,wheelIndex);

    float rx,ry,rz;

    chassisWorldTrans.getBasis().getEulerYPR(rx, ry, rz);

    _att.set( osg::RadiansToDegrees( rx ), osg::RadiansToDegrees( rz ), osg::RadiansToDegrees( ry ) );

    while ( _att.x() > 180.0 )
        _att.x() = _att.x() - 360.0;

    while ( _att.x() < 0 )
        _att.x() = _att.x() + 360.0;

    osg::Matrixd mat = OpenIG::Base::Math::instance()->toMatrix(_pos.x(),
                                                          _pos.y(),
                                                          _pos.z(),
                                                          _att.x(),
                                                          _att.y(),
                                                          _att.z());
    _ig->updateEntity(vehicle_data->id, mat);

    int id = vehicle_data->id*1000;

    for ( int i=0;i<getNumWheels();i++ )
    {
        //synchronize the wheels with the (interpolated) chassis worldtransform
        updateWheelTransform(i,false);

        btWheelInfo& wheel = getWheelInfo(i);

        //draw wheels (cylinders)
        btTransform btt = getWheelTransformWS( i );

        osg::Vec3 pos = osgbCollision::asOsgVec3(btt.getOrigin());

        btt.setOrigin( btVector3(pos.x(), pos.y(), pos.z() ) );

        osg::Matrix m = osgbCollision::asOsgMatrix( btt );

        if( i == 0 || i == 3 )
            m.preMult( osg::Matrix::rotate( osg::DegreesToRadians(270.), 0., 0., 1. ) );
        else
            m.preMult( osg::Matrix::rotate( osg::DegreesToRadians(90.), 0., 0., 1. ) );

        _ig->updateEntity(++id, m);
    }

    _speed = fabs(getCurrentSpeedKmHour());
}
