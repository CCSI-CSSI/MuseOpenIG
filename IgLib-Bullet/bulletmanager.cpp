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
#include <BulletCollision/CollisionShapes/btShapeHull.h>
#include <btBulletDynamicsCommon.h>

#include <IgLib-Bullet/bulletmanager.h>
#include <IgLib-Bullet/configreader.h>

#include <osg/Node>
#include <osg/ShapeDrawable>
#include <osg/MatrixTransform>

#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include <osgbCollision/CollisionShapes.h>
#include <osgbCollision/Utils.h>
#include <osgbCollision/GLDebugDrawer.h>



#include <IgCore/mathematics.h>

using namespace iglib;

void BulletManager::setupTerrain(osg::Node& entity)
{
	osg::ref_ptr<osg::Node> nodeDB = &entity;
	osg::ref_ptr< osg::MatrixTransform > node = new osg::MatrixTransform;

	if (!nodeDB.valid())
	{
        osg::notify(osg::NOTICE) << "BulletManager::setupTerrain:  Can't find node: " << entity.getName() << std::endl;
		//exit(0);
	}

	node->addChild(nodeDB.get());
	nodeDB->setName("terrain");

	btCollisionShape* collision = osgbCollision::btTriMeshCollisionShapeFromOSG(node);

	btScalar mass(0.0);
	btVector3 inertia(0, 0, 0);
	btRigidBody::btRigidBodyConstructionInfo rb(mass, 0, collision, inertia);
	btRigidBody* body = new btRigidBody(rb);
	body->setUserPointer(nodeDB);

	dynamicsWorld->addRigidBody(body);
}

void BulletManager::setupVehicle(unsigned int id, igcore::ImageGenerator::Entity& entity, const std::string& fileName)
{
	std::string xmlFileName = fileName + ".bullet.xml";
	
	if (!osgDB::fileExists(xmlFileName) && id < 1000)
	{
        osg::notify(osg::NOTICE) << "BulletManager::setupVehicle: Can't find vehicle xml file: " << xmlFileName << std::endl;
        osg::notify(osg::NOTICE) << "BulletManager::setupVehicle: Attempting to use common.bullet.xml!!!" << std::endl;
        xmlFileName = osgDB::getFilePath(fileName) + "common.bullet.xml";
		if (!osgDB::fileExists(xmlFileName)) return;
	}
		
	ConfigReader cfg;
	cfg.readXML(osgDB::getFilePath(xmlFileName)+"/", osgDB::getSimpleFileName(xmlFileName));

	std::vector<VehicleData*> vehicle_data_list = cfg.getVehicleDataList();

	for (std::vector<VehicleData*>::iterator vit = vehicle_data_list.begin(); vit != vehicle_data_list.end(); ++vit)
	{
		if (!(*vit)->model.empty())
		{
			osg::Matrixd mx1;
			mx1.makeTranslate((*vit)->pos);
			
			entity->setMatrix(mx1);

			createVehicle((*vit), entity->getChild(0), osg::Vec4(0, 0, 1, (*vit)->heading));
		}
	}
}

BulletManager* BulletManager::instance()
{
	static BulletManager s_BulletManager;
	return &s_BulletManager;
}


BulletManager::BulletManager()
	: _ig(0)
{

}

BulletManager::~BulletManager()
{

}

void BulletManager::clean()
{
	// TODO: Clean stuff here
}

void BulletManager::init(igcore::ImageGenerator* ig, bool debug)
{
	_ig = ig;
	debug_mode = debug;

    dbgDraw = NULL;

    initial_contact = false;

    initPhysics();

    if( debug_mode )
    {
        dbgDraw = new osgbCollision::GLDebugDrawer();
        dbgDraw->setDebugMode( ~btIDebugDraw::DBG_DrawText );
        dynamicsWorld->setDebugDrawer( dbgDraw );
    }
}

void
BulletManager::
initPhysics( void )
{
    btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
    btCollisionDispatcher* dispatcher = new btCollisionDispatcher( collisionConfiguration );
    btConstraintSolver* solver = new btSequentialImpulseConstraintSolver;

    btDbvtBroadphase* inter = new btDbvtBroadphase();

    dynamicsWorld = new btDiscreteDynamicsWorld( dispatcher, inter, solver, collisionConfiguration );
    dynamicsWorld->setGravity( btVector3( 0, 0, -9.8 ) );
}

void
BulletManager::
createVehicle( VehicleData* vd, osg::ref_ptr< osg::Node > nodeDB, osg::Vec4 r )
{
    osg::ref_ptr< osg::MatrixTransform> mat = new osg::MatrixTransform;
    mat->addChild( nodeDB.get() );

    btConvexShape* originalConvexShape = osgbCollision::btConvexHullCollisionShapeFromOSG( mat.get() );

    //create a hull approximation
    btShapeHull* hull = new btShapeHull(originalConvexShape);
    btScalar margin = originalConvexShape->getMargin();
    hull->buildHull(margin);
    btConvexHullShape* collision = new btConvexHullShape();
    for (int i=0;i<hull->numVertices();i++)
    {
        collision->addPoint(hull->getVertexPointer()[i]);
    }

    btScalar mass( vd->mass );
    btVector3 inertia(0,0,0);
    collision->calculateLocalInertia( mass, inertia );

    // Vehicle
    btTransform tr;
    tr.setIdentity();
    tr.setOrigin( btVector3( vd->pos.x(), vd->pos.y(), vd->pos.z() ) );

    btQuaternion rot;
    rot.setRotation(btVector3(r.x(), r.y(), r.z()), osg::DegreesToRadians(r.w()));
    tr.setRotation( rot );

    osg::Matrix m;
    m = osg::Matrix::rotate( rot.getAngle(), osgbCollision::asOsgVec3(rot.getAxis()) ) * osg::Matrix::translate(vd->pos);

    mat->setMatrix(m);

    btDefaultMotionState* myMotionState = new btDefaultMotionState(tr);

    btCompoundShape* compound = new btCompoundShape();
    btTransform localTrans;
    localTrans.setIdentity();

    compound->addChildShape(localTrans,collision);

     btRigidBody::btRigidBodyConstructionInfo rbinfo( mass, myMotionState, compound, inertia );
    btRigidBody* body = new btRigidBody( rbinfo );
    dynamicsWorld->addRigidBody( body );

    //gVehicleSteering = 0.f;
    body->setLinearVelocity(btVector3(0,0,0));
    body->setAngularVelocity(btVector3(0.0,0.0,0.0));
    dynamicsWorld->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs(body->getBroadphaseHandle(),dynamicsWorld->getDispatcher());

    btVehicleRaycaster* m_vehicleRayCaster = new btDefaultVehicleRaycaster(dynamicsWorld);
    Vehicle* m_vehicle = new Vehicle( vd, _ig, body,m_vehicleRayCaster );

    ///never deactivate the vehicle
    body->setActivationState(DISABLE_DEACTIVATION);

    dynamicsWorld->addVehicle(m_vehicle);

    vehicle_list.push_back( m_vehicle );

    update( 16 );
}

Vehicle*
BulletManager::
getVehicle( int id )
{
    std::vector<Vehicle*>::iterator vit;

    for( vit = vehicle_list.begin(); vit != vehicle_list.end(); ++vit )
    {
        if( (*vit)->getID() == id )
            return (*vit);
    }

    return NULL;
}

void
BulletManager::
setDebug( void )
{
    if( debug_mode )
        debug_mode = false;
    else
        debug_mode = true;
}

osg::MatrixTransform*
BulletManager::
createOSGBox( osg::Vec3 size, std::string texture )
{
    osg::Box* box = new osg::Box();

    box->setHalfLengths( size );

    osg::ShapeDrawable* shape = new osg::ShapeDrawable( box );

    osg::Geode* geode = new osg::Geode();
    geode->addDrawable( shape );

    osg::StateSet* stateset = new osg::StateSet();

    if( !texture.empty() )
    {
        osg::Image* image = osgDB::readImageFile( texture );

        if (image)
        {
            osg::Texture2D* texture = new osg::Texture2D;
            texture->setImage(image);
            texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
            stateset->setTextureAttributeAndModes(0,texture, osg::StateAttribute::ON);
        }
        else
            std::cout << "image not found ######### " << std::endl;
    }

    stateset->setMode(GL_LIGHTING, osg::StateAttribute::ON);

    geode->setStateSet( stateset );

    osg::MatrixTransform* transform = new osg::MatrixTransform();
    transform->addChild( geode );

    return( transform );
}

void
BulletManager::
resetScene( void )
{
    initial_contact = false;

    std::vector<Vehicle*>::iterator vit;

    for( vit = vehicle_list.begin(); vit != vehicle_list.end(); ++vit )
    {
        (*vit)->reset();
    }
}

void
BulletManager::
verifyContact( void )
{
   if( !initial_contact )
   {
       int numManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();

       for (int i=0;i<numManifolds;i++)
       {
           btPersistentManifold* contactManifold =  dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);

           if( !contactManifold->getNumContacts() )
               continue;

           btCollisionObject* colObj0 = (btCollisionObject*)contactManifold->getBody0();
           btCollisionObject* colObj1 = (btCollisionObject*)contactManifold->getBody1();

           osg::Node* node0 = (osg::Node*)colObj0->getUserPointer();
           osg::Node* node1 = (osg::Node*)colObj1->getUserPointer();

           if( node0 )
           {
               if( node0->getName() == "terrain" )
                   initial_contact = true;
           }

           if( node1 )
           {
               if( node1->getName() == "terrain" )
                   initial_contact = true;
           }
       }
   }
}

void
BulletManager::
update( const double sim_time )
{
    dynamicsWorld->stepSimulation( sim_time );

    // check for a initial contact to the ground
    verifyContact();

    if( debug_mode )
    {
        if( dbgDraw != NULL )
            dbgDraw->BeginDraw();
    }

    std::vector<Vehicle*>::iterator vit;

    for( vit = vehicle_list.begin(); vit != vehicle_list.end(); ++vit )
    {
        (*vit)->update();
    }

    if( debug_mode )
    {
        if( dbgDraw != NULL )
        {
            dynamicsWorld->debugDrawWorld();
            dbgDraw->EndDraw();
        }
    }
}

