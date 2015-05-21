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

#include "openig.h"

#include <osg/Image>
#include <osg/TextureRectangle>
#include <osg/Geometry>
#include <osg/Geode>

#include <osgDB/ReadFile>

using namespace openig;

void OpenIG::initSplashScreen()
{
    if (!_viewer.valid()) return;
#if 0
    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi)
    {
        osg::notify(osg::NOTICE)<<"Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
        return;
    }

    unsigned int screen_width, screen_height;
    wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(0), screen_width, screen_height);
#else
    unsigned int screen_width, screen_height;
    osg::Viewport* viewport = _viewer->getView(0)->getCamera()->getViewport();
    screen_width = viewport->width();
    screen_height = viewport->height();
#endif

#if defined(_WIN32)
    osg::ref_ptr<osg::Image> splash = osgDB::readImageFile("igdata/OpenIG-Splash.jpg");
#else
    osg::ref_ptr<osg::Image> splash = osgDB::readImageFile("/usr/local/lib/igdata/OpenIG-Splash.jpg");
#endif
    if (splash.valid())
    {
        osg::ref_ptr<osg::TextureRectangle> texture = new osg::TextureRectangle;
        texture->setImage(splash);

        float x = screen_width/2.0 - splash->s()/2.0;
        float y = screen_height/2.0 - splash->t()/2.0;
        float width = splash->s();
        float height = splash->t();

        _splashCamera = new osg::Camera;
        _viewer->getView(0)->getSceneData()->asGroup()->addChild(_splashCamera);

        _splashCamera->setViewport(new osg::Viewport(0,0, screen_width, screen_height));
        _splashCamera->setProjectionMatrix(osg::Matrix::ortho2D(0,screen_width,0,screen_height));
        _splashCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
        _splashCamera->setViewMatrix(osg::Matrix::identity());
        _splashCamera->setClearMask(GL_DEPTH_BUFFER_BIT);
        _splashCamera->setRenderOrder(osg::Camera::POST_RENDER);
        _splashCamera->setAllowEventFocus(false);
        _splashCamera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
        _splashCamera->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF|osg::StateAttribute::OVERRIDE|osg::StateAttribute::PROTECTED);

        osg::Geode* geode = new osg::Geode;
        _splashCamera->addChild(geode);

        geode->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF|osg::StateAttribute::OVERRIDE|osg::StateAttribute::PROTECTED);

        osg::Geometry* geometry = new osg::Geometry;
        geode->addDrawable(geometry);

        float depth = 0.0f;

        osg::Vec3Array* vertices = new osg::Vec3Array;
        vertices->push_back(osg::Vec3(x,y,depth));
        vertices->push_back(osg::Vec3(x+width,y,depth));
        vertices->push_back(osg::Vec3(x+width,y+height,depth));
        vertices->push_back(osg::Vec3(x,y+height,depth));
        geometry->setVertexArray(vertices);

        osg::Vec2Array* uvs = new osg::Vec2Array;
        uvs->push_back(osg::Vec2(0,0));
        uvs->push_back(osg::Vec2(splash->s(),0));
        uvs->push_back(osg::Vec2(splash->s(),splash->t()));
        uvs->push_back(osg::Vec2(0,splash->t()));
        geometry->setTexCoordArray(0,uvs);

        osg::Vec3Array* normals = new osg::Vec3Array;
        normals->push_back(osg::Vec3(0,0,-1));
        geometry->setNormalArray(normals,osg::Array::BIND_OVERALL);

        osg::Vec4Array* colors = new osg::Vec4Array;
        colors->push_back(osg::Vec4(1,1,1,1));
        geometry->setColorArray(colors,osg::Array::BIND_OVERALL);

        geometry->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,4));
        geometry->getOrCreateStateSet()->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
    }
}
