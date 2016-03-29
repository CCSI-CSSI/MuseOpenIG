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
#include <sstream>

#include <osgViewer/CompositeViewer>

#include <osgGA/TrackballManipulator>

#include <osgText/Text>

#include <Core-OpenIG/openig.h>

#include <Core-Base/commands.h>

#if defined(_WIN32)
#include <windows.h>
#include <osgViewer/api/Win32/GraphicsHandleWin32>
#endif

using namespace std;

// Let make some info HUD for the keyboard bindings
// based on the osghud example
osg::Node* createInfoHUD(OpenIG::Engine* ig)
{
	unsigned int screen_width = 1600, screen_height = 1200;

	// create a camera to set up the projection and model view matrices, and the subgraph to draw in the HUD
	osg::Camera* camera = new osg::Camera;

	// set the projection matrix
	camera->setProjectionMatrix(osg::Matrix::ortho2D(0, screen_width, 0, screen_height));

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
	stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);


	osgText::Text* text = new  osgText::Text;
	geode->addDrawable(text);

	std::ostringstream oss;
	oss << "(F1) OTW / EO / IR";

	osgText::String str(oss.str());

	text->setFont(timesFont);
	text->setPosition(osg::Vec3(10, 30, 1));
	text->setText(str);
	text->setAlignment(osgText::TextBase::LEFT_BOTTOM_BASE_LINE);
	text->setColor(osg::Vec4(1, 1, 1, 1));
	text->setCharacterSize(16);
	text->setCharacterSizeMode(osgText::TextBase::OBJECT_COORDS);
	text->setAxisAlignment(osgText::TextBase::XY_PLANE);
	text->setFontResolution(256, 256);

	float height = text->getBoundingBox().yMax() - text->getBoundingBox().yMin();
	
	text->setCharacterSize(16);
	text->setMaximumHeight(height);

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

class CameraTrackballManipulator : public osgGA::TrackballManipulator
{
public:
    CameraTrackballManipulator()
        : osgGA::TrackballManipulator()
    {

    }

    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& us)
    {
        switch(ea.getEventType())
            {
            case(osgGA::GUIEventAdapter::KEYDOWN):
                {
                    if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Space)
                    {
                        if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_CTRL)
                        {
                            flushMouseEventStack();
                            home(ea,us);
                            us.requestRedraw();
                            us.requestContinuousUpdate(false);
                        }
                        return true;
                    }
                }
            default:
                return osgGA::TrackballManipulator::handle(ea,us);
            }
        return false;
    }
};

osg::ref_ptr<osgGA::CameraManipulator> cm;

class SetCameraManipulatorCommand : public OpenIG::Base::Commands::Command
{
public:
    SetCameraManipulatorCommand(OpenIG::Base::ImageGenerator* ig)
        : _ig(ig)
    {

    }

    virtual const std::string getUsage() const
    {
        return "id name";
    }

	virtual const std::string getArgumentsFormat() const
	{
		return	"I:{trackball;earth}";
	}

    virtual const std::string getDescription() const
    {
        return  "sets camera manipulator\n"
                "     id - the id of the model\n"
                "     name - one of these:\n"
                "           trackball - the OSG Trackball camera manipulator\n"
                "           earth - the osgEarth camera manipulator";
    }

    virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
    {
        if (tokens.size() == 2)
        {
            unsigned int id     = atoi(tokens.at(0).c_str());
            std::string name    = tokens.at(1);

            OpenIG::Base::ImageGenerator::Entity& entity = _ig->getEntityMap()[id];
            if (entity.valid())
            {
                osg::ref_ptr<osgGA::CameraManipulator> manip;
                if (name.compare(0,9,"trackball") == 0)
                {
                    osg::ref_ptr<CameraTrackballManipulator> tb = new CameraTrackballManipulator;
                    tb->setAutoComputeHomePosition(false);
                    tb->setNode(entity->getChild(0));

                    double radius = entity->getChild(0)->getBound().radius();
                    double factor = 2.0;
                    osg::Vec3d center = entity->getChild(0)->getBound().center();

                    osg::Matrixd mx;
                    mx.makeLookAt(osg::Vec3d(0,radius*factor,0),center,osg::Vec3d(0,0,1));

                    tb->setByMatrix(mx);
                    tb->setHomePosition(osg::Vec3d(0,radius*factor,0),center,osg::Vec3d(0,0,1));
                    tb->setCenter(center);
                    tb->setDistance(radius*factor);

                    manip = tb;

                }
                else
                if (!(name.compare(0,5,"earth")))
                {
                    osg::notify(osg::NOTICE) << "OpenIG: earth camera manipulator not implemented yet" << std::endl;
                }

                if (manip.valid())
                {
                    _ig->getViewer()->getView(0)->setCameraManipulator(manip,true);
					_ig->getViewer()->getView(1)->setCameraManipulator(manip, true);					

					osg::Matrixd mx = manip->getMatrix();

					_ig->bindCameraToEntity(id, mx, 0);                   
					_ig->bindCameraToEntity(id, mx, 1);										

                    cm = manip;

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

struct SwitchViewOptionsEventHandler : public osgGA::GUIEventHandler
{
	SwitchViewOptionsEventHandler(osgViewer::CompositeViewer& viewer, OpenIG::Engine* openIG)
		: _viewer(viewer)
		, _openIG(openIG)
	{

	}

	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us)
	{
		switch (ea.getEventType())
		{
			case(osgGA::GUIEventAdapter::KEYDOWN) :
			{
				if (ea.getKey() == osgGA::GUIEventAdapter::KEY_F1)
				{
					if (_viewer.getNumViews() > 1)
					{
						osgViewer::View* view = _viewer.getView(1);

						std::map< unsigned int, OpenIG::Engine::ViewType >	types;
						types[0] = OpenIG::Engine::OTW;
						types[1] = OpenIG::Engine::EO;
						types[2] = OpenIG::Engine::IR;

						static unsigned int typesIndex = 0;
						typesIndex = (typesIndex + 1) % types.size();

						_openIG->setViewType(view, types[typesIndex]);
					}
				}
			}
			break;
			default:
				break;
		}
		return false;
	}

protected:
	osgViewer::CompositeViewer&	_viewer;
	OpenIG::Engine*				_openIG;
};


int main(int argc, char** argv)
{
    osg::ref_ptr<osgViewer::CompositeViewer>    viewer = new osgViewer::CompositeViewer;

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

		// View 0 - OTW
		{
			osgViewer::View* view = new osgViewer::View;
			viewer->addView(view);
			view->getCamera()->setGraphicsContext(gc.get());
			view->getCamera()->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));
			aspectratio = static_cast<double>(traits->width) / static_cast<double>(traits->height);
			view->getCamera()->setProjectionMatrixAsPerspective(45, aspectratio, 1.0, 100000);
			view->getCamera()->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
			view->getCamera()->setCullingMode(osg::CullSettings::VIEW_FRUSTUM_CULLING);
			view->setLightingMode(osgViewer::View::SKY_LIGHT);		
		}
		// View 1 
		{
			osgViewer::View* view = new osgViewer::View;
			viewer->addView(view);
			view->getCamera()->setGraphicsContext(gc.get());
			view->getCamera()->setViewport(new osg::Viewport(100.0, 100.0, traits->width/4.0, traits->height/4.0));
			aspectratio = (static_cast<double>(traits->width)/4.0) / (static_cast<double>(traits->height)/4.0);
			view->getCamera()->setProjectionMatrixAsPerspective(45, aspectratio, 1.0, 100000);
			view->getCamera()->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
			view->getCamera()->setCullingMode(osg::CullSettings::VIEW_FRUSTUM_CULLING);
			view->setLightingMode(osgViewer::View::SKY_LIGHT);
		}		

		viewer->setThreadingModel(osgViewer::ViewerBase::CullDrawThreadPerContext);

    }

    osg::ref_ptr<OpenIG::Engine> ig = new OpenIG::Engine;

	viewer->getView(0)->addEventHandler(new SwitchViewOptionsEventHandler(*viewer, ig));	

	// Here we show a way to make OpenIG works
	// with specific views, at init time
	OpenIG::Base::ImageGenerator::ViewIdentifiers ids;
	ids.push_back(0);

	//ig->setupInitFlags(OpenIG::Engine::None);
    ig->init(viewer.get(), "igdata/openig.xml", ids);

	// and in runtime with dynamicaly setting Views
#if 0
	ig->initView(viewer->getView(1), OpenIG::Engine::EO);	
#else
	ig->initView(viewer->getView(1));	
#endif

	viewer->getView(0)->getSceneData()->asGroup()->addChild(createInfoHUD(ig));


    OpenIG::Base::Commands::instance()->addCommand("manip", new SetCameraManipulatorCommand(ig));

    if (argc > 1)
    {
        ig->loadScript(std::string(argv[1]));
    }
    else
    {
        ig->loadScript(std::string("default.txt"));
    }

    viewer->realize();    

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

        if (cm.valid() && ig->isCameraBoundToEntity())
        {
			osg::Matrixd mx = cm->getMatrix();

            ig->bindCameraUpdate(mx, 0);
			ig->bindCameraUpdate(mx, 1);			
        }


        ig->frame();
    }

    ig->cleanup();    	
	ig = NULL;
}
