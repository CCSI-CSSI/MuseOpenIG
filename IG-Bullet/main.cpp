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

#include <osg/AnimationPath>
#include <osg/ValueObject>

#include <osgViewer/CompositeViewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>

#include <osgText/Text>

#include <OpenIG/openig.h>

#include <IgCore/commands.h>
#include <IgCore/mathematics.h>
#include <IgCore/imagegenerator.h>

#if defined(_WIN32)
#include <windows.h>
#include <osgViewer/api/Win32/GraphicsHandleWin32>
#endif

// Let make some info HUD for the keyboard bindings
// based on the osghud example
osg::Node* createInfoHUD()
{
    // Get the screen resolution
    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi)
    {
        osg::notify(osg::NOTICE)<<"Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
        return 0;
    }

    unsigned int screen_width, screen_height;
    wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(0), screen_width, screen_height);

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

    text->setFont(timesFont);
    text->setPosition(osg::Vec3(20,30,1));
    text->setText("Bullet demo");
    text->setAlignment(osgText::TextBase::LEFT_BOTTOM_BASE_LINE);
    text->setColor(osg::Vec4(0,0,1,1));
    text->setCharacterSize(20);
    text->setCharacterSizeMode(osgText::TextBase::OBJECT_COORDS);
    text->setAxisAlignment(osgText::TextBase::XY_PLANE);
    text->setFontResolution(256,256);

    return camera;
}

// Keep the new Camera Manipulator available
// accross the whole app
static osg::ref_ptr<osgGA::CameraManipulator> s_CameraManipulator;

// We might want to bind the camera manipulator to
// an entity but update from network, so in this case
// the following will be set to false and will not get
// updates from the mouse/keyboard
static bool s_CameraManipulatorOn = true;

class CameraTrackballManipulator : public osgGA::TrackballManipulator
{
public:
	CameraTrackballManipulator(openig::OpenIG* ig)
        : osgGA::TrackballManipulator()        
		, _openIG(ig)
    {

    }

	void pick(osgViewer::View* view, const osgGA::GUIEventAdapter& ea)
	{
		

	}


    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& us)
    {
		
        switch(ea.getEventType())
            {
#if 0
			case osgGA::GUIEventAdapter::PUSH:
			{
				if (ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
				{
					if (_openIG->isCameraBoundToEntity())
					{
						_openIG->unbindCameraFromEntity();
					}

					osgUtil::LineSegmentIntersector::Intersections intersections;

					if (_openIG->getViewer()->getView(0)->computeIntersections(ea, intersections))
					{
						for (osgUtil::LineSegmentIntersector::Intersections::iterator hitr = intersections.begin();
							hitr != intersections.end();
							++hitr)
						{
							if (!hitr->nodePath.empty() && !(hitr->nodePath.back()->getName().empty()))
							{
								osg::ref_ptr<osg::Node> pickedNode = hitr->nodePath.back();
								if (pickedNode.valid())
								{

								}
							}
						}
					}
				}
			}
			break;
#endif
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
					if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Left)
					{			
						_openIG->getPluginContext().getOrCreateValueObject()->setUserValue("left", (bool)true);
						_openIG->getPluginContext().getOrCreateValueObject()->setUserValue("BulletSteeringCommand", (bool)true);
						_openIG->getPluginContext().getOrCreateValueObject()->setUserValue("vehicleID", (unsigned int)1);
						return true;
					}
					if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Right)
					{
						_openIG->getPluginContext().getOrCreateValueObject()->setUserValue("left", (bool)false);
						_openIG->getPluginContext().getOrCreateValueObject()->setUserValue("BulletSteeringCommand", (bool)true);
						_openIG->getPluginContext().getOrCreateValueObject()->setUserValue("vehicleID", (unsigned int)1);
						return true;
					}
					if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Up)
					{
						_openIG->getPluginContext().getOrCreateValueObject()->setUserValue("engine", (bool)true);
						_openIG->getPluginContext().getOrCreateValueObject()->setUserValue("BulletEngineCommand", (bool)true);
						_openIG->getPluginContext().getOrCreateValueObject()->setUserValue("vehicleID", (unsigned int)1);
						return true;
					}
					if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Down)
					{
						_openIG->getPluginContext().getOrCreateValueObject()->setUserValue("engine", (bool)false);
						_openIG->getPluginContext().getOrCreateValueObject()->setUserValue("BulletEngineCommand", (bool)true);
						_openIG->getPluginContext().getOrCreateValueObject()->setUserValue("vehicleID", (unsigned int)1);
						return true;
					}
                }
            break;
			case(osgGA::GUIEventAdapter::KEYUP) :
			{				
				if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Left)
				{					
					_openIG->getPluginContext().getOrCreateValueObject()->setUserValue("BulletSteeringCommand", (bool)false);
					return true;
				}
				if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Right)
				{
					_openIG->getPluginContext().getOrCreateValueObject()->setUserValue("BulletSteeringCommand", (bool)false);
					return true;
				}
				if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Up)
				{
					_openIG->getPluginContext().getOrCreateValueObject()->setUserValue("BulletEngineCommand", (bool)false);
					return true;
				}
				if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Down)
				{
					_openIG->getPluginContext().getOrCreateValueObject()->setUserValue("BulletEngineCommand", (bool)false);
					return true;
				}
			}
												  break;
            default:
                return osgGA::TrackballManipulator::handle(ea,us);
            }
        return false;
    }
       
    openig::OpenIG* _openIG;
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
class SetCameraManipulatorCommand : public igcore::Commands::Command
{
public:
    SetCameraManipulatorCommand(openig::OpenIG* ig)
        : _ig(ig)
    {

    }

    // Convinient method for the onscreen
    // help. This one is a must
    virtual const std::string getUsage() const
    {
        return "id name [optional:off]";
    }

    // Convinient method for the onscreen
    // help. This one is a must too
    virtual const std::string getDescription() const
    {
		return  "sets camera manipulator\n"
			"     id - the id of the model\n"
			"     name - one of these:\n"
			"           trackball - the OSG Trackball camera manipulator\n"
			"           earth - the osgEarth camera manipulator\n"
			"     off - optional, if provided the camera manipulator will be bound\n"
			"           to the entity but will not be updated (ex: might be over network)";
				
    }

    // This method is called when you invoke a command in the
    // terminal, call a command through the Commands class
    // tokens is std::vector<std::string> holding the arguments
    // provided
    virtual int exec(const igcore::StringUtils::Tokens& tokens)
    {        
        // We need only two arguments: the entity ID and the
        // manipulator name
        if (tokens.size() >= 2)
        {
            unsigned int id     = atoi(tokens.at(0).c_str());
            std::string name    = tokens.at(1);

            // The enitites are stores in std::map, id based, the id
            // is the Entity ID you refer accross the whole application
            // We get a reference to the entity
            igcore::ImageGenerator::Entity& entity = _ig->getEntityMap()[id];
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
                    osg::ref_ptr<CameraTrackballManipulator> tb = new CameraTrackballManipulator(_ig);
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
                    mx.makeLookAt(osg::Vec3d(10,radius*factor,2.5),center,osg::Vec3d(0,0,1));

                    tb->setByMatrix(mx);
                    tb->setHomePosition(osg::Vec3d(10,radius*factor,2.5),center,osg::Vec3d(0,0,1));
                    tb->setCenter(center);
                    tb->setDistance(radius*factor);

                    // Set the manipulator
                    // to be used - see bellow
                    manip = tb;

                }
                else
                if (name.compare(0,5,"earth") == 0)
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
                    _ig->bindCameraSetFixedUp(true,true);

                    // Set our app wide manipulator
                    // on this one as well. We will
                    // need it in the loop to
                    // update the OpenIG camera since
                    // it is bound to an Entity
                    s_CameraManipulator = manip;

                    osg::notify(osg::NOTICE) << "IG: " << name << " bound to entity " << id << std::endl;

                    if (tokens.size() >= 3 && (tokens.at(2).compare(0,3,"off") == 0) )
					{
						s_CameraManipulatorOn = false;
                        osg::notify(osg::NOTICE) << "Manipulator set to: OFF" << std::endl;

					}
//                    else
//                        osg::notify(osg::NOTICE) << "Manipulator set to: ON" << std::endl;

                }
            }

            return 0;
        }

        return -1;
    }
protected:
	openig::OpenIG* _ig;
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

    double aspectratio = 1.0;
    if (viewer->getNumViews()==0)
    {
        osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;

        // NOTE: This #ifdefs are here for
        // easeier switching between fullscreen
        // and windowed mode for the viewer
        // Yah, we can do it with Event Handler
        // too :-)
#if 0
        traits->x = 10;
        traits->y = 10;
		traits->width = 800;
		traits->height = 405;
        traits->windowDecoration = true;
#else
        traits->x = 0;
        traits->y = 0;
        traits->width = screen_width;
        traits->height = screen_height;
        traits->windowDecoration = false;
#endif
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
        view->getCamera()->setProjectionMatrixAsPerspective(45, aspectratio, 1.0, 100000);
        view->getCamera()->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
        view->getCamera()->setCullingMode(osg::CullSettings::VIEW_FRUSTUM_CULLING);
        view->setLightingMode(osgViewer::View::SKY_LIGHT);

        viewer->setThreadingModel(osgViewer::ViewerBase::DrawThreadPerContext);

    }

    // Add the stats handler bound to F2
    osg::ref_ptr<osgViewer::StatsHandler> stats = new osgViewer::StatsHandler;
    stats->setKeyEventTogglesOnScreenStats(osgGA::GUIEventAdapter::KEY_F2);
    viewer->getView(0)->addEventHandler(stats);

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
    osg::ref_ptr<openig::OpenIG> ig = new openig::OpenIG;

    // We init OpenIG with your Viewer
    ig->init(viewer.get(),"igdata/openig.xml");

    // OpenIG provides hook to add entity to
    // the shadowed scene which is default, by
    // ig->getScene(). Now we don't want this,
    // instead we want to attach our info HUD
    // to the view scene data directly. This
    // way you can avoid the scene management
    ig->getViewer()->getView(0)->getSceneData()->asGroup()->addChild(createInfoHUD());

    // The scene can be defined in a startup script
    // using commands. Please have a look at the
    // sample script provided with this demo
    // for reference. We are going to use these
    // scripts only for adding entities, since
    // you might have different paths for the
    // data provided with the OpenIG demo. Also
    // code bellow is present to see how to load
    // entities in the scene too

	// Add this custom command from above
	// the first argument is the name of
	// the command, so we invoke it by
	// 'manip'
	igcore::Commands::instance()->addCommand("manip", new SetCameraManipulatorCommand(ig));

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

    // Show our viewer
    viewer->realize();        

    // The demo database is very small,
    // so let make the scene nicer with fog
    // The parameter is in meters
    ig->setFog(5000);

    // By default the OpenIG Demo is setup
    // to use some experimental runway
    // lighting via plugin, so we change
    // the time of day. Anytime while running
    // the demo you can change it to what you
    // want with the 'tod' command - please
    // refer to the onscreen help on F7, and
    // launch the command line terminal at the
    // bottom of the screen with F8
    ig->setTimeOfDay(17,0);

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
		if (s_CameraManipulatorOn && s_CameraManipulator.valid() && ig->isCameraBoundToEntity())
        {			
			ig->bindCameraUpdate(s_CameraManipulator->getMatrix());
        }

        // Here we call the frame
        ig->frame(); 

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

