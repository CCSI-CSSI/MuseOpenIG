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

#include "commands.h"
#include "stringutils.h"

#include <fstream>
#include <iostream>

#include <osg/Notify>

using namespace OpenIG::Base;

Commands::Commands()
{
}

Commands::~Commands()
{
}

Commands* Commands::instance()
{
	static Commands s_Commands;
	return &s_Commands;
}

void Commands::addCommand(const std::string& command, Commands::Command* cmd)
{
	_commands[command] = cmd;
}

void Commands::removeCommand(const std::string& command)
{
	CommandsMap::iterator itr = _commands.find(command);
	if (itr != _commands.end())
		_commands.erase(itr);
}

void Commands::setCommandExecCallback(CommandExecCallback* callback)
{
	_commandExecCallback = callback;
}


int Commands::exec(const std::string& commands)
{
	StringUtils::Tokens multiCommandsTokens = StringUtils::instance()->tokenize(commands, "&");
	StringUtils::TokensIterator itr = multiCommandsTokens.begin();
	for (; itr != multiCommandsTokens.end(); ++itr)
	{
		const std::string command = *itr;
		osg::notify(osg::NOTICE) << "Command: " << command << std::endl;

		StringUtils::Tokens tokens = StringUtils::instance()->tokenizeExtended(command);

		if (tokens.size())
		{
			std::string cmd = tokens.at(0);

			bool process = true;
			if (cmd.find_first_of('{') != std::string::npos && cmd.find_first_of('}') != std::string::npos)
			{
				StringUtils::Tokens conditionTokens = StringUtils::instance()->tokenize(cmd, "{}");

				if (conditionTokens.size())
				{
					std::string condition = conditionTokens.at(0);

					// Make this simple. For now we support only '==' and '!='
					// We are not going to do full support

					// First try '=='
					bool equals = true;
					conditionTokens = StringUtils::instance()->tokenize(condition, "=");

					// Then '!='
					if (conditionTokens.size() == 1)
					{
						equals = false;
						conditionTokens = StringUtils::instance()->tokenize(condition, "!=");
					}

					if (conditionTokens.size() == 2)
					{
						std::string env = conditionTokens.at(0);
						std::string val = conditionTokens.at(1);

						osg::notify(osg::NOTICE) << "ImageGenerator Core: Commands: Checking " << env << (equals ? "==" : "!=") << val;

						env = StringUtils::instance()->env(env);

						osg::notify(osg::NOTICE) << "\t" << env << (equals ? "==" : "!=") << val;

						switch (equals)
						{
						case true:
							process = env == val;
							break;
						case false:
							process = env != val;
							break;
						}

						osg::notify(osg::NOTICE) << "\t" << std::boolalpha << process << std::endl;
						if (!process)
							osg::notify(osg::NOTICE) << "\tcommand rejected: " << command << std::endl;
					}
				}

				tokens.erase(tokens.begin());
				cmd = tokens.at(0);
			}
			if (!process) continue;						

			tokens.erase(tokens.begin());

			CommandsMapIterator iter = _commands.find(cmd);
			if (iter != _commands.end())
			{
				iter->second.get()->exec(tokens);

				if (_commandExecCallback.valid())
				{
					_commandExecCallback->operator()(command);
				}

			}
		}
	}

	return -1;
}

void Commands::loadScript(const std::string& fileName)
{
    std::ifstream file;
    file.open(fileName.c_str(),std::ios::in);
    if (file.is_open())
    {
        std::string line;
        while (std::getline(file,line))
        {
            //If line starts with a blank or # , or is empty we ignore that line....
            if ( (line.size() < 2) || (line.at(0) == ' ') || (line.size() && line.at(0) == '#') ) continue;
            //if ( line.size() && line.at(0) == '#' ) continue;
            exec(line);
        }
        file.close();
    }
}

void Commands::clear()
{
    _commands.clear();
}
