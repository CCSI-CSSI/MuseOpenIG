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
#ifndef PLUGINHOST_H
#define PLUGINHOST_H

#if defined(OPENIG_SDK)
	#include <OpenIG-PluginBase/Export.h>
	#include <OpenIG-PluginBase/Plugin.h>
	#include <OpenIG-PluginBase/PluginOperation.h>
#else
	#include <Core-PluginBase/Export.h>
	#include <Core-PluginBase/Plugin.h>
	#include <Core-PluginBase/PluginOperation.h>
#endif

#include <osg/ref_ptr>
#include <osgDB/DynamicLibrary>

#include <string>
#include <map>

namespace OpenIG {
	namespace PluginBase {

		/*! The PluginHost class. It is managing plugins
		 * \brief The PluginHost class
		 * \author    Trajce Nikolov Nick openig@compro.net
		 * \copyright (c)Compro Computer Services, Inc.
		 * \date      Fri Jan 16 2015
		 */
		class IGPLUGINCORE_EXPORT PluginHost
		{
		public:
			/*!
			 * \brief Constructor
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Fri Jan 16 2015
			 */
			PluginHost();

			/*!
			 * \brief Destructor
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Fri Jan 16 2015
			 */
			~PluginHost();

			/*!
			 * \brief Loads all the plugins
			 * \param The path where the plugins are located
			 * \param configFileName The plugin configuration file. \ref openig::OpenIG is
			 *      expecting it in: Windows in igdata/openig.xml, Linux and MacOS in /usr/local/lib/igdata/openig.xml
			 *      or Linux 64bit in /usr/local/lib64/igdata/openig.xml
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Fri Jan 16 2015
			 */
			void loadPlugins(const std::string& path, const std::string& configFileName = "");

			/*!
			 * \brief Unloads all plugins
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Fri Jan 16 2015
			 */
			void unloadPlugins();

			/*! Apply plugin operation on all the plugins in a sorted fashion. The plugins are
			 * orderd by ther order number. See \ref OpenIG::PluginBase::Plugin::getOrderNumber
			 * \brief Apply plugin operation on all the plugins in a sorted fashion.
			 * \param operation The plugin operation
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Fri Jan 16 2015
			 */
			void applyPluginOperation(OpenIG::PluginBase::PluginOperation* operation);

			typedef std::map< int, osg::ref_ptr<OpenIG::PluginBase::Plugin> >                 PluginsMap;
			typedef std::map< int, osg::ref_ptr<OpenIG::PluginBase::Plugin> >::iterator       PluginsMapIterator;
			typedef std::map< int, osg::ref_ptr<OpenIG::PluginBase::Plugin> >::const_iterator PluginsMapConstIterator;

			/*!
			 * \brief Gets all the plugins in order number based std::map
			 * \return
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Fri Jan 16 2015
			 */
			const PluginsMap& getPlugins() const
			{
				return _plugins;
			}

		protected:
			PluginsMap              _plugins;               /*! \brief  The plugin order number based std::map */

			typedef std::map< std::string, osg::ref_ptr<osgDB::DynamicLibrary> >                    PluginLibrariesMap;
			typedef std::map< std::string, osg::ref_ptr<osgDB::DynamicLibrary> >::iterator          PluginLibrariesMapIterator;
			typedef std::map< std::string, osg::ref_ptr<osgDB::DynamicLibrary> >::const_iterator    PluginLibrariesMapConstIterator;

			PluginLibrariesMap      _pluginLibraries;       /*! \brief Filename based std::map of the plugins shared libraries */

			/*!
			 * \brief Performs if a file match the plugin naming convention
			 * \param fileName The file name of a file
			 * \return true if the file is plugin, false otherwise
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Fri Jan 16 2015
			 */
			bool isPlugin(const std::string& fileName) const;

		};
	}
} // namespace

#endif // PLUGINHOST_H
