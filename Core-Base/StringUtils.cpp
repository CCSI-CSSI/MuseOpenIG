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
#include "StringUtils.h"
#include <stdlib.h>

using namespace OpenIG::Base;

StringUtils::StringUtils()
{
}

StringUtils::~StringUtils()
{
}

StringUtils* StringUtils::instance()
{
	static StringUtils s_StringUtils;
	return &s_StringUtils;
}

StringUtils::Tokens StringUtils::tokenizeExtended(const std::string& str)
{
	StringUtils::Tokens tokens = StringUtils::instance()->tokenize(str, "\"");

	unsigned int remainder = tokens.size() % 2;
	if (remainder && tokens.size() != 1)
	{
		StringUtils::Tokens newtokens;

		bool decompose = false;

		std::string s = str;
		std::string::size_type pos = s.find_first_of("\"");
		while (pos != std::string::npos)
		{
			std::string token = s.substr(0, pos);

			decompose = !decompose;

			if (decompose)
			{
				StringUtils::Tokens t = StringUtils::instance()->tokenize(token);
				StringUtils::Tokens::iterator itr = t.begin();
				for (; itr != t.end(); ++itr)
				{
					newtokens.push_back(*itr);
				}
			}
			else
			{
				newtokens.push_back(token);
			}

			s = s.substr(pos, s.length() - pos);
			s.erase(s.begin());

			pos = s.find_first_of("\"");
		}
		std::string token = s;

		StringUtils::Tokens t = StringUtils::instance()->tokenize(token);
		StringUtils::Tokens::iterator itr = t.begin();
		for (; itr != t.end(); ++itr)
		{
			newtokens.push_back(*itr);
		}

		tokens = newtokens;
	}
	else
	{
		tokens = StringUtils::instance()->tokenize(str);
	}

	return tokens;
}

StringUtils::Tokens StringUtils::tokenize(const std::string& str, const std::string& delimiters)
{
	Tokens tokens;
    std::string::size_type delimPos = 0, tokenPos = 0, pos = 0;

	if(str.length()<1)  return tokens;
	while(1)
	{
		delimPos = str.find_first_of(delimiters, pos);
		tokenPos = str.find_first_not_of(delimiters, pos);
		if (tokenPos != std::string::npos && str[tokenPos]=='\"')
		{
			delimPos = str.find_first_of("\"", tokenPos+1);
			pos++;
		}

		if(std::string::npos != delimPos)
		{
			if(std::string::npos != tokenPos)
			{
				if(tokenPos<delimPos)
				{
					std::string token = str.substr(pos,delimPos-pos);
					if (token.length()) tokens.push_back(token);
				}
			}
			pos = delimPos+1;
		}
		else
		{
			if(std::string::npos != tokenPos)
			{
				std::string token = str.substr(pos);
				if (token.length()) tokens.push_back(token);
			}
			break;
		}
	}
	return tokens;
}

unsigned int StringUtils::numberOfLines(const std::string& text)
{
    if (text.empty()) return 0;

    unsigned int numOfLines = 1;

    std::string::size_type pos = text.find_first_of('\n');
    while (pos != std::string::npos)
    {
        ++numOfLines;
        pos = text.find_first_of('\n',pos+1);
    }

    return numOfLines;
}

std::string StringUtils::env(const std::string& var)
{
	std::string result;
	if (var.size() && var.at(0) == '$')
	{
		std::string envVar = var;
		envVar.erase(envVar.begin());

#if defined (__linux) || defined (__APPLE__)
		char * value = NULL;
		value = getenv(envVar.c_str());
		if (value)
		{
			result = std::string(value);
		}
#elif   defined (_WIN32)
		size_t requiredSize;

		//Use win32 safe version of getenv()
		getenv_s(&requiredSize, NULL, 0, envVar.c_str());
		if (requiredSize)
		{
			char* value = (char*)malloc(requiredSize * sizeof(char));
			if (value)
			{
				getenv_s(&requiredSize, value, requiredSize, envVar.c_str());
				result = std::string(value);
				free(value);
			}
		}
#endif
	}
	return result;
}

