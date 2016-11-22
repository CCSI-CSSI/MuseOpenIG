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

#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#if defined(OPENIG_SDK)
	#include <OpenIG-Base/Export.h>
#else
	#include <Core-Base/Export.h>
#endif

#include <string>
#include <vector>
#include <algorithm> 
#include <cctype>
#include <functional>

namespace OpenIG {
	namespace Base {

		/*! Handy class to use wit std::strings
		 * \brief The singleton StringUtils class
		 * \author    Trajce Nikolov Nick openig@compro.net
		 * \copyright (c)Compro Computer Services, Inc.
		 * \date      Sun Jan 11 2015
		 */
		class IGCORE_EXPORT StringUtils
		{
		protected:
			StringUtils();
			~StringUtils();

		public:
			/*!
			 * \brief The singleton
			 * \return The singleton
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Sun Jan 11 2015
			 */
			static StringUtils* instance();

			typedef std::vector<std::string>					Tokens;
			typedef std::vector<std::string>::iterator			TokensIterator;
			typedef std::vector<std::string>::const_iterator	TokensConstIterator;

			typedef std::vector< std::string >                  StringList;
			typedef std::vector< std::string >::iterator        StringListIterator;
			typedef std::vector< std::string >::const_iterator  StringListConstIterator;

			/*!
			 * \brief Creates tokens std::vector<std::string> from a string and given delimiters
			 * \param str The input string to tokenize
			 * \param delimiters String of delimiter chars
			 * \return std::vector<std::string> of tokens found
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Sun Jan 11 2015
			 */
			Tokens tokenize(const std::string& str, const std::string& delimiters = " ");

			/*!
			* \brief Creates tokens std::vector<std::string> from a string and "" delimiters
			* \param str The input string to tokenize
			* \param delimiters String of delimiter chars
			* \return std::vector<std::string> of tokens found
			* \author    Trajce Nikolov Nick openig@compro.net
			* \copyright (c)Compro Computer Services, Inc.
			* \date      Tue Jun 30 2015
			*/
			Tokens tokenizeExtended(const std::string& str);

			/*!
			 * \brief trim a string from left
			 * \param s             The text to trim
			 * \param istestchar    Function to test a char if it is a space char
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Sun Jan 11 2015
			 */
			template<typename T>
			inline std::string &ltrim(std::string &s, T istestchar = std::isspace)
			{
				s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(istestchar)));
				return s;
			}

			/*!
			 * \brief trim a string from right
			 * \param s             The text to trim
			 * \param istestchar    Function to test a char if it is a space char
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Sun Jan 11 2015
			 */
			template<typename T>
			inline std::string &rtrim(std::string &s, T istestchar = std::isspace)
			{
				s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(istestchar)).base(), s.end());
				return s;
			}

			/*!
			 * \brief trim a string from left and right
			 * \param s             The text to trim
			 * \param istestchar    Function to test a char if it is a space char
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Sun Jan 11 2015
			 */
			template<typename T>
			inline std::string &trim(std::string &s, T istestchar = std::isspace)
			{
				return ltrim(rtrim(s, istestchar), istestchar);
			}

			/*!
			 * \brief Computes the number of lines in a string
			 * \param text the input string
			 * \return number of lines in the input string
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Sun Jan 11 2015
			 */
			unsigned int numberOfLines(const std::string& text);

			/*!
			* \brief Returns the value of an ENV variable, provided in form $ENV_VAR
			* \param var the ENV variable, provided in form $ENV_VAR
			* \return the value of an ENV variable
			* \author    Trajce Nikolov Nick openig@compro.net
			* \copyright (c)Compro Computer Services, Inc.
			* \date      Fri Oct 23 2015
			*/
			std::string env(const std::string& var);
		};
	} // namespace
} // namespace

#endif // STRINGUTILS_H
