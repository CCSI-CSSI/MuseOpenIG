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


#include "ForwardPlusEngine.h"
#include "OSGtoFPUtils.h"

#include <Core-OpenIG/Engine.h>

#include <Core-Base/Configuration.h>

#include <osg/Texture2D>

using namespace OpenIG::Plugins;

ForwardPlusEngine::ForwardPlusEngine(
		OpenIG::Base::ImageGenerator* ig, 
		osg::Group* scene,
		LightManager& lightManager, 
		FPLightMap& fplights, 
		LightData& lightData, 
		TileSpaceLightGrid& tileSpaceLightGrid)
	: _ig(ig)
	, _scene(scene)
	, _lightManager(lightManager)
	, _fplights(fplights)
	, _lightData(lightData)
	, _tileSpaceLightGrid(tileSpaceLightGrid)
	, _isLodCullingEnabled(false)
{
	std::string cullingActive = OpenIG::Base::Configuration::instance()->getConfig("ForwardPlusLightsLODCulling", "yes");
	if (cullingActive.compare(0, 3, "yes") == 0)
	{
		_isLodCullingEnabled = true;
	}	
}

void ForwardPlusEngine::updateFPCamera(void)
{
	osg::Vec3d vEye, vCenter, vUp;
	osg::Camera* pOsgCamera = _ig->getViewer()->getView(0)->getCamera();

	double fovy, aspectRatio, zNear, zFar;
	pOsgCamera->getProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);
	_fpCamera.SetPerspective(Math::ToRadians(fovy), aspectRatio, zNear, zFar);

	pOsgCamera->getViewMatrixAsLookAt(vEye, vCenter, vUp);
	_fpCamera.LookAt(OsgToFPUtils::toVector3_64(vEye), OsgToFPUtils::toVector3_64(vCenter), OsgToFPUtils::toVector3_64(vUp));

	Matrix4_64 mat1 = _fpCamera.GetViewProjectionMatrix();
	Matrix4_64 mat2;
	osg::Matrixd osgmat = pOsgCamera->getViewMatrix()*pOsgCamera->getProjectionMatrix();
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			mat2[i][j] = osgmat(j, i);
		}
	}
	ASSERT_PREDICATE(mat1.IsEqual(mat2, 0.001));

	osg::Viewport* viewport = pOsgCamera->getViewport();
	_fpViewport = Vector2_uint32(viewport->width(), viewport->height());
}

void ForwardPlusEngine::updateFPEngine(void)
{
	updateFPCamera();
	_lightManager.Update(&_fpCamera);
}

void ForwardPlusEngine::turnAllFPLights(bool on)
{
	for (FPLightMap::const_iterator fpLightIter = _fplights.begin(); fpLightIter != _fplights.end(); ++fpLightIter)
	{
		Light* pLight = fpLightIter->second;
		pLight->SetOn(on);
	}
}

void ForwardPlusEngine::setUpSunOrMoonLight()
{
	OpenThreads::ScopedLock<OpenThreads::Mutex>	lock(_updateSunMoonMutex);	

	OpenIG::Engine* openIG = dynamic_cast<OpenIG::Engine*>(_ig);
	if (openIG == 0)
	{
		return;
	}

	const osg::Matrixd& viewMatrix = openIG->getViewer()->getView(0)->getCamera()->getViewMatrix();

	static const unsigned int id = 0; // PPP: Reserved

	FPLightMap::const_iterator it = _fplights.find(id);
	Light* pFPLight = 0;
	if (it == _fplights.end())
	{
		pFPLight = _lightManager.CreateLight(OpenIG::Library::Graphics::LT_DIRECTIONAL);
		float fCustomFloats[3];
		fCustomFloats[0] = 99; fCustomFloats[1] = 0; fCustomFloats[2] = 0;
		pFPLight->SetCustomFloats(fCustomFloats);
		pFPLight->GetUserObjectBindings().SetUserAny("id", id);
		_fplights.insert(std::make_pair(id, pFPLight));
	}
	else
	{
		pFPLight = it->second;
	}
	ASSERT_PREDICATE(pFPLight);

	OpenIG::PluginBase::PluginContext& context = openIG->getPluginContext();

	bool ok = false;

	osg::Vec4 sunOrMoonDiffuseColor = _ig->getSunOrMoonLight()->getLight()->getDiffuse();
	osg::Vec3 sunOrMoonColor(sunOrMoonDiffuseColor.r(), sunOrMoonDiffuseColor.g(), sunOrMoonDiffuseColor.b());
	//ok = context.getOrCreateValueObject()->getUserValue("SilverLining-Atmosphere-SunOrMoonColor", sunOrMoonColor);
	//if (ok==false)
	//{
	//    return;
	//}

	osg::Vec4 sunOrMoonAmbientColor = _ig->getSunOrMoonLight()->getLight()->getAmbient();
	osg::Vec3 ambient(sunOrMoonAmbientColor.r(), sunOrMoonAmbientColor.g(), sunOrMoonAmbientColor.b());
	//ok = context.getOrCreateValueObject()->getUserValue("SilverLining-Atmosphere-Ambient", ambient);
	//if (ok==false)
	//{
	//    return;
	//}

	osg::Vec4 sunOrMoonPos = _ig->getSunOrMoonLight()->getLight()->getPosition();
	osg::Vec3 sunOrMoonPosition(sunOrMoonPos.x(), sunOrMoonPos.y(), sunOrMoonPos.z());
	//ok = context.getOrCreateValueObject()->getUserValue("SilverLining-Atmosphere-SunOrMoonPosition", sunOrMoonPosition);
	//if (ok==false)
	//{
	//    return;
	//}

	pFPLight->SetAmbientColor(OsgToFPUtils::toColorValue(ambient));
	pFPLight->SetDiffuseColor(OsgToFPUtils::toColorValue(sunOrMoonColor));
	pFPLight->SetSpecularColor(OsgToFPUtils::toColorValue(sunOrMoonColor));

	osg::Vec3d vViewSpaceDir = computeViewSpaceDirection(osg::Vec4d(0, 0, 0, 1), sunOrMoonPosition, viewMatrix);
	vViewSpaceDir.normalize();
	pFPLight->SetVec3_32_2(Vector3_32((float)-vViewSpaceDir.x(), -vViewSpaceDir.y(), -vViewSpaceDir.z()));

	pFPLight->SetOn(true);	
}

bool ForwardPlusEngine::isLodCulled(osg::DummyLight* pLight, const osg::Vec3d& vWorldPos, const osg::Vec3d& vEyePos)
{
	double lod = 0.0;
	if (pLight->getUserValue("realLightLOD", lod) && lod > 0.0)
	{
		if ((vEyePos - vWorldPos).length() > lod)
		{
			return true;
		}
	}
	return false;
}

osg::Matrix ForwardPlusEngine::computeWorldMatrix(osg::LightSource* lightSource, osgUtil::CullVisitor* cv)
{
	if (lightSource->getDataVariance() == osg::Object::STATIC)
	{
		osg::RefMatrix* rmx = dynamic_cast<osg::RefMatrix*>(lightSource->getUserData());
		if (rmx != NULL)
		{
			return *rmx;
		}
	}

#if 0
	osg::NodePath np;
	np.push_back(lightSource);

	osg::ref_ptr<osg::Group> parent = lightSource->getNumParents() ? lightSource->getParent(0) : 0;
	while (parent)
	{
		np.insert(np.begin(), parent);
		parent = parent->getNumParents() ? parent->getParent(0) : 0;
	}

	osg::Matrixd mx = osg::computeLocalToWorld(np);
#else
        osg::Matrixd mx = osg::computeLocalToWorld(cv->getNodePath());
#endif
	if (lightSource->getDataVariance() == osg::Object::STATIC)
	{
		lightSource->setUserData(new osg::RefMatrixd(mx));
	}
	return mx;
}

#if 0
void ForwardPlusEngine::findOsgLightsAndMatrices(LightsToModelViewMatrices& lmv, LightsToWorldMatrices& lw, osgUtil::RenderStage* rs)
{
	osgUtil::PositionalStateContainer::AttrMatrixList& aml = rs->getPositionalStateContainer()->getAttrMatrixList();

	for (osgUtil::PositionalStateContainer::AttrMatrixList::iterator itr = aml.begin(); itr != aml.end(); ++itr)
	{
		const osg::DummyLight* light = dynamic_cast<const osg::DummyLight*>(itr->first.get());
		if (light == 0)
		{
			continue;
		}
		osg::DummyLight* ncLight = const_cast<osg::DummyLight*>(light);
		osg::LightSource* lightSource = ncLight->getLightSource();
		if (lightSource == 0)
		{
			continue;
		}

		osg::RefMatrixd *refMatrix = itr->second.get();

		lmv.insert(std::make_pair(ncLight, osg::Matrixd(refMatrix ? *refMatrix : osg::Matrix::identity())));

		osg::Matrixd worldMatrix = computeWorldMatrix(lightSource);

		lw.insert(std::make_pair(ncLight, worldMatrix));
	}
}
#endif

osg::Vec4d ForwardPlusEngine::computeWorldPosition(osg::DummyLight* light, const osg::Matrixd& worldMatrix)
{
	return osg::Vec4d(light->getPosition().x(), light->getPosition().y(), light->getPosition().z(), 1)*worldMatrix;
}

osg::Vec3d ForwardPlusEngine::computeWorldDirection(osg::DummyLight* light, const osg::Matrixd& worldMatrix)
{
	osg::Vec4d pos1 = osg::Vec4d(light->getPosition().x(), light->getPosition().y(), light->getPosition().z(), 1)*worldMatrix;
	pos1.w() = 1;

	osg::Vec4d pos2 = osg::Vec4d(light->getPosition().x(), light->getPosition().y(), light->getPosition().z(), 1) + osg::Vec4d(light->getDirection(), 1);
	pos2.w() = 1;
	pos2 = pos2*worldMatrix;
	pos2.w() = 1;

	osg::Vec4d litDir4 = pos2 - pos1;
	osg::Vec3d litDir(litDir4.x(), litDir4.y(), litDir4.z());

	litDir.normalize();
	return litDir;
}

osg::Vec4d ForwardPlusEngine::computeViewSpacePosition(const osg::Vec4d& worldPos, const osg::Matrixd& viewMatrix)
{
	return worldPos*viewMatrix;
}

osg::Vec3d ForwardPlusEngine::computeViewSpaceDirection(const osg::Vec4d& worldPos, const osg::Vec3d& worldDirection, const osg::Matrixd& viewMatrix)
{
	osg::Vec4d pos1 = worldPos*viewMatrix;
	pos1.w() = 1;

	osg::Vec4d pos2 = worldPos + osg::Vec4d(worldDirection, 1);
	pos2.w() = 1;
	pos2 = pos2*viewMatrix;
	pos2.w() = 1;

	osg::Vec4d litDir4 = pos2 - pos1;
	osg::Vec3d litDir(litDir4.x(), litDir4.y(), litDir4.z());

	litDir.normalize();
	return litDir;
}

void ForwardPlusEngine::updateLightFromOsgLight(Light* pFPLight, osg::DummyLight* pOsgLight)
{
	ASSERT_PREDICATE(pFPLight && pOsgLight);

	pFPLight->SetAmbientColor(OsgToFPUtils::toColorValue(pOsgLight->getAmbient()));
	pFPLight->SetDiffuseColor(OsgToFPUtils::toColorValue(pOsgLight->getDiffuse()));
	pFPLight->SetSpecularColor(OsgToFPUtils::toColorValue(pOsgLight->getSpecular()));

	OpenIG::Library::Graphics::LightType lightType = pFPLight->GetLightType();

	ASSERT_PREDICATE(lightType != OpenIG::Library::Graphics::LT_UNKNOWN);

	if (pFPLight->GetLightType() == OpenIG::Library::Graphics::LT_SPOT || pFPLight->GetLightType() == OpenIG::Library::Graphics::LT_POINT)
	{
		double fStartRange, fEndRange;
		pOsgLight->getUserValue("fStartRange", fStartRange);
		pOsgLight->getUserValue("fEndRange", fEndRange);
		pFPLight->SetRanges(fStartRange, fEndRange);
	}
	if (pFPLight->GetLightType() == OpenIG::Library::Graphics::LT_SPOT)
	{
		double fSpotInnerAngle, fSpotOuterAngle;
		pOsgLight->getUserValue("fSpotInnerAngle", fSpotInnerAngle);
		pOsgLight->getUserValue("fSpotOuterAngle", fSpotOuterAngle);
		pFPLight->SetSpotLightAngles(fSpotInnerAngle, fSpotOuterAngle);
	}
}

bool ForwardPlusEngine::apply(osg::LightSource* lightSource, osgUtil::CullVisitor* cv)
{
	EXTRA_LOGGING_STATIC_HANDLER

	osg::DummyLight* pLight = dynamic_cast<osg::DummyLight*>(lightSource->getLight());
	if (pLight == NULL) return false;

	bool enabled = false;
	pLight->getUserValue("enabled", enabled);
	if (enabled == false) return false;
			
	const osg::Matrixd& worldMatrix = computeWorldMatrix(lightSource, cv);	
	const osg::Matrixd& viewMatrix = _ig->getViewer()->getView(0)->getCamera()->getViewMatrix();

	osg::Vec3d eye;
	osg::Vec3d ignore;
	_ig->getViewer()->getView(0)->getCamera()->getViewMatrixAsLookAt(eye,ignore,ignore);

	osg::Vec4d vWorldPos = computeWorldPosition(pLight, worldMatrix);
	osg::Vec3d vWorldDir = computeWorldDirection(pLight, worldMatrix);

	osg::Vec4d vViewSpacePos = computeViewSpacePosition(vWorldPos, viewMatrix);
	osg::Vec3d vViewSpaceDir = computeViewSpaceDirection(vWorldPos, vWorldDir, viewMatrix);

	osg::Vec3d vWorldPos3(osg::Vec3d(vWorldPos.x(), vWorldPos.y(), vWorldPos.z()));

	if (_isLodCullingEnabled)
	{
		bool bIsLodCulled = isLodCulled(pLight, vWorldPos3, eye);
		if (bIsLodCulled) return false;		
	}

	FPLightMapIterator fpIter = _fplights.find(pLight->getLightNum());
	ASSERT_PREDICATE(fpIter != _fplights.end());

	Light* pFPLight = fpIter->second;
	if (pFPLight->GetLightType() == OpenIG::Library::Graphics::LT_UNKNOWN) return false;

	pFPLight->SetOn(true);

#if EXTRA_LOGGING
	++numOfLightsEnabled;
#endif

	updateLightFromOsgLight(pFPLight, pLight);

	if (pFPLight->GetLightType() == OpenIG::Library::Graphics::LT_POINT || pFPLight->GetLightType() == OpenIG::Library::Graphics::LT_SPOT)
	{
		pFPLight->SetPosition(OsgToFPUtils::toVector3_64(vWorldPos));
	}
	if (pFPLight->GetLightType() == OpenIG::Library::Graphics::LT_SPOT || pFPLight->GetLightType() == OpenIG::Library::Graphics::LT_DIRECTIONAL)
	{
		pFPLight->SetDirection(OsgToFPUtils::toVector3_64(vWorldDir));
	}

	pFPLight->SetVec3_32_1(Vector3_32((float)vViewSpacePos.x(), vViewSpacePos.y(), vViewSpacePos.z()));
	pFPLight->SetVec3_32_2(Vector3_32((float)vViewSpaceDir.x(), vViewSpaceDir.y(), vViewSpaceDir.z()));	

#if 0
	if (_rampTexture.valid())
	{
		static bool once = true;
		static const int textureSlot = 8;
		if (once)
		{
			once = false;

			_scene->getOrCreateStateSet()->setTextureAttribute(textureSlot, _rampTexture);
			_scene->getOrCreateStateSet()->addUniform(new osg::Uniform("rampTexture", textureSlot));
		}
	}
#endif

	return true;
}

