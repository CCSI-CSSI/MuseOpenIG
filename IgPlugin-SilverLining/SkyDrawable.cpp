// Copyright (c) 2008-2012 Sundog Software, LLC. All rights reserved worldwide.

#include "SkyDrawable.h"
#include <SilverLining.h>
#include "AtmosphereReference.h"
#include <sstream>
#include <IgCore/configuration.h>

#if(__APPLE__)
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#else
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif

#include <assert.h>

#include <osg/Texture2D>
#include <osg/ValueObject>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#define UPDATE_DISTANCE_SQ (500.0 * 500.0)
#define CLOUD_SHADOW_TEXTURE 6
#define M_PER_NMI           1852.000001 /* No. of meters in a nautical mile   */

using namespace SilverLining;
using namespace igplugins;

SkyDrawable::SkyDrawable()
        : osg::Drawable()
		, _view(0)
		, _skyboxSize(100000)
		, _cloudShadowTexgen(0)
        , _cloudShadowTextureWhiteSubstitute(0)
        , _cloudShadowTextureStage(CLOUD_SHADOW_TEXTURE)
        , _cloudShadowTexgenStage(CLOUD_SHADOW_TEXTURE)
		, _shadowTexHandle(0)
        , _cloudShadowsEnabled(false)
        , _init_shadows_once(false)
        , _needsShadowUpdate(true)
        , _cloudReflections(false)
        , _rainFactor(0.0)
        , _snowFactor(0.0)
        , _removeAllCloudLayers(false)
        , _todDirty(false)        
        , _enableCloudShadows(true)
		, _geocentric(false)
{
}

SkyDrawable::SkyDrawable(const std::string& path, osgViewer::View* view, osg::Light* light, osg::Fog* fog, bool geocentric)
        : osg::Drawable()
        , _view(view)
		, _skyboxSize(100000)
		, _light(light)
		, _fog(fog)
        , _path(path)
		, _cloudShadowTexgen(0)
        , _cloudShadowTextureWhiteSubstitute(0)
        , _cloudShadowTextureStage(CLOUD_SHADOW_TEXTURE)
        , _cloudShadowTexgenStage(CLOUD_SHADOW_TEXTURE)
		, _shadowTexHandle(0)
        , _todHour(12)
        , _todMinutes(0)
        , _cloudShadowsEnabled(false)
        , _init_shadows_once(false)
        , _needsShadowUpdate(true)
        , _cloudReflections(false)
        , _rainFactor(0.0)
        , _snowFactor(0.0)
        , _removeAllCloudLayers(false)
        , _todDirty(true)
        , _windSpeed(0.f)
        , _windDirection(0.f)
        , _windDirty(false)
        , _windVolumeHandle(0)
        , _enableCloudShadows(true)
		, _geocentric(geocentric)
{    
    _cloudShadowTextureStage =  _cloudShadowTexgenStage = igcore::Configuration::instance()->getConfig("Clouds-Shadows-Texture-Slot",6);

    const std::string value = igcore::Configuration::instance()->getConfig("Enable-Clouds-Shadows","yes");
    _enableCloudShadows =  value == "yes";

    initializeDrawable();
	initializeShadow();
}

void SkyDrawable::initializeShadow()
{
	{ // Create white fake texture for use if no cloud layer casting shadows is present
       osg::Image * image = new osg::Image;
       image->allocateImage( 1, 1, 1, GL_LUMINANCE, GL_UNSIGNED_BYTE );
       image->data()[0] = 0xFF;
        
       _cloudShadowTextureWhiteSubstitute = new osg::Texture2D( image );
       _cloudShadowTextureWhiteSubstitute->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::REPEAT);
       _cloudShadowTextureWhiteSubstitute->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::REPEAT);
       _cloudShadowTextureWhiteSubstitute->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::NEAREST);
       _cloudShadowTextureWhiteSubstitute->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::NEAREST);
    }

    { // Create cloud shadow casting texgen
       _cloudShadowTexgen = new osg::TexGen();
       _cloudShadowTexgen->setMode( osg::TexGen::EYE_LINEAR ); 
    }

    _cloudShadowCoordMatrixUniform = new osg::Uniform( "cloudShadowCoordMatrix", osg::Matrixf() );
    _view->getSceneData()->getOrCreateStateSet()->addUniform( _cloudShadowCoordMatrixUniform );
    _view->getSceneData()->getOrCreateStateSet()->addUniform( new osg::Uniform( "cloudShadowTexture", (int)_cloudShadowTextureStage ) );
}


void SkyDrawable::initializeDrawable()
{
    setDataVariance(osg::Object::DYNAMIC);
    setUseVertexBufferObjects(false);
    setUseDisplayList(false);

#if 1
    cullCallback = new SilverLiningCullCallback();
    setCullCallback(cullCallback);
#endif

    updateCallback = new SilverLiningUpdateCallback();
    updateCallback->camera = _view->getCamera();
    setUpdateCallback(updateCallback);

#if 1
    computeBoundingBoxCallback = new SilverLiningSkyComputeBoundingBoxCallback();
    computeBoundingBoxCallback->camera = _view->getCamera();
    setComputeBoundingBoxCallback(computeBoundingBoxCallback);
#endif
	
}

void SkyDrawable::setLighting(SilverLining::Atmosphere *atmosphere) const
{
    osg::Light *light = _light;
    osg::Vec4 ambient, diffuse;
    osg::Vec3 direction;

    if (atmosphere && light)
    {
        float ra, ga, ba, rd, gd, bd, x, y, z;
        atmosphere->GetAmbientColor(&ra, &ga, &ba);
        atmosphere->GetSunOrMoonColor(&rd, &gd, &bd);
		if (_geocentric)
		{
			atmosphere->GetSunPositionGeographic(&x, &y, &z);
			light->setConstantAttenuation(1.f / 100000000);
		}
		else
		{
			atmosphere->GetSunOrMoonPosition(&x, &y, &z);
		}

        osg::Vec3 ambientDiffuseFactor = osg::Vec3(0.f,0.f,0.f);
#if 1
		if ((_todHour >= 18.0 || _todHour < 5.0) && !_geocentric)
        {
            ambientDiffuseFactor = osg::Vec3(0.2f,0.2f,0.2f);
        }
#endif

        direction = osg::Vec3(x, y, z);
        ambient = osg::Vec4(ra+ambientDiffuseFactor.x(), ga+ambientDiffuseFactor.y(), ba+ambientDiffuseFactor.z(), 1.0);
        diffuse = osg::Vec4(rd+ambientDiffuseFactor.x(), gd+ambientDiffuseFactor.y(), bd+ambientDiffuseFactor.z(), 1.0);
        direction.normalize();

        light->setAmbient(ambient);
        light->setDiffuse(diffuse);
		light->setSpecular(osg::Vec4(1, 1, 1, 1));
        light->setPosition(osg::Vec4(direction.x(), direction.y(), direction.z(), 0));

		float density = 0.f;
		float	hR;
		float	hG;
		float	hB;

        if(_fog.valid())
        {
            if(atmosphere->GetFogEnabled())
            {//we are inside the stratus layer

                atmosphere->GetFogSettings(&density, &hR, &hG, &hB);
                _fog->setColor(osg::Vec4(hG,hG,hG,1.f));
                _fog->setDensity(density);
            }
            else
            {
                atmosphere->GetHorizonColor(0,&hR, &hG, &hB);
                density = _fog->getDensity();
                atmosphere->GetConditions()->SetFog(density,hG,hG,hG);
                _fog->setColor(osg::Vec4(hG,hG,hG,1.f));
                atmosphere->SetHaze(hG,hG,hG,1.5*M_PER_NMI, density);
            }
        }
    }
}

void SkyDrawable::initializeSilverLining(AtmosphereReference *ar) const
{
    if (ar && !ar->atmosphereInitialized)
    {
        ar->atmosphereInitialized = true; // only try once.
		SilverLining::Atmosphere *atmosphere = ar->atmosphere;

		if (atmosphere)
        {
			srand(1234); // constant random seed to ensure consistent clouds across windows

            // Update the path below to where you installed SilverLining's resources folder.
            const char *slPath = getenv("SILVERLINING_PATH");
            if (!slPath)
            {
                slPath = _path.c_str();
                if (!slPath)
                {
#if 0
                    printf("Can't find SilverLining; set the SILVERLINING_PATH environment variable ");
                    printf("to point to the directory containing the SDK.\n");
#else
                    osg::notify(osg::FATAL) << "SilverLining: Can't find SilverLining" << std::endl;
                    osg::notify(osg::FATAL) << "\t Either set the environmental variable SILVERLINING_PATH to point to " << std::endl;
                    osg::notify(osg::FATAL) << "\t the SilverLining installation or set the PATH in the:" << std::endl;
                    osg::notify(osg::FATAL) << "\t      Windows: igplugins\\IgPlugin-SilverLining.dll.xml" << std::endl;
                    osg::notify(osg::FATAL) << "\t      MacOS: /usr/local/lib/igplugins/libIgPlugin-SilverLining.dylib.xml" << std::endl;
                    osg::notify(osg::FATAL) << "\t      Linux: /usr/local/lib/igplugins/libIgPlugin-SilverLining.so.xml" << std::endl;
#endif
                    exit(0);
                }
            }

            std::string resPath(slPath);
#ifdef _WIN32
            resPath += "\\Resources\\";
#else
			resPath += "/Resources/";
#endif
            int ret = atmosphere->Initialize(SilverLining::Atmosphere::OPENGL, resPath.c_str(),
                                             true, 0);
            if (ret != SilverLining::Atmosphere::E_NOERROR)
            {
#if 0
                printf("SilverLining failed to initialize; error code %d.\n", ret);
                printf("Check that the path to the SilverLining installation directory is set properly ");
                printf("in SkyDrawable.cpp (in SkyDrawable::initializeSilverLining)\n");
#else
                osg::notify(osg::FATAL) << "SilverLining: SilverLining failed to initialize; error code " << ret << std::endl;
                osg::notify(osg::FATAL) << "SilverLining: Check the path in:" << std::endl;
                osg::notify(osg::FATAL) << "\t      Windows: igplugins\\IgPlugin-SilverLining.dll.xml" << std::endl;
                osg::notify(osg::FATAL) << "\t      MacOS: /usr/local/lib/igplugins/libIgPlugin-SilverLining.dylib.xml" << std::endl;
                osg::notify(osg::FATAL) << "\t      Linux: /usr/local/lib/igplugins/libIgPlugin-SilverLining.so.xml" << std::endl;
                osg::notify(osg::FATAL) << "\t or the environmental variable SILVERLINING_PATH" << std::endl;
#endif
                exit(0);
            }

			//atmosphere->SetConfigOption( "render-offscreen", "yes" );
			// This config option must set to get shadow clouds
            // if not set shadow clouds are black and hence overall shadow gets black too
            //atmosphere->SetConfigOption( "cumulus-lighting-quick-and-dirty", "no" );

            // Agreed to not hard code any of the config as long as they are
            // required by some very special effect
#if 0
			atmosphere->SetConfigOption("shadow-map-texture-size", "8192");
            atmosphere->SetConfigOption("enable-precipitation-visibility-effects", "no:");
#endif
            // Let SilverLining know which way is up. OSG usually has Z going up.
            atmosphere->SetUpVector(0, 0, 1);
            atmosphere->SetRightVector(1, 0, 0);

            // Set our location (change this to your own latitude and longitude)
            SilverLining::Location loc;
            loc.SetAltitude(0);
            loc.SetLatitude(12);
            loc.SetLongitude(42);
            atmosphere->GetConditions()->SetLocation(loc);

            // Set the time to noon in PST
            SilverLining::LocalTime t;
            t.SetFromSystemTime();
			t.SetHour(11);
			t.SetTimeZone(CET);
            atmosphere->GetConditions()->SetTime(t);

			atmosphere->EnableLensFlare(true);

            // Center the clouds around the camera's initial position
            osg::Vec3d pos;
			osg::Vec3d center;
			osg::Vec3d up;
			_view->getCamera()->getViewMatrixAsLookAt(pos,center,up);

            cullCallback->atmosphere = atmosphere;
        }
    }
}

void SkyDrawable::setShadow(SilverLining::Atmosphere *atmosphere, osg::RenderInfo & renderInfo )
{    
    if (!atmosphere || !renderInfo.getState()) return;

    osg::State & state = *renderInfo.getState(); 

    state.setActiveTextureUnit(_cloudShadowTextureStage);
    _cloudShadowTextureWhiteSubstitute->apply( state );

	// Tell state about our changes
    state.haveAppliedTextureAttribute(_cloudShadowTextureStage, _cloudShadowTextureWhiteSubstitute);

    osg::Matrix cloudShadowProjection;
    cloudShadowProjection.makeIdentity();
    	
    if (_enableCloudShadows && atmosphere->GetShadowMap(_shadowTexHandle, &_lightMVP, &_worldToShadowMapTexCoord, true, 0.1f))
	{

        GLuint shadowMap = (GLuint)(long)(_shadowTexHandle);

		cloudShadowProjection.set( _worldToShadowMapTexCoord.ToArray() );
        cloudShadowProjection = renderInfo.getCurrentCamera()->getInverseViewMatrix() * cloudShadowProjection;

        SkyDrawable* mutableThis = const_cast<SkyDrawable*>(this);

		if (!_texture.valid()) 
        {
            mutableThis->_texture = new osg::Texture2D;
        }


		osg::ref_ptr<osg::Texture::TextureObject> textureObject = new osg::Texture::TextureObject(_texture.get(),shadowMap,GL_TEXTURE_2D);
		textureObject->setAllocated();

		_texture->setTextureObject(renderInfo.getContextID(),textureObject.get());
		
		state.setActiveTextureUnit(_cloudShadowTextureStage);
		_texture->apply( state );

		// Tell state about our changes
		state.haveAppliedTextureAttribute(_cloudShadowTextureStage, _texture.get());
		

	}
	else
	{
		state.setActiveTextureUnit(_cloudShadowTextureStage);
		_cloudShadowTextureWhiteSubstitute->apply( state );

		// Tell state about our changes
		state.haveAppliedTextureAttribute(_cloudShadowTextureStage, _cloudShadowTextureWhiteSubstitute);

	}

    _cloudShadowCoordMatrixUniform->set( cloudShadowProjection );       
    _cloudShadowTexgen->setPlanesFromMatrix( cloudShadowProjection );

    // Now goes tricky part of the code !!!
    // We need to interact with OSG during render phase in delicate way and
    // not go out of sync with OpenGL states recorded by osg::State. 

    // Change texture stage to proper one
    state.setActiveTextureUnit(_cloudShadowTexgenStage);
    
    // We set texgen with identity on OpenGL modelview matrix            
    // since our TexGen was already premultiplied with inverse view.
    // This method minimizes precision errors as we used OSG double matrices.
    // If we had not premultiplied the texgen then we would need to set
    // view on OpenGL modelview matrix which would cause internal OpenGL
    // premutiplication of TexGen by inverse view using float matrices.
    // Such float computation may cause shadow texture jittering between 
    // frames if scenes/terrains spanning large coordinate ranges are used.
    state.applyModelViewMatrix( NULL );

    // Now apply our TexGen
    _cloudShadowTexgen->apply( state );

    state.applyTextureMode(_cloudShadowTexgenStage,GL_TEXTURE_GEN_S,osg::StateAttribute::ON);
    state.applyTextureMode(_cloudShadowTexgenStage,GL_TEXTURE_GEN_T,osg::StateAttribute::ON);
    state.applyTextureMode(_cloudShadowTexgenStage,GL_TEXTURE_GEN_R,osg::StateAttribute::ON);
    state.applyTextureMode(_cloudShadowTexgenStage,GL_TEXTURE_GEN_Q,osg::StateAttribute::ON);

    // Tell state about our changes
    state.haveAppliedTextureAttribute(_cloudShadowTexgenStage, _cloudShadowTexgen);

    // Set this TexGen as a global default 
    state.setGlobalDefaultTextureAttribute(_cloudShadowTexgenStage, _cloudShadowTexgen);
}

void SkyDrawable::setTimeOfDay(unsigned int hour, unsigned int minutes)
{
    _todHour = hour;
    _todMinutes = minutes;

    _todDirty = true;
}

void SkyDrawable::setWind(float speed, float direction)
{
    _windSpeed = speed;
    _windDirection = direction;
    _windDirty = true;
}

void SkyDrawable::setVisibility(double visibility)
{
    if (_fog.valid())
    {
        _fog->setDensity(1.0/visibility);
    }
}

void SkyDrawable::addCloudLayer(int id, int type, double altitude, double thickness, double density)
{
    CloudLayersIterator itr = _clouds.find(id);
    if (itr != _clouds.end()) return;

    CloudLayerInfo cli;
    cli._type = type;
    cli._altitude = altitude;
    cli._density = density;
    cli._thickness = thickness;
    cli._id = id;

    _cloudsQueueToAdd.push_back(cli);
}

void SkyDrawable::removeCloudLayer(int id)
{
    CloudLayersIterator itr = _clouds.find(id);
    if (itr == _clouds.end()) return;

    CloudLayerInfo cli;
    cli._id = id;
    cli._handle = itr->second._handle;

    _cloudsQueueToRemove.push_back(cli);

    _clouds.erase(itr);
}

void SkyDrawable::updateCloudLayer(int id, double altitude, double thickness, double density)
{
    CloudLayersIterator itr = _clouds.find(id);
    if (itr == _clouds.end()) return;

    CloudLayerInfo& cli = itr->second;
    if(cli._altitude != altitude)
    {
        cli._altitude = altitude;
        cli._dirty = true;
        cli._needReseed = false;
    }

    if (cli._density != density)
    {
        cli._density = density;
        cli._dirty = true;
        cli._needReseed = true;
    }

    if (cli._thickness != thickness)
    {
        cli._thickness = thickness;
        cli._dirty = true;
        cli._needReseed = true;
    }
}

void SkyDrawable::addClouds(SilverLining::Atmosphere *atmosphere, const osg::Vec3d& position)
{
    CloudLayersQueueIterator itr = _cloudsQueueToAdd.begin();
    for ( ; itr != _cloudsQueueToAdd.end(); ++itr)
    {
        SilverLining::CloudLayer *cloudLayer = SilverLining::CloudLayerFactory::Create((CloudTypes)itr->_type);
//        SilverLining::CloudLayer *cloudLayer = SilverLining::CloudLayerFactory::Create(itr->_type);
        cloudLayer->SetIsInfinite(true);
        cloudLayer->SetBaseAltitude(itr->_altitude);
        cloudLayer->SetDensity(itr->_density);

        // TDB: Read this from config or change the API
        // to support these parameters
        cloudLayer->SetThickness(itr->_thickness);
        cloudLayer->SetBaseLength(40000);
        cloudLayer->SetBaseWidth(40000);
        cloudLayer->SetCloudAnimationEffects(0,false);
        cloudLayer->SetLayerPosition(position.x(),-position.y());

        cloudLayer->SeedClouds(*atmosphere);
        cloudLayer->GenerateShadowMaps(true);
        int handle = atmosphere->GetConditions()->AddCloudLayer(cloudLayer);

        //osg::notify(osg::NOTICE) << "adding clouds: " << handle << std::endl;

        CloudLayerInfo cli = *itr;
        cli._handle = handle;

        _clouds[cli._id] = cli;
    }

    _cloudsQueueToAdd.clear();
}

void SkyDrawable::removeClouds(SilverLining::Atmosphere *atmosphere)
{
    CloudLayersQueueIterator itr = _cloudsQueueToRemove.begin();
    for ( ; itr != _cloudsQueueToRemove.end(); ++itr)
    {
        atmosphere->GetConditions()->RemoveCloudLayer(itr->_handle);

        //osg::notify(osg::NOTICE) << "removing clouds: " << itr->_handle << std::endl;
    }

    _cloudsQueueToRemove.clear();
}

void SkyDrawable::setGeocentric(bool geocentric)
{
	_geocentric = geocentric;
}

void SkyDrawable::removeAllCloudLayers()
{
    _removeAllCloudLayers = true;
    _clouds.clear();
}

void SkyDrawable::updateClouds(SilverLining::Atmosphere *atmosphere)
{
    CloudLayersIterator itr = _clouds.begin();
    for ( ; itr != _clouds.end(); ++itr)
    {
        CloudLayerInfo& cli= itr->second;
        if (!cli._dirty) continue;

        cli._dirty = false;

        CloudLayer* cloudLayer = 0;
        atmosphere->GetConditions()->GetCloudLayer(cli._handle, &cloudLayer);
        if (cloudLayer)
        {
            cloudLayer->SetBaseAltitude(cli._altitude);

            if (cli._needReseed)
            {
                cli._needReseed = false;
                cloudLayer->SetDensity(cli._density);
                cloudLayer->SeedClouds(*atmosphere);
            }
        }
    }
}

void SkyDrawable::setRain(double factor)
{
    _rainFactor = factor * 30.0;
    _snowFactor = 0.0;
}

void SkyDrawable::setSnow(double factor)
{
    _snowFactor = factor * 30;
    _rainFactor = 0.0;
}

void SkyDrawable::drawImplementation(osg::RenderInfo& renderInfo) const
{
    SilverLining::Atmosphere *atmosphere = 0;

	AtmosphereReference *ar = dynamic_cast<AtmosphereReference *>(renderInfo.getCurrentCamera()->getUserData());
	if (ar) atmosphere = ar->atmosphere;

	renderInfo.getState()->disableAllVertexArrays();

    if (atmosphere)
    {
        initializeSilverLining(ar);

        atmosphere->GetConditions()->SetPrecipitation(CloudLayer::RAIN, _rainFactor);
        atmosphere->GetConditions()->SetPrecipitation(CloudLayer::WET_SNOW, _snowFactor);

        atmosphere->SetCameraMatrix(renderInfo.getCurrentCamera()->getViewMatrix().ptr());
        atmosphere->SetProjectionMatrix(renderInfo.getCurrentCamera()->getProjectionMatrix().ptr());

        SilverLining::LocalTime t;
#if 0
        t.SetFromSystemTime();
#else
        t.SetYear(2015);
        t.SetMonth(8);
        t.SetDay(2);
#endif
        t.SetHour(_todHour);
        t.SetMinutes(_todMinutes);
        t.SetTimeZone(CET);
        atmosphere->GetConditions()->SetTime(t);

        osg::Vec3d position = renderInfo.getCurrentCamera()->getInverseViewMatrix().getTrans();

        if (_removeAllCloudLayers)
        {
            atmosphere->GetConditions()->RemoveAllCloudLayers();

            SkyDrawable* mutableThis = const_cast<SkyDrawable*>(this);
            mutableThis->_removeAllCloudLayers = false;
        }
        else
        {
            SkyDrawable* mutableThis = const_cast<SkyDrawable*>(this);
            mutableThis->removeClouds(atmosphere);
            mutableThis->addClouds(atmosphere,position);
            mutableThis->updateClouds(atmosphere);
        }

        if (_windDirty)
        {
            SkyDrawable* mutableThis = const_cast<SkyDrawable*>(this);

            mutableThis->_windDirty = false;

            SilverLining::WindVolume windVolume;
            windVolume.SetWindSpeed(_windSpeed);
            windVolume.SetDirection(_windDirection);

            atmosphere->GetConditions()->RemoveWindVolume(_windVolumeHandle);
            mutableThis->_windVolumeHandle = atmosphere->GetConditions()->SetWind(windVolume);
        }

		if (_geocentric)
		{
            osg::Vec3d ceye;
			osg::Vec3d ccenter;
			osg::Vec3d cup;
			renderInfo.getCurrentCamera()->getViewMatrixAsLookAt(ceye, ccenter, cup);

			osg::Vec3d up = ceye;
			up.normalize();
			osg::Vec3d north = osg::Vec3d(0, 0, 1);
			osg::Vec3d east = north ^ up;
			// Check for edge case of north or south pole
			if (east.length2() == 0) {
				east = osg::Vec3d(1, 0, 0);
			}
			east.normalize();

			atmosphere->SetUpVector(up.x(), up.y(), up.z());
			atmosphere->SetRightVector(east.x(), east.y(), east.z());

			double latitude = 0.0;
			double longitude = 0.0;
			double altitude = 0.0;

			osg::EllipsoidModel em;
			em.convertXYZToLatLongHeight(ceye.x(), ceye.y(), ceye.z(), latitude, longitude, altitude);

			SilverLining::Location loc;
			loc.SetAltitude(altitude);
			loc.SetLongitude(osg::RadiansToDegrees(longitude));
			loc.SetLatitude(osg::RadiansToDegrees(latitude));
			atmosphere->GetConditions()->SetLocation(loc);
			
			boost::posix_time::ptime t(
				boost::gregorian::date(2015, boost::gregorian::Aug, 2),
				boost::posix_time::hours(_todHour) + boost::posix_time::minutes(_todMinutes)
			);			
			boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));
			boost::posix_time::time_duration::sec_type x = (t - epoch).total_seconds();
			
			SilverLining::LocalTime utcTime;
			utcTime.SetFromEpochSeconds(time_t(x));
			utcTime.SetTimeZone(0);
			atmosphere->GetConditions()->SetTime(utcTime);
		}

		atmosphere->DrawSky(true, _geocentric , _skyboxSize, true, false);

        setLighting(atmosphere);
        const_cast<SkyDrawable*>(this)->setShadow(atmosphere,renderInfo);
    }


	renderInfo.getState()->dirtyAllVertexArrays();
}

void SilverLiningUpdateCallback::update(osg::NodeVisitor *nv, osg::Drawable* drawable)
{
    SilverLining::Atmosphere *atmosphere = 0;
    AtmosphereReference *ar = dynamic_cast<AtmosphereReference *>(camera->getUserData());
    if (ar) {
        if (!ar->atmosphereInitialized) return;

        atmosphere = ar->atmosphere;
    }

    if (atmosphere) {
        //atmosphere->UpdateSkyAndClouds();
    }

    // Since the skybox bounds are a function of the camera position, always update the bounds.
    drawable->dirtyBound();
}

osg::BoundingBox SilverLiningSkyComputeBoundingBoxCallback::computeBound(const osg::Drawable& drawable) const
{
    osg::BoundingBox box;

    if (camera)
    {
        const SkyDrawable& skyDrawable = dynamic_cast<const SkyDrawable&>(drawable);

        SilverLining::Atmosphere *atmosphere = 0;
        AtmosphereReference *ar = dynamic_cast<AtmosphereReference *>(camera->getUserData());
        if (ar) {
            atmosphere = ar->atmosphere;
        }

        if (atmosphere) 
        {
            double skyboxSize;

            if (skyDrawable.getSkyboxSize() != 0.0) {
                skyboxSize = skyDrawable.getSkyboxSize();
            } else {
                skyboxSize = atmosphere->GetConfigOptionDouble("sky-box-size");
                if (skyboxSize == 0.0) skyboxSize = 1000.0;
            }
          
            double radius = skyboxSize * 0.5;
            osg::Vec3f eye, center, up;
            camera->getViewMatrixAsLookAt(eye, center, up);
            osg::Vec3d camPos = eye;
            osg::Vec3d min(camPos.x() - radius, camPos.y() - radius, camPos.z() - radius);
            osg::Vec3d max(camPos.x() + radius, camPos.y() + radius, camPos.z() + radius);

            box.set(min, max);

            double dToOrigin = camPos.length();

		    bool hasLimb = atmosphere->GetConfigOptionBoolean("enable-atmosphere-from-space");
		    if (hasLimb)
		    {
			    // Compute bounds of atmospheric limb centered at 0,0,0
			    double earthRadius = atmosphere->GetConfigOptionDouble("earth-radius-meters");
			    double atmosphereHeight = earthRadius +
				    + atmosphere->GetConfigOptionDouble("atmosphere-height");
			    double atmosphereThickness = atmosphere->GetConfigOptionDouble("atmosphere-scale-height-meters")
				    + earthRadius;

                osg::BoundingBox atmosphereBox;
                osg::Vec3d atmMin(-atmosphereThickness, -atmosphereThickness, -atmosphereThickness);
                osg::Vec3d atmMax(atmosphereThickness, atmosphereThickness, atmosphereThickness);

                // Expand these bounds by it
                box.expandBy(atmosphereBox);
		    }
        }
    }

    return box;
}

