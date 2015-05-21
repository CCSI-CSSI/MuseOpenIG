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
#include <IgPluginCore/plugin.h>
#include <IgPluginCore/plugincontext.h>
#include <IgCore/imagegenerator.h>
#include <IgCore/attributes.h>
#include <IgCore/mathematics.h>

#include <cstshare/cstshareobject.h>

#include <osg/ref_ptr>
#include <osg/Vec3>

#include <osgDB/XmlParser>

#include <iostream>

namespace igplugins
{

class SLScenePlugin : public igplugincore::Plugin
{
public:
    virtual std::string getName() { return "SLScene"; }

    virtual std::string getDescription() { return "SLScene replacement for Muse"; }

    virtual std::string getVersion() { return "1.0.0"; }

    virtual std::string getAuthor() { return "ComPro, Nick"; }

    virtual void init(igplugincore::PluginContext&)
    {
        ShareObj->addRegionSegment( "LOCAL", "AMX" );

        _cameraPos = ShareObj->createShare<osg::Vec3d>( "otwPos" );
        _cameraAtt = ShareObj->createShare<osg::Vec3>( "otwAtt" );
    }


    virtual void update(igplugincore::PluginContext& context)
    {
        osg::Matrixd eye = igcore::Math::instance()->toMatrix(
                    _cameraPos->x(),
                    _cameraPos->y(),
                    _cameraPos->z(),
                    _cameraAtt->x(),
                    _cameraAtt->y()+90,
                    _cameraAtt->z());

        context.getImageGenerator()->setCameraPosition(eye);
#if 0
        osg::notify(osg::NOTICE) << "Camera Position from CstShare: "
                  << _cameraPos->x() << ", "
                  << _cameraPos->y() << ", "
                  << _cameraPos->z() << std::endl;
#endif
    }


protected:

    osg::Vec3d*     _cameraPos;
    osg::Vec3*      _cameraAtt;
};

} // namespace

#if defined(_MSC_VER) || defined(__MINGW32__)
    //  Microsoft
    #define EXPORT __declspec(dllexport)
    #define IMPORT __declspec(dllimport)
#elif defined(__GNUG__)
    //  GCC
    #define EXPORT __attribute__((visibility("default")))
    #define IMPORT
#else
    //  do nothing and hope for the best?
    #define EXPORT
    #define IMPORT
    #pragma warning Unknown dynamic link import/export semantics.
#endif

extern "C" EXPORT igplugincore::Plugin* CreatePlugin()
{
    return new igplugins::SLScenePlugin;
}

extern "C" EXPORT void DeletePlugin(igplugincore::Plugin* plugin)
{
    osg::ref_ptr<igplugincore::Plugin> p(plugin);
}
