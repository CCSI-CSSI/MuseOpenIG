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
#include <Core-PluginBase/plugin.h>
#include <Core-PluginBase/plugincontext.h>

#include <Core-Base/imagegenerator.h>
#include <Core-Base/attributes.h>
#include <Core-Base/mathematics.h>

#if !defined (_WIN32)
	#include <cstshare/cstshareobject.h>
#endif

#include <osg/ref_ptr>
#include <osg/Vec3>

#include <osgDB/XmlParser>

#include <iostream>

namespace OpenIG {
	namespace Plugins {

		class MusePlugin : public PluginBase::Plugin
		{
		public:
			MusePlugin()
				: _cameraPos(0)
				, _cameraAtt(0)
			{
			}

			virtual std::string getName() { return "Muse"; }

			virtual std::string getDescription() { return "Integration sample of OpenIG and MUSE"; }

			virtual std::string getVersion() { return "2.0.0"; }

			virtual std::string getAuthor() { return "ComPro, Nick"; }

			virtual void clean(PluginBase::PluginContext&)
			{
#if defined (_WIN32)
				if (_cameraPos) delete _cameraPos;
				if (_cameraAtt) delete _cameraAtt;
#endif
			}


			virtual void init(PluginBase::PluginContext&)
			{
#if !defined (_WIN32)
				ShareObj->addRegionSegment("LOCAL", "AMX");

				_cameraPos = ShareObj->createShare<osg::Vec3d>("otwPos");
				_cameraAtt = ShareObj->createShare<osg::Vec3>("otwAtt");
#else
				_cameraPos = new osg::Vec3d;
				_cameraAtt = new osg::Vec3;
#endif
			}


			virtual void update(PluginBase::PluginContext& context)
			{
				osg::Matrixd eye = Base::Math::instance()->toMatrix(
					_cameraPos->x(),
					_cameraPos->y(),
					_cameraPos->z(),
					_cameraAtt->x(),
					_cameraAtt->y() + 90,
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

extern "C" EXPORT OpenIG::PluginBase::Plugin* CreatePlugin()
{
	return new OpenIG::Plugins::MusePlugin;
}

extern "C" EXPORT void DeletePlugin(OpenIG::PluginBase::Plugin* plugin)
{
	osg::ref_ptr<OpenIG::PluginBase::Plugin> p(plugin);
}
