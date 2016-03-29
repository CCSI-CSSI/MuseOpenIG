// Triton OpenSceneGraph Sample Project
// Illustrates integration of Triton with an OpenSceneGraph application

// Copyright (c) 2011-2012 Sundog Software LLC. All rights reserved worldwide.

// The Triton.h header includes everything you need

#include "TritonDrawable.h"

#include <osg/GL2Extensions>
#include <osg/CoordinateSystemNode>

#include <Library-Graphics/OIGMath.h>
#include <Library-Graphics/Matrix4.h>
#include <Library-Graphics/MatrixUtils.h>

#include <Core-Base/filesystem.h>
#include <Core-Base/glerrorutils.h>
#include <Core-Base/shaderutils.h>
#include <Core-Base/configuration.h>

static const int tboStartOffset = 15;

using namespace OpenIG::Plugins;

// Create the Triton objects at startup, once we have a valid GL context in place
TritonDrawable::TritonDrawable(const std::string& resourcePath, const std::string& userName, const std::string& licence, bool geocentric, bool forwardPlusEnabled, osg::TextureCubeMap * _environmentMap)
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
	, _tod(12)
	, _forwardPlusEnabled(forwardPlusEnabled)
	, _forwardPlusSetUpTried(false)
	, _logz_tried(false)
{
	setDataVariance(osg::Object::DYNAMIC);
	setUseVertexBufferObjects(false);
	setUseDisplayList(false);
}

// Clean up our resources
TritonDrawable::~TritonDrawable()
{
	cleanup();
}

void TritonDrawable::setPlanarReflectionBlend(float value)
{
	_planarReflectionBlend = value;
}

static bool useLogZDepthBuffer(void)
{
	std::string strLogZDepthBuffer = OpenIG::Base::Configuration::instance()->getConfig("LogZDepthBuffer","yes");
	if (strLogZDepthBuffer.compare(0, 3, "yes") == 0)
		return true;
	else
		return false;
}

void TritonDrawable::initializeLogZDepthBuffer(osg::RenderInfo& renderInfo, std::vector<GLint>& userShaders)
{
	if (_logz_tried==true)
	{
		return;
	}
	_logz_tried = true;

	std::string logZPreamble = useLogZDepthBuffer()? "#define USE_LOG_DEPTH_BUFFER\n" : "";

	std::string resourcesPath = OpenIG::Base::FileSystem::path(OpenIG::Base::FileSystem::Resources, "../resources");

	osg::GLExtensions* ext = osg::GLExtensions::Get(renderInfo.getState()->getContextID(), true);

	std::string strSource = logZPreamble + OpenIG::Base::FileSystem::readFileIntoString(resourcesPath + "/shaders/logz_vs.glsl");

	GLint shaderID = osg::ShaderUtils::compileShader(strSource, osg::Shader::VERTEX, ext);
	if (shaderID == 0)
	{
		shaderID = osg::ShaderUtils::compileShader(strSource, osg::Shader::VERTEX, ext);
	}
	if (shaderID == 0)
	{
		osg::notify(osg::ALWAYS)<<"Triton: error: shader compilation error: /shaders/logz_vs.glsl"<<std::endl;
	}
	else
	{
		osg::notify(osg::ALWAYS) << "Triton: Log Z Vertex Shader compiled successfully..." << std::endl;
		userShaders.push_back(shaderID);
	}

	strSource = logZPreamble + OpenIG::Base::FileSystem::readFileIntoString(resourcesPath + "/shaders/logz_ps.glsl");

	shaderID = osg::ShaderUtils::compileShader(strSource, osg::Shader::FRAGMENT, ext);
	if (shaderID == 0)
	{
		shaderID = osg::ShaderUtils::compileShader(strSource, osg::Shader::FRAGMENT, ext);
	}
	if (shaderID == 0)
	{
		osg::notify(osg::ALWAYS)<<"Triton: error: shader compilation error: /shaders/logz_ps.glsl"<<std::endl;
	}
	else
	{
		osg::notify(osg::ALWAYS) << "Triton: Log Z Pixel Shader compiled successfully..." << std::endl;
		userShaders.push_back(shaderID);
	}
}

void TritonDrawable::initializeForwardPlus(osg::RenderInfo& renderInfo, std::vector<GLint>& userShaders)
{	
	if (_forwardPlusSetUpTried==true)
	{
		return;
	}
	_forwardPlusSetUpTried = true;

	std::string resourcesPath = OpenIG::Base::FileSystem::path(OpenIG::Base::FileSystem::Resources, "../resources");

	std::string strForward_Plus_Triton_PS;
	std::string strForwardPlusDefine;

	if (_forwardPlusEnabled)
	{
		strForwardPlusDefine = "#define TRITON_USE_FORWARD_PLUS_LIGHTING\n";
	}

	strForward_Plus_Triton_PS = strForwardPlusDefine + OpenIG::Base::FileSystem::readFileIntoString(resourcesPath + "/shaders/forward_plus_triton_ps.glsl");

	osg::GLExtensions* ext = osg::GLExtensions::Get(renderInfo.getState()->getContextID(), true);
	if (ext==0)
	{
		osg::notify(osg::ALWAYS) << "Triton: error: Extensions are Null" << std::endl;
		return;
	}

	{
		GLint shaderID = osg::ShaderUtils::compileShader(strForward_Plus_Triton_PS, osg::Shader::FRAGMENT, ext);
		if (shaderID==0)
		{
			osg::notify(osg::ALWAYS)<<"Triton: error: shader compilation error: "<<std::endl;
		}
		else
		{
			osg::notify(osg::ALWAYS) << "Triton:strForward_Plus_Triton_PS Shader compiled successfully..." << std::endl;
			userShaders.push_back(shaderID);
		}
	}

	if (_forwardPlusEnabled)
	{
		// A fair metric for water
		std::string strMAX_LIGHTS_PER_PIXEL = "#define MAX_LIGHTS_PER_PIXEL 200\n";

		std::string strSource = OpenIG::Base::FileSystem::readFileIntoString(resourcesPath + "/shaders/forwardplus_preamble.glsl") 
			+ strMAX_LIGHTS_PER_PIXEL
			+ OpenIG::Base::FileSystem::readFileIntoString(resourcesPath + "/shaders/lighting_math.glsl") 
			+ OpenIG::Base::FileSystem::readFileIntoString(resourcesPath + "/shaders/forwardplus_math.glsl");

		GLint shaderID = osg::ShaderUtils::compileShader(strSource, osg::Shader::FRAGMENT, ext);
		if (shaderID==0)
		{
			osg::notify(osg::ALWAYS)<<"Triton: error: shader compilation error: "<<std::endl;
		}
		else
		{
			osg::notify(osg::ALWAYS) << "Triton: F+ Pixel Shader compiled successfully..." << std::endl;
			userShaders.push_back(shaderID);
		}
	}
}

void TritonDrawable::setup(osg::RenderInfo& renderInfo)
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

	std::vector<GLint> stdVecUserShaders;
	initializeLogZDepthBuffer(renderInfo, stdVecUserShaders);
	initializeForwardPlus(renderInfo, stdVecUserShaders);	

	if (stdVecUserShaders.empty()==false)
	{
		TRITON_VECTOR(unsigned int) vectorUserShaders;
		for(int i = 0; i < stdVecUserShaders.size(); ++i)
		{
			vectorUserShaders.push_back(stdVecUserShaders[i]);
		}
		_ocean = Triton::Ocean::Create(_environment, vectorUserShaders, Triton::JONSWAP);
	}
	else
	{
		// Finally, create the _ocean object using the _environment we've created.
		// If NULL is returned, something's wrong - enable the enable-debug-messages option
		// in resources/triton.config to get more details on the problem.
		_ocean = Triton::Ocean::Create(_environment, Triton::JONSWAP);
	}

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
void TritonDrawable::cleanup()
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

void TritonDrawable::setUpEnvironment(void) const
{
	//Check to see if any xmldatafile has new _belowWaterVisibility value...CGR
	double visibility;
	Triton::Vector3 color;
	_environment->GetBelowWaterVisibility(visibility,color);
	if(visibility != _belowWaterVisibility)
	{
		_environment->SetBelowWaterVisibility(_belowWaterVisibility,color);
	}

	osg::Vec3	fogColor;
	float		fogDensity = -1.f;

	_openIG->getPluginContext().getOrCreateValueObject()->getUserValue("SilverLining-Atmosphere-FogColor", fogColor);
	_openIG->getPluginContext().getOrCreateValueObject()->getUserValue("SilverLining-Atmosphere-InClouds-FogDensity", fogDensity);

	Triton::Vector3 _fogColor(fogColor.x(), fogColor.y(), fogColor.z());

	visibility = 0.0;
	Triton::Vector3 fc;
	_environment->GetAboveWaterVisibility(visibility, fc);

	if (fogDensity >= 0.f)
	{
		//density = 3.912 / visibility
		_environment->SetAboveWaterVisibility(3.912f/fogDensity, _fogColor);
	}
	else
	{
		_environment->SetAboveWaterVisibility(visibility, _fogColor);
	}

	int	envMap = 0;
	if (_environmentalMapping && _openIG->getPluginContext().getOrCreateValueObject()->getUserValue("SilverLining-EnvironmentMap", envMap))
	{
		//std::cout << "env map is: " << envMap << std::endl;
		_environment->SetEnvironmentMap((Triton::TextureHandle)envMap, Triton::Matrix3::Identity);
	}
}

static void setUniform(const char* name, const osg::Vec4i& vec, Triton::Shaders _program,Triton::Ocean* ocean, osg::GLExtensions* ext)
{
	if (name==0||ocean==0||ext==0)
	{
		return;
	}
	GLint program = (GLint)ocean->GetShaderObject(_program);
	ext->glUseProgram(program);

	GLint loc = ext->glGetUniformLocation(program, name);
	if (loc != -1)
	{
		//std::cout << "Triton (WATER_SURFACE): Fcoef = " << Fcoef << std::endl;
		ext->glUniform4i(loc, (int)vec.x(), (int)vec.y(), (int)vec.z(), (int)vec.w());
	}
}

static void setUniform(const char* name, int val, Triton::Shaders _program,Triton::Ocean* ocean, osg::GLExtensions* ext)
{
	if (name==0||ocean==0||ext==0)
	{
		return;
	}
	GLint program = (GLint)ocean->GetShaderObject(_program);
	ext->glUseProgram(program);

	GLint loc = ext->glGetUniformLocation(program, name);
	if (loc != -1)
	{
		//std::cout << "Triton (WATER_SURFACE): Fcoef = " << Fcoef << std::endl;
		ext->glUniform1i(loc, val);
	}
}

static float LightBrightnessOnWater = 0.f;

static void setUniform(const char* name, float val, Triton::Shaders _program, Triton::Ocean* ocean, osg::GLExtensions* ext)
{
	if (name == 0 || ocean == 0 || ext == 0)
	{
		return;
	}
	GLint program = (GLint)ocean->GetShaderObject(_program);
	ext->glUseProgram(program);

	GLint loc = ext->glGetUniformLocation(program, name);
	if (loc != -1)
	{
		//std::cout << "Triton (WATER_SURFACE): Fcoef = " << Fcoef << std::endl;
		ext->glUniform1f(loc, val);
	}
}

static void setUniform(const char* name, const OpenIG::Library::Graphics::Matrix4_32& matrix, Triton::Shaders _program,Triton::Ocean* ocean, osg::GLExtensions* ext)
{
	if (name==0||ocean==0||ext==0)
	{
		return;
	}
	GLint program = (GLint)ocean->GetShaderObject(_program);
	ext->glUseProgram(program);

	GLint loc = ext->glGetUniformLocation(program, name);
	if (loc != -1)
	{
		//std::cout << "Triton (WATER_SURFACE): Fcoef = " << Fcoef << std::endl;
		ext->glUniformMatrix4fv(loc, 1, GL_TRUE, (float*)(matrix.ptr()));
	}
}


static OpenIG::Library::Graphics::Matrix4_64 OsgMatrixToGraphicsMatrix(const osg::Matrix& matrix)
{
	return OpenIG::Library::Graphics::Matrix4_64(
		   matrix(0,0), matrix(0,1), matrix(0,2), matrix(0,3)
		, matrix(1,0), matrix(1,1), matrix(1,2), matrix(1,3)
		, matrix(2,0), matrix(2,1), matrix(2,2), matrix(2,3)
		, matrix(3,0), matrix(3,1), matrix(3,2), matrix(3,3)
		);
}

void TritonDrawable::setUpForwardPlus(osg::GLExtensions* ext, osg::Camera* camera) const
{
	if (ext==0)
	{
		osg::notify(osg::ALWAYS) << "Triton: error: Extensions are Null" << std::endl;
		return;
	}
	if (camera==0)
	{
		osg::notify(osg::ALWAYS) << "Triton: error: Camera is Null" << std::endl;
		return;
	}
	
	osg::ValueObject* valueObject = _openIG->getPluginContext().getOrCreateValueObject();
	if (valueObject==0)
	{
		return;
	}

	osg::Vec4d vTilingParams;
	if (valueObject->getUserValue("FowardPlus-vTilingParams", vTilingParams))
	{
		osg::Vec4i vTilingParmsi((int)vTilingParams.x(),(int)vTilingParams.y(),(int)vTilingParams.z(),(int)vTilingParams.w());
		setUniform("vTilingParams", vTilingParmsi, Triton::WATER_SURFACE	  , _ocean, ext);
		setUniform("vTilingParams", vTilingParmsi, Triton::WATER_SURFACE_PATCH, _ocean, ext);
	}

	setUniform("lightDataTBO", tboStartOffset, Triton::WATER_SURFACE	  , _ocean, ext);
	setUniform("lightDataTBO", tboStartOffset, Triton::WATER_SURFACE_PATCH, _ocean, ext);

	setUniform("lightIndexListTBO", tboStartOffset-1, Triton::WATER_SURFACE	  , _ocean, ext);
	setUniform("lightIndexListTBO", tboStartOffset-1, Triton::WATER_SURFACE_PATCH, _ocean, ext);

	setUniform("lightGridOffsetAndSizeTBO", tboStartOffset-2, Triton::WATER_SURFACE	  , _ocean, ext);
	setUniform("lightGridOffsetAndSizeTBO", tboStartOffset-2, Triton::WATER_SURFACE_PATCH, _ocean, ext);

	osg::Matrixd osgViewMatrix = camera->getViewMatrix();

	OpenIG::Library::Graphics::Matrix4_64 view_matrix = OsgMatrixToGraphicsMatrix(osgViewMatrix).GetTranspose();
	view_matrix[0][3] = 0;
	view_matrix[1][3] = 0;
	view_matrix[2][3] = 0;

	OpenIG::Library::Graphics::Matrix4_64 view_inverse_transpose_matrix = view_matrix.GetInverse().GetTranspose();

	OpenIG::Library::Graphics::Matrix4_32 view_matrix_32                   = OpenIG::Library::Graphics::MatrixPrecisionConvert::ToFloat32(view_matrix);
	OpenIG::Library::Graphics::Matrix4_32 view_inverse_transpose_matrix_32 = OpenIG::Library::Graphics::MatrixPrecisionConvert::ToFloat32(view_inverse_transpose_matrix);

	setUniform("model_view_matrix", view_matrix_32, Triton::WATER_SURFACE	   , _ocean, ext);
	setUniform("model_view_matrix", view_matrix_32, Triton::WATER_SURFACE_PATCH, _ocean, ext);

	setUniform("model_view_inverse_transpose_matrix", view_inverse_transpose_matrix_32, Triton::WATER_SURFACE	   , _ocean, ext);
	setUniform("model_view_inverse_transpose_matrix", view_inverse_transpose_matrix_32, Triton::WATER_SURFACE_PATCH, _ocean, ext);

	setUniform("LightBrightnessOnWater", LightBrightnessOnWater, Triton::WATER_SURFACE, _ocean, ext);
	setUniform("LightBrightnessOnWater", LightBrightnessOnWater, Triton::WATER_SURFACE_PATCH, _ocean, ext);
}

void TritonDrawable::setUpSunOrMoonLight(void) const
{
	osg::Vec4 diffuse;
	bool ok = _openIG->getPluginContext().getOrCreateValueObject()->getUserValue("SilverLining-Light-Diffuse", diffuse);
	if (ok==false)
	{
		return;
	}

	osg::Vec4 position;
	osg::Vec3 sunOrMoonColor;
	osg::Vec3 sunOrMoonPosition;
	osg::Vec3 ambient;
	osg::Vec3 horizonColor;

	osg::ValueObject* valueObject = _openIG->getPluginContext().getOrCreateValueObject();
	valueObject->getUserValue("SilverLining-Light-Position", position);
	valueObject->getUserValue("SilverLining-Atmosphere-SunOrMoonColor", sunOrMoonColor);
	valueObject->getUserValue("SilverLining-Atmosphere-SunOrMoonPosition", sunOrMoonPosition);
	valueObject->getUserValue("SilverLining-Atmosphere-Ambient", ambient);
	valueObject->getUserValue("SilverLining-Atmosphere-HorizonColor", horizonColor);

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


void TritonDrawable::setUpTransparency(osg::GLExtensions* ext, osg::Camera* camera) const
{
	if (ext==0||camera==0)
	{
		return;
	}

	if (_geocentric)
	{
		GLint shader = (GLint)_ocean->GetShaderObject(::Triton::WATER_SURFACE);
		ext->glUseProgram(shader);

		//std::cout << "Triton: shader = " << shader << std::endl;

		GLint transparency = ext->glGetUniformLocation(shader, "u_transparency");
		if (transparency != -1)
		{
			float factor = 1.0f;

			osg::Vec3d eye;
			osg::Vec3d center;
			osg::Vec3d up;
			camera->getViewMatrixAsLookAt(eye, center, up);

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

		shader = (GLint)_ocean->GetShaderObject(::Triton::WATER_SURFACE_PATCH);
		ext->glUseProgram(shader);

		//std::cout << "Triton: shader = " << shader << std::endl;

		transparency = ext->glGetUniformLocation(shader, "u_transparency");
		if (transparency != -1)
		{

			//std::cout << "Triton: transparency = " << transparency << std::endl;
			ext->glUniform1f(transparency, 1.f);
		}
	}
}

void TritonDrawable::setUpLogZBuffer(osg::GLExtensions* ext, osg::Camera* camera) const
{
	if (ext==0||camera==0)
	{
		return;
	}

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

	const osg::Matrix& proj = camera->getProjectionMatrix();

	proj.getPerspective(fovy, ar, zNear, zFar);
#endif

	float Fcoef = (float)(2.0f / OpenIG::Library::Graphics::Math::Log2(zFar + 1.0f));

	GLint shader = (GLint)_ocean->GetShaderObject(::Triton::WATER_SURFACE_PATCH);
	ext->glUseProgram(shader);

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

void TritonDrawable::setUpHeightMap(unsigned int contextID) const
{
	if ( _heightMap && _heightMapcamera )
	{
		//std::cout << "IgPlugin-Triton::TritonDrawable::drawImplementation processed _heightMap 0" << std::endl;
		osg::Texture::TextureObject* textureObject = _heightMap->getTextureObject(contextID);
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
}

void TritonDrawable::setUpPlanarReflections(const osg::State& state) const
{
	if (_planarReflectionMap.valid()==false)
	{
		return;
	}

	osg::Matrix & p = *_planarReflectionProjection;

	Triton::Matrix3 planarProjection(p(0, 0), p(0, 1), p(0, 2),
		p(1, 0), p(1, 1), p(1, 2),
		p(2, 0), p(2, 1), p(2, 2));

	_environment->SetPlanarReflectionMap((Triton::TextureHandle)
		_planarReflectionMap->getTextureObject(state.getContextID())->id(),
		planarProjection, 0.125);

	_ocean->SetPlanarReflectionBlend(_planarReflectionBlend);
}

// Draw Triton _ocean
void TritonDrawable::drawImplementation(osg::RenderInfo& renderInfo) const
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	osg::State & state = *renderInfo.getState();

	state.disableAllVertexArrays();

	static bool setupAttempted = false;
	if (!_ocean && !setupAttempted) {
		const_cast< TritonDrawable *>( this )->setup(renderInfo);

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

		setUpHeightMap(renderInfo.getContextID());

		// Draw the _ocean for the current time sample
		if (_ocean)
		{
			osg::GL2Extensions* ext = osg::GL2Extensions::Get(state.getContextID(), true);
			setUpTransparency(ext, renderInfo.getCurrentCamera());
			setUpLogZBuffer(ext, renderInfo.getCurrentCamera());

			if (_openIG)
			{
				if (_lightBrightness_enable)
				{
					if (_tod > 4 && _tod < 19)
						LightBrightnessOnWater = _lightBrightness_day;
					else
						LightBrightnessOnWater = _lightBrightness_night;
				}
				else
				{
					LightBrightnessOnWater = 0.f;
				}

				if (_forwardPlusEnabled)
				{
					setUpForwardPlus(ext, renderInfo.getCurrentCamera());
				}
				setUpEnvironment();
				setUpSunOrMoonLight();
			}

			setUpPlanarReflections(state);

			glPushAttrib(GL_DEPTH_BUFFER_BIT);
			glDisable(GL_DEPTH_CLAMP);

			//Change the TU from non-zero to zero, or zero to one to get around caching
			int tu = state.getActiveTextureUnit();
			state.setActiveTextureUnit(tu ? 0 : 1);
			
			_ocean->Draw( renderInfo.getView()->getFrameStamp()->getSimulationTime() );

			//Change the TU from non-zero to zero, or zero to one to get around caching
			state.setActiveTextureUnit(tu);

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


