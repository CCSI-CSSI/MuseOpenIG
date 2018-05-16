//#******************************************************************************
//#*
//#*      Copyright (C) 2017  Compro Computer Services
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
//#*****************************************************************************
#include <Core-Base/Commands.h>
#include <Core-Base/StringUtils.h>

#include <osg/Geode>
#include <osg/Group>
#include <osg/Node>
#include <osg/Camera>

#include <osgText/Text>

#include <vector>
#include <sstream>

#include "Engine.h"
#include "RenderBins.h"

namespace OpenIG
{

class OnScreenMessages: public osgGA::GUIEventHandler
{
public:
    OnScreenMessages(   Engine* ig,
                    osgText::Text* text,
                    osg::Geometry* geometry,
                    osgViewer::CompositeViewer* v,
                    osg::Group* scene,
                    osg::Node* crashg)
        : _ig(ig)
        , _cviewer(v)
        , _scene(scene)
        , _crashg(crashg)
        , _text(text)
        , _geometry(geometry)
        , _isON(false)
    {

    }

    void turnOffMessage()
    {
        //osg::notify(osg::NOTICE)<<"CrashScreen::turnOffCrash()!!" << std::endl;
        _scene->removeChild(_crashg);
        _isON=false;
    }

    void turnOnMessage(std::string text, bool crashScreen=false )
    {
        //osg::notify(osg::NOTICE)<<"CrashScreen::turnOnCrash()!!" << std::endl;

        //if screen messages are already on, we turn off to reset before
        //turning on again to display new message and to keep our
        //child list at a minimum as we are not keeping track of multiples.
        if(_isON)
            turnOffMessage();

        //Set the screen background color to RED for the crash screen
        //or GREY for the regular messages screen.
        osg::Vec4Array* colors = new osg::Vec4Array;
        if(crashScreen)//RED
        {
            colors->push_back(osg::Vec4(1.0f,0.0,0.0f,0.5f));
        }
        else//GREY
        {
            colors->push_back(osg::Vec4(0.2f,0.2,0.2f,0.7f));
        }
        _geometry->setColorArray(colors);
        _geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
        _geometry->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,4));
        _geometry->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::ON);
        _geometry->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

        if(text.size())
        {
            //Lets try to fit text longer than 47 chars onto multiple lines
            //if/when we encounter any long incoming text messages.
            int textsize = text.size();
            if(textsize>45)
            {
                for(int x=1;x<textsize;x++)
                {
                    //osg::notify(osg::NOTICE)<<"onScreenMessages::turnOnMessage() -- text.at(x): "<< text.at(x) << std::endl;
                    if(x % 35 == 0)
                    {
                        int y = x;
                        //osg::notify(osg::NOTICE)<<"onScreenMessages::turnOnMessage()!! x: "<< x << ", y: " << y << std::endl;
                        while(text.at(y) != ' ' && y!=0)
                        {
                           //osg::notify(osg::NOTICE)<<"onScreenMessages::turnOnMessage() -- text.at(y): "<< text.at(y) << std::endl;
                            y--;
                            if(y<0) y=0;//safety
                        }
                        //osg::notify(osg::NOTICE)<<"onScreenMessages::turnOnMessage()!! replacing at y: " << y << std::endl;
                        text.replace(y,1,1,'\n');
                        continue;
                    }
                }
            }

            _text->setText(text.c_str());
        }
        //Put back to default if no text provided to display per description of command..
        else
        {
            if(crashScreen)
                _text->setText("Crash Detected!!");
            else
                _text->setText("OpenIG Message Screen\nNo Text Entered To Be Displayed!");
        }
        _scene->addChild(_crashg);
        _isON=true;
    }


protected:
    Engine*             _ig;
    osgViewer::CompositeViewer* _cviewer;
    osg::Group*                 _scene;
    osg::ref_ptr<osg::Node>     _crashg;
    osg::ref_ptr<osgText::Text> _text;
    osg::Geometry*          _geometry;
    bool _isON;
};
OnScreenMessages *_onScreenMessages;

void Engine::initOnScreenMessages()
{
    if (!_viewer.valid())
        return;

    osg::ref_ptr<osg::Group> crashGroup = new osg::Group;

    osg::Camera* camera = new osg::Camera;
    crashGroup->addChild(camera);

    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi)
    {
        osg::notify(osg::NOTICE)<<"OpenIG: Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
        return;
    }

    unsigned int width, height;
    wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(0), width, height);

    //We are over-riding screen diminsions right now due to
    //not properly handling of multiple screens on one computer support in OSG at this time
    //the width is returning the width of ALL screens connected, not just your current screen...
    width = 1600;
    height = 1200;
    camera->setProjectionMatrix(osg::Matrix::ortho2D(0,width,0,height));
    camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    camera->setViewMatrix(osg::Matrix::identity());
    camera->setClearMask(GL_DEPTH_BUFFER_BIT);
    camera->setRenderOrder(osg::Camera::POST_RENDER);
    camera->setAllowEventFocus(false);
    camera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

    osg::Geode* geode = new osg::Geode;
    camera->addChild(geode);

    geode->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF|osg::StateAttribute::OVERRIDE|osg::StateAttribute::PROTECTED);

    osg::Geometry* geometry = new osg::Geometry;
    geode->addDrawable(geometry);

    float depth = 0.0f;

    osg::Vec3Array* vertices = new osg::Vec3Array;
    vertices->push_back(osg::Vec3(0,22,depth));
    vertices->push_back(osg::Vec3(width,22,depth));
    vertices->push_back(osg::Vec3(width,height,depth));
    vertices->push_back(osg::Vec3(0,height,depth));
    geometry->setVertexArray(vertices);

    osg::Vec3Array* normals = new osg::Vec3Array;
    normals->push_back(osg::Vec3(0.0f,0.0f,1.0f));
    geometry->setNormalArray(normals);
    geometry->setNormalBinding(osg::Geometry::BIND_OVERALL);

//    osg::Vec4Array* colors = new osg::Vec4Array;
//    colors->push_back(osg::Vec4(0.2f,0.2,0.2f,0.5f));
//    geometry->setColorArray(colors);
//    geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

//    geometry->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,4));
//    geometry->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::ON);
//    geometry->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

    osgText::Text* text = new osgText::Text;
    geode->addDrawable(text);

    geode->getOrCreateStateSet()->setRenderBinDetails(HUD_RENDER_BIN, "RenderBin");

    text->setPosition(osg::Vec3(width/2.0f,height/2.0f,0.0f));
    text->setFont("fonts/arial.ttf");
    text->setColor(osg::Vec4(1,1,1,1));
    text->setCharacterSize(60);
    text->setCharacterSizeMode(osgText::TextBase::OBJECT_COORDS);
    text->setAxisAlignment(osgText::TextBase::XY_PLANE);
    text->setFontResolution(256,256);
    text->setText("OpenIG Message Screen, no Text Entered!");
    text->setDataVariance(osg::Object::DYNAMIC);
    text->setAlignment(osgText::Text::CENTER_CENTER);

    _onScreenMessages = new OnScreenMessages(
                this,
                text,
                geometry,
                _viewer.get(),
                _viewer->getView(0)->getSceneData()->asGroup(),
                crashGroup.get());

}

void Engine::turnOnCrashScreen(const std::string& crashtext)
{
    _onScreenMessages->turnOnMessage(crashtext, true);
}

void Engine::turnOffCrashScreen()
{
    _onScreenMessages->turnOffMessage();
}

void Engine::turnOnScreenMessage(const std::string& text)
{
    _onScreenMessages->turnOnMessage(text, false);
}

void Engine::turnOffScreenMessage()
{
    _onScreenMessages->turnOffMessage();
}

}
