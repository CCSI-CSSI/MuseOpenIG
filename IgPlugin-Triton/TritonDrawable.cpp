// Triton OpenSceneGraph Sample Project
// Illustrates integration of Triton with an OpenSceneGraph application

// Copyright (c) 2011-2012 Sundog Software LLC. All rights reserved worldwide.

// The Triton.h header includes everything you need

#include "TritonDrawable.h"

#include <osg/GL2Extensions>
#include <osg/CoordinateSystemNode>

// Create the Triton objects at startup, once we have a valid GL context in place
TritonDrawable::TritonDrawable(const std::string& resourcePath, const std::string& userName, const std::string& licence, bool geocentric, osg::TextureCubeMap * _environmentMap)
	: _resourcePath(resourcePath)
	, _userName(userName)
	, _license(licence)
    , _geocentric(geocentric)
	, _resourceLoader(0)
    , _environment(0)
    , _ocean(0)
    , _cubeMap(_environmentMap)
    , _belowWaterVisibility(3.5)
	, _openIG(0)
	, _environmentalMapping(false)
	, _planarReflectionBlend(2.f)
{
    setDataVariance(osg::Object::DYNAMIC);
    setUseVertexBufferObjects(false);
    setUseDisplayList(false);
}

// Clean up our resources
TritonDrawable::~TritonDrawable()
{
    Cleanup();
}

void TritonDrawable::setPlanarReflectionBlend(float value)
{
	_planarReflectionBlend = value;
}

void TritonDrawable::Setup( )
{
    Triton::CoordinateSystem _coord;

    //Are we running in geocentric for osgEarth or a Flat database
    _geocentric ? _coord=Triton::WGS84_ZUP : _coord = Triton::FLAT_ZUP;

    // We use the default resource loader that just loads files from disk. You'll need
    // to redistribute the resources folder if using this. You can also extend the
    // _resourceLoader class to hook into your own resource manager if you wish.
    // Update the path below to where you installed SilverLining's resources folder.
    const char *tritonPath = getenv("TRITON_PATH");
    if (!tritonPath) {
		tritonPath = _resourcePath.c_str();

		if (std::string(tritonPath).empty())
		{
            osg::notify(osg::ALWAYS) << "Triton: Can't find Triton" << std::endl;
            osg::notify(osg::ALWAYS) << "\t Either set the environmental variable TRITON_PATH to point to " << std::endl;
            osg::notify(osg::ALWAYS) << "\t the Triton installation or set the PATH in the:" << std::endl;
            osg::notify(osg::ALWAYS) << "\t      Windows: igplugins\\IgPlugin-Triton.dll.xml" << std::endl;
            osg::notify(osg::ALWAYS) << "\t      MacOS: /usr/local/lib/igplugins/libIgPlugin-Triton.dylib.xml" << std::endl;
            osg::notify(osg::ALWAYS) << "\t      Linux: /usr/local/lib/igplugins/libIgPlugin-Triton.so.xml" << std::endl;
			
			exit(0);
		}
    }

    std::string resPath(tritonPath);
#ifdef _WIN32
    resPath += "\\Resources\\";
#else
    resPath += "/Resources/";
#endif
    _resourceLoader = new Triton::ResourceLoader(resPath.c_str());	

    // Create an _environment for the water, with a flat-Earth coordinate system with Y
    // pointing up and using an OpenGL 2.0 capable context.
    _environment = new Triton::Environment();
    Triton::EnvironmentError err = _environment->Initialize(_coord, Triton::OPENGL_2_0, _resourceLoader);
    if (err != Triton::SUCCEEDED) {
        osg::notify(osg::ALWAYS) << "Triton: setting resource path to: " << resPath << std::endl;
        osg::notify( osg::ALWAYS ) << "Failed to initialize Triton - is the resource path passed in to the ResourceLoader constructor valid?" << std::endl;
    }

    // Substitute your own license name and code, otherwise the app will terminate after
    // 5 minutes. Visit www.sundog-soft.com to purchase a license if you're so inclined.
    _environment->SetLicenseCode(_userName.c_str(), _license.c_str());

    // Set up wind of 10 m/s blowing North
//    Triton::WindFetch wf;
//    wf.SetWind(10.0, 0.0);
//    _environment->AddWindFetch(wf);
    _environment->SetSeaLevel(0.0);

    Triton::Vector3 visibility(0.7f,0.7f,0.9f);
    //Triton::Vector3 visibility(1.f, 1.f, 1.f);
    _environment->SetBelowWaterVisibility(_belowWaterVisibility, visibility);

    // Set visibility
    //_environment->SetAboveWaterVisibility( 1000, Triton::Vector3( 0.3f, 0.3f, 0.7f ) );

    // Finally, create the _ocean object using the _environment we've created.
    // If NULL is returned, something's wrong - enable the enable-debug-messages option
    // in resources/triton.config to get more details on the problem.
    _ocean = Triton::Ocean::Create(_environment, Triton::JONSWAP);

    // Set up an update callback so we can update the FFT model form the update thread
    if (_ocean) {
        //_ocean->SetDepthOffset(0.01);

        setUpdateCallback(new TritonUpdateCallback(_ocean));
        // Set up the "surge depth" - this is the depth at which wave displacements
        // will start getting flattened out as they approach the shore...
//        Triton::BreakingWavesParameters params;
//        params.SetAmplitude(0);
//        params.SetSurgeDepth(0.15);
//        _environment->SetBreakingWavesParameters(params);
    }
}

// Clean up our resources
void TritonDrawable::Cleanup()
{
    if (_ocean) {
        delete _ocean;
        _ocean = NULL;
    }

    if (_environment) {
        delete _environment;
        _environment = NULL;
    }

    if (_resourceLoader) {
        delete _resourceLoader;
        _resourceLoader = NULL;
    }
}

void TritonDrawable::setEnvironmentalMap(int id)
{
	_environment->SetEnvironmentMap((Triton::TextureHandle)id, Triton::Matrix3::Identity);
	std::cout << "T env map: " << id << std::endl;
}

void TritonDrawable::setPlanarReflection(osg::Texture2D* texture, osg::RefMatrix* mx)
{
	_planarReflectionMap = texture;
	_planarReflectionProjection = mx;
}

// Draw Triton _ocean
void TritonDrawable::drawImplementation(osg::RenderInfo& renderInfo) const
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    double visibility;
    Triton::Vector3 color;
    osg::State & state = *renderInfo.getState();

    state.disableAllVertexArrays();

	static bool setupAttempted = false;
	if (!_ocean && !setupAttempted) {
        const_cast< TritonDrawable *>( this )->Setup();

		setupAttempted = true;
    }

    // Pass the final view and projection matrices into Triton.
    if (_environment) {
        _environment->SetCameraMatrix( state.getModelViewMatrix().ptr() );
        _environment->SetProjectionMatrix( renderInfo.getCurrentCamera()->getProjectionMatrix().ptr() /* state.getProjectionMatrix().ptr()*/ );
    }

    state.dirtyAllVertexArrays();

    // Now light and draw the ocean:
    if (_environment) {
        if ( _heightMap && _heightMapcamera )
        {
            //std::cout << "IgPlugin-Triton::TritonDrawable::drawImplementation processed _heightMap 0" << std::endl;
            osg::Texture::TextureObject* textureObject = _heightMap->getTextureObject(renderInfo.getContextID());
            if (textureObject)
            {
                 const osg::Matrixd bias(0.5, 0.0, 0.0, 0.0,
                                         0.0, 0.5, 0.0, 0.0,
                                         0.0, 0.0, 0.5, 0.0,
                                         0.5, 0.5, 0.5, 1.0);

                osg::Matrixd HeightMapMatrix = _heightMapcamera->getViewMatrix() * _heightMapcamera->getProjectionMatrix() * bias;

                const Triton::Matrix4 worldToTextureCoords(HeightMapMatrix.ptr());
                _environment->SetHeightMap((Triton::TextureHandle)textureObject->id(),worldToTextureCoords);
            }
        }

        // Draw the _ocean for the current time sample
        if (_ocean)
		{
#if 0
			double left = 0.0;
			double right = 0.0;
			double bottom = 0.0;
			double top = 0.0;
			double zNear = 0.0;
			double zFar = 0.0;
			renderInfo.getCurrentCamera()->getProjectionMatrixAsFrustum(left, right, bottom, top, zNear, zFar);
#else
			
			float fovy = 0.0;
			float ar = 0.0;
			float zNear = 0.0;
			float zFar = 0.0;
			
			const osg::Matrix& proj = renderInfo.getCurrentCamera()->getProjectionMatrix();

			proj.getPerspective(fovy, ar, zNear, zFar);
#endif

			float Fcoef = (float)(2.0f / log2(zFar + 1.0f));

            osg::GL2Extensions* ext = NULL;            
            {
                GLint shader = (GLint)_ocean->GetShaderObject(::Triton::WATER_SURFACE);
                ext = osg::GL2Extensions::Get(state.getContextID(), true);


                if (ext && _geocentric)
                {
                    ext->glUseProgram(shader);

                    //std::cout << "Triton: shader = " << shader << std::endl;

                    GLint transparency = ext->glGetUniformLocation(shader, "u_transparency");
                    if (transparency != -1)
                    {
                        float factor = 1.0f;

                        osg::Vec3d eye;
                        osg::Vec3d center;
                        osg::Vec3d up;
                        renderInfo.getCurrentCamera()->getViewMatrixAsLookAt(eye, center, up);

                        double alt = eye.length();
                        double upperLimit = osg::WGS_84_RADIUS_EQUATOR * 1.001;
                        double lowerLimit = osg::WGS_84_RADIUS_EQUATOR * 1.0001;

                        if (alt > upperLimit)
                            factor = 0.0f;
                        else
                            if (alt < lowerLimit)
                                factor = 1.f;
                            else
                                factor = 1.f - (alt - lowerLimit)/ (upperLimit-lowerLimit);

                        //std::cout << "Triton: transparency = " << transparency << std::endl;
                        ext->glUniform1f(transparency, _geocentric ? factor : 1.f);
                    }


                }
                if(ext)
                {
                    ext->glUseProgram(shader);

                    GLint loc = ext->glGetUniformLocation(shader, "lightsEnabled");
                    if (_openIG && loc != -1)
                    {
                        std::vector<GLint> lightsEnabled;
                        for (size_t i = 0; i < 8; ++i)
                        {
                            lightsEnabled.push_back(_openIG->isLightEnabled(i) ? 1 : 0);
                            //std::cout << "WATER_SURFACE Light i: " << i << ", enable: " << _openIG->isLightEnabled(i) << std::endl;
                        }
                        ext->glUniform1iv(loc, lightsEnabled.size(), &lightsEnabled.front());
                    }

                    GLint cbloc = ext->glGetUniformLocation(shader, "lightsBrightness");
                    if (_openIG && cbloc != -1)
                    {
                        std::vector<GLfloat> lightsBrightness;
                        for (size_t i = 0; i<8; ++i)
                        {
                            igcore::LightAttributes attr = _openIG->getLightAttributes(i);
                            lightsBrightness.push_back(attr._waterBrightness);

                            //std::cout << "WATER_SURFACE Light brighness: " << attr._waterBrightness << std::endl;
                        }
                        ext->glUniform1fv(cbloc, lightsBrightness.size(), &lightsBrightness.front());
                    }
//                    else
//                    {
//                        //std::cout << "lightsBrightness not found" << std::endl;
//                    }
                }

                shader = (GLint)_ocean->GetShaderObject(::Triton::WATER_SURFACE_PATCH);
                if (ext && _geocentric)
                {
                    ext->glUseProgram(shader);

                    //std::cout << "Triton: shader = " << shader << std::endl;

                    GLint transparency = ext->glGetUniformLocation(shader, "u_transparency");
                    if (transparency != -1)
                    {

                        //std::cout << "Triton: transparency = " << transparency << std::endl;
                        ext->glUniform1f(transparency, 1.f);
                    }
                }
				
				if (ext)
				{					
					ext->glUseProgram(shader);

					GLint loc = ext->glGetUniformLocation(shader, "lightsEnabled");
					if (_openIG && loc != -1)
					{
                        std::vector<GLint> lightsEnabled;
						for (size_t i = 0; i < 8; ++i)
						{
							lightsEnabled.push_back(_openIG->isLightEnabled(i) ? 1 : 0);
                            //std::cout << "WATER_SURFACE_PATCH Light i: " << i << ", enable: " << _openIG->isLightEnabled(i) << std::endl;
                        }
						ext->glUniform1iv(loc, lightsEnabled.size(), &lightsEnabled.front());
					}

					GLint cbloc = ext->glGetUniformLocation(shader, "lightsBrightness");
					if (_openIG && cbloc != -1)
					{
						std::vector<GLfloat> lightsBrightness;
						for (size_t i = 0; i<8; ++i)
						{
							igcore::LightAttributes attr = _openIG->getLightAttributes(i);
							lightsBrightness.push_back(attr._waterBrightness);							

                            //std::cout << "WATER_SURFACE_PATCH Light brighness: " << attr._waterBrightness << std::endl;
						}
						ext->glUniform1fv(cbloc, lightsBrightness.size(), &lightsBrightness.front());
					}
//					else
//					{
//						//std::cout << "lightsBrightness not found" << std::endl;
//                    }

                    GLint fcoef = ext->glGetUniformLocation(shader, "Fcoef");
					if (fcoef != -1)
					{
						//std::cout << "Triton (WATER_SURFACE_PATCH): Fcoef = " << Fcoef << std::endl;
						ext->glUniform1f(fcoef, Fcoef);
                    }

                    shader = (GLint)_ocean->GetShaderObject(::Triton::WATER_SURFACE);

                    ext->glUseProgram(shader);

                    fcoef = ext->glGetUniformLocation(shader, "Fcoef");
                    if (fcoef != -1)
                    {
                        //std::cout << "Triton (WATER_SURFACE): Fcoef = " << Fcoef << std::endl;
                        ext->glUniform1f(fcoef, Fcoef);
                    }

                    shader = (GLint)_ocean->GetShaderObject(::Triton::WAKE_SPRAY_PARTICLES);

                    ext->glUseProgram(shader);

                    fcoef = ext->glGetUniformLocation(shader, "Fcoef");
                    if (fcoef != -1)
                    {
                        //std::cout << "Triton (WAKE_SPRAY_PARTICLES): Fcoef = " << Fcoef << std::endl;
                        ext->glUniform1f(fcoef, Fcoef);
                    }

                    shader = (GLint)_ocean->GetShaderObject(::Triton::SPRAY_PARTICLES);

                    ext->glUseProgram(shader);

                    fcoef = ext->glGetUniformLocation(shader, "Fcoef");
                    if (fcoef != -1)
                    {
                        //std::cout << "Triton (SPRAY_PARTICLES): Fcoef = " << Fcoef << std::endl;
                        ext->glUniform1f(fcoef, Fcoef);
                    }

                }
            }

            //Check to see if any xmldatafile has new _belowWaterVisibility value...CGR
            _environment->GetBelowWaterVisibility(visibility,color);
            if(visibility != _belowWaterVisibility)
            {
                _environment->SetBelowWaterVisibility(_belowWaterVisibility,color);
            }

			if (_openIG)
			{
				osg::Vec3	fogColor;
				float		fogDensity = -1.f;

				_openIG->getPluginContext().getOrCreateValueObject()->getUserValue("SilverLining-Atmosphere-FogColor", fogColor);
				_openIG->getPluginContext().getOrCreateValueObject()->getUserValue("SilverLining-Atmosphere-InClouds-FogDensity", fogDensity);

				Triton::Vector3 _fogColor(fogColor.x(), fogColor.y(), fogColor.z());

				double			visibility = 0.0;
				Triton::Vector3 fc;
				_environment->GetAboveWaterVisibility(visibility, fc);

				if (fogDensity >= 0.f)
				{
					//density = 3.912 / visibility
					_environment->SetAboveWaterVisibility(3.912f/fogDensity, _fogColor);
				}
				else
					_environment->SetAboveWaterVisibility(visibility, _fogColor);

				osg::Vec4 diffuse;
				osg::Vec4 position;
				osg::Vec3 sunOrMoonColor;
				osg::Vec3 sunOrMoonPosition;
				osg::Vec3 ambient;
				osg::Vec3 horizonColor;

				int	envMap = 0;
				if (_environmentalMapping && _openIG->getPluginContext().getOrCreateValueObject()->getUserValue("SilverLining-EnvironmentMap", envMap))
				{
					//std::cout << "env map is: " << envMap << std::endl;
#if 1
					_environment->SetEnvironmentMap((Triton::TextureHandle)envMap, Triton::Matrix3::Identity);
#endif
				}
				if (_openIG->getPluginContext().getOrCreateValueObject()->getUserValue("SilverLining-Light-Diffuse", diffuse))
				{
					_openIG->getPluginContext().getOrCreateValueObject()->getUserValue("SilverLining-Light-Position", position);
					_openIG->getPluginContext().getOrCreateValueObject()->getUserValue("SilverLining-Atmosphere-SunOrMoonColor", sunOrMoonColor);
					_openIG->getPluginContext().getOrCreateValueObject()->getUserValue("SilverLining-Atmosphere-SunOrMoonPosition", sunOrMoonPosition);
					_openIG->getPluginContext().getOrCreateValueObject()->getUserValue("SilverLining-Atmosphere-Ambient", ambient);
					_openIG->getPluginContext().getOrCreateValueObject()->getUserValue("SilverLining-Atmosphere-HorizonColor", horizonColor);

					_environment->SetDirectionalLight(
						Triton::Vector3(position[0], position[1], position[2]),
						Triton::Vector3(1, 1, 1)
						);					

					osg::Matrix lightLocalToWorldMatrix = osg::Matrix::identity();
					sunOrMoonPosition = sunOrMoonPosition * lightLocalToWorldMatrix;

					Triton::Vector3 lightPosition(sunOrMoonPosition.x(), sunOrMoonPosition.y(), sunOrMoonPosition.z());
					Triton::Vector3 lightColor(sunOrMoonColor.x(), sunOrMoonColor.y(), sunOrMoonColor.z());

					_environment->SetDirectionalLight(lightPosition, lightColor);

					//std::cout << "T position: " << sunOrMoonPosition.x() << "," << sunOrMoonPosition.y() << "," << sunOrMoonPosition.z() << std::endl;

					_environment->SetAmbientLight(Triton::Vector3(ambient.x(), ambient.y(), ambient.z()));

					//std::cout << "Data from SilverLining passed on to Triton" << std::endl;

					
				}
			}

			if (_planarReflectionMap.valid())
			{
				osg::Matrix & p = *_planarReflectionProjection;

				Triton::Matrix3 planarProjection(p(0, 0), p(0, 1), p(0, 2),
					p(1, 0), p(1, 1), p(1, 2),
					p(2, 0), p(2, 1), p(2, 2));

				_environment->SetPlanarReflectionMap((Triton::TextureHandle)
					_planarReflectionMap->getTextureObject(state.getContextID())->id(),
					planarProjection, 0.125);

				_ocean->SetPlanarReflectionBlend(_planarReflectionBlend);
			}

            glPushAttrib(GL_DEPTH_BUFFER_BIT);
            glDisable(GL_DEPTH_CLAMP);
            _ocean->Draw( renderInfo.getView()->getFrameStamp()->getSimulationTime() );
            glPopAttrib();

            if (ext)
            {
                ext->glUseProgram(0);
            }
        }
    }

    state.dirtyAllVertexArrays();

    glPopAttrib();
}


