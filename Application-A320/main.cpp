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
#include <iostream>
#include <fstream>
#include <sstream>

#include <osg/AnimationPath>
#include <osg/ValueObject>

#include <osgViewer/CompositeViewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>

#include <osgText/Text>

#include <osgSim/HeightAboveTerrain>

#include <Core-OpenIG/openig.h>

#include <Core-Base/commands.h>
#include <Core-Base/mathematics.h>
#include <Core-Base/imagegenerator.h>

#if defined(_WIN32)
#include <windows.h>
#include <osgViewer/api/Win32/GraphicsHandleWin32>
#endif

// Let make some info HUD for the keyboard bindings
// based on the osghud example
osg::Node* createInfoHUD(OpenIG::Engine* ig)
{
    unsigned int screen_width = 1600, screen_height = 1200;
    
    // create a camera to set up the projection and model view matrices, and the subgraph to draw in the HUD
    osg::Camera* camera = new osg::Camera;

    // set the projection matrix
    camera->setProjectionMatrix(osg::Matrix::ortho2D(0,screen_width,0,screen_height));

    // set the view matrix
    camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    camera->setViewMatrix(osg::Matrix::identity());

    // only clear the depth buffer
    camera->setClearMask(GL_DEPTH_BUFFER_BIT);

    // draw subgraph after main camera view.
    camera->setRenderOrder(osg::Camera::POST_RENDER);

    // we don't want the camera to grab event focus from the viewers main camera(s).
    camera->setAllowEventFocus(false);

    osg::Geode* geode = new osg::Geode();
    camera->addChild(geode);

    std::string timesFont("fonts/arial.ttf");

    // turn lighting off for the text and disable depth test to ensure it's always ontop.
    osg::StateSet* stateset = geode->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

	
    osgText::Text* text = new  osgText::Text;
    geode->addDrawable( text );
	
	std::ostringstream oss;
	oss << "(F1) reset (F2) statistic (F3) wireframe/points/fill (F4) home (F5) play/pause (F6) fixed up mode on/off (F7) onscreen help (F8) command line terminal ";

	// Look up for the UI plugin
	const OpenIG::PluginBase::PluginHost::PluginsMap& plugins = ig->getPlugins();
	OpenIG::PluginBase::PluginHost::PluginsMap::const_iterator itr = plugins.begin();
	for (; itr != plugins.end(); ++itr)
	{
		if (itr->second->getName() == "UI")
		{
			oss << "(F9) UI ";
			break;
		}
	}

	oss << "(F10) toggle fullscreen on/off";

	osgText::String str(oss.str());

    text->setFont(timesFont);
    text->setPosition(osg::Vec3(10,30,1));
    text->setText(str);
    text->setAlignment(osgText::TextBase::LEFT_BOTTOM_BASE_LINE);
    text->setColor(osg::Vec4(1,1,1,1));
    text->setCharacterSize(16);
    text->setCharacterSizeMode(osgText::TextBase::OBJECT_COORDS);
    text->setAxisAlignment(osgText::TextBase::XY_PLANE);
    text->setFontResolution(256,256);
	
	float height = text->getBoundingBox().yMax() - text->getBoundingBox().yMin();

	float ratio = (screen_width-20) / (text->getBoundingBox().xMax() - text->getBoundingBox().xMin());
	text->setCharacterSize(16*ratio);
	text->setMaximumHeight(height*ratio);

	height = text->getBoundingBox().yMax() - text->getBoundingBox().yMin();

	osg::Geometry* geometry = new osg::Geometry;
	geode->addDrawable(geometry);

	float depth = 0.f;

	osg::Vec3Array* vertices = new osg::Vec3Array;
	vertices->push_back(osg::Vec3(0, 25, depth));
	vertices->push_back(osg::Vec3(screen_width, 25, depth));
	vertices->push_back(osg::Vec3(screen_width, 25 + 6 + height, depth));
	vertices->push_back(osg::Vec3(0, 25 + 6 + height, depth));
	geometry->setVertexArray(vertices);

	osg::Vec3Array* normals = new osg::Vec3Array;
	normals->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));
	geometry->setNormalArray(normals);
	geometry->setNormalBinding(osg::Geometry::BIND_OVERALL);

	osg::Vec4Array* colors = new osg::Vec4Array;
	colors->push_back(osg::Vec4(.2f, .2, 1.f, 0.7f));
	geometry->setColorArray(colors);
	geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

	geometry->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, 4));
	geometry->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
	geometry->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);


    return camera;
}

// Keep the new Camera Manipulator available
// accross the whole app
static osg::ref_ptr<osgGA::CameraManipulator> s_CameraManipulator;

// Some pre-defined frames from the flight path
// on which we call events. We keep it simple here
#define LANDING_GEAR_UP             44.4498
#define FLAPS_UP                    47.0624
#define LANDING_GEAR_DOWN           172.515
#define FLAPS_DOWN                  176.33
#define BREAKS_UP                   221.822
#define FLIGHT_PATH_EOF             223.5

static bool s_LandingGearUp         = false;
static bool s_FlapsUp               = false;
static bool s_LandingGearDown       = false;
static bool s_FlapsDown             = false;
static bool s_BreaksUp              = false;

// Keep this one around too, used to
// reset the demo on F1
static bool s_Reset = false;

// This is offset for th flight path
// Can be triggered by a custom command
// - see bellow
static osg::Vec3 s_Offset;

// And this one. The beginning of the playback
static osg::Timer_t s_PlaybackStartTime  = 0;

// And this one too. We keep track of the total
// time in pause mode
static double s_PausedTime = 0.0;

// These are flags for the wing test
static bool s_WingTestDone = false;
static bool s_WingTestStarted = false;

// This is flag when we have landed
static bool s_Landed = false;

// This class is to have the same behavior
// as the standard osg Trackball camera
// manipulator only we change the key
// bindings. Since we have keyboard interaction
// in OpenIG, like for entering commands
// in the command terminal on F8, the onscreen
// help on F7, we change the key bindings to not cause
// intercepting the manipulator commands by the
// mentioned onscreen help and terminal Handlers.
// So, here are the bindings now:
//      F4: home
//      F5: start playback/pause
class CameraTrackballManipulator : public osgGA::TrackballManipulator
{
public:
    CameraTrackballManipulator(bool& resetDemo, OpenIG::Base::ImageGenerator* ig)
        : osgGA::TrackballManipulator()
        , playbackOn(false)
        , reset(resetDemo)
        , startOfPauseTime(0.0)
        , openIG(ig)
        , fixedUpMode(false)
    {

    }

    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& us)
    {
        switch(ea.getEventType())
            {
            case(osgGA::GUIEventAdapter::KEYDOWN):
                {
                    if (ea.getKey() == osgGA::GUIEventAdapter::KEY_F4)
                    {                        
                        flushMouseEventStack();
                        home(ea,us);
                        us.requestRedraw();
                        us.requestContinuousUpdate(false);
                        return true;
                    }
                    if (ea.getKey() == osgGA::GUIEventAdapter::KEY_F5)
                    {
                        playbackOn = !playbackOn;

                        // handle pause
                        if (!playbackOn)
                        {
                            startOfPauseTime = osg::Timer::instance()->tick();
                        }
                        else
                        if (startOfPauseTime > 0.0)
                        {
                            // add the total time in pause
                            s_PausedTime += osg::Timer::instance()->delta_s(startOfPauseTime,osg::Timer::instance()->tick());
                            startOfPauseTime = 0.0;
                        }

                        return true;
                    }
                    if (ea.getKey() == osgGA::GUIEventAdapter::KEY_F1)
                    {
                        reset = true;
                        return true;
                    }                    
                    if (ea.getKey() == osgGA::GUIEventAdapter::KEY_F6)
                    {
                        fixedUpMode = !fixedUpMode;
                        openIG->bindCameraSetFixedUp(fixedUpMode,true);
                        return true;
                    }
                }
            break;
            default:
                return osgGA::TrackballManipulator::handle(ea,us);
            }
        return false;
    }

    bool                    playbackOn;
    bool&                   reset;
    osg::Timer_t            startOfPauseTime;
    OpenIG::Base::ImageGenerator* openIG;
    bool                    fixedUpMode;
};

// For this demo to fly the model we use a
// recorded path from our MUSE simulation
// system. This command is to offset the
// path coordinates, if one might want to use
// the same path on different database. Simple
// but handy
class FlightPathOffsetCommand : public OpenIG::Base::Commands::Command
{
public:
	FlightPathOffsetCommand(osg::Vec3& offset)
		: _offset(offset)
	{

	}

	// Convinient method for the onscreen
	// help. This one is a must
	virtual const std::string getUsage() const
	{
		return "x y z";
	}

	virtual const std::string getArgumentsFormat() const
	{
		return	"D:D:D";
	}

	// Convinient method for the onscreen
	// help. This one is a must too
	virtual const std::string getDescription() const
	{
		return  "offsets the flight path for this demo\n"
			"     x y z - the offset in 3D";
	}

	// This method is called when you invoke a command in the
	// terminal, call a command through the Commands class
	// tokens is std::vector<std::string> holding the arguments
	// provided
	virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
	{
		if (tokens.size() == 3)
		{
			_offset.x() = atof(tokens.at(0).c_str());
			_offset.y() = atof(tokens.at(1).c_str());
			_offset.z() = atof(tokens.at(2).c_str());
		}
		return 0;
	}
protected:
	osg::Vec3&	_offset;
};

// By default, OpenIG have only one Camera Manipulator
// which is the keypad manipulator - binds the numeric
// keypad to move the camera, or entities if it is bound
// to an entity. The keypad manipualor is available through
// the 'keypad' command - please refer to the onscreen help
// on F7 Otherwise, the way how you set the View
// in the Viewer with Handlers and Manipulator is up to you.
// So now we will introduce a custom command that will bind
// our new Camera Manipulator to the a320 entity
class SetCameraManipulatorCommand : public OpenIG::Base::Commands::Command
{
public:
    SetCameraManipulatorCommand(OpenIG::Base::ImageGenerator* ig)
        : _ig(ig)
    {

    }

    // Convinient method for the onscreen
    // help. This one is a must
    virtual const std::string getUsage() const
    {
        return "id name";
    }

	virtual const std::string getArgumentsFormat() const
	{
		return	"I:{trackball;earth}";
	}

    // Convinient method for the onscreen
    // help. This one is a must too
    virtual const std::string getDescription() const
    {
        return  "sets camera manipulator\n"
                "     id - the id of the model\n"
                "     name - one of these:\n"
                "           trackball - the OSG Trackball camera manipulator\n"
                "           earth - the osgEarth camera manipulator";
    }

    // This method is called when you invoke a command in the
    // terminal, call a command through the Commands class
    // tokens is std::vector<std::string> holding the arguments
    // provided
    virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
    {        
        // We need only two arguments: the entity ID and the
        // manipulator name
        if (tokens.size() == 2)
        {
            unsigned int id     = atoi(tokens.at(0).c_str());
            std::string name    = tokens.at(1);

            // The enitites are stores in std::map, id based, the id
            // is the Entity ID you refer accross the whole application
            // We get a reference to the entity
            OpenIG::Base::ImageGenerator::Entity& entity = _ig->getEntityMap()[id];
            if (entity.valid())
            {
                // We have this one if we have
                // implemented more then one
                // manipulator. For now, only
                // the trackball is implemented
                osg::ref_ptr<osgGA::CameraManipulator> manip;
                if (name.compare(0,9,"trackball") == 0)
                {
                    // Set up the trackball
                    // manipulator with some
                    // defaults
                    osg::ref_ptr<CameraTrackballManipulator> tb = new CameraTrackballManipulator(s_Reset,_ig);
                    tb->setAutoComputeHomePosition(false);
                    tb->setNode(entity->getChild(0));

                    // Our Entity is osg::MatrixTransoform and
                    // we want to use the manipulator with the model
                    // loaded and attached to this osg::MatrixTransform,
                    // so we work with it's child. Might look unsafe, we
                    // are not checking the getChild(0) pointer, but if the
                    // Entity is valid, then it will have a child
                    double radius = entity->getChild(0)->getBound().radius();
                    double factor = 1.5;
                    osg::Vec3d center = entity->getChild(0)->getBound().center();

                    // This will be the initial
                    // position
                    osg::Matrixd mx;
                    mx.makeLookAt(osg::Vec3d(10,radius*factor,-2.5),center,osg::Vec3d(0,0,1));

                    tb->setByMatrix(mx);
                    tb->setHomePosition(osg::Vec3d(10,radius*factor,-2.5),center,osg::Vec3d(0,0,1));
                    tb->setCenter(center);
                    tb->setDistance(radius*factor);

                    // Set the manipulator
                    // to be used - see bellow
                    manip = tb;

                }
                else
                if( name.compare(0,5,"earth") )
                {
                    osg::notify(osg::NOTICE) << "OpenIG: earth camera manipulator not implemented yet" << std::endl;
                }

                // Here we want to bind the manipulator
                // to the camera. We have to do this through
                // the OpenIG Camera binding and update
                if (manip.valid())
                {
                    // We set the Camera Manipulator
                    _ig->getViewer()->getView(0)->setCameraManipulator(manip,true);

                    // Bind the camera to the a320
                    // Entity. We can provide offset here
                    // good if you would like to have pilot-view
                    _ig->bindCameraToEntity(id,osg::Matrix::identity());

                    // You can uncomment this if you would
                    // want to see the model rotation, by
                    // default if follow the orientation
                    // of the model. This is changed now in
                    // real time with F6
                    //_ig->bindCameraSetFixedUp(true);

                    // Set our app wide manipulator
                    // on this one as well. We will
                    // need it in the loop to
                    // update the OpenIG camera since
                    // it is bound to an Entity
                    s_CameraManipulator = manip;

                    osg::notify(osg::NOTICE) << "IG: " << name << " bound to entity " << id << std::endl;
                }
            }

            return 0;
        }

        return -1;
    }
protected:
    OpenIG::Base::ImageGenerator* _ig;
};


// NOTE: If you have different locations
// for the data files (the Compro model
// and the terrain) provided with this
// OpenIG demo, please correct it here
#if defined(_WIN32)
static std::string s_ModelPath("data\\model\\a320\\a320.obj");
static std::string s_TerrainPath("data\\terrain\\master.flt.osg");
static std::string s_FlightRecordingPath("demo\\flightpath.path");
#else
static std::string s_ModelPath("/usr/local/database/model/A320/a320.obj");
static std::string s_TerrainPath("/usr/local/database/terrain/OPEN_IG_Demo_NO_UTM31N_r5/master.flt.osg");
static std::string s_FlightRecordingPath("demo/flightpath.path");
#endif

// These are our entity IDs for the
// model and terrain we will refer
// to accross the scene management
// in OpenIG. OpenIG works with
// IDs
#define TERRAIN_ENTITY_ID               0
#define MODEL_ENTITY_ID                 1
#define NOSE_LANDING_GEAR_ID            10090
#define NOSE_LANDING_GEAR_LIGHT_ID      1

// Now, the loaded model for this demo, a320.obj
// has about 100 parts that can be articulated
// independently. Some of them are animated with
// the OpenIG animation core - these animation are
// defined in the a320.obj.xml. However, we would like
// to show how to articulate sub-models based on 
// mathematicaly computed values - the core of every 
// simulation when a math model is present. We do it
// simple here, we are going to articulate the ailerons
// based on the plane roll in an update callback. All of 
// these 100 sub-entiies are composed into a main entity
// withe the ModelComposition plugin which rely on the mentioned
// a320.obj.xml.

// For the beginning, we write a nodevisitor to finds these
// sub-models in our entity and map them to entity which can 
// be controlled with osg::Matrix as any other entity
struct FindSubEntitiesNodeVisitor : public osg::NodeVisitor
{
	// This will map sub-entity name to Entity
	typedef std::map< std::string, OpenIG::Base::ImageGenerator::Entity >		SubEntities;

	// This will map sub-entity name to it's ID
	typedef std::map< std::string, unsigned int>						SubEntitiesID;

	FindSubEntitiesNodeVisitor(const OpenIG::Base::StringUtils::StringList& names, OpenIG::Base::ImageGenerator* ig)
		: osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN), subEntitiesNames(names), imageGenerator(ig) {}

	// Let Find all of them
	virtual void apply(osg::Node& node)
	{
		// This is one of the internals of OpenIG
		// The Entity/Sub-Entity name is set as
		// user value
		std::string name;
		if (node.getUserValue("Name", name) && !name.empty())
		{
			// Iterate over the names we are interested in
			OpenIG::Base::StringUtils::StringListIterator itr = subEntitiesNames.begin();
			for (; itr != subEntitiesNames.end(); ++itr)
			{
				// Here we found one
				if (name == *itr)
				{
					// Get the Entity ID, again internals of OpenG,
					// it is stored as UserValue
					unsigned int ID = 0;
					if (node.getUserValue("ID", ID) && ID != 0)
					{
						OpenIG::Base::ImageGenerator::Entity entity = imageGenerator->getEntityMap()[ID];
						if (entity.valid())
						{
							// We save it in the map
							subEntities[name] = entity;
							subEntitiesID[name] = ID;
							break;
						}
					}
				}
			}
		}
		traverse(node);
	}

	// our map
	SubEntities	subEntities;

	// our map of IDs
	SubEntitiesID subEntitiesID;

	// the names we will search for
	OpenIG::Base::StringUtils::StringList subEntitiesNames;

	// Handle of the IG
	OpenIG::Base::ImageGenerator* imageGenerator;
};

// Now we have a finder for our ailerons submodels. Let
// write update callback that will take the roll of the
// plane and adjust the ailerons, just for visual not
// needed to be aero-dynamics precise :-)
struct UpdateAileronsNodeCallback : public osg::NodeCallback
{
	UpdateAileronsNodeCallback(unsigned int id, OpenIG::Base::ImageGenerator::Entity& mainEntity, OpenIG::Base::ImageGenerator* ig, bool flip = false)
		: ID(id), a320(mainEntity), imageGenerator(ig), originalSaved(false)
		, original(0.0), flipRoll(flip), lastCheckedRoll(0.0) {}

	// Here we check the roll of the plane and
	// we adjust the pitch of the ailerons based on that roll
	virtual void operator()(osg::Node* node, osg::NodeVisitor*)
	{
		// a bit of paranoia
		osg::ref_ptr<osg::MatrixTransform> mxt = dynamic_cast<osg::MatrixTransform*>(node);
		if (!mxt.valid()) return;

		// The main a320 current matrix
		osg::Matrixd mx = a320->getMatrix();

		// decompose
		double x = 0;
		double y = 0;
		double z = 0;
		double h = 0;
		double p = 0;
		double r = 0;
		OpenIG::Base::Math::instance()->fromMatrix(mx, x, y, z, h, p, r);

		// save the a320 roll 
		// Also to mention, some modellers like Blender 
		// with shich the a320 model was modeled have
		// swapped pitch and roll, for our case
		double roll = flipRoll ? -(r - lastCheckedRoll) : (r - lastCheckedRoll);
		lastCheckedRoll = r;

		diffs.push_back(roll);
		roll = getAverageRollDiff();

		// This will be same as the mxt from above
		// we only show here the usage of the API
		// We get its current matrix
		osg::Matrixd aileronMx;

		OpenIG::Base::ImageGenerator::Entity aileron = imageGenerator->getEntityMap()[ID];
		if (aileron.valid())
		{
			aileronMx = aileron->getMatrix();
		}
		// Now construct anew one
		// for the ailerons. We take the roll
		// of the plane and set it as a pitch,
		// a bit tuned. We should be carefull here
		// since there might be some offset already
		// for the aileron sub-entity defined in the
		// xml, so we just change the pitch, the rest
		// should remain
		OpenIG::Base::Math::instance()->fromMatrix(aileronMx, x, y, z, h, p, r);

		// Save the original value that might come
		// from the xml
		if (!originalSaved)
		{
			originalSaved = true;
			original = p;
		}

		// Since our blender swaps pitch and roll
		// we compose the final matrix ourselfs
		osg::Matrixd mxR;
		mxR.makeRotate(osg::DegreesToRadians(original + 90.0 + roll*70), osg::Vec3(1, 0, 0));
		osg::Matrixd mxH;
		mxH.makeRotate(osg::DegreesToRadians(h), osg::Vec3(0, 0, 1));
		osg::Matrixd mxP;
		mxP.makeRotate(osg::DegreesToRadians(r), osg::Vec3(0, 1, 0));

		aileronMx = mxR * mxP * mxH * osg::Matrixd::translate(osg::Vec3d(x, y, z));

		// update the aileron
		imageGenerator->updateEntity(ID, aileronMx);
	}

	// the main plane entity
	OpenIG::Base::ImageGenerator::Entity a320;

	// Handle to the IG
	OpenIG::Base::ImageGenerator* imageGenerator;

	// The ID of the ailerons sub-entity
	// although in the update callback we can
	// set it directly since this update call
	// is set to the entity which is MatrixTransform
	unsigned int ID;

	// let save the original roll of the ailerons
	bool	originalSaved;
	double	original;

	// True to flip the main roll
	// we are using this callback for left amd right
	// wings, and they work in oposite
	bool flipRoll;

	// we need this for the difference
	double lastCheckedRoll;

	// we will average some roll differences to make it smoother
	typedef std::list<double>		RollDiffs;
	RollDiffs diffs;

	inline double getAverageRollDiff()
	{
		double result = 0.0;

		RollDiffs::iterator itr = diffs.begin();
		for (; itr != diffs.end(); ++itr)
			result += *itr;
		
		if (diffs.size() > 20)
			diffs.pop_front();

		return result/(diffs.size() ? (double)diffs.size() : 1.0);
	}

};

// We do the same for wheels. Although we have integrated
// bulet physics into OpenIG, which is simple and experimental
// let implement some wheels rotation based on the a320 speed
struct UpdateWheelUpdateCallback : public osg::NodeCallback
{
	UpdateWheelUpdateCallback(unsigned int id, double wheelRadius, OpenIG::Base::ImageGenerator::Entity& mainEntity, OpenIG::Base::ImageGenerator* ig, bool touchDownEffect = true)
		: ID(id), radius(wheelRadius), a320(mainEntity), lastCheckedTime(0.0)
		, currentSpeed(0.0), imageGenerator(ig), inTheAir(false), touchDown(touchDownEffect) {}

	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
	{
		// a bit of paranoia
		osg::ref_ptr<osg::MatrixTransform> mxt = dynamic_cast<osg::MatrixTransform*>(node);
		if (!mxt.valid()) return;

		osg::Timer_t now = osg::Timer::instance()->tick();
		if (lastCheckedTime == 0.0)
		{
			lastCheckedTime = now;
			lastCheckedPosition = a320->getMatrix().getTrans();
		}

		double dt = osg::Timer::instance()->delta_s(lastCheckedTime, now);
		if (dt > 0.1)
		{
			double distance = (a320->getMatrix().getTrans() - lastCheckedPosition).length();

			// save these
			lastCheckedTime = now;
			lastCheckedPosition = a320->getMatrix().getTrans();

			currentSpeed = distance / dt;
		}

		double w = (currentSpeed / radius) * dt;

		double x, y, z, h, p, r;
		OpenIG::Base::Math::instance()->fromMatrix(mxt->getMatrix(), x, y, z, h, p, r);
		p += 90;
		p -= osg::RadiansToDegrees(w);

		// Now here we test intersection against the terrain model
		// if it is in the air we break the wheel movement
		osgSim::LineOfSight los;
		osgSim::HeightAboveTerrain hat;
		hat.setDatabaseCacheReadCallback(los.getDatabaseCacheReadCallback());

		// Since this is sub-entity, we need the localtoworld matrix to work
		// in world space
		osg::Matrixd l2w = osg::computeLocalToWorld(nv->getNodePath());

		// we add the intersection ray here e
		osg::Vec3d s = osg::Vec3d(0, 0, 0) * l2w;
		hat.addPoint(s);

		// save this flag. We check later
		// if we just land and we touch down, and we
		// launch a particle effect there
		bool savedInTheAir = inTheAir;

		// compute the intersections
		OpenIG::Base::ImageGenerator::Entity terrain = imageGenerator->getEntityMap()[TERRAIN_ENTITY_ID];
		if (terrain.valid())
		{
			hat.computeIntersections(terrain);
			if (hat.getNumPoints())
			{
				if (hat.getHeightAboveTerrain(hat.getNumPoints() - 1) > 0.6 /* this is some offset */)
					inTheAir = true;
				else
					inTheAir = false;
			}
		}

		// we update only if we are not in the air
		if (!inTheAir)
		{
			osg::Matrixd mx = OpenIG::Base::Math::instance()->toMatrix(x, y, z, h, p, r);
			mxt->setMatrix(mx);
		}
	}

	// the radius of the wheel
	double							radius;
	// the a320 model
	OpenIG::Base::ImageGenerator::Entity	a320;
	// last checked time
	osg::Timer_t					lastCheckedTime;
	// position on the last checked time
	osg::Vec3d						lastCheckedPosition;
	// the speed of the a320 model
	double							currentSpeed;
	// Handle of the IG
	OpenIG::Base::ImageGenerator*			imageGenerator;
	// true if we are in the air
	bool							inTheAir;
	// true to fire an effect on touch down
	bool							touchDown;
	// The ID of the submodel, used for the touchDown effect
	unsigned int					ID;
};

// Another update callback is for the elevators
// we simple monitor the a320 altitude and based on 
// the change we control them
struct UpdateElevatorsUpdateCallback : public osg::NodeCallback
{
	UpdateElevatorsUpdateCallback(OpenIG::Base::ImageGenerator::Entity& mainEntity)
		: a320(mainEntity), lastCheckedTime(0.0), currentAltSpeed(0.0) {}

	virtual void operator()(osg::Node* node, osg::NodeVisitor*)
	{
		// a bit of paranoia
		osg::ref_ptr<osg::MatrixTransform> mxt = dynamic_cast<osg::MatrixTransform*>(node);
		if (!mxt.valid()) return;

		osg::Timer_t now = osg::Timer::instance()->tick();
		if (lastCheckedTime == 0.0)
		{
			lastCheckedTime = now;
			lastCheckedPosition = a320->getMatrix().getTrans();

			OpenIG::Base::Math::instance()->fromMatrix(mxt->getMatrix(), x, y, z, h, p, r);
		}

		double dt = osg::Timer::instance()->delta_s(lastCheckedTime, now);
		if (dt > 0.1)
		{
			double z = a320->getMatrix().getTrans().z() - lastCheckedPosition.z();

			// save these
			lastCheckedTime = now;
			lastCheckedPosition = a320->getMatrix().getTrans();

			currentAltSpeed = z / dt;
		}
		p = -currentAltSpeed;

		osg::Matrixd mx = OpenIG::Base::Math::instance()->toMatrix(x, y, z, h, p, r);
		mxt->setMatrix(mx);

	}
	// the a320 model
	OpenIG::Base::ImageGenerator::Entity	a320;
	// last checked time
	osg::Timer_t				lastCheckedTime;
	// position on the last checked time
	osg::Vec3d					lastCheckedPosition;
	// the speed of the a320 model
	double						currentAltSpeed;
	// initial positions
	double x, y, z, h, p, r;
};

// This one will update the Rudder
struct UpdateRudderUpdateCallback : public osg::NodeCallback
{
	UpdateRudderUpdateCallback(OpenIG::Base::ImageGenerator::Entity& mainEntity)
		: a320(mainEntity), originalSaved(false) {}

	// Here we check the roll of the plane and
	// we adjust the heading of the rudder based on that roll
	virtual void operator()(osg::Node* node, osg::NodeVisitor*)
	{
		// a bit of paranoia
		osg::ref_ptr<osg::MatrixTransform> mxt = dynamic_cast<osg::MatrixTransform*>(node);
		if (!mxt.valid()) return;

		// decompose
		double x;
		double y;
		double z;
		double h;
		double p;
		double r;
		OpenIG::Base::Math::instance()->fromMatrix(a320->getMatrix(), x, y, z, h, p, r);

		// save the a320 roll 
		// Also to mention, some modellers like Blender 
		// with shich the a320 model was modeled have
		// swapped pitch and roll, for our case
		double roll = r - lastCheckedRoll;
		lastCheckedRoll = r;

		diffs.push_back(roll);
		roll = getAverageRollDiff();

		// Save the original value that might come
		// from the xml
		if (!originalSaved)
		{
			mx = mxt->getMatrix();
			originalSaved = true;
		}
		OpenIG::Base::Math::instance()->fromMatrix(mx, x, y, z, h, p, r);

		// update the rudder
		osg::Matrixd mxH;
		mxH.makeRotate(osg::DegreesToRadians(h+roll*70.0), osg::Vec3(0, 0, 1));
		osg::Matrixd mxP;
		mxP.makeRotate(osg::DegreesToRadians(p+90), osg::Vec3(1, 0, 0));

		mxt->setMatrix(mxH * mxP * osg::Matrixd::translate(osg::Vec3d(x, y, z)));
	}

	// the main plane entity
	OpenIG::Base::ImageGenerator::Entity a320;
	// let save the original heading of the rudder
	bool	originalSaved;
	// original mx
	osg::Matrixd mx;
	// we need this for the difference
	double lastCheckedRoll;

	// we will average some roll differences to make it smoother
	typedef std::list<double>		RollDiffs;
	RollDiffs diffs;

	inline double getAverageRollDiff()
	{
		double result = 0.0;

		RollDiffs::iterator itr = diffs.begin();
		for (; itr != diffs.end(); ++itr)
			result += *itr;

		if (diffs.size() > 20)
			diffs.pop_front();

		return result / (diffs.size() ? (double)diffs.size() : 1.0);
	}
};

// This one will update the engines
struct UpdateEngineRotorUpdateCallback : public osg::NodeCallback
{
	UpdateEngineRotorUpdateCallback(bool &enable, bool& landed, bool reverse = false) 
		: _enabled(enable), _landed(landed), _lastCheckedEnabled(false)
		, _startTime(0.0), _reverse(reverse), _lastCheckedLanded(false), _landingTime(0.0)
	{
	}

	virtual void operator()(osg::Node* node, osg::NodeVisitor*)
	{
		// a bit of paranoia
		osg::ref_ptr<osg::MatrixTransform> mxt = dynamic_cast<osg::MatrixTransform*>(node);
		if (!mxt.valid()) return;

		// Check to stop it
		if (_lastCheckedEnabled && !_enabled)
		{
			_lastCheckedEnabled = false;
			_lastCheckedLanded = false;
			return;
		}
		if (!_enabled) return;

		// Here we check for the start time
		osg::Timer_t now = osg::Timer::instance()->tick();
		if (!_lastCheckedEnabled && _enabled)
		{
			_lastCheckedEnabled = true;
			_startTime = now;
		}
		double dt = osg::Timer::instance()->delta_s(_startTime, now);

		// this is our rotation change
		double rotation = osg::minimum(20.0, dt);

		if (!_lastCheckedLanded && _landed)
		{
			_lastCheckedLanded = true;
			_landingTime = now;
		}
		if (_landed)
		{
			dt = rotation / osg::maximum(osg::Timer::instance()->delta_s(_landingTime, now)*10.0, 0.000001);
		}

		// decompose
		double x;
		double y;
		double z;
		double h;
		double p;
		double r;
		OpenIG::Base::Math::instance()->fromMatrix(mxt->getMatrix(), x, y, z, h, p, r);

		rotation *= (_reverse ? -1.0 : 1.0);
		osg::Matrixd mx = OpenIG::Base::Math::instance()->toMatrix(x,y,z,h,p+90+rotation,r);

		// set the new stuff
		mxt->setMatrix(mx);
	}

protected:
	// we start when this is true
	bool&			_enabled;
	// We use this to know when to start
	bool			_lastCheckedEnabled;
	// the time when we started
	osg::Timer_t	_startTime;
	// for left and right engine
	bool			_reverse;
	// true for landed. We have to slow down the engines
	bool&			_landed;
	// last check on the landed var
	bool			_lastCheckedLanded;
	// landing time
	osg::Timer_t	_landingTime;
};

int main(int argc, char** argv)
{
    // We use osgViewer::CompositeViewer in OpenIG
    // to have more then one views. At present, the
    // version 1.0.0 of OpenIG does not works with
    // views, it means all the calls are related to
    // the first osgViewer::View (0). Later version
    // will have ID based view management
    osg::ref_ptr<osgViewer::CompositeViewer>    viewer = new osgViewer::CompositeViewer;

    // The following code snippet is the standard
    // setup of osgViewer::View in osgViewer::CompositeViewer
    // We assume the reader of the code is somewhat
    // familiar with reading OSG code at least at some
    // modest level.
    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi)
    {
        osg::notify(osg::NOTICE)<<"Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
        return 1;
    }

    unsigned int screen_width, screen_height;
    wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(0), screen_width, screen_height);

    double aspectratio = 1.33;
    if (viewer->getNumViews()==0)
    {
        osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;

        traits->x = 0;
        traits->y = 0;
        traits->width = screen_width;
        traits->height = screen_height;
        traits->windowDecoration = false;
		traits->doubleBuffer = true;
        traits->screenNum = 0;
        traits->sharedContext = 0;
        traits->vsync = false;

        osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
        if (gc.valid())
        {
            gc->setClearColor(osg::Vec4f(0.2f,0.2f,0.6f,1.0f));
            gc->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }
        else
        {
            osg::notify(osg::NOTICE)<<" \tGraphicsWindow has not been created successfully."<<std::endl;
            return 1;
        }

        osgViewer::View* view = new osgViewer::View;
        viewer->addView(view);
        view->getCamera()->setGraphicsContext(gc.get());
        view->getCamera()->setViewport(new osg::Viewport(0,0, traits->width, traits->height));
        aspectratio = static_cast<double>(traits->width) / static_cast<double>(traits->height);
        view->getCamera()->setProjectionMatrixAsPerspective(50, aspectratio, 1.0, 50000);
        view->getCamera()->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
        view->getCamera()->setCullingMode(osg::CullSettings::VIEW_FRUSTUM_CULLING);
		viewer->setThreadingModel(osgViewer::ViewerBase::CullThreadPerCameraDrawThreadPerContext);

    }

    // Add the stats handler bound to F2
    osg::ref_ptr<osgViewer::StatsHandler> stats = new osgViewer::StatsHandler;
    stats->setKeyEventTogglesOnScreenStats(osgGA::GUIEventAdapter::KEY_F2);
    viewer->getView(0)->addEventHandler(stats);

	// Add the Windowsing handler to F10
	osg::ref_ptr<osgViewer::WindowSizeHandler> wshandler = new osgViewer::WindowSizeHandler;
	wshandler->setKeyEventToggleFullscreen(osgGA::GUIEventAdapter::KEY_F10);
	viewer->getView(0)->addEventHandler(wshandler);

    // Add the StateSet event handler bound to F3
    // This shows wireframe mode etc
    // add the state manipulator
    osg::ref_ptr<osgGA::StateSetManipulator> stateSetManipulator = new osgGA::StateSetManipulator(
                viewer->getView(0)->getCamera()->getOrCreateStateSet()
    );
    stateSetManipulator->setKeyEventCyclePolygonMode(osgGA::GUIEventAdapter::KEY_F3);
    stateSetManipulator->setKeyEventToggleBackfaceCulling(0);
    stateSetManipulator->setKeyEventToggleLighting(0);
    stateSetManipulator->setKeyEventToggleTexturing(0);
    viewer->getView(0)->addEventHandler( stateSetManipulator );

    // Make an instance of OpenIG
	osg::ref_ptr<OpenIG::Engine> ig = new OpenIG::Engine;

    // We init OpenIG with your Viewer
    ig->init(viewer.get(),"igdata/openig.xml");

    // OpenIG provides hook to add entity to
    // the shadowed scene which is default, by
    // ig->getScene(). Now we don't want this,
    // instead we want to attach our info HUD
    // to the view scene data directly. This
    // way you can avoid the scene management
    ig->getViewer()->getView(0)->getSceneData()->asGroup()->addChild(createInfoHUD(ig.get()));

    // Add custom commands we have for this demo
    OpenIG::Base::Commands::instance()->addCommand("manip", new SetCameraManipulatorCommand(ig));
	OpenIG::Base::Commands::instance()->addCommand("fpo", new FlightPathOffsetCommand(s_Offset));

    // The scene can be defined in a startup script
    // using commands. Please have a look at the
    // sample script provided with this demo
    // for reference. We are going to use these
    // scripts only for adding entities, since
    // you might have different paths for the
    // data provided with the OpenIG demo. Also
    // code bellow is present to see how to load
    // entities in the scene too

    // we espect an argument with the name
    // of the script. If none provided, search
    // for defaults
    if (argc == 2)
    {
        std::string script = argv[1];
        ig->loadScript(script);
    }
    else
    {
        ig->loadScript(std::string("default.txt"));
    }

    // add the Compro a320 model to the scene
    // and bind it to entity ID 1. We will use
    // this entity ID to refer to this model.
    // We will place it at 0,0,HAT


    //ig->addEntity(MODEL_ENTITY_ID,s_ModelPath,modelMx);

	// Here we lookup for the Ailerons sub-entities
	// We have added also to find the wheels and some more
	OpenIG::Base::StringUtils::StringList names;
	names.push_back("Aileron-Right");
	names.push_back("Aileron-Left");
	names.push_back("Landing-Gear-Nose-Wheels");
	names.push_back("Landing-Gear-Left-Wheels");
	names.push_back("Landing-Gear-Right-Wheels");
	names.push_back("Elevator-Right");
	names.push_back("Elevator-Left");
	names.push_back("Rudder");
	names.push_back("Horizontal-Stabilizer-Right");
	names.push_back("Horizontal-Stabilizer-Left");
	names.push_back("Engine-Rotor-Left");
	names.push_back("Engine-Rotor-Right");

	FindSubEntitiesNodeVisitor nv(names, ig);

	// We lookup for the a320 Entity based on the ID
	OpenIG::Base::ImageGenerator::Entity a320 = ig->getEntityMap()[MODEL_ENTITY_ID];
	if (a320.valid())
	{
		a320->accept(nv);

		// Let install callbacks
		OpenIG::Base::ImageGenerator::Entity leftAileron = nv.subEntities["Aileron-Left"];
		unsigned int leftAileronID = nv.subEntitiesID["Aileron-Left"];

		// Check it again and install the callback
		if (leftAileron.valid() && leftAileronID)
		{
			leftAileron->setUpdateCallback(new UpdateAileronsNodeCallback(leftAileronID, a320, ig));
		}

		// Same for the right one
		OpenIG::Base::ImageGenerator::Entity rightAileron = nv.subEntities["Aileron-Right"];
		unsigned int rightAileronID = nv.subEntitiesID["Aileron-Right"];

		// Check it again and install the callback
		if (rightAileron.valid() && rightAileronID)
		{
			rightAileron->setUpdateCallback(new UpdateAileronsNodeCallback(rightAileronID, a320, ig, true));
		}

		// Here we have some callbacks for the wheels as well
		OpenIG::Base::ImageGenerator::Entity noseWheel = nv.subEntities["Landing-Gear-Nose-Wheels"];
		if (noseWheel.valid())
		{
			unsigned int id = nv.subEntitiesID["Landing-Gear-Nose-Wheels"];
			noseWheel->setUpdateCallback(new UpdateWheelUpdateCallback(id, 0.35, a320, ig, false));
		}
		OpenIG::Base::ImageGenerator::Entity leftWheel = nv.subEntities["Landing-Gear-Left-Wheels"];
		if (leftWheel.valid())
		{
			unsigned int id = nv.subEntitiesID["Landing-Gear-Left-Wheels"];
			leftWheel->setUpdateCallback(new UpdateWheelUpdateCallback(id, 0.52, a320, ig));
		}
		OpenIG::Base::ImageGenerator::Entity rightWheel = nv.subEntities["Landing-Gear-Right-Wheels"];
		if (rightWheel.valid())
		{
			unsigned int id = nv.subEntitiesID["Landing-Gear-Right-Wheels"];
			rightWheel->setUpdateCallback(new UpdateWheelUpdateCallback(id, 0.52, a320, ig));
		}

		// Update elevators
		OpenIG::Base::ImageGenerator::Entity rightElevator = nv.subEntities["Elevator-Right"];
		if (rightElevator.valid())
		{
			rightElevator->setUpdateCallback(new UpdateElevatorsUpdateCallback(a320));
		}
		OpenIG::Base::ImageGenerator::Entity leftElevator = nv.subEntities["Elevator-Left"];
		if (leftElevator.valid())
		{
			leftElevator->setUpdateCallback(new UpdateElevatorsUpdateCallback(a320));
		}

		// And the rudder
		OpenIG::Base::ImageGenerator::Entity rudder = nv.subEntities["Rudder"];
		if (rudder.valid())
		{
			rudder->setUpdateCallback(new UpdateRudderUpdateCallback(a320));
		}

		// Engines
		OpenIG::Base::ImageGenerator::Entity leftEngine = nv.subEntities["Engine-Rotor-Left"];
		if (leftEngine.valid())
		{
			leftEngine->setUpdateCallback(new UpdateEngineRotorUpdateCallback(s_WingTestStarted,s_Landed));
		}
		OpenIG::Base::ImageGenerator::Entity rightEngine = nv.subEntities["Engine-Rotor-Right"];
		if (rightEngine.valid())
		{
			rightEngine->setUpdateCallback(new UpdateEngineRotorUpdateCallback(s_WingTestStarted, s_Landed, true));
		}
	}

    // now we are going to attach a light source
    // to the nose landing gear. OpenIG has the
    // Entity/Sub-Entity paradigm. OpenIG expects
    // the model definition to come from an XML
    // file where many model attributes can be
    // defined, like sub-entities, lights,animations
    // tuning of textures and effects. For a
    // reference have a look at the XML file
    // in the IgPlugin-ModelComposition project.
    // Also, this plugin is taking care to compose
    // the Entity tree-like structure from all the
    // sub-models and generate sub-entities IDs. And
    // controlling of sub-entities is transparent,
    // exactly the same as controlling and Entity
    // When loading our a320 model, the ModelComposition
    // plugin is printing in the console the automatically
    // generated sub-entities IDs so we will use this easy
    // way to identify the nose landing gear. The proper
    // way is with NodeVisitor on loading and get handles
    // of all the interesting parts we want to control.
    // Also, lights are controlled through IDs. 0 ID is
    // reserved for sun/moon light source.

    // add light with an ID. We can offset the
    // light with a matrix

    //ig->addLight(NOSE_LANDING_GEAR_LIGHT_ID,osg::Matrixd::identity());

    // bind the new light to the submodel

    //ig->bindLightToEntity(NOSE_LANDING_GEAR_LIGHT_ID,NOSE_LANDING_GEAR_ID);

    // Now we show how to update
    // Light attributes, colors etc
    //OpenIG::Base::LightAttributes attr;
    //attr._ambient = osg::Vec4(0,0,0,1);
    //attr._diffuse = osg::Vec4(1,1,1,1);
    //attr._brightness = 5;
    //attr._cloudBrightness = 0.1;
    //attr._constantAttenuation = 100;
    //attr._specular = osg::Vec4(0,0,0,1);
    //attr._spotCutoff = 30;
    //attr._enabled = true;
    //attr._dirtyMask = OpenIG::Base::LightAttributes::ALL;
    //ig->updateLightAttributes(NOSE_LANDING_GEAR_LIGHT_ID,attr);

    // add the Demo terrain to the scene the
    // same way as for the model. Here we will
    // use osgDB::Options string to offset
    // the terrain model. The VDBOffset plugin
    // listen to this addEntity action and if
    // option string is provided with offset
    // coordinates, then the visual database
    // model will be shifted to avoid the known
    // precision issues known when rendering large
    // terrains

    //osg::ref_ptr<osgDB::Options> options = new osgDB::Options("-1.20639e+06,-5.099e+06,0");
    //osg::Matrixd terrainMx = osg::Matrixd::identity();
    //ig->addEntity(TERRAIN_ENTITY_ID,s_TerrainPath,terrainMx,options);

    // Show our viewer
    viewer->realize();    

    // This is our recorded flight path.
    // We used our MUSE simulation software
    // to make some simple recording of
    // taking off, fly around and landing.
    // It is now converted to osg::AnimationPath
    // to ensure same playback on any machine
    std::ifstream file;
    file.open(s_FlightRecordingPath.c_str(),std::ios::in);

    // The animation path
    osg::ref_ptr<osg::AnimationPath> path = new osg::AnimationPath;

    // let read the animation path from the stream
    if (file.is_open())
    {
        path->read(file);
        file.close();

        if (!path->empty())
        {
            // And we update our model.
            osg::Matrixd mx;
            path->getMatrix(path->getFirstTime(),mx);

			// we apply offset here
			mx = osg::Matrixd::translate(s_Offset) * mx;

			// and update the model
            ig->updateEntity(MODEL_ENTITY_ID,mx);
        }
    }

    // The demo database is very small,
    // so let make the scene nicer with fog
    // The parameter is in meters
    //ig->setFog(5000);

    // By default the OpenIG Demo is setup
    // to use some experimental runway
    // lighting via plugin, so we change
    // the time of day. Anytime while running
    // the demo you can change it to what you
    // want with the 'tod' command - please
    // refer to the onscreen help on F7, and
    // launch the command line terminal at the
    // bottom of the screen with F8
    //ig->setTimeOfDay(17,0);

    // Now we have event processing in OpenIG
    // like the command line 'terminal' when
    // you press F8, or the onscreen help on
    // F7. Windows for a reason is not passing
    // events when switching with Alt+Tab so
    // we provide these events this way. Only
    // Windows, MacOS and Linux are fine
    osgViewer::CompositeViewer::Windows wins;
    viewer->getWindows(wins);

    while (!viewer->done())
    {

#if defined(_WIN32)
        MSG msg;
        if (::PeekMessage(&msg,NULL,0,0,PM_NOREMOVE))
        {
            ::GetMessage(&msg, NULL, 0, 0);

            if (wins.size())
            {
                osgViewer::GraphicsHandleWin32 *hdl = dynamic_cast<osgViewer::GraphicsHandleWin32*>(wins.at(0));
                if(hdl)
                {
                    WNDPROC fWndProc = (WNDPROC)::GetWindowLongPtr(hdl->getHWND(), GWLP_WNDPROC);
                    if (fWndProc && hdl->getHWND())
					{
						::CallWindowProc(fWndProc,hdl->getHWND(),msg.message, msg.wParam, msg.lParam);
					}
                }
            }
        }
#endif

        // We update the OpenIG camera
        // based on the Trackball manipulators
        // camera only if we set it using our
        // custom command from above
        if (s_CameraManipulator.valid() && ig->isCameraBoundToEntity())
        {			
			ig->bindCameraUpdate(s_CameraManipulator->getMatrix());
        }

        // Here we call the frame
        ig->frame();

        // Now, if we are doing playback,
        // we update the model from the
        // recorded path. By default, if
        // we have used another manipulator
        // and not captured the F5 event
        // we sit frozen
        osg::ref_ptr<CameraTrackballManipulator> trackball = dynamic_cast<CameraTrackballManipulator*>(s_CameraManipulator.get());
        bool playbackOn = trackball.valid() ? trackball->playbackOn : false;

		// If we are still and not in playback
		// we might want to tune the offset a bit
		if (!path->empty() && !playbackOn && !s_WingTestDone)
		{
			// And we update our model.
			osg::Matrixd mx;
			path->getMatrix(path->getFirstTime(), mx);

			// we apply offset here
			mx = osg::Matrixd::translate(s_Offset) * mx;

			// and update the model
			ig->updateEntity(MODEL_ENTITY_ID, mx);
		}

        // We hard code here something. Let make
        // the wing tests by calling animations
        // on the wings. In the model XML file
        // there are pre-defined animations on
        // some wing parts OpenIG is keeping them
        // as sub-entities. The length of these
        // animations are in the XML file as well.
        // So before take off we will call few animations
        // for demo purpose


        // the start of the flaps down animation
        // this animation takes 5 seconds - have
        // a look at the model xml file
        static osg::Timer_t flapsDownStart = 0;


        // the start of the breaks up/down animation
        static osg::Timer_t breaksUpDownStart = 0;

        // switch for the breaks up animations
        static bool breaksUpDone = false;

        // here we start the test
        if (!s_WingTestDone && playbackOn && !s_WingTestStarted)
        {
            s_WingTestStarted = true;

            // We are going to call few
            // animations at once, this is
            // the OpenIG method for
            OpenIG::Base::StringUtils::StringList animations;
            animations.push_back("flaps-down-left");
            animations.push_back("flaps-down-right");

            // call the animations on the model
            ig->playAnimation(MODEL_ENTITY_ID,animations);

            // save the start of the animations
            flapsDownStart = osg::Timer::instance()->tick();
        }

        // Here we check for finished animations
        // first the flaps animation, takes 5 seconds
        if (!s_WingTestDone && playbackOn && flapsDownStart > 0)
        {
            osg::Timer_t now = osg::Timer::instance()->tick();
            if (osg::Timer::instance()->delta_s(flapsDownStart,now) > 5 && breaksUpDownStart == 0)
            {
                // Flaps down completed. Now call
                // the breaks test. They go up and down
                // in sepparate animations. NOTE: Although
                // we are calling named pre-baked animations,
                // they are really sub-entities. If you have
                // plane math model you can update them in real
                // time with a Matrix as you do for Entity

				OpenIG::Base::StringUtils::StringList animations;
                animations.push_back("breaks-up-left");
                animations.push_back("breaks-up-right");

                // call the animations on the model
                ig->playAnimation(MODEL_ENTITY_ID,animations);

                // save this
                breaksUpDownStart = osg::Timer::instance()->tick();
            }
        }

        // once the flaps-down animation is completed
        // we test the breaks
        if (!s_WingTestDone && playbackOn && breaksUpDownStart > 0)
        {
            osg::Timer_t now = osg::Timer::instance()->tick();
            if (!breaksUpDone && osg::Timer::instance()->delta_s(breaksUpDownStart,now) > 2)
            {
                breaksUpDone = true;

                // now the breaks are up. Put them down
				OpenIG::Base::StringUtils::StringList animations;
                animations.push_back("breaks-down-left");
                animations.push_back("breaks-down-right");

                // call the animations on the model
                ig->playAnimation(MODEL_ENTITY_ID,animations);
            }

            if (breaksUpDone && osg::Timer::instance()->delta_s(breaksUpDownStart,now) > 5)
            {
                // here the wing test is completed
                s_WingTestDone = true;
            }
        }


        // We update the model from the
        // recorded flight file here
        if (s_WingTestDone && playbackOn )
        {
            // Here we update the model
            // from the animation path
            if (s_PlaybackStartTime == 0)
            {
                s_PlaybackStartTime = osg::Timer::instance()->tick();
            }
            osg::Timer_t now = osg::Timer::instance()->tick();

            // Calculate the time for the animation path
			double time = osg::Timer::instance()->delta_s(s_PlaybackStartTime, now) - s_PausedTime;

            // Check if we hit the first frame
            time = osg::maximum(time,path->getFirstTime());

            // Get the matrix at that time
            osg::Matrixd mx;
            path->getMatrix(time,mx);

			// we apply offset here
			mx = osg::Matrixd::translate(s_Offset) * mx;

            // And we update the model
            ig->updateEntity(MODEL_ENTITY_ID,mx);            
        }

        // only if playback
        if (playbackOn && s_PlaybackStartTime > 0)
        {
            // Here we call animations on pre-defined
            // frames from the flight path - for a demo
            // purpose
            osg::Timer_t time = osg::Timer::instance()->delta_s(s_PlaybackStartTime,osg::Timer::instance()->tick());

			if (!s_LandingGearUp && time > (LANDING_GEAR_UP + s_PausedTime))
            {
				OpenIG::Base::StringUtils::StringList animations;
                animations.push_back("close-landing-gear-leg-nose");
                animations.push_back("close-landing-gear-leg-left");
                animations.push_back("close-landing-gear-leg-right");

                // call the animations on the model
                ig->playAnimation(MODEL_ENTITY_ID,animations);

                // update this
                s_LandingGearUp = true;
            }

			if (!s_FlapsUp && time > (FLAPS_UP + s_PausedTime))
            {
				OpenIG::Base::StringUtils::StringList animations;
                animations.push_back("flaps-up-left");
                animations.push_back("flaps-up-right");

                // call the animations on the model
                ig->playAnimation(MODEL_ENTITY_ID,animations);

                // update this
                s_FlapsUp = true;
            }

			if (!s_LandingGearDown && (time > LANDING_GEAR_DOWN + s_PausedTime))
            {
				OpenIG::Base::StringUtils::StringList animations;
                animations.push_back("open-landing-gear-leg-nose");
                animations.push_back("open-landing-gear-leg-left");
                animations.push_back("open-landing-gear-leg-right");

                // call the animations on the model
                ig->playAnimation(MODEL_ENTITY_ID,animations);

                // update this
                s_LandingGearDown = true;
            }

			if (!s_FlapsDown && (time > FLAPS_DOWN + s_PausedTime))
            {
				OpenIG::Base::StringUtils::StringList animations;
                animations.push_back("flaps-down-left");
                animations.push_back("flaps-down-right");

                // call the animations on the model
                ig->playAnimation(MODEL_ENTITY_ID,animations);

                // update this
                s_FlapsDown = true;
            }

			if (!s_BreaksUp && (time > BREAKS_UP + s_PausedTime))
            {
				OpenIG::Base::StringUtils::StringList animations;
                animations.push_back("breaks-up-left");
                animations.push_back("breaks-up-right");

                // call the animations on the model
                ig->playAnimation(MODEL_ENTITY_ID,animations);

                // update this
                s_BreaksUp = true;
            }

			if (time > (FLIGHT_PATH_EOF + s_PausedTime))
            {
				OpenIG::Base::StringUtils::StringList animations;
                animations.push_back("breaks-down-left");
                animations.push_back("breaks-down-right");
                animations.push_back("flaps-up-left");
                animations.push_back("flaps-up-right");

                // call the animations on the model
                ig->playAnimation(MODEL_ENTITY_ID,animations);

                // We are not in playback now
                if (trackball.valid())
                {
                    trackball->playbackOn = false;
                }

				// we set this
				s_Landed = true;
            }
        }

        // now we check for reset
        // if true, we set up stuff
        // to get us on the beginning
        if (s_Reset)
        {
            s_Reset = false;

            // Place the model at the first frame
            osg::Matrixd mx;
            path->getMatrix(path->getFirstTime(),mx);

			// we apply offset here
			mx = osg::Matrixd::translate(s_Offset) * mx;

            // And we update our model.
            ig->updateEntity(MODEL_ENTITY_ID,mx);

            // update this
            s_PlaybackStartTime = 0;

            // We are not in playback now
            if (trackball.valid())
            {
                trackball->playbackOn = false;
            }

            // Reset all of these for the initial
            // wing tests as well
            flapsDownStart      = 0;
            breaksUpDownStart   = 0;
            breaksUpDone        = false;

            s_LandingGearUp         = false;
            s_FlapsUp               = false;
            s_LandingGearDown       = false;
            s_FlapsDown             = false;
            s_BreaksUp              = false;
			s_Landed				= false;
			s_WingTestDone			= false;
			s_WingTestStarted		= false;

            s_PausedTime = 0.0;

            // Reset and stop all the animations here.
            // NOTE: to bring back all the animation players
            // to their initial position, we call resetAnimation
            // only on certain animations since it resets
            // th players position/orientation to the first frame

            // reset animations
            ig->resetAnimation(MODEL_ENTITY_ID,"breaks-up-left");
            ig->stopAnimation(MODEL_ENTITY_ID,"breaks-up-left");
            ig->resetAnimation(MODEL_ENTITY_ID,"breaks-up-right");
            ig->stopAnimation(MODEL_ENTITY_ID,"breaks-up-right");
            ig->stopAnimation(MODEL_ENTITY_ID,"breaks-down-left");
            ig->stopAnimation(MODEL_ENTITY_ID,"breaks-down-right");

            // flaps animations
            ig->resetAnimation(MODEL_ENTITY_ID,"flaps-down-left");
            ig->stopAnimation(MODEL_ENTITY_ID,"flaps-down-left");
            ig->resetAnimation(MODEL_ENTITY_ID,"flaps-down-right");
            ig->stopAnimation(MODEL_ENTITY_ID,"flaps-down-right");
            ig->stopAnimation(MODEL_ENTITY_ID,"flaps-up-left");
            ig->stopAnimation(MODEL_ENTITY_ID,"flaps-up-right");

            // landing gears
            ig->resetAnimation(MODEL_ENTITY_ID,"close-landing-gear-leg-nose");
            ig->stopAnimation(MODEL_ENTITY_ID,"close-landing-gear-leg-nose");
            ig->stopAnimation(MODEL_ENTITY_ID,"open-landing-gear-leg-nose");
            ig->resetAnimation(MODEL_ENTITY_ID,"close-landing-gear-leg-left");
            ig->stopAnimation(MODEL_ENTITY_ID,"close-landing-gear-leg-left");
            ig->stopAnimation(MODEL_ENTITY_ID,"open-landing-gear-leg-left");
            ig->resetAnimation(MODEL_ENTITY_ID,"close-landing-gear-leg-right");
            ig->stopAnimation(MODEL_ENTITY_ID,"close-landing-gear-leg-right");
            ig->stopAnimation(MODEL_ENTITY_ID,"open-landing-gear-leg-right");
        }

    }

    // Here the mandatory cleanup
    ig->cleanup();

    // There is problem with how windows manages
    // the referenced pointers accross dlls and there
    // is crash on exit - this is Windows only, Linux
    // and Mac are fine. So till fixed we kill it this
    // way. Nick.
    exit(-1);

}

