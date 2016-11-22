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
#ifndef PLUGINOPERATION_H
#define PLUGINOPERATION_H

#if defined(OPENIG_SDK)
	#include <OpenIG-PluginBase/Export.h>
	#include <OpenIG-PluginBase/Plugin.h>
#else
	#include <Core-PluginBase/Export.h>
	#include <Core-PluginBase/Plugin.h>
#endif

#include <osg/Referenced>

namespace OpenIG {
	namespace PluginBase {

		/*! Convinient class to apply operation of plugins. Obviouselly the
		 * \ref OpenIG::PluginBase::PluginHost is calling these to call plugin
		 * hooks in the ImageGenerator. Here is an example of a plugin operation
		 * that will print the plugin name :
		 *      \code{.cpp}
		 *      class PrintPluginNamePluginOperation : public OpenIG::PluginBase::PluginOperation
		 *      {
		 *          public:
		 *              virtual void apply(igplugincore::Plugin* plugin)
		 *              {
		 *                  osg::notify(osg::NOTICE) << "Plugin : " << plugin->getName() << std::endl;
		 *              }
		 *      };
		 *      // Apply this operation to all the plugins sorted by their order number
		 *      osg::ref_ptr<PrintPluginNamePluginOperation> printPluginOperation(new PrintPluginNamePluginOperation);
		 *      PluginHost::applyPluginOperation(printPluginOperation.get());
		 *      \endcode
		 * \brief The PluginOperation class
		 * \author    Trajce Nikolov Nick openig@compro.net
		 * \copyright (c)Compro Computer Services, Inc.
		 * \date      Fri Jan 16 2015
		 */
		class IGPLUGINCORE_EXPORT PluginOperation : public osg::Referenced
		{
		public:
			/*!
			 * \brief Constructor
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Fri Jan 16 2015
			 */
			PluginOperation()
				: osg::Referenced() {}

			/*!
			 * \brief Apply this operation on a plugin. Obviouselly call some plugin method
			 * \param plugin The plugin
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Fri Jan 16 2015
			 */
			virtual void apply(OpenIG::PluginBase::Plugin* plugin) = 0;
		};
	}
} // namespace

#endif // PLUGINOPERATION_H
