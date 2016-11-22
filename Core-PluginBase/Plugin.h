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
#ifndef PLUGIN_H
#define PLUGIN_H

#if defined(OPENIG_SDK)
	#include <OpenIG-PluginBase/Export.h>
	#include <OpenIG-PluginBase/PluginContext.h>
#else
	#include <Core-PluginBase/Export.h>
	#include <Core-PluginBase/PluginContext.h>
#endif

#include <string>

#include <osg/Referenced>
#include <osg/Node>

namespace OpenIG {
	namespace PluginBase {

		class PluginHost;

		/*! The \ref OpenIG Image Generator is designed with plugin-in based architecture
		 * This class is the abstract interface definition of a plugin
		 *
		 * As the Plugin is defined, it is providing methods that are hooks giving the writers
		 * of inheritants chance to intercept in events happening in the \ref OpenIG.
		 * These events are the visual database or model loading, hooks in the DatabaseReadCallback,
		 * in the \ref OpenIG::Base::ImageGenerator::frame method for update and pre/post frame processing,
		 * on \ref OpenIG::Base::ImageGenerator::Entity addition.
		 *
		 * In \ref OpenIG, a DatabaseReadCallback is implemented in a way that uses the
		 * default osg read implementatoion and then it calls all the plugins loaded method
		 * \ref igplugincore::Plugin::databaseRead to postprocess the loaded model before it's addition
		 * to the scene. Example can be the \ref igplugins::LightingPlugin that traverse the loaded
		 * model, looks for runways and adds shader based light sources as addition to the light points
		 * After the process of the loading of the model, there is NodeVisitor applied to the model, which
		 * calls the \ref igplugincore::Plugin::databaseReadInVisitorBeforeTraverse and
		 * \ref igplugincore::Plugin::databaseReadInVisitorAfterTraverse hook methods. It is a common place
		 * to use this one NodeVisitor and perform actions/settings on nodes from the loaded model.
		 * Here is a snippet in pseudo code for the DatabaseReadCallback - for details refer to the source.
		 * \code{.cpp}
		 * class DatabaseReadCallback : public osgDB::Registry::ReadFileCallback
		 * {
		 * ....
		 *     virtual osgDB::ReaderWriter::ReadResult readNode(const std::string& filename, const osgDB::Options* options)
		 *     {
		 *         osgDB::ReaderWriter::ReadResult result = osgDB::Registry::instance()->readNodeImplementation(filename,options);
		 *         if (result.getNode())
		 *         {
		 *             // Call the databaseRead hook on all plugins
		 *             // through PluginOperation
		 *             Plugin::databaseRead(filename,options);
		 *
		 *             // Traverse the model and calls
		 *             // the other mention hooks, see bellow
		 *             DatabaseReadNodeVisitor nv;
		 *             result.getNode()->accept(nv);
		 *         }
		 *         return result;
		 *     }
		 * ....
		 * };
		 *
		 * class DatabaseReadNodeVisitor : public osg::NodeVisitor
		 * {
		 * ...
		 *     virtual void apply(osg::Node& node)
		 *     {
		 *         // Call all the plugins hook before
		 *         // traversal
		 *         Plugin::databaseReadInVisitorBeforeTraverse(...);
		 *
		 *         traverse(node);
		 *
		 *         // Call all the plugins hook
		 *         // after traversal
		 *         Plugin::databaseReadInVisitorAfterTraverse(...);
		 *     }
		 * ...
		 * };
		 * \endcode
		 *
		 * The other hooks are tied in \ref OpenIG::Base::ImageGenerator::init to give
		 * chance to init and configure the plugins. These are:
		 *      - \ref igplugincore::Plugin::init
		 *      - \ref igplugincore::Plugin::config
		 *
		 * Then the frame hooks, that are happening each frame:
		 *      - \ref igplugincore::Plugin::update
		 *      - \ref igplugincore::Plugin::preFrame
		 *      - \ref igplugincore::Plugin::postFrame
		 * See \ref OpenIG::Base::ImageGenerator::frame where these are explained
		 *
		 * And the last hook added is for the addEntity event, happens in
		 * \ref OpenIG::Base::ImageGenerator::addEntity
		 *
		 * \brief The Plugin class
		 * \author    Trajce Nikolov Nick openig@compro.net
		 * \copyright (c)Compro Computer Services, Inc.
		 * \date      Fri Jan 16 2015
		 */
		class IGPLUGINCORE_EXPORT Plugin : public osg::Referenced
		{
		public:
			friend class PluginHost;
			/*!
			 * \brief Constructor
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Fri Jan 16 2015
			 */
			Plugin()
				: osg::Referenced()
				, _orderNumber(0)
			{}

			/*! The name of the plugin. It is used by the igplugincore::PluginHost to manage
			 * them. The active plugins are mentioned in the \ref opeing::OpenIG configuration
			 * file openig.xml on Windows found in igdata folder, on Linux and MacOS in
			 * /usr/local/lib/igdata.
			 * \brief The name of the plugin
			 * \return The name of the plugin
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Fri Jan 16 2015
			 */
			virtual std::string getName() = 0;

			/*! Ihnerits are requred to provide description of what the
			 * implemented plugin does
			 * \brief The description of the plugin
			 * \return The description of the plugin
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Fri Jan 16 2015
			 */
			virtual std::string getDescription() = 0;

			/*!
			 * \brief The version of the plugin
			 * \return The version of the plugin
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Fri Jan 16 2015
			 */
			virtual std::string getVersion() = 0;

			/*!
			 * \brief The author of the plugin
			 * \return The author of the plugin
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Fri Jan 16 2015
			 */
			virtual std::string getAuthor() = 0;

			/*! The shared library name where the plugin is defined. This
			 * is set by the \ref igplugincore::PluginHost
			 * \brief The shared library name where the plugin is defined
			 * \return
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Fri Jan 16 2015
			 */
			std::string getLibrary() const { return _library; }

			/*! The order number of the plugin. It is read by the
			 * \ref igplugincore::PluginHost from the configuration
			 * file as openig.xml is. The plugins are then executed
			 * based on their order number
			 * \brief The order number of the plugin
			 * \return The order number of the plugin
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Fri Jan 16 2015
			 */
			int getOrderNumber() const { return _orderNumber; }

			/*! This is a hook that is attached to a database read callback in
			 * osg. It is called when load of a model is happened and gives the
			 * user to intercept this event and perform custom operation from
			 * within the plugin
			 * \brief Hook for database read
			 * \param The first parameter is the file name of the model loaded
			 * \param The second parameter is the osg::Node representing the loaded model
			 * \param The last parameter is the osgDB::Options provided for loading of the model
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Fri Jan 16 2015
			 */
			virtual void databaseRead(const std::string&, osg::Node*, const osgDB::Options*) {}

			/*! Hook for begining of the loaded model traveral. The mention database read callback
			 * is traversing the loaded model and this method is called before traversal. It gives the
			 * user ability to perform operations on all the nodes being traversed. Example can be
			 * the \ref LightingPlugin which replaces lightpoints in the loaded model with real lights
			 * computed in a shader
			 * \brief Hook for begining of the loaded model traveral
			 * \param The node traversed is the first parameter
			 * \param The options used to load the model
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Fri Jan 16 2015
			 */
			virtual void databaseReadInVisitorBeforeTraverse(osg::Node&, const osgDB::Options*) {}

			/*! Hook for end of the loaded model traveral. The mention database read callback
			 * is traversing the loaded model and this method is called after traversal. It gives the
			 * user ability to perform operations on all the nodes being traversed. Example can be
			 * the \ref LightingPlugin which replaces lightpoints in the loaded model with real lights
			 * computed in a shader
			 * \brief Hook for begining of the loaded model traveral
			 * \param The node traversed is the first parameter
			 * \param The options used to load the model
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Fri Jan 16 2015
			 */
			virtual void databaseReadInVisitorAfterTraverse(osg::Node&) {}

			/*! Method to configure the plugin from a XML file. It is called once at \ref
			 * OpenIG::Base::ImageGenerator::init. The file name is composed from the file name of
			 * the library containing the plugin implementation with .xml extension
			 * \brief Method to configure the plugin from a XML file
			 * \param The file name of the XML configuration
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Fri Jan 16 2015
			 */
			virtual void config(const std::string&) {}

			/*! Method to init the plugin with a given \ref igplugincore::PluginContext. The
			 * \ref igplugincore::PluginHost is calling this once to give the user to perform
			 * initialization of the plugin
			 * \brief Method to init the plugin with a given \ref igplugincore::PluginContext
			 * \param The \ref igplugincore::PluginContext
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Fri Jan 16 2015
			 */
			virtual void init(PluginContext&) {}

			/*! Update hook. This is called in a frame with a given \ref igplugincore::PluginContext.\
			 *  See \ref OpenIG::Base::ImageGenerator::frame for more info
			 * \brief Update hook
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Fri Jan 16 2015
			 */
			virtual void update(PluginContext&) {}

			/*! preFrame hook. This is called in a frame with a given \ref igplugincore::PluginContext.\
			 *  See \ref OpenIG::Base::ImageGenerator::frame for more info
			 * \brief Preframe hook
			 * \param The \ref igplugincore::PluginContext
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Fri Jan 16 2015
			 */
			virtual void preFrame(PluginContext&, double) {}

			/*! postFrame hook. This is called in a frame with a given \ref igplugincore::PluginContext.\
			 *  See \ref OpenIG::Base::ImageGenerator::frame for more info
			 * \brief Postframe hook
			 * \param The \ref igplugincore::PluginContext
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Fri Jan 16 2015
			 */
			virtual void postFrame(PluginContext&, double) {}

			/*! Hook called on exit to give the user chance to perform cleanup
			 *  See \ref OpenIG::Base::ImageGenerator::frame for more info
			 * \brief Method called on exit to give the user chance to perform cleanup
			 * \param The \ref igplugincore::PluginContext
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Fri Jan 16 2015
			 */
			virtual void clean(PluginContext&) {}

			/*! Hook called when \ref OpenIG::Base::Entity is added to the scene. This
			 * gives the user chance to intercept this event and perform actions
			 * on the \ref OpenIG::Base::Entity when it is added to the scene with
			 * \ref OpenIG::Base::ImageGenerator::addEntity
			 * \brief Hook called when \ref OpenIG::Base::Entity is added to the scene
			 * \param the \ref igplugincore::PluginContext
			 * \param the \ref OpenIG::Base::ImageGenerator::Entity ID assigned
			 * \param the osg::Node representing this \ref OpenIG::Base::ImageGenerator::Entity
			 * \param the filename of the model
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Fri Jan 16 2015
			 */
			virtual void entityAdded(PluginContext&, unsigned int, osg::Node&, const std::string&) {}

			/*! Hook called on the very beginning of the frame before any processing.
			* \brief Hook called on the very beginning of the frame before any processing
			* \param the \ref igplugincore::PluginContext
			* \author    Trajce Nikolov Nick openig@compro.net
			* \copyright (c)Compro Computer Services, Inc.
			* \date      Sat May 14 2016
			*/
			virtual void beginningOfFrame(PluginContext&) {}

			/*! Hook called on the end of the frame after all the frame processing.
			* \brief Hook called on the end of the frame after all the frame processing
			* \param the \ref igplugincore::PluginContext
			* \author    Trajce Nikolov Nick openig@compro.net
			* \copyright (c)Compro Computer Services, Inc.
			* \date      Sat May 14 2016
			*/
			virtual void endOfFrame(PluginContext&) {}

		protected:
			std::string     _library;       /*! \brief The shared library file name where this plugin is implemented */
			int             _orderNumber;   /*! \brief The order number of the plugin. See \ref igplugincore::Plugin::getOrderNumber */

			/*! Sets the shared library file name. \ref igplugincore::PluginHost is setting this internaly
			 * \brief Sets the shared library file name
			 * \param library
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Fri Jan 16 2015
			 */
			void setLibrary(const std::string& library)
			{
				_library = library;
			}

			/*! Sets the order number of the plugin. The order number of the plugin is set by \ref igplugincore::PluginHost
			 * when loading the plugins and in calls the plugin and their plugin hooks in sorted by this number fashion
			 * \brief Sets the plugin order number
			 * \param The order number of the plugin
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Fri Jan 16 2015
			 */
			void setOrderNumber(int orderNumber)
			{
				_orderNumber = orderNumber;
			}
		};

	}
} // namespace

#endif // PLUGIN_H
