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

#include "forwardpluslightimplementationcallback.h"
#include "lightmanagerstateattribute.h"
#include "dummylight.h"
#include "osgtofputils.h"
#include "forwardplusengine.h"

#include <Core-Base/imagegenerator.h>
#include <Core-Base/configuration.h>
#include <Core-Base/filesystem.h>
#include <Core-Base/framelogging.h>

#include <Core-OpenIG/openig.h>

#include <Library-Graphics/CommonUtils.h>
#include <Library-Graphics/LightManager.h>
#include <Library-Graphics/LightData.h>
#include <Library-Graphics/TileSpaceLightGrid.h>
#include <Library-Graphics/Camera.h>

#include <osgDB/ReadFile>
#include <osg/Texture2D>

#include <OpenThreads/Mutex>
#include <OpenThreads/ScopedLock>

#include <iostream>

using namespace OpenIG::Plugins;
using namespace OpenIG::Library::Graphics;

static const std::string strTilingParamsUniform = "vTilingParams";


ForwardPlusLightImplementationCallback::ForwardPlusLightImplementationCallback(OpenIG::Base::ImageGenerator* ig)
	: _ig(ig)	
	, _fpEngine(0)
{
	_lightManager = new LightManager();

	_lightData = new LightData(341, FORMAT_R32G32B32A32_FLOAT);

	Vector2_uint32 tileSize(32, 32);
	_tileSpaceLightGrid = new TileSpaceLightGrid(tileSize);
	_tileSpaceLightGrid->SetScreenAreaCullSize(tileSize*2);	
	
	_fpEngine = new ForwardPlusEngine(_ig, _ig->getScene()->asGroup(), *_lightManager, _fplights, *_lightData, *_tileSpaceLightGrid);	
}

ForwardPlusLightImplementationCallback::~ForwardPlusLightImplementationCallback()
{
	SAFE_DELETE(_tileSpaceLightGrid);
	SAFE_DELETE(_lightData);
	SAFE_DELETE(_lightManager);
	SAFE_DELETE(_fpEngine);
}

void ForwardPlusLightImplementationCallback::setInitialOSGLightParameters(osg::Light* light, const OpenIG::Base::LightAttributes& definition, const osg::Vec4d& pos, const osg::Vec3f& dir)
{
	if (light==0)
	{
		return;
	}

	light->setAmbient(definition.ambient);
	light->setDiffuse(definition.diffuse*definition.brightness);
	light->setSpecular(definition.specular);
	light->setConstantAttenuation(1.f/definition.constantAttenuation);
	light->setSpotCutoff(definition.spotCutoff);
	light->setPosition(pos);
	light->setDirection(dir);
	light->setUserValue("realLightLOD", (double)definition.realLightLOD);
	light->setUserValue("fStartRange", (double)definition.fStartRange);
	light->setUserValue("fEndRange", (double)definition.fEndRange);
	light->setUserValue("fSpotInnerAngle", (double)definition.fSpotInnerAngle);
	light->setUserValue("fSpotOuterAngle", (double)definition.fSpotOuterAngle);
}

void ForwardPlusLightImplementationCallback::updateOSGLightParameters(osg::Light* light, const OpenIG::Base::LightAttributes& definition)
{
	if (light==0)
	{
		return;
	}

	if (definition.dirtyMask & OpenIG::Base::LightAttributes::AMBIENT)
		light->setAmbient(definition.ambient);

	if (definition.dirtyMask & OpenIG::Base::LightAttributes::DIFFUSE)
	{
		light->setDiffuse(definition.diffuse*definition.brightness);
	}
	if (definition.dirtyMask & OpenIG::Base::LightAttributes::BRIGHTNESS)
	{
		light->setDiffuse(definition.diffuse*definition.brightness);
	}
	if (definition.dirtyMask & OpenIG::Base::LightAttributes::SPECULAR)
		light->setSpecular(definition.specular);

	if (definition.dirtyMask & OpenIG::Base::LightAttributes::CONSTANTATTENUATION)
		light->setConstantAttenuation(1.f/definition.constantAttenuation);

	if (definition.dirtyMask & OpenIG::Base::LightAttributes::SPOTCUTOFF)
		light->setSpotCutoff(definition.spotCutoff);

	if (definition.dirtyMask & OpenIG::Base::LightAttributes::REALLIGHTLOD)
	{
		light->setUserValue("realLightLOD", (double)definition.realLightLOD);
	}
	if (definition.dirtyMask & OpenIG::Base::LightAttributes::RANGES)
	{
		light->setUserValue("fStartRange", (double)definition.fStartRange);
		light->setUserValue("fEndRange", (double)definition.fEndRange);
	}
	if (definition.dirtyMask & OpenIG::Base::LightAttributes::ANGLES)
	{
		light->setUserValue("fSpotInnerAngle", (double)definition.fSpotInnerAngle);
		light->setUserValue("fSpotOuterAngle", (double)definition.fSpotOuterAngle);
	}
	if ( (definition.dirtyMask & OpenIG::Base::LightAttributes::ENABLED) == OpenIG::Base::LightAttributes::ENABLED)
	{
		light->setUserValue("enabled", definition.enabled);
	}
}

OpenIG::Library::Graphics::LightType ForwardPlusLightImplementationCallback::toFPLightType(OpenIG::Base::LightType lightType)
{
	if (lightType==OpenIG::Base::LT_DIRECTIONAL)
	{
		return OpenIG::Library::Graphics::LT_DIRECTIONAL;
	}
	if (lightType==OpenIG::Base::LT_POINT)
	{
		return OpenIG::Library::Graphics::LT_POINT;
	}
	if (lightType==OpenIG::Base::LT_SPOT)
	{
		return OpenIG::Library::Graphics::LT_SPOT;
	}
	if (lightType==OpenIG::Base::LT_UNKNOWN)
	{
		return OpenIG::Library::Graphics::LT_UNKNOWN;
	}
	ASSERT_PREDICATE(false);
	return OpenIG::Library::Graphics::LT_UNKNOWN;
}

struct ForwardPlusEngineCullCallback : public osg::NodeCallback
{
	ForwardPlusEngineCullCallback(ForwardPlusEngine* fpEngine)
		: _fpEngine(fpEngine)
	{
	}

	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
	{
		_fpEngine->turnAllFPLights(false);
		_fpEngine->setUpSunOrMoonLight();

		traverse(node, nv);

		_fpEngine->updateFPEngine();		
	}

protected:
	ForwardPlusEngine*	_fpEngine;
};

osg::Referenced* ForwardPlusLightImplementationCallback::createLight(unsigned int id, const OpenIG::Base::LightAttributes& definition, osg::Group* lightsGroup)
{
	// special case, light ID==0, the sun/moon light
	if (id == 0)
	{		
		return 0;
	}	

	osg::DummyLight* osgLight = new osg::DummyLight(id);
	osgLight->setUserValue("id", id);
	osgLight->setUserValue("enabled", definition.enabled);

	osg::ref_ptr<osg::LightSource> lightSource = new osg::LightSource;
	lightSource->setLight(osgLight);		
	// When true it is casuing the UFO lighting artefacts
	// TODO: Investigate and fix
	// lightSource->setCullingActive(definition.cullingActive);
	lightSource->setDataVariance(definition.dataVariance);
	lightSource->setNodeMask(0x4);

	osgLight->setLightSource(lightSource);

	setInitialOSGLightParameters(lightSource->getLight(), definition, osg::Vec4d(0, 0, 0, 1), osg::Vec3f(0, 1, 0));

	// Create an equivalent FP Light
	ASSERT_PREDICATE(_fplights.find(id)==_fplights.end());
	Light* pFPLight = _lightManager->CreateLight(toFPLightType(definition.lightType));

	// the original code didn't actually update the lights (or create them for that matter) in the tbo, so this is all we are going to set for now
	pFPLight->GetUserObjectBindings().SetUserAny("id", id);
	_fplights.insert(std::make_pair(id, pFPLight));

	osg::ref_ptr<osg::StateAttribute> attr = lightSource->getOrCreateStateSet()->getAttribute(osg::StateAttribute::LIGHT);
	if (attr.valid())
	{
		lightSource->getOrCreateStateSet()->removeAttribute(attr);
		osg::notify(osg::NOTICE) << "Light attr removed" << std::endl;
	}

	_lightSourcesMap[id] = lightSource;
	_lightsGroup = lightsGroup;	

	osg::Node* root = _ig->getViewer()->getView(0)->getSceneData();
	if (root->getCullCallback() == NULL)
	{
		root->setCullCallback(new ForwardPlusEngineCullCallback(_fpEngine));
	}

	if (_lightManagerStateAttribute.valid()==false)
	{
		_lightManagerStateAttribute = new LightManagerStateAttribute();
		_lightManagerStateAttribute->set(_lightManager, _fpEngine->getFPCamera(), _fpEngine->getFPViewport(), _ig->getScene()->asGroup(), _ig);
		osg::StateSet* stateset =  _ig->getScene()->asGroup()->getOrCreateStateSet();
		stateset->setAttribute(_lightManagerStateAttribute, osg::StateAttribute::ON);
	}	

	return lightSource;
}

void ForwardPlusLightImplementationCallback::updateLight(unsigned int id, const OpenIG::Base::LightAttributes& definition)
{
	LightSourcesMap::iterator itr = _lightSourcesMap.find(id);
	if (itr==_lightSourcesMap.end())
	{
		return;
	}

	osg::LightSource* lightSource = itr->second;
	osg::Light* light = lightSource->getLight();

	updateOSGLightParameters(light, definition);

	// PPP: The original code didn't really update the light parameters that were set above, but just set the light id and then updated
	// based upon the raw DummyLight parameters. So we are not going to do anything for now
}