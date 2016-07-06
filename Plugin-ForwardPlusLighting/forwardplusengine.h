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

#include <Core-Base/imagegenerator.h>

#include <Library-Graphics/CommonUtils.h>
#include <Library-Graphics/LightManager.h>
#include <Library-Graphics/LightData.h>
#include <Library-Graphics/TileSpaceLightGrid.h>
#include <Library-Graphics/Camera.h>

using namespace OpenIG::Library::Graphics;

#include "forwardpluslightimplementationcallback.h"
#include "dummylight.h"

namespace OpenIG {
	namespace Plugins {

		typedef boost::unordered_map<osg::DummyLight*, osg::Matrixd> LightsToModelViewMatrices;
		typedef boost::unordered_map<osg::DummyLight*, osg::Matrixd> LightsToWorldMatrices;

		class ForwardPlusEngine
		{
		public:
			ForwardPlusEngine(
				OpenIG::Base::ImageGenerator* ig, 
				osg::Group* scene, 
				LightManager& lightManager, 
				FPLightMap& fplights, 
				LightData& lightData, 
				TileSpaceLightGrid& tileSpaceLightGrid
			);
			
			
		protected:
			osg::observer_ptr<osg::Group>		_scene;
			OpenIG::Base::ImageGenerator*		_ig;

			LightManager&						_lightManager;
			FPLightMap&							_fplights;
			LightData&							_lightData;
			TileSpaceLightGrid&					_tileSpaceLightGrid;

			Camera_64							_fpCamera;
			Vector2_uint32						_fpViewport;

			osg::ref_ptr<osg::Image>            _rampImage;
			osg::ref_ptr<osg::Texture2D>        _rampTexture;

			OpenThreads::Mutex					_updateSunMoonMutex;
			bool								_isLodCullingEnabled;

		public:
			void updateFPCamera(void);
			void updateFPEngine(void);
			void packLights(void);
			void updateLightDataTBO();
			void updateTileLightGridOffsetAndSizeTBO();
			void updateTileLightIndexListTBO();
			void updateTilingParams(const Vector2_uint32& tileSize, const Vector2_uint32& viewport);
			void updateTiledShadingStuff(void);
			void initializeRampTexture(void);
			void setUpSunOrMoonLight();		
			void turnAllFPLights(bool on);
			bool isLodCulled(osg::DummyLight* pLight, const osg::Vec3d& vWorldPos, const osg::Vec3d& vEyePos);
			void findOsgLightsAndMatrices(LightsToModelViewMatrices& lmv, LightsToWorldMatrices& lw, osgUtil::RenderStage* rs);
			void updateLightFromOsgLight(Light* pFPLight, osg::DummyLight* pOsgLight);
			bool apply(osg::LightSource* lightSource);

			osg::Matrix computeWorldMatrix(osg::LightSource* lightSource);
			osg::Vec4d	computeWorldPosition(osg::DummyLight* light, const osg::Matrixd& worldMatrix);
			osg::Vec3d	computeWorldDirection(osg::DummyLight* light, const osg::Matrixd& worldMatrix);
			osg::Vec4d	computeViewSpacePosition(const osg::Vec4d& worldPos, const osg::Matrixd& viewMatrix);
			osg::Vec3d	computeViewSpaceDirection(const osg::Vec4d& worldPos, const osg::Vec3d& worldDirection, const osg::Matrixd& viewMatrix);

			Camera_64*		getFPCamera()	{ return &_fpCamera; }
			Vector2_uint32* getFPViewport()	{ return &_fpViewport; }
			
		};
	} // namespace
} // namespace