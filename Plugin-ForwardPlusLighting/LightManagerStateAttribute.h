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
#pragma once

#include <osg/StateAttribute>

#include <Library-Graphics/Vector2.h>
#include <Library-Graphics/CameraFwdDeclare.h>

#include <Core-Utils/TBO.h>

#include <osg/Group>
#include <osg/observer_ptr>

namespace OpenIG {
	namespace Library {
		namespace Graphics {
			class LightManager;
			class LightData;
			class TileSpaceLightGrid;
		}
	}
}

namespace OpenIG {
	namespace Base {
		class ImageGenerator;
	}
}

namespace OpenIG {
	namespace Plugins {

		class glActiveTextureWrapped;

		class LightManagerStateAttribute : public osg::StateAttribute
		{
		public:
			LightManagerStateAttribute();
			LightManagerStateAttribute(const LightManagerStateAttribute& text, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

			void set(OpenIG::Library::Graphics::LightManager*	 _lightManager
				, OpenIG::Library::Graphics::Camera_64*	  _fpCamera
				, OpenIG::Library::Graphics::Vector2_uint32* _fpViewport
				, osg::Group* scene
				, OpenIG::Base::ImageGenerator* ig);


			META_StateAttribute(igplugins, OpenIG::Plugins::LightManagerStateAttribute, osg::StateAttribute::Type(PATCH_PARAMETER + 10));

			virtual int compare(const StateAttribute& rhs) const;

			virtual void apply(osg::State& state) const;

		protected:
			virtual ~LightManagerStateAttribute();
			OpenIG::Library::Graphics::LightManager*	 _lightManager;
			OpenIG::Library::Graphics::Camera_64*		 _fpCamera;
			OpenIG::Library::Graphics::Vector2_uint32* _fpViewport;
			OpenIG::Base::ImageGenerator* _ig;

			void packLights(void);
			void updateTiledShadingStuff(void);
			void updateLightDataTBO(void);
			void updateTileLightGridOffsetAndSizeTBO();
			void updateTileLightIndexListTBO();
			void updateTilingParams();

			OpenIG::Library::Graphics::LightData*      _lightData;
			OpenIG::Library::Graphics::TileSpaceLightGrid* _tileSpaceLightGrid;

			osg::TBO* _lightDataTBO;
			osg::TBO*  _lightGridOffsetAndSizeTBO;
			osg::TBO*  _lightIndexListTBO;

			osg::GLExtensions* _extensions;

			osg::observer_ptr<osg::Group> _scene;

			void setAttributesAndUniformsIfNotSet(void);

			glActiveTextureWrapped* _glActiveTextureWrapped;


		};
	}
}
