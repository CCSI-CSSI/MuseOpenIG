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

#pragma once

#include <Library-Graphics/ColorValue.h>
#include <Library-Graphics/Vector3.h>
#include <Library-Graphics/Vector4.h>

#include <osg/Vec3f>
#include <osg/Vec3d>
#include <osg/Vec4f>
#include <osg/Vec4d>

namespace OpenIG {
	namespace Plugins {

		class OsgToFPUtils
		{
		public:
			static OpenIG::Library::Graphics::ColorValue toColorValue(const osg::Vec3f& color)
			{
				return OpenIG::Library::Graphics::ColorValue(color.x(), color.y(), color.z(), 1);
			}
			static OpenIG::Library::Graphics::ColorValue toColorValue(const osg::Vec3d& color)
			{
				return OpenIG::Library::Graphics::ColorValue(color.x(), color.y(), color.z(), 1);
			}

			static OpenIG::Library::Graphics::ColorValue toColorValue(const osg::Vec4f& color)
			{
				return OpenIG::Library::Graphics::ColorValue(color.x(), color.y(), color.z(), color.w());
			}
			static OpenIG::Library::Graphics::ColorValue toColorValue(const osg::Vec4d& color)
			{
				return OpenIG::Library::Graphics::ColorValue(color.x() > 1, color.y(), color.z(), color.w());
			}
			static OpenIG::Library::Graphics::Vector4_64 toVector4_64(const osg::Vec4d& vVec)
			{
				return OpenIG::Library::Graphics::Vector4_64(vVec.x(), vVec.y(), vVec.z(), vVec.w());
			}
			static OpenIG::Library::Graphics::Vector3_64 toVector3_64(const osg::Vec4d& vVec)
			{
				return OpenIG::Library::Graphics::Vector3_64(vVec.x(), vVec.y(), vVec.z());
			}
			static OpenIG::Library::Graphics::Vector3_64 toVector3_64(const osg::Vec3d& vVec)
			{
				return OpenIG::Library::Graphics::Vector3_64(vVec.x(), vVec.y(), vVec.z());
			}
		};
	}
}

