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
#pragma once

#if defined(OPENIG_SDK)
	#include <OpenIG-Utils/Export.h>
#else
	#include <Core-Utils/Export.h>
#endif

#include <osg/GL>
#include <osg/GLExtensions>
#include <osg/GLDefines>

#include <Library-Graphics/CommonUtils.h>
#include <Library-Graphics/DataFormat.h>

#include "FrameLogging.h"

namespace osg
{
    class IGCOREUTILS_EXPORT TBO
    {
    public:
        TBO(size_t width, OpenIG::Library::Graphics::DataFormat format, const osg::GLExtensions* ext);
        ~TBO();
        size_t getWidth(void) const;
        void resize(size_t width);

        int getSizeInBytes(void) const;

        void copyData(const void* srcData, int offsetInBytes, int sizeToCopyInBytes);

        void bindTexture();
		void bindTBO();

        void* lock();
        void  unlock();

		bool isValid(void) const{return _valid;}

		int tboid(void) const {return _tboTextureID;}
    private:
        bool _valid;
        const osg::GLExtensions* _ext;

        GLuint _tboID;
        int _sizeInBytes;

        GLuint _tboTextureID;

        GLint _internalFormat;

        void init();
        void tearDown();

        bool _isLocked;

#if EXTRA_LOGGING
		FrameLogger _logger;
#endif
    };
}

