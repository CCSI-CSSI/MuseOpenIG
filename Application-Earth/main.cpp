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
//#*	author    Trajce Nikolov Nick openig@compro.net
//#*	copyright(c)Compro Computer Services, Inc.
//#*
//#*    Please direct any questions or comments to the OpenIG Forums
//#*    Email address: openig@compro.net
//#*
//#*
//#*    Please direct any questions or comments to the OpenIG Forums
//#*    Email address: openig@compro.net
//#*
#include <iostream>

#include <osg/CoordinateSystemNode>

#include <osgViewer/CompositeViewer>

#include <osgGA/TrackballManipulator>

#include <osgViewer/ViewerEventHandlers>

#include <osgEarth/Notify>
#include <osgEarthUtil/EarthManipulator>
#include <osgEarthUtil/ExampleResources>

#include <osgEarth/Units>
#include <osgEarth/Viewpoint>

#include <Core-OpenIG/Engine.h>

#include <Core-Base/Commands.h>
#include <Core-Base/Mathematics.h>

#if defined(_WIN32)
#include <windows.h>
#include <osgViewer/api/Win32/GraphicsHandleWin32>
#endif

#define LC "[viewer] "

using namespace std;

class CameraManipulator : public osgEarth::Util::EarthManipulator
{
public:
    CameraManipulator()
        : osgEarth::Util::EarthManipulator()
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
                return osgEarth::Util::EarthManipulator::handle(ea, us);
            }
        return false;
    }
};

osg::Matrixd toMatrix(double x, double y, double z, double h, double p, double r)
{
    osg::Matrixd mxR;
    mxR.makeRotate(osg::DegreesToRadians(r), osg::Vec3(1, 0, 0));
    osg::Matrixd mxP;
    mxP.makeRotate(osg::DegreesToRadians(p), osg::Vec3(0, 1, 0));
    osg::Matrixd mxH;
    mxH.makeRotate(osg::DegreesToRadians(-h), osg::Vec3(0, 0, 1));
    osg::Matrixd mxT;
    mxT.makeTranslate(osg::Vec3(x, y, z));

    return (mxP*mxR*mxH*mxT);
}

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

        osgViewer::View* view = new osgViewer::View;
        viewer->addView(view);
        view->getCamera()->setGraphicsContext(gc.get());
        view->getCamera()->setViewport(new osg::Viewport(0,0, traits->width, traits->height));
        aspectratio = static_cast<double>(traits->width) / static_cast<double>(traits->height);
        view->getCamera()->setProjectionMatrixAsPerspective(45, aspectratio, 1.0, 100000);
        view->setLightingMode(osgViewer::View::SKY_LIGHT);
        viewer->setThreadingModel(osgViewer::ViewerBase::DrawThreadPerContext);
    }

    // Add the Windowsing handler to F10
    osg::ref_ptr<osgViewer::WindowSizeHandler> wshandler = new osgViewer::WindowSizeHandler;
    wshandler->setKeyEventToggleFullscreen(osgGA::GUIEventAdapter::KEY_F10);
    viewer->getView(0)->addEventHandler(wshandler);

    osg::ref_ptr<OpenIG::Engine> ig = new OpenIG::Engine;
    ig->init(viewer.get(),"igdata/openig.xml");

    if (argc > 1)
    {
        ig->loadScript(std::string(argv[1]));
    }
    else
    {
        ig->loadScript(std::string("default.txt"));
    }

#if 0
    viewer->getView(0)->setCameraManipulator(new CameraManipulator());
#endif

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

        ig->frame();
    }

    ig->cleanup();
    ig = NULL;
}

