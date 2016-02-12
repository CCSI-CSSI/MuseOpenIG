// Copyright (c) 2008-2012 Sundog Software, LLC. All rights reserved worldwide

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

#include "CloudsDrawable.h"
#include "AtmosphereReference.h"

#include <Core-Base/attributes.h>
#include <Core-Base/filesystem.h>
#include <Core-Base/glerrorutils.h>
#include <Core-Base/shaderutils.h>
#include <Core-Base/configuration.h>

#include <SilverLining.h>

#include <Library-Graphics/OIGAssert.h>
#include <Library-Graphics/MatrixUtils.h>

#include <osg/Material>
#include <osg/GL2Extensions>
#include <osg/ValueObject>

using namespace SilverLining;
using namespace OpenIG::Plugins;

CloudsDrawable::CloudsDrawable()
        : osg::Drawable()
        , _envMapDirty(false)
        , _pluginContext(0)
		, _forwardPlusEnabled(false)
{
    initialize();
}

CloudsDrawable::CloudsDrawable(osgViewer::View* view, OpenIG::Base::ImageGenerator* ig, bool forwardPlusEnabled)
        : osg::Drawable()
        , _view(view)
        , _ig(ig)
        , _envMapDirty(false)
        , _pluginContext(0)
		, _forwardPlusEnabled(forwardPlusEnabled)
{
    initialize();
}

// Set up our attributes and callbacks.
void CloudsDrawable::initialize()
{
	setDataVariance(osg::Object::DYNAMIC);
    setUseVertexBufferObjects(false);
    setUseDisplayList(false);

    if (_view) {
        SilverLiningCloudsUpdateCallback *updateCallback = new SilverLiningCloudsUpdateCallback;
        setUpdateCallback(updateCallback);

#if 0
        SilverLiningCloudsComputeBoundingBoxCallback *boundsCallback = new SilverLiningCloudsComputeBoundingBoxCallback;
        boundsCallback->camera = _view->getCamera();
        setComputeBoundingBoxCallback(boundsCallback);
#endif
    }
}

static SilverLining::Atmosphere* getAtmosphere(osg::RenderInfo& renderInfo)
{
	if (renderInfo.getCurrentCamera()==0)
	{
		return 0;
	}
	AtmosphereReference *ar = dynamic_cast<AtmosphereReference *>(renderInfo.getCurrentCamera()->getUserData());
	if (ar==0)
	{
		return 0;
	}
	return ar->atmosphere;
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

static const int tboStartOffset = 15;

static float LightBrightnessOnClouds = 0.f;

static void setUniform(const char* name, osg::GLExtensions* ext, GLint program, const osg::Vec4i& vec)
{
	if (name==0||ext==0)
	{
		return;
	}
	GLint loc = ext->glGetUniformLocation(program, name);
	if (loc != -1)
	{
		ext->glUniform4i(loc, (int)vec.x(), (int)vec.y(), (int)vec.z(), (int)vec.w());
	}
}

static void setUniform(const char* name, osg::GLExtensions* ext, GLint program, int val)
{
	if (name==0||ext==0)
	{
		return;
	}

	GLint loc = ext->glGetUniformLocation(program, name);
	if (loc != -1)
	{
		ext->glUniform1i(loc, val);
	}
}

static void setUniform(const char* name, osg::GLExtensions* ext, GLint program, float val)
{
	if (name == 0 || ext == 0)
	{
		return;
	}

	GLint loc = ext->glGetUniformLocation(program, name);
	if (loc != -1)
	{
		ext->glUniform1f(loc, val);
	}
}

static void setUpForwardPlus(osg::GL2Extensions* ext, GLint program, osg::Vec4d vTilingParams, const std::string& programName = "")
{
	if (ext == 0)
	{
		//osg::notify(osg::NOTICE) << "SilverLining: NULL GL2 Extension" << std::endl;
		return;
	}
	if (program == 0)
	{
		//osg::notify(osg::NOTICE) << "SilverLining: NULL Program: " << programName << std::endl;
		return;
	}

	osg::Vec4i vTilingParmsi((int)vTilingParams.x(),(int)vTilingParams.y(),(int)vTilingParams.z(),(int)vTilingParams.w());
	setUniform("vTilingParams", ext, program, vTilingParmsi);

	setUniform("lightDataTBO", ext, program, tboStartOffset);
	setUniform("lightIndexListTBO", ext, program, tboStartOffset-1);
	setUniform("lightGridOffsetAndSizeTBO", ext, program, tboStartOffset-2);
	setUniform("LightBrightnessOnClouds", ext, program, LightBrightnessOnClouds);
}

void CloudsDrawable::setUpShaders(SilverLining::Atmosphere *atmosphere, osg::RenderInfo& renderInfo) const
{
	osg::GL2Extensions* ext = osg::GL2Extensions::Get(renderInfo.getContextID(),true);
	if (ext==0)
	{
		return;
	}

	CloudsDrawable* mutableThis = const_cast<CloudsDrawable*>(this);
	if (mutableThis==0||mutableThis->_pluginContext==0)
	{
		return;
	}
	osg::ValueObject* valueObject = mutableThis->_pluginContext->getOrCreateValueObject();
	if (valueObject==0)
	{
		return;
	}

	osg::Camera* camera = renderInfo.getCurrentCamera();
	if (camera==0)
	{
		return;
	}

	osg::Vec4d vTilingParams;
	if (valueObject->getUserValue("FowardPlus-vTilingParams", vTilingParams))
	{
		osg::Vec4i vTilingParmsi((int)vTilingParams.x(),(int)vTilingParams.y(),(int)vTilingParams.z(),(int)vTilingParams.w());
	}
	
	OpenIG::Library::Graphics::Matrix4_64 view_matrix = OsgMatrixToGraphicsMatrix(camera->getViewMatrix()).GetTranspose();

	view_matrix[0][3] = 0;
	view_matrix[1][3] = 0;
	view_matrix[2][3] = 0;

	OpenIG::Library::Graphics::Matrix4_64 view_inverse_transpose_matrix = view_matrix.GetInverse().GetTranspose();

	OpenIG::Library::Graphics::Matrix4_32 view_matrix_32                   = OpenIG::Library::Graphics::MatrixPrecisionConvert::ToFloat32(view_matrix);

	GLint program = (GLint)atmosphere->GetBillboardShader();
	ext->glUseProgram(program);
	setUpForwardPlus(ext, program, vTilingParams,"Billboard");

	SL_VECTOR(unsigned int) shaders = atmosphere->GetActivePlanarCloudShaders();
	for (SL_VECTOR(unsigned int)::iterator itr = shaders.begin(); itr != shaders.end(); ++itr)
	{
		program = *itr;
		ext->glUseProgram(program);
		setUpForwardPlus(ext, program, vTilingParams, "Planar");
	}

	ext->glUseProgram(0);
}

static bool useLogZDepthBuffer(void)
{
	std::string strLogZDepthBuffer = OpenIG::Base::Configuration::instance()->getConfig("LogZDepthBuffer","yes");
	if (strLogZDepthBuffer.compare(0, 3, "yes") == 0)
		return true;
	else
		return false;
}

void CloudsDrawable::initializeLogZDepthBuffer(osg::RenderInfo& renderInfo, std::vector<GLint>& userShaders) const
{
	static bool _logz_tried = false;
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
	if (shaderID==0)
	{
		osg::notify(osg::ALWAYS)<<"SilverLining: error: shader compilation error: "<<std::endl;
	}
	else
	{
		osg::notify(osg::ALWAYS) << "SilverLining: Log Z Vertex Shader compiled successfully..." << std::endl;
		userShaders.push_back(shaderID);
	}

	strSource = logZPreamble + OpenIG::Base::FileSystem::readFileIntoString(resourcesPath + "/shaders/logz_ps.glsl");

	shaderID = osg::ShaderUtils::compileShader(strSource, osg::Shader::FRAGMENT, ext);
	if (shaderID==0)
	{
		osg::notify(osg::ALWAYS)<<"SilverLining: error: shader compilation error: "<<std::endl;
	}
	else
	{
		userShaders.push_back(shaderID);
		osg::notify(osg::ALWAYS) << "SilverLining: Log Z Pixel Shader compiled successfully..." << std::endl;
	}
}

void CloudsDrawable::initializeForwardPlus(SilverLining::Atmosphere *atmosphere, osg::RenderInfo& renderInfo, std::vector<GLint>& userShaders) const
{
	static bool _forwardPlusSetUpTried = false;

	if (_forwardPlusSetUpTried==true)
	{
		return;
	}
	_forwardPlusSetUpTried = true;

	osg::GLExtensions* ext = osg::GLExtensions::Get(renderInfo.getState()->getContextID(), true);

	std::string resourcesPath = OpenIG::Base::FileSystem::path(OpenIG::Base::FileSystem::Resources, "../resources");

	std::string strForward_Plus_SL_PS;
	std::string strForwardPlusDefine;

	if (_forwardPlusEnabled)
	{
		strForwardPlusDefine = "#define SL_USE_FORWARD_PLUS_LIGHTING\n";
	}

	strForward_Plus_SL_PS = strForwardPlusDefine + OpenIG::Base::FileSystem::readFileIntoString(resourcesPath + "/shaders/forward_plus_sl_ps.glsl");
	{
		GLint shaderID = osg::ShaderUtils::compileShader(strForward_Plus_SL_PS, osg::Shader::FRAGMENT, ext);
		if (shaderID==0)
		{
			osg::notify(osg::ALWAYS) << "Silverlining: error: Could not create forward_plus_sl_ps Shader" << std::endl;
		}
		else
		{
			osg::notify(osg::ALWAYS) << "Silverlining: forward_plus_sl_ps Shader compiled successfully!" << std::endl;
			userShaders.push_back(shaderID);
		}
	}

	// A fair metric for sky
	std::string strMAX_LIGHTS_PER_PIXEL = "#define MAX_LIGHTS_PER_PIXEL 200\n";

	std::string strSource = OpenIG::Base::FileSystem::readFileIntoString(resourcesPath + "/shaders/forwardplus_preamble.glsl") 
		+ strMAX_LIGHTS_PER_PIXEL
		+ OpenIG::Base::FileSystem::readFileIntoString(resourcesPath + "/shaders/lighting_math.glsl") 
		+ OpenIG::Base::FileSystem::readFileIntoString(resourcesPath + "/shaders/forwardplus_math.glsl");


	GLint shaderID = osg::ShaderUtils::compileShader(strSource, osg::Shader::FRAGMENT, ext);
	if (shaderID==0)
	{
		osg::notify(osg::ALWAYS) << "Silverlining: error: Could not create User Functions Shader" << std::endl;
	}
	else
	{
		osg::notify(osg::ALWAYS) << "Silverlining: User Functions Shader compiled successfully!" << std::endl;
		userShaders.push_back(shaderID);
	}
}

// Draw the clouds.
void CloudsDrawable::drawImplementation(osg::RenderInfo& renderInfo) const
{
	if (_lightBrightness_enable)
	{
		if (_todHour > 4 && _todHour < 19)			
			LightBrightnessOnClouds = _lightBrightness_day;
		else
			LightBrightnessOnClouds = _lightBrightness_night;
	}
	else
	{
		LightBrightnessOnClouds = 0.f;
	}

	AtmosphereReference *ar = dynamic_cast<AtmosphereReference *>(renderInfo.getCurrentCamera()->getUserData());
	SilverLining::Atmosphere *atmosphere = 0;
	if (ar) atmosphere = ar->atmosphere;

	osg::State & state = *renderInfo.getState();
	state.disableAllVertexArrays();

    if (atmosphere)
    {		
		std::vector<GLint> stdVecUserShaders;

		initializeLogZDepthBuffer(renderInfo, stdVecUserShaders);
		initializeForwardPlus(atmosphere, renderInfo, stdVecUserShaders);

		SL_VECTOR(unsigned int) vectorUserShaders;

		if (stdVecUserShaders.empty()==false)
		{
			SL_VECTOR(unsigned int) vectorUserShaders;
			for(int i = 0; i < stdVecUserShaders.size(); ++i)
			{
				vectorUserShaders.push_back(stdVecUserShaders[i]);
			}
			atmosphere->ReloadShaders(vectorUserShaders);
		}

		setUpShaders(atmosphere, renderInfo);
	 
        atmosphere->SetCameraMatrix(renderInfo.getCurrentCamera()->getViewMatrix().ptr());
        atmosphere->SetProjectionMatrix(renderInfo.getCurrentCamera()->getProjectionMatrix().ptr());

		glPushAttrib(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_CLAMP);

		//Change the TU from non-zero to zero, or zero to one to get around caching
		int tu = state.getActiveTextureUnit();
		state.setActiveTextureUnit(tu ? 0 : 1);

		atmosphere->DrawObjects(true, true, true);

		//Change the TU from non-zero to zero, or zero to one to get around caching
		state.setActiveTextureUnit(tu);

		glPopAttrib();

        if (_envMapDirty)
        {
            CloudsDrawable *mutableThis = const_cast<CloudsDrawable*>(this);
            void*           envPtr = 0;

            if (atmosphere->GetEnvironmentMap(envPtr,6,false,0,true))
            {

                mutableThis->_envMapID = (GLint)(long)envPtr;

                mutableThis->_envMapDirty = false;

                if (_pluginContext)
                {
                    //std::cout << "env map set to: " << mutableThis->_envMapID << std::endl;

                    mutableThis->_pluginContext->getOrCreateValueObject()->setUserValue("SilverLining-EnvironmentMap", (int)mutableThis->_envMapID);
                }
            }
        }
    }

    state.dirtyAllVertexArrays();
}

// We use the Atmosphere::GetCloudBounds() to get the bounds of the clouds in the scene. This is updated
// and persisted after the update pass, so it fits well with OSG's threading model.
osg::BoundingBox SilverLiningCloudsComputeBoundingBoxCallback::computeBound(const osg::Drawable&) const
{
    osg::BoundingBox box;
    
    if (camera) 
    {
        AtmosphereReference *ar = dynamic_cast<AtmosphereReference *>( camera->getUserData() );
        if ( ar && ar->atmosphere)
        {
            SilverLining::Atmosphere * atmosphere = ar->atmosphere;            

            double minX, minY, minZ, maxX, maxY, maxZ;
            atmosphere->GetCloudBounds(minX, minY, minZ, maxX, maxY, maxZ);
            osg::Vec3d min(minX, minY, minZ);
            osg::Vec3d max(maxX, maxY, maxZ);
            box = osg::BoundingBox(min, max);
        }
    }

    return box;
}

// Recompute the bounds of the clouds every frame.
void SilverLiningCloudsUpdateCallback::update(osg::NodeVisitor*, osg::Drawable* drawable)
{
    if (drawable) {
        drawable->dirtyBound();
    }
}
