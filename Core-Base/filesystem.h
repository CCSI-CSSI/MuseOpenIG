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
//#*****************************************************************************

#pragma once

#if defined(OPENIG_SDK)
	#include <OpenIG-Base/export.h>
	#include <OpenIG-Base/stringutils.h>
#else
	#include <Core-Base/export.h>
	#include <Core-Base/stringutils.h>
#endif

#include <string>

namespace OpenIG {
	namespace Base {

		/*! Handy class to do few filesystem opertaions
		* \brief The FileSystem class
		* \author    Poojan Prabhu <openig@compro.net>
		* \copyright (c)Compro Computer Services, Inc.
		* \date      Thu Oct 22 2015
		*/
		class IGCORE_EXPORT FileSystem
		{
		public:
			/*! Checks if a given file exists
			* \brief Checks if a given file exists
			* \param strFileName the name of the file
			* \return true if a file exists false otherwise
			* \author    Poojan Prabhu <openig@compro.net>
			* \copyright (c)Compro Computer Services, Inc.
			* \date      Thu Oct 22 2015
			*/
			static bool fileExists(const std::string& strFileName);

			/*! Reads a whole file into a string
			* \brief Reads a whole file into a string
			* \param strFileName the name of the file
			* \return the file as a string
			* \author    Poojan Prabhu <openig@compro.net>
			* \copyright (c)Compro Computer Services, Inc.
			* \date      Thu Oct 22 2015
			*/
			static std::string readFileIntoString(const std::string& strFileName);

			/*! Returns the full path of a file
			* \brief Returns the full path of a file
			* \param strFileName the name of the file
			* \return the full path
			* \author    Poojan Prabhu <openig@compro.net>
			* \copyright (c)Compro Computer Services, Inc.
			* \date      Thu Oct 22 2015
			*/
			static std::string fileFullPath(const std::string& strFileName);

			/*\brief Enum for Resource types
			* \author    Trajce Nikolov Nick <openig@compro.net>
			* \copyright(c)Compro Computer Services, Inc.
//#*
//#*    Please direct any questions or comments to the OpenIG Forums
//#*    Email address: openig@compro.net
//#*
			* \date      Thu Oct 22 2015
			*/
			enum PathType
			{
				None,
				Plugins,
				Data,
				Resources,
				PathList // This is in form of {path1;path2;path3}/path/file - should return the first valid path found
			};

			/*! Returns the full path of a given type based on the host system
			* \brief Returns the full path of a given type based on the host system
			* \param type the enum above
			* \param path optional additional path to be appended
			* \return the full path based on ENV vars and host system, differs for Linux, Windows and Mac
			* \author    Trajce Nikolov Nick <openig@compro.net>
			* \copyright (c)Compro Computer Services, Inc.
			* \date      Thu Oct 22 2015
			*/
			static std::string path(PathType type, const std::string& path = "");

			/*! Returns the matched existing file from a given FileList. You can use wild cards and regex
			* \brief Returns the matched existing file from a given FileList. You can use wild cards
			* \param patterns StringList containing file names with path. You can use wildcard
			* \return the full path of the file found
			* \author    Trajce Nikolov Nick <openig@compro.net>
			* \copyright (c)Compro Computer Services, Inc.
			* \date      Sat Oct 31 2015
			*/
			static bool match(const OpenIG::Base::StringUtils::StringList& patterns, const std::string& simpleFileName);

			/*! Returns the time stamp of the last write
			* \brief Returns the time stamp of the last write
			* \param fileName the file name to check
			* \return the time stamp
			* \author    Trajce Nikolov Nick <openig@compro.net>
			* \copyright (c)Compro Computer Services, Inc.
			* \date      Thu Jan 14 2016
			*/
			static time_t lastWriteTime(const std::string& fileName);
		};
	} // namespace
} // namespace

