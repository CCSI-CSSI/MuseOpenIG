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
#include <Core-Base/commands.h>
#include <Core-Base/stringutils.h>

#include <osg/Geode>
#include <osg/Group>
#include <osg/Node>
#include <osg/Camera>

#include <osgText/Text>

#include <vector>
#include <sstream>

#include "openig.h"
#include "renderbins.h"

namespace OpenIG
{

class OnScreenHelp: public osgGA::GUIEventHandler
{
public:
    OnScreenHelp(   Engine* ig,
                    unsigned int numLines,
                    osgText::Text* text,
                    osgViewer::CompositeViewer* v,
                    osg::Group* scene,
                    osg::Node* help)
        : _ig(ig)
        , _viewer(v)
        , _scene(scene)
        , _help(help)
        , _text(text)
        , _numOnScreenTextLines(numLines)
        , _currentPage(0)
    {

    }

    bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter&)
    {
        switch(ea.getEventType())
        {
            case(osgGA::GUIEventAdapter::KEYDOWN):
                //if ((ea.getModKeyMask() & osgGA::GUIEventAdapter::KEY_Alt_L) == 0)
                {
                    switch (ea.getKey())
                    {
                    case osgGA::GUIEventAdapter::KEY_F7:
                        {
                            static bool helpOn = false;
                            helpOn = !helpOn;
                            if (helpOn)
                            {
                                createPages();
                                _scene->addChild(_help);
                            }
                            else
                            {
                                _scene->removeChild(_help);
                            }
                            return true;
                        }
                        break;
                    case osgGA::GUIEventAdapter::KEY_Page_Down:
                        setCurrentPage(_currentPage+1);
                        break;
                    case osgGA::GUIEventAdapter::KEY_Page_Up:
                        if (_currentPage >= 1)
                        {
                            setCurrentPage(_currentPage-1);
                        }
                        break;
                    default:
                        break;
                    }
                }
                break;
            default:
                break;
        }
        return false;
    }

    void createPages()
    {
       if (_pages.size()) return;

       unsigned int numLinesPerPage = 0;

       CommandsPage page;
       _pages.push_back(page);

       OpenIG::Base::Commands::CommandsMapConstIterator itr = OpenIG::Base::Commands::instance()->getCommands().begin();
       for ( ; itr != OpenIG::Base::Commands::instance()->getCommands().end(); ++itr)
       {
           OpenIG::Base::Commands::Command*   cmd = itr->second;
           std::string                  cmdName = itr->first;

           unsigned int offset = 1;
           if ( itr == OpenIG::Base::Commands::instance()->getCommands().begin())
           {
               offset = 3;
           }

           unsigned int numLines = offset;
           numLines += OpenIG::Base::StringUtils::instance()->numberOfLines(cmd->getUsage());
           numLines += OpenIG::Base::StringUtils::instance()->numberOfLines(cmd->getDescription());

           if (numLinesPerPage+numLines+1 > _numOnScreenTextLines)
           {
               CommandsPage page;
               _pages.push_back(page);

               numLinesPerPage = 0;
           }

           CommandsPage& page = _pages.back();

           Command command;
           command._cmd = cmd;
           command._name = cmdName;

           page.push_back(command);

           numLinesPerPage += numLines+1;
       }

       setCurrentPage(0);
    }

    void setCurrentPage(unsigned int pageIdx)
    {
        if (pageIdx >= _pages.size()) return;

        _currentPage = pageIdx;

        std::ostringstream oss;

        if (_currentPage == 0)
        {
            oss << "OpenIG version: " << _ig->version() << " build: " << __DATE__ << " " __TIME__ << std::endl;
            oss << std::endl;
        }

        CommandsPage& page = _pages[_currentPage];
        CommandsPageIterator itr = page.begin();
        for ( ; itr != page.end(); ++itr )
        {
            Command cmd = *itr;

            oss << cmd._name << std::endl;
            oss << cmd._cmd->getUsage() << std::endl;
            oss << cmd._cmd->getDescription() << std::endl;
            oss << std::endl;
        }

        _text->setText(oss.str());

    }


protected:
    Engine*             _ig;
    osgViewer::CompositeViewer* _viewer;
    osg::Group*                 _scene;
    osg::ref_ptr<osg::Node>     _help;
    osg::ref_ptr<osgText::Text> _text;
    unsigned int                _numOnScreenTextLines;
    unsigned int                _currentPage;

    struct Command
    {
        OpenIG::Base::Commands::Command*  _cmd;
        std::string                 _name;
    };

    typedef std::vector<Command>             CommandsPage;
    typedef std::vector<Command>::iterator   CommandsPageIterator;

    typedef std::vector<CommandsPage>           Pages;
    typedef std::vector<CommandsPage>::iterator PagesIterator;

    Pages                       _pages;

};

void Engine::initOnScreenHelp()
{
    if (!_viewer.valid())
        return;

    osg::ref_ptr<osg::Group> helpGroup = new osg::Group;

    osg::Camera* camera = new osg::Camera;
    helpGroup->addChild(camera);

    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi)
    {
        osg::notify(osg::NOTICE)<<"OpenIG: Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
        return;
    }

    unsigned int width, height;
    wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(0), width, height);

    width = 1280;
    height = 1024;
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
    vertices->push_back(osg::Vec3(1280,22,depth));
    vertices->push_back(osg::Vec3(1280,1024,depth));
    vertices->push_back(osg::Vec3(0,1024,depth));
    geometry->setVertexArray(vertices);

    osg::Vec3Array* normals = new osg::Vec3Array;
    normals->push_back(osg::Vec3(0.0f,0.0f,1.0f));
    geometry->setNormalArray(normals);
    geometry->setNormalBinding(osg::Geometry::BIND_OVERALL);

    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(osg::Vec4(.2f,.2,0.2f,0.7f));
    geometry->setColorArray(colors);
    geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    geometry->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,4));
    geometry->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::ON);
    geometry->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

    osgText::Text* text = new osgText::Text;
    geode->addDrawable(text);

    geode->getOrCreateStateSet()->setRenderBinDetails(HUD_RENDER_BIN, "RenderBin");

    text->setPosition(osg::Vec3(2,1024,1.f));
    text->setFont("fonts/arial.ttf");
    text->setColor(osg::Vec4(1,1,1,1));
    text->setCharacterSize(20);
    text->setCharacterSizeMode(osgText::TextBase::OBJECT_COORDS);
    text->setAxisAlignment(osgText::TextBase::XY_PLANE);
    text->setFontResolution(256,256);
    text->setText("");
    text->setDataVariance(osg::Object::DYNAMIC);
    text->setAlignment(osgText::Text::LEFT_TOP);

    _viewer->getView(0)->addEventHandler(
        new OnScreenHelp(
                this,
                height/22,
                text,
                _viewer.get(),
                _viewer->getView(0)->getSceneData()->asGroup(),
                helpGroup.get())
     );

}

}
