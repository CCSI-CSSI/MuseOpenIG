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

#include "dummylight.h"
#include "osgtofputils.h"

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

#include "lightmanagerstateattribute.h"

using namespace OpenIG::Plugins;
using namespace OpenIG::Library::Graphics;

static const std::string strTilingParamsUniform = "vTilingParams";

class UpdateLightAttribscallback : public osg::NodeCallback
{
public:
    UpdateLightAttribscallback(OpenIG::Base::ImageGenerator* ig, osg::Group* scene
		, LightManager& lightManager, FPLightMap& fplights, LightData& lightData, TileSpaceLightGrid& tileSpaceLightGrid);
    virtual void operator()(osg::Node*, osg::NodeVisitor* nv);

	Camera_64     _fpCamera;
	Vector2_uint32 _fpViewport;
protected:
    osg::observer_ptr<osg::Group>   _scene;
    OpenIG::Base::ImageGenerator*			_ig;

    LightManager& _lightManager;
    FPLightMap& _fplights;
    LightData& _lightData;
    TileSpaceLightGrid& _tileSpaceLightGrid;

    void updateFPCamera(void);
    void updateFPEngine(void);
    void packLights(void);
    void updateLightDataTBO();
    void updateTileLightGridOffsetAndSizeTBO();
    void updateTileLightIndexListTBO();
    void updateTilingParams(const Vector2_uint32& tileSize, const Vector2_uint32& viewport);
    void updateTiledShadingStuff(void);
    void initializeRampTexture(void);
public:
    void setUpSunOrMoonLight(const osg::Matrixd& viewMatrix);
    
    osg::ref_ptr<osg::Image>            _rampImage;
    osg::ref_ptr<osg::Texture2D>        _rampTexture;

	OpenThreads::Mutex					_updateSunMoonMutex;

    ~UpdateLightAttribscallback();
};

UpdateLightAttribscallback::UpdateLightAttribscallback(OpenIG::Base::ImageGenerator* ig, osg::Group* scene
    , LightManager& lightManager, FPLightMap& fplights, LightData& lightData, TileSpaceLightGrid& tileSpaceLightGrid)
    : _ig(ig)
    , _scene(scene)
    , _lightManager(lightManager)
    , _fplights(fplights)
    , _lightData(lightData)
    , _tileSpaceLightGrid(tileSpaceLightGrid)
{

}

UpdateLightAttribscallback::~UpdateLightAttribscallback()
{

}

typedef boost::unordered_map<osg::DummyLight*, osg::Matrixd> LightsToModelViewMatrices;
typedef boost::unordered_map<osg::DummyLight*, osg::Matrixd> LightsToWorldMatrices;

static osg::Matrix computeWorldMatrix(osg::LightSource* lightSource)
{
	if (lightSource->getUserData())
	{
		osg::ref_ptr<osg::RefMatrixd> mx = dynamic_cast<osg::RefMatrixd*>(lightSource->getUserData());
		if (mx.valid())
			return *mx;
	}
	
	osg::NodePath np;
	np.push_back(lightSource);

	osg::ref_ptr<osg::Group> parent = lightSource->getNumParents() ? lightSource->getParent(0) : 0;
	while (parent)
	{
		np.insert(np.begin(), parent);
		parent = parent->getNumParents() ? parent->getParent(0) : 0;
	}	

	osg::Matrixd mx = osg::computeLocalToWorld(np);

	if (lightSource->getDataVariance() == osg::Object::STATIC)
	{
		lightSource->setUserData(new osg::RefMatrixd(mx));
	}

	return mx;
}

static void findOsgLightsAndMatrices(LightsToModelViewMatrices& lmv, LightsToWorldMatrices& lw, osgUtil::RenderStage* rs)
{
    osgUtil::PositionalStateContainer::AttrMatrixList& aml = rs->getPositionalStateContainer()->getAttrMatrixList();
    
	//std::cout << aml.size() << std::endl;
    for (osgUtil::PositionalStateContainer::AttrMatrixList::iterator itr = aml.begin(); itr != aml.end(); ++itr)
    {		
        const osg::DummyLight* light = dynamic_cast<const osg::DummyLight*>(itr->first.get());
        if (light==0)
        {
            continue;
        }

		osg::DummyLight* ncLight = const_cast<osg::DummyLight*>(light);
        osg::LightSource* lightSource = ncLight->getLightSource();
        if (lightSource==0)
        {
            continue;
        }

        osg::RefMatrixd *refMatrix = itr->second.get();

        lmv.insert(std::make_pair(ncLight, osg::Matrixd(refMatrix?*refMatrix:osg::Matrix::identity())));

        osg::Matrixd worldMatrix = computeWorldMatrix(lightSource);

        lw.insert(std::make_pair(ncLight, worldMatrix));
    }
}

static osg::Vec4d computeWorldPosition(osg::DummyLight* light, const osg::Matrixd& worldMatrix)
{
	return osg::Vec4d(light->getPosition().x(),light->getPosition().y(),light->getPosition().z(),1)*worldMatrix;
}

static osg::Vec3d computeWorldDirection(osg::DummyLight* light, const osg::Matrixd& worldMatrix)
{
    osg::Vec4d pos1 = osg::Vec4d(light->getPosition().x(),light->getPosition().y(),light->getPosition().z(),1)*worldMatrix;
    pos1.w() = 1;

    osg::Vec4d pos2 = osg::Vec4d(light->getPosition().x(),light->getPosition().y(),light->getPosition().z(),1) + osg::Vec4d(light->getDirection(),1);
    pos2.w() = 1;
    pos2 = pos2*worldMatrix;
    pos2.w() = 1;

    osg::Vec4d litDir4 = pos2 - pos1;
    osg::Vec3d litDir(litDir4.x(), litDir4.y(), litDir4.z());

    litDir.normalize();
    return litDir;
}

static osg::Vec4d computeViewSpacePosition(const osg::Vec4d& worldPos, const osg::Matrixd& viewMatrix)
{
    return worldPos*viewMatrix;
}

static osg::Vec3d computeViewSpaceDirection(const osg::Vec4d& worldPos, const osg::Vec3d& worldDirection, const osg::Matrixd& viewMatrix)
{
    osg::Vec4d pos1 = worldPos*viewMatrix;
    pos1.w() = 1;

    osg::Vec4d pos2 = worldPos + osg::Vec4d(worldDirection,1);
    pos2.w() = 1;
    pos2 = pos2*viewMatrix;
    pos2.w() = 1;

    osg::Vec4d litDir4 = pos2 - pos1;
    osg::Vec3d litDir(litDir4.x(), litDir4.y(), litDir4.z());

    litDir.normalize();
    return litDir;
}

static void updateLightFromOsgLight(Light* pFPLight, osg::DummyLight* pOsgLight)
{
    ASSERT_PREDICATE(pFPLight && pOsgLight);

    pFPLight->SetAmbientColor(OsgToFPUtils::toColorValue(pOsgLight->getAmbient()));
    pFPLight->SetDiffuseColor(OsgToFPUtils::toColorValue(pOsgLight->getDiffuse()));
    pFPLight->SetSpecularColor(OsgToFPUtils::toColorValue(pOsgLight->getSpecular()));

    OpenIG::Library::Graphics::LightType lightType = pFPLight->GetLightType();

    ASSERT_PREDICATE(lightType!=OpenIG::Library::Graphics::LT_UNKNOWN);

    if (pFPLight->GetLightType()==OpenIG::Library::Graphics::LT_SPOT||pFPLight->GetLightType()==OpenIG::Library::Graphics::LT_POINT)
    {
        double fStartRange, fEndRange;
        pOsgLight->getUserValue("fStartRange", fStartRange);
        pOsgLight->getUserValue("fEndRange", fEndRange);
        pFPLight->SetRanges(fStartRange, fEndRange);
    }
    if (pFPLight->GetLightType()==OpenIG::Library::Graphics::LT_SPOT)
    {
        double fSpotInnerAngle, fSpotOuterAngle;
        pOsgLight->getUserValue("fSpotInnerAngle", fSpotInnerAngle);
        pOsgLight->getUserValue("fSpotOuterAngle", fSpotOuterAngle);
        pFPLight->SetSpotLightAngles(fSpotInnerAngle, fSpotOuterAngle);
    }
}

void UpdateLightAttribscallback::updateFPCamera(void)
{
    osg::Vec3d vEye, vCenter, vUp;
    osg::Camera* pOsgCamera = _ig->getViewer()->getView(0)->getCamera();

    double fovy, aspectRatio,zNear, zFar;
    pOsgCamera->getProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);
    _fpCamera.SetPerspective(Math::ToRadians(fovy), aspectRatio, zNear, zFar);

    pOsgCamera->getViewMatrixAsLookAt(vEye,vCenter,vUp);
    _fpCamera.LookAt(OsgToFPUtils::toVector3_64(vEye), OsgToFPUtils::toVector3_64(vCenter), OsgToFPUtils::toVector3_64(vUp));

    Matrix4_64 mat1 = _fpCamera.GetViewProjectionMatrix();
    Matrix4_64 mat2;
    osg::Matrixd osgmat =  pOsgCamera->getViewMatrix()*pOsgCamera->getProjectionMatrix();
    for(int i = 0; i < 4; ++i)
    {
        for(int j = 0; j < 4; ++j)
        {
            mat2[i][j] = osgmat(j, i);
        }
    }
    ASSERT_PREDICATE(mat1.IsEqual(mat2,0.001));

	osg::Viewport* viewport = pOsgCamera->getViewport();
	_fpViewport = Vector2_uint32(viewport->width(), viewport->height());
}

void UpdateLightAttribscallback::updateFPEngine(void)
{
	updateFPCamera();
    _lightManager.Update(&_fpCamera);
}

bool isLodCulled(osg::DummyLight* pLight, const osg::Vec3d& vWorldPos, const osg::Vec3d& vEyePos)
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

void turnAllFPLights(const FPLightMap& fplights, bool on)
{
    for(FPLightMap::const_iterator fpLightIter = fplights.begin(); fpLightIter != fplights.end(); ++fpLightIter)
    {
        Light* pLight = fpLightIter->second;
        pLight->SetOn(on);
    }
}

void UpdateLightAttribscallback::setUpSunOrMoonLight(const osg::Matrixd& viewMatrix)
{
	OpenThreads::ScopedLock<OpenThreads::Mutex>	lock(_updateSunMoonMutex);

	turnAllFPLights(_fplights, false);

    OpenIG::Engine* openIG = dynamic_cast<OpenIG::Engine*>(_ig);
    if (openIG==0)
    {
        return;
    }

    static const unsigned int id = 0; // PPP: Reserved

    FPLightMap::const_iterator it = _fplights.find(id);
    Light* pFPLight = 0;
    if (it == _fplights.end())
    {
        pFPLight = _lightManager.CreateLight(OpenIG::Library::Graphics::LT_DIRECTIONAL);
		float fCustomFloats[3];
		fCustomFloats[0] = 99;fCustomFloats[1] = 0;fCustomFloats[2] = 0;
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

    osg::Vec3d vViewSpaceDir = computeViewSpaceDirection(osg::Vec4d(0,0,0,1), sunOrMoonPosition, viewMatrix);
    vViewSpaceDir.normalize();
    pFPLight->SetVec3_32_2(Vector3_32((float)-vViewSpaceDir.x(), -vViewSpaceDir.y(), -vViewSpaceDir.z()));

    pFPLight->SetOn(true);

	// PPP: What is this for????
	if (_fplights.size() == 1)
	{
		updateFPEngine();
	}
}

void UpdateLightAttribscallback::operator()(osg::Node*, osg::NodeVisitor* nv)
{
	EXTRA_LOGGING_STATIC_HANDLER

    osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
    if (!cv) return;

    static LightsToModelViewMatrices lightsToModelViewMatrices;
    static LightsToWorldMatrices     lightsToWorldMatrices;

    lightsToModelViewMatrices.clear();
    lightsToWorldMatrices.clear();

    const osg::Matrixd& viewMatrix = _ig->getViewer()->getView(0)->getCamera()->getViewMatrix();
    const osg::Vec3d& vEyePos = cv->getEyePoint();

    findOsgLightsAndMatrices(lightsToModelViewMatrices, lightsToWorldMatrices, cv->getCurrentRenderStage());

#if EXTRA_LOGGING
	if (log)
	{
		osg::notify(osg::NOTICE) << "UpdateLightAttribscallback::(): Number of Lights found: "<<lightsToWorldMatrices.size()<<std::endl;
	}
#endif
    
    ASSERT_PREDICATE(lightsToWorldMatrices.size()==lightsToModelViewMatrices.size());

#if EXTRA_LOGGING
	unsigned int numOfLightsEnabled = 0;
#endif
    for(LightsToModelViewMatrices::const_iterator lightIter = lightsToModelViewMatrices.begin(); lightIter != lightsToModelViewMatrices.end(); ++lightIter)
    {
		osg::DummyLight* pLight = lightIter->first;

        bool enabled = false;
        pLight->getUserValue("enabled", enabled);

        if (enabled==false)
        {
            unsigned int id = 0;
            pLight->getUserValue("id",id);
            //std::cout<<"Enabled/Disabled Light(F+): "<<id<<" "<<enabled<<std::endl;
            continue;
        }

        ASSERT_PREDICATE(lightsToWorldMatrices.find(pLight)!=lightsToWorldMatrices.end());
        const osg::Matrixd& worldMatrix = lightsToWorldMatrices[pLight];
        
        osg::Vec4d vWorldPos = computeWorldPosition(pLight, worldMatrix);
        osg::Vec3d vWorldDir = computeWorldDirection(pLight, worldMatrix);

        osg::Vec4d vViewSpacePos = computeViewSpacePosition(vWorldPos, viewMatrix);
        osg::Vec3d vViewSpaceDir = computeViewSpaceDirection(vWorldPos, vWorldDir, viewMatrix);

		osg::Vec3d vWorldPos3(osg::Vec3d(vWorldPos.x(), vWorldPos.y(), vWorldPos.z()));

		std::string cullingActive = OpenIG::Base::Configuration::instance()->getConfig("ForwardPlusLightsLODCulling","yes");
		if (cullingActive.compare(0, 3, "yes") == 0)
		{

			bool bIsLodCulled = isLodCulled(pLight, vWorldPos3, vEyePos);

			if (bIsLodCulled)
			{
				continue;
			}
		}

        FPLightMapIterator fpIter = _fplights.find(pLight->getLightNum()); 
        ASSERT_PREDICATE(fpIter!=_fplights.end());

        Light* pFPLight = fpIter->second;
		if (pFPLight->GetLightType() == OpenIG::Library::Graphics::LT_UNKNOWN) continue;

        pFPLight->SetOn(true);

#if EXTRA_LOGGING
		++numOfLightsEnabled;
#endif
        updateLightFromOsgLight(pFPLight, pLight);

		if (pFPLight->GetLightType()==OpenIG::Library::Graphics::LT_POINT || pFPLight->GetLightType()==OpenIG::Library::Graphics::LT_SPOT)
		{
			pFPLight->SetPosition(OsgToFPUtils::toVector3_64(vWorldPos));
		}
        if (pFPLight->GetLightType()==OpenIG::Library::Graphics::LT_SPOT || pFPLight->GetLightType()==OpenIG::Library::Graphics::LT_DIRECTIONAL)
        {
            pFPLight->SetDirection(OsgToFPUtils::toVector3_64(vWorldDir));
        }

        pFPLight->SetVec3_32_1(Vector3_32((float)vViewSpacePos.x(), vViewSpacePos.y(), vViewSpacePos.z()));
        pFPLight->SetVec3_32_2(Vector3_32((float)vViewSpaceDir.x(), vViewSpaceDir.y(), vViewSpaceDir.z()));
    }
#if EXTRA_LOGGING
	if (log)
	{
		osg::notify(osg::NOTICE) << "UpdateLightAttribscallback::(): Number of Lights enabled: "<<numOfLightsEnabled<<std::endl;
	}
#endif

	updateFPEngine();
#if 0
    if (_rampTexture.valid())
    {
        static bool once = true;
        static const int textureSlot = 8;
        if (once)
        {
            once = false;

            _scene->getOrCreateStateSet()->setTextureAttribute(textureSlot,_rampTexture);
            _scene->getOrCreateStateSet()->addUniform(new osg::Uniform("rampTexture", textureSlot));
        }
    }
#endif
}


ForwardPlusLightImplementationCallback::ForwardPlusLightImplementationCallback(OpenIG::Base::ImageGenerator* ig)
    : _ig(ig)
{
    _lightManager = new LightManager();
    _lightData = new LightData(341, FORMAT_R32G32B32A32_FLOAT);
    Vector2_uint32 tileSize(32, 32);
    _tileSpaceLightGrid = new TileSpaceLightGrid(tileSize);
    _tileSpaceLightGrid->SetScreenAreaCullSize(tileSize*2);
}

ForwardPlusLightImplementationCallback::~ForwardPlusLightImplementationCallback()
{
    SAFE_DELETE(_tileSpaceLightGrid);
    SAFE_DELETE(_lightData);
    SAFE_DELETE(_lightManager);
}

static void setInitialOSGLightParameters(osg::Light* light, const OpenIG::Base::LightAttributes& definition, const osg::Vec4d& pos, const osg::Vec3f& dir)
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

static void updateOSGLightParameters(osg::Light* light, const OpenIG::Base::LightAttributes& definition)
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

OpenIG::Library::Graphics::LightType toFPLightType(OpenIG::Base::LightType lightType)
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

struct UpdateSunMoonCallback : public osg::NodeCallback
{
	UpdateSunMoonCallback(UpdateLightAttribscallback* cb, OpenIG::Base::ImageGenerator* ig)
		: callback(cb), ig(ig)
	{
	}

	virtual void operator()(osg::Node*, osg::NodeVisitor*)
	{
		if (callback && ig)
		{
			const osg::Matrixd& viewMatrix = ig->getViewer()->getView(0)->getCamera()->getViewMatrix();

			callback->setUpSunOrMoonLight(viewMatrix);
		}
	}

	osg::ref_ptr<UpdateLightAttribscallback>	callback;
	OpenIG::Base::ImageGenerator*						ig;
};

osg::Referenced* ForwardPlusLightImplementationCallback::createLight(unsigned int id, const OpenIG::Base::LightAttributes& definition, osg::Group* lightsGroup)
{
	// special case, light ID==0, the sun/moon light
	if (id == 0)
	{
		if (lightsGroup->getUpdateCallback() == 0)
		{
			osg::ref_ptr<UpdateLightAttribscallback> cb = new UpdateLightAttribscallback(
				_ig, _ig->getScene()->asGroup(), *_lightManager, _fplights, *_lightData, *_tileSpaceLightGrid
			);
			lightsGroup->setUpdateCallback(new UpdateSunMoonCallback(cb, _ig));
		}
		return 0;
	}
    osg::ref_ptr<osg::LightSource> lightSource = new osg::LightSource;
    osg::DummyLight* osgLight = new osg::DummyLight(id);
    osgLight->setUserValue("id", id);
    osgLight->setUserValue("enabled", definition.enabled);

    lightSource->setLight(osgLight);
	lightSource->setDataVariance(definition.dataVariance);

    setInitialOSGLightParameters(lightSource->getLight(), definition, osg::Vec4d(0,0,0,1), osg::Vec3f(0,1,0));
    osgLight->setLightSource(lightSource);

	lightSource->setCullingActive(true);

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

    if (!_dummyGroup.valid() && _lightsGroup.valid())
    {
        _dummyGroup = new osg::Group;
        _lightsGroup->addChild(_dummyGroup);

		osg::ref_ptr<UpdateLightAttribscallback> updateAttributesCallback;

		UpdateSunMoonCallback* cb = dynamic_cast<UpdateSunMoonCallback*>(lightsGroup->getUpdateCallback());
		if (cb)
		{
			updateAttributesCallback = cb->callback;
		}
		else
		{
			updateAttributesCallback = new UpdateLightAttribscallback(
				_ig, _ig->getScene()->asGroup(), *_lightManager, _fplights, *_lightData, *_tileSpaceLightGrid
			);
		}
        _dummyGroup->setCullCallback(updateAttributesCallback);

		if (_lightManagerStateAttribute.valid()==false)
		{
			_lightManagerStateAttribute = new LightManagerStateAttribute();
			_lightManagerStateAttribute->set(_lightManager, &updateAttributesCallback->_fpCamera, &updateAttributesCallback->_fpViewport, _ig->getScene()->asGroup(), _ig);
			osg::StateSet* stateset =  _ig->getScene()->asGroup()->getOrCreateStateSet();
			//osg::StateSet* stateset =  _dummyGroup->getOrCreateStateSet();
			stateset->setAttribute(_lightManagerStateAttribute, osg::StateAttribute::ON);
		}
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