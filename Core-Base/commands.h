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

#ifndef COMMANDS_H
#define COMMANDS_H

#if defined(OPENIG_SDK)
	#include <OpenIG-Base/stringutils.h>
	#include <OpenIG-Base/export.h>
#else
	#include <Core-Base/stringutils.h>
	#include <Core-Base/export.h>
#endif

#include <map>
#include <string>

#include <osg/ref_ptr>
#include <osg/Referenced>

namespace OpenIG {
	namespace Base {

		/*! This singleton gives the user ability to run commands.
		 * \brief The Commands singleton class
		 * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
		 * \copyright (c)Compro Computer Services, Inc.
		 * \date      Sun Jan 11 2015
		 */
		class IGCORE_EXPORT Commands
		{
		protected:
			/*!
			 * \brief Constructor
			 * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Sun Jan 11 2015
			 */
			Commands();

			/*!
			 * \brief Destructor
			 * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Sun Jan 11 2015
			 */
			~Commands();

		public:
			/*! Abstract for a single command
			 * \brief The Command class
			 * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Sun Jan 11 2015
			 */
			class Command : public osg::Referenced
			{
			public:
				/*! Inheritants are expected to implement the command execution
				 * \brief Custom command execution implementation
				 * \param tokens    Contains the arguments
				 * \return          Should return 0 on succes, -1 on failure
				 * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
				 * \copyright (c)Compro Computer Services, Inc.
				 * \date      Sun Jan 11 2015
				 */
				virtual int exec(const StringUtils::Tokens& tokens) = 0;

				/*!
				 * \brief Gets the command usage, brief, like the syntax
				 * \return  Brief description on how to use this command
				 * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
				 * \copyright (c)Compro Computer Services, Inc.
				 * \date      Sun Jan 11 2015
				 */
				virtual const std::string getUsage() const = 0;

				/*!
				 * \brief Gets a detailed command description
				 * \return The description
				 * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
				 * \copyright (c)Compro Computer Services, Inc.
				 * \date      Sun Jan 11 2015
				 */
				virtual const std::string getDescription() const = 0;

				/*!
				* \brief Gets format of all the arguments. This is the all available
				*	formats encoded:
				*		D - float
				*		I - integer
				*		S - String
				*		{}- List, example: {item1;item2;...}
				*		F - File name
				*		B - boolean
				*		P - position, x y z
				*		O - orientation, h p r
				*		C - color, r g b
				*	an example format can be like this: "F:I:S:{item1;item2;...}:F:B"
				* \return The arguments format
				* \author    Trajce Nikolov Nick <trajce.nikolov.nick@gmail.com>
				* \copyright (c)Compro Computer Services, Inc.
				* \date      Sat Nov 14 2015
				*/
				virtual const std::string getArgumentsFormat() const = 0;
			};

			/*!
			 * \brief   The Commands singleton
			 * \return  The Commands singleton
			 * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Sun Jan 11 2015
			 */
			static Commands*	instance();

			/*!
			 * \brief Performs execution of a command
			 * \param command The command with its arguments
			 * \return see \ref Command::exec
			 * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Sun Jan 11 2015
			 */
			int					exec(const std::string& command);

			/*!
			 * \brief Add a command
			 * \param command The name of the command. You use it to invoke the command
			 * \param cmd Instance of a command
			 * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Sun Jan 11 2015
			 */
			void				addCommand(const std::string& command, Command* cmd);

			/*!
			 * \brief Loads a text file with commands in and perform their execition
			 * \param fileName The file name of the script
			 * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Sun Jan 11 2015
			 */
			void                loadScript(const std::string& fileName);

			/*!
			 * \brief Clears the commands map
			 * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Sun Jan 11 2015
			 */
			void                clear();

			typedef osg::ref_ptr<Command>											CommandPtr;
			typedef std::map<std::string, osg::ref_ptr<Command> >					CommandsMap;
			typedef std::map<std::string, osg::ref_ptr<Command> >::iterator			CommandsMapIterator;
			typedef std::map<std::string, osg::ref_ptr<Command> >::const_iterator	CommandsMapConstIterator;

			/*!
			 * \brief Gives the current commands
			 * \return Gives the current commands map
			 * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Sun Jan 11 2015
			 */
			const CommandsMap& getCommands() const
			{
				return _commands;
			}

			/*!
			 * \brief Removes a command
			 * \param command The name of the command to be removed
			 * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Sun Oct 18 2015
			 */
			void removeCommand(const std::string& command);

		protected:
			/*! \brief name based std::map of commands added */
			CommandsMap		_commands;

		};
	} // namespace
} // namespace

#endif // COMMANDS_H
