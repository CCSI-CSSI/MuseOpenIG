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
#include "stringutils.h"

using namespace igcore;

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

StringUtils::Tokens StringUtils::tokenize(const std::string& str, const std::string& delimiters)
{
	Tokens tokens;
	std::string::size_type delimPos = 0, tokenPos = 0, pos = 0, q = 0;

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
