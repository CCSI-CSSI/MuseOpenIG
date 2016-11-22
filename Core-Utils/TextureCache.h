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
	#include <OpenIG-Base/StringUtils.h>
#else
	#include <Core-Utils/Export.h>
	#include <Core-Base/StringUtils.h>
#endif

#include <string>
#include <vector>
#include <map>

#include <osg/ref_ptr>
#include <osg/Texture2D>
#include <osg/TextureCubeMap>

namespace osg
{
   typedef osg::ref_ptr<osg::Texture2D> Texture2DPointer;

   class IGCOREUTILS_EXPORT TextureCache
   {
   public:
      void addPath(const std::string& path);
      Texture2DPointer get(const std::string& strFileName);
   private: 
      typedef std::vector< std::string > Paths;
      Paths	_paths;

      typedef std::map< std::string, Texture2DPointer >	MapNamesToTexture2DPointers;
      MapNamesToTexture2DPointers _cache;
   };

   typedef osg::ref_ptr<osg::TextureCubeMap> TextureCubeMapPointer;

   class IGCOREUTILS_EXPORT TextureCubeMapCache
   {
   public:
	   void addPath(const std::string& path);

	   typedef std::vector<std::string> FileNames;

	   TextureCubeMapPointer get(const FileNames& fileNames);
   private:
	   typedef std::map< unsigned, TextureCubeMapPointer > MapNamesToTextureCubeMapPointers;

	   typedef std::vector< std::string >	Paths;
	   Paths						_paths;
	   MapNamesToTextureCubeMapPointers	_cache;

   };
}
