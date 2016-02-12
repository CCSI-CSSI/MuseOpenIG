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
#include "lightmanagerstateattribute.h"
#include <iostream>
#include <cassert>
using namespace osg;
using namespace OpenIG::Plugins;
using namespace OpenIG::Library::Graphics;

#include <Library-Graphics/LightManager.h>
#include <Library-Graphics/LightData.h>
#include <Library-Graphics/TileSpaceLightGrid.h>
#include <Library-Graphics/DataFormat.h>

#include <Core-Base/glerrorutils.h>
#include <Core-Base/imagegenerator.h>

#include <Core-OpenIG/openig.h>

#include <osg/State>

const int _lightDataTBOTexUnit              = 15;
const int _lightIndexListTBOTexUnit         = 14;
const int _lightGridOffsetAndSizeTBOTexUnit = 13;

//! \brief CTOR

//! \brief DTOR
LightManagerStateAttribute::~LightManagerStateAttribute()
{
}

void LightManagerStateAttribute::setAttributesAndUniformsIfNotSet(void)
{
	osg::StateSet* stateSet = _scene->getStateSet();
	if (stateSet->getUniform("lightDataTBO")==0)
	{
		stateSet->addUniform(new osg::Uniform("lightDataTBO", _lightDataTBOTexUnit));
	}
	if (stateSet->getUniform("lightIndexListTBO")==0)
	{
		stateSet->addUniform(new osg::Uniform("lightIndexListTBO", _lightIndexListTBOTexUnit));
	}
	if (stateSet->getUniform("lightGridOffsetAndSizeTBO")==0)
	{
		stateSet->addUniform(new osg::Uniform("lightGridOffsetAndSizeTBO", _lightGridOffsetAndSizeTBOTexUnit));
	}
}

namespace OpenIG {
	namespace Plugins {

		class glActiveTextureWrapped
		{
		public:
			typedef void (GL_APIENTRY * ActiveTextureProc) (GLenum texture);
			glActiveTextureWrapped(osg::GLExtensions* extensions)
			{
				_glActiveTexture = 0;
				setGLExtensionFuncPtr(_glActiveTexture, "glActiveTexture", "glActiveTextureARB");
			}
			void operator()(int unit)
			{
				if (_glActiveTexture) _glActiveTexture(GL_TEXTURE0 + unit);
			}
		private:
			ActiveTextureProc _glActiveTexture;
		};
	}
}

//! \brief Apply the GLLights in the DtOsgLightManager.
void LightManagerStateAttribute::apply(osg::State& state) const
{
	if (_lightManager==0)
	{
		return;
	}

	LightManagerStateAttribute* nonConst = const_cast<LightManagerStateAttribute*>(this);
	if (_extensions==0)
	{
		nonConst->_extensions = state.get<osg::GLExtensions>();
		if (nonConst->_glActiveTextureWrapped==0)
		{
			nonConst->_glActiveTextureWrapped = new glActiveTextureWrapped(nonConst->_extensions);
		}
	}


	nonConst->packLights();
	nonConst->updateTiledShadingStuff();

	if (_glActiveTextureWrapped==0)
	{
		return;
	}

	int tu = state.getActiveTextureUnit();
	//Change the TU from non-zero to zero, or zero to one to get around caching
	state.setActiveTextureUnit(tu ? 0 : 1);

	if (_lightDataTBO)
	{
		(*_glActiveTextureWrapped)(_lightDataTBOTexUnit);
		_lightDataTBO->bindTexture();
	}
	if (_lightIndexListTBO)
	{
		(*_glActiveTextureWrapped)(_lightIndexListTBOTexUnit);
		_lightIndexListTBO->bindTexture();
	}
	if (_lightGridOffsetAndSizeTBO)
	{
		(*_glActiveTextureWrapped)(_lightGridOffsetAndSizeTBOTexUnit);
		_lightGridOffsetAndSizeTBO->bindTexture();
	}
	state.setActiveTextureUnit(tu);
}

void LightManagerStateAttribute::set(OpenIG::Library::Graphics::LightManager*	 lightManager
	, OpenIG::Library::Graphics::Camera_64*	  fpCamera
	, OpenIG::Library::Graphics::Vector2_uint32* fpViewport
	, osg::Group* scene
	, OpenIG::Base::ImageGenerator* ig)
{
	_lightManager = lightManager;
	_fpCamera = fpCamera;
	_fpViewport = fpViewport;
	_scene = scene;
	_ig = ig;

	_lightData = new LightData(341, FORMAT_R32G32B32A32_FLOAT);
	Vector2_uint32 tileSize(32, 32);
	_tileSpaceLightGrid = new TileSpaceLightGrid(tileSize);
	_tileSpaceLightGrid->SetScreenAreaCullSize(tileSize*2);
}

int LightManagerStateAttribute::compare(const StateAttribute& sa) const
{
	// check the types are equal and then create the rhs variable
	// used by the COMPARE_StateAttribute_Parameter macros below.
	COMPARE_StateAttribute_Types(LightManagerStateAttribute,sa)
	return 0;
}

LightManagerStateAttribute::LightManagerStateAttribute()
	: _lightManager(0)
	, _fpCamera(0)
	, _fpViewport(0)
	, _lightDataTBO(0)
	, _lightGridOffsetAndSizeTBO(0)
	, _lightIndexListTBO(0)
	, _extensions(0)
	, _ig(0)
	, _glActiveTextureWrapped(0)
{
	
}


LightManagerStateAttribute::LightManagerStateAttribute(const LightManagerStateAttribute& rhs,const CopyOp& copyop)
	: _lightManager(0)
	, _fpCamera(0)
	, _fpViewport(0)
	, _lightDataTBO(0)
	, _lightGridOffsetAndSizeTBO(0)
	, _lightIndexListTBO(0)
	, _extensions(0)
	, _ig(0)
	, _glActiveTextureWrapped(0)
{

}


void LightManagerStateAttribute::updateLightDataTBO(void)
{
	if (_lightDataTBO==0)
	{
		_lightDataTBO = new osg::TBO(_lightData->GetWidth(), _lightData->GetFormat(), _extensions);
	}
	if (_lightDataTBO->isValid()==false)
	{
		std::cout<<"Error"<<std::endl;
		return;
	}
	if (_lightDataTBO->getWidth()!=_lightData->GetWidth())
	{
		_lightDataTBO->resize(_lightData->GetWidth());
	}
	if (_lightDataTBO->isValid()==false)
	{
		std::cout<<"Error"<<std::endl;
		return;
	}
	ASSERT_PREDICATE(_lightDataTBO->getWidth()==_lightData->GetWidth());

	_lightDataTBO->copyData(_lightData->GetPackedData(), 0, _lightData->GetPackedDataSizeInBytes());
}

void LightManagerStateAttribute::packLights(void)
{
	_lightData->PackLights(_lightManager->GetFrustumAffectingLights());
	updateLightDataTBO();
}

static const std::string strTilingParamsUniform = "vTilingParams";

static void updateTilingParamsForGlobalContext(OpenIG::Base::ImageGenerator* _ig, const osg::Vec4i& _vTilingParams)
{
	OpenIG::Engine* openIG = dynamic_cast<OpenIG::Engine*>(_ig);
	if (openIG==0)
	{
		return;
	}
	OpenIG::PluginBase::PluginContext& context = openIG->getPluginContext();
	osg::Vec4d vTilingParams(_vTilingParams.x(),_vTilingParams.y(),_vTilingParams.z(),_vTilingParams.w());
	context.getOrCreateValueObject()->setUserValue("FowardPlus-vTilingParams", vTilingParams);
}
void LightManagerStateAttribute::updateTilingParams()
{
	osg::StateSet* stateSet = _scene->getStateSet();
	if (stateSet->getUniform(strTilingParamsUniform)==0)
	{
		int i = 0;
		osg::Uniform* vTilingParamsUniform = new osg::Uniform(strTilingParamsUniform.c_str(), i, i, i, i);
		stateSet->addUniform(vTilingParamsUniform);
	}
	ASSERT_PREDICATE(stateSet && stateSet->getUniform(strTilingParamsUniform));
	Vector2_uint32 tileSize = _tileSpaceLightGrid->GetTileSize();
	Vector2_uint32 viewport = *_fpViewport;
	stateSet->getUniform(strTilingParamsUniform)->set((int)tileSize.x, (int)tileSize.y, (int)viewport.x, (int)viewport.y);
	updateTilingParamsForGlobalContext(_ig, osg::Vec4i((int)tileSize.x, (int)tileSize.y, (int)viewport.x, (int)viewport.y));
}

void LightManagerStateAttribute::updateTileLightGridOffsetAndSizeTBO()
{
	if (_lightGridOffsetAndSizeTBO==0)
	{
		_lightGridOffsetAndSizeTBO = new osg::TBO(_tileSpaceLightGrid->GetTileGridOffsetAndSizeWidth()
												, _tileSpaceLightGrid->GetTileGridOffsetAndSizeDataFormat(), _extensions);
	}
	if (_lightGridOffsetAndSizeTBO->isValid()==false)
	{
		std::cout<<"Error"<<std::endl;
		return;
	}
	if (_lightGridOffsetAndSizeTBO->getWidth()!=_tileSpaceLightGrid->GetTileGridOffsetAndSizeWidth())
	{
		_lightGridOffsetAndSizeTBO->resize(_tileSpaceLightGrid->GetTileGridOffsetAndSizeWidth());
	}
	if (_lightGridOffsetAndSizeTBO->isValid()==false)
	{
		std::cout<<"Error"<<std::endl;
		return;
	}
	ASSERT_PREDICATE(_lightGridOffsetAndSizeTBO->getWidth()==_tileSpaceLightGrid->GetTileGridOffsetAndSizeWidth());

	_lightGridOffsetAndSizeTBO->copyData(_tileSpaceLightGrid->GetTileGridOffsetAndSizeDataPtr(), 0, _tileSpaceLightGrid->GetTileGridOffsetAndSizeSizeInBytes());
}

void LightManagerStateAttribute::updateTileLightIndexListTBO()
{
	if (_tileSpaceLightGrid->GetTotalTileLightIndexListLength()==0)
	{
		return;
	}
	unsigned int widthRequired = Math::GetUpperPowerOfTwo(_tileSpaceLightGrid->GetTotalTileLightIndexListLength()/4 + 1);

	if (_lightIndexListTBO==0)
	{
		_lightIndexListTBO = new osg::TBO(widthRequired
			, FORMAT_R32G32B32A32_SINT, _extensions);
	}
	else
	{
		if (_lightIndexListTBO->getWidth()<widthRequired)
		{
			_lightIndexListTBO->resize(widthRequired);
		}
	}
	if (_lightIndexListTBO->isValid()==false)
	{
		std::cout<<"Error"<<std::endl;
		return;
	}
	size_t sizeToCopy = _tileSpaceLightGrid->GetTotalTileLightIndexListLength()*sizeof(int32);
	_lightIndexListTBO->copyData(_tileSpaceLightGrid->GetTileLightIndexListsPtr(), 0, sizeToCopy);
}

void LightManagerStateAttribute::updateTiledShadingStuff(void)
{
	_tileSpaceLightGrid->Update(_lightManager->GetFrustumAffectingLights(), _fpCamera, *_fpViewport);
	updateTileLightGridOffsetAndSizeTBO();
	updateTileLightIndexListTBO();
	updateTilingParams();
	setAttributesAndUniformsIfNotSet();
	//initializeRampTexture();
}


#if 0
void UpdateLightAttribscallback::initializeRampTexture()
{
	static bool once = true;

	if (once==false)
	{
		return;
	}
	once = false;
	std::string resourcesPath = OpenIG::Base::FileSystem::path(OpenIG::Base::FileSystem::Resources, "../resources");
	_rampImage = osgDB::readImageFile(resourcesPath + "textures/color_ramp_4.dds");
	if (_rampImage.valid())
	{
		_rampTexture = new osg::Texture2D;
		_rampTexture->setImage(_rampImage);
	}
}
#endif
