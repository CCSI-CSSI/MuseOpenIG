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

#pragma once

#include <map>

#include <Core-Base/attributes.h>

#include <Library-Graphics/Light.h>

#include <osg/Group>
#include <osg/LightSource>
#include <osg/observer_ptr>

#include <boost/unordered_map.hpp>

#include "lightmanagerstateattribute.h"


namespace OpenIG {
	namespace Base {
		class ImageGenerator;
		struct LightAttributes;
	}
}


namespace OpenIG {
	namespace Library {
		namespace Graphics {
			class Light;
			class LightManager;
			class LightData;
			class TileSpaceLightGrid;
		}
	}
}

namespace OpenIG {
	namespace Plugins
	{
		typedef boost::unordered_map<unsigned int, osg::ref_ptr<osg::LightSource> > LightSourcesMap;
		typedef boost::unordered_map<unsigned int, osg::ref_ptr<osg::LightSource> >::iterator LightSourcesMapIterator;

		typedef boost::unordered_map<unsigned int, OpenIG::Library::Graphics::Light* > FPLightMap;
		typedef boost::unordered_map<unsigned int, OpenIG::Library::Graphics::Light* >::iterator FPLightMapIterator;

		class ForwardPlusEngine;

		class ForwardPlusLightImplementationCallback : public OpenIG::Base::LightImplementationCallback
		{
		public:
			ForwardPlusLightImplementationCallback(OpenIG::Base::ImageGenerator* ig);
			~ForwardPlusLightImplementationCallback();

			virtual osg::Referenced*	createLight(unsigned int id, const OpenIG::Base::LightAttributes& definition, osg::Group* lightsGroup);
			virtual void				updateLight(unsigned int id, const OpenIG::Base::LightAttributes& definition);
			virtual void				deleteLight(unsigned int id);
			virtual void				setLightUserData(unsigned int id, osg::Referenced* data);

			ForwardPlusEngine*			getFPEngine() { return _fpEngine;  }

		protected:
			OpenIG::Base::ImageGenerator*					_ig;
			osg::observer_ptr<osg::Group>					_lightsGroup;
			osg::ref_ptr<osg::Group>						_dummyGroup;
			OpenIG::Library::Graphics::LightManager*	_lightManager;
			LightSourcesMap									_lightSourcesMap;
			FPLightMap										_fplights;
			OpenIG::Library::Graphics::GPGPUDevice*	_gpgpudevice;
			osg::ref_ptr<LightManagerStateAttribute>		_lightManagerStateAttribute;
			ForwardPlusEngine*								_fpEngine;

			void setInitialOSGLightParameters(
				osg::Light* light, 
				const OpenIG::Base::LightAttributes& definition, 
				const osg::Vec4d& pos, 
				const osg::Vec3f& dir
				);

			void updateOSGLightParameters(
				osg::Light* light, 
				const OpenIG::Base::LightAttributes& definition
				);

			OpenIG::Library::Graphics::LightType toFPLightType(
				OpenIG::Base::LightType lightType
				);
		};
	}
}
