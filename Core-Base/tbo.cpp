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

#include "tbo.h"

#include <osg/BufferObject>
#include <osg/Texture>

#include <Library-Graphics/OIGAssert.h>
#include <Library-Graphics/OIGMath.h>

#include <Core-Base/glerrorutils.h>

#include <iostream>


using namespace OpenIG::Library::Graphics;

using namespace osg;

GLint toGLInternalFormat(DataFormat format)
{
    if (format==FORMAT_R32G32B32A32_FLOAT)
    {
        return GL_RGBA32F_ARB;
    }
    if (format==FORMAT_R16G16B16A16_FLOAT)
    {
        return GL_RGBA16F_ARB;
    }
    if (format==FORMAT_R32G32B32A32_SINT)
    {
        return GL_RGBA32I_EXT;
    }
    if (format==FORMAT_R32G32_SINT)
    {
        return GL_RG32I;
    }
    if (format==FORMAT_R32_SINT)
    { 
        return GL_R32I;
    }

    ASSERT_PREDICATE(false);
    return -1;
}

DataFormat toFormat(GLint format)
{
    if (format==GL_RGBA32F_ARB)
    {
        return FORMAT_R32G32B32A32_FLOAT;
    }
    if (format==GL_RGBA16F_ARB)
    {
        return FORMAT_R16G16B16A16_FLOAT;
    }
    if (format==GL_RGBA32I_EXT)
    {
        return FORMAT_R32G32B32A32_SINT;
    }
    if (format==GL_RG32I)
    {
        return FORMAT_R32G32_SINT;
    }
    if (format==GL_R32I)
    { 
        return FORMAT_R32_SINT;
    }

    ASSERT_PREDICATE(false);
    return FORMAT_UNKNOWN;
}

void TBO::init()
{
    _tboTextureID = 0;
#if 0
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_BUFFER, 0);
#endif
    glGenTextures(1, &_tboTextureID);
#if 0
	glBindTexture(GL_TEXTURE_BUFFER, _tboTextureID);
#endif
	GLenum errorCode = OpenIG::Base::GLErrorUtils::checkAndLogError();
	if (errorCode!=GL_NO_ERROR)
	{
		_valid = false;
		return;
	}


    _tboID = 0;
    _ext->glGenBuffers(1, &_tboID);
	errorCode = OpenIG::Base::GLErrorUtils::checkAndLogError();
	if (errorCode!=GL_NO_ERROR)
	{
		_valid = false;
		return;
	}
}

void TBO::tearDown()
{
    if (_tboID!=0)
    {
        _ext->glDeleteBuffers(1, &_tboID);
        _tboID = 0;
    }
    if (_tboTextureID!=0)
    {
        glDeleteTextures(1, &_tboTextureID);
        _tboTextureID = 0;
    }
    _sizeInBytes = 0;
}
void TBO::resize(size_t width)
{
    ASSERT_PREDICATE(Math::IsPowerOfTwo(width));

    tearDown();

    init();

	if (_valid==false)
	{
		return;
	}

    DataFormat format = toFormat(_internalFormat);
    int sizeInBytes = width*DataFormatUtils::GetNumComponents(format)*DataFormatUtils::GetEachComponentSizeInBytes(format);

    _ext->glBindBuffer(GL_TEXTURE_BUFFER, _tboID);
	GLenum errorCode = OpenIG::Base::GLErrorUtils::checkAndLogError();
	if (errorCode!=GL_NO_ERROR)
	{
		osg::notify(osg::NOTICE) <<"TBO::resize: OpenGL Error: glBindBuffer"<<std::endl;
	}

    _ext->glBufferData(GL_TEXTURE_BUFFER, sizeInBytes, NULL, GL_DYNAMIC_DRAW);
	errorCode = OpenIG::Base::GLErrorUtils::checkAndLogError();
	if (errorCode!=GL_NO_ERROR)
	{
		osg::notify(osg::NOTICE) <<"TBO::resize: OpenGL Error: glBufferData"<<std::endl;
	}

    // Check size
    _sizeInBytes = 0;
    _ext->glGetBufferParameteriv(GL_TEXTURE_BUFFER, GL_BUFFER_SIZE_ARB, &_sizeInBytes);
    if(_sizeInBytes != sizeInBytes)
    {
        tearDown();
        _valid = false;
    }
    else
    {
        _valid = true;
    }

	bindTexture();
	_ext->glTexBuffer(GL_TEXTURE_BUFFER, _internalFormat, _tboID);

	_ext->glBindBuffer(GL_TEXTURE_BUFFER, 0);
	glBindTexture(GL_TEXTURE_BUFFER, 0);
}

size_t TBO::getWidth(void) const
{
    DataFormat format = toFormat(_internalFormat);
    return _sizeInBytes/(DataFormatUtils::GetNumComponents(format)*DataFormatUtils::GetEachComponentSizeInBytes(format));
}

TBO::TBO(size_t width, DataFormat format, const osg::GLExtensions* ext)
    : _ext(ext)
    , _isLocked(false)
	, _valid(true)
	, _tboID(0)
	, _tboTextureID(0)
#if EXTRA_LOGGING
	, _logger(50)
#endif
{
    ASSERT_PREDICATE(Math::IsPowerOfTwo(width));

    _internalFormat = toGLInternalFormat(format);
    resize(width);
}

TBO::~TBO()
{
    tearDown();      
}
void TBO::copyData(const void* srcData, int offsetInBytes, int sizeToCopyInBytes)
{
	EXTRA_LOGGING_STATIC_HANDLER
    ASSERT_PREDICATE(offsetInBytes+sizeToCopyInBytes<=_sizeInBytes);
	if (offsetInBytes+sizeToCopyInBytes>_sizeInBytes)
	{
		return;
	}
	bindTBO();
    _ext->glBufferSubData(GL_TEXTURE_BUFFER, offsetInBytes, sizeToCopyInBytes, srcData);
#if EXTRA_LOGGING
	errorCode = OpenIG::Base::GLErrorUtils::checkAndLogError();
	if (errorCode!=GL_NO_ERROR)
	{
		osg::notify(osg::NOTICE) <<"TBO::copyData: OpenGL Error: glBufferSubData"<<std::endl;
	}
#endif
#if EXTRA_LOGGING
	bool bLog = _logger.tick();
	if (bLog)
	{
		osg::notify(osg::NOTICE) <<"TBO::copyData: offset: "<<offsetInBytes<<" sizeToCopy: "<<sizeToCopyInBytes<<" ptr="<<srcData<<std::endl;
	}
#endif
	_ext->glBindBuffer(GL_TEXTURE_BUFFER, 0);
}

void TBO::bindTexture()
{
	glBindTexture(GL_TEXTURE_BUFFER, _tboTextureID);
#if EXTRA_LOGGING
	GLenum errorCode = OpenIG::Base::GLErrorUtils::checkAndLogError();
	if (errorCode!=GL_NO_ERROR)
	{
		osg::notify(osg::NOTICE) <<"TBO::bind: OpenGL Error: glBindTexture"<<std::endl;
	}
#endif
}

void TBO::bindTBO()
{
	_ext->glBindBuffer(GL_TEXTURE_BUFFER, _tboID);
#if EXTRA_LOGGING
	errorCode = OpenIG::Base::GLErrorUtils::checkAndLogError();
	if (errorCode!=GL_NO_ERROR)
	{
		osg::notify(osg::NOTICE) <<"TBO::bind: OpenGL Error: glTexBuffer"<<std::endl;
	}
#endif
}

void* TBO::lock()
{
    ASSERT_PREDICATE(_isLocked==false);
    _ext->glBindBuffer(GL_TEXTURE_BUFFER, _tboID);
#if EXTRA_LOGGING
	GLenum errorCode = OpenIG::Base::GLErrorUtils::checkAndLogError();
	if (errorCode!=GL_NO_ERROR)
	{
		osg::notify(osg::NOTICE) <<"TBO::lock: OpenGL Error: glBindBuffer"<<std::endl;
	}
#endif
    void* pData = _ext->glMapBuffer(GL_TEXTURE_BUFFER, GL_WRITE_ONLY_ARB);

#if EXTRA_LOGGING
	errorCode = OpenIG::Base::GLErrorUtils::checkAndLogError();
	if (errorCode!=GL_NO_ERROR)
	{
		osg::notify(osg::NOTICE) <<"TBO::lock: OpenGL Error: glMapBuffer"<<std::endl;
	}
#endif

    _isLocked = true;
    return pData;
}
void  TBO::unlock()
{
    ASSERT_PREDICATE(_isLocked==true);
    _ext->glUnmapBuffer(GL_TEXTURE_BUFFER);
    _isLocked = false;
}