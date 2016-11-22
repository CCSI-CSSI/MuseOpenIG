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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#if defined(OPENIG_SDK)
	#include <OpenIG-Base/Export.h>
#else
	#include <Core-Base/Export.h>
#endif

#include <string>
#include <map>

namespace OpenIG {
	namespace Base {
		/*! Handy singleton to get access to values from an XML file
		 * \brief The Configuration class
		 * \author    Trajce Nikolov Nick openig@compro.net
		 * \copyright (c)Compro Computer Services, Inc.
		 * \date      Sun Jan 11 2015
		 */
		class IGCORE_EXPORT Configuration
		{
		public:
			/*!
			 * \brief The Configuration singleton
			 * \return  The Configuration singleton
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Sun Jan 11 2015
			 */
			static Configuration*       instance();

			/*!
			 * \brief Reads an XML file section and fills in the values in a token based std::map
			 * \param The file name of the XML
			 * \param The section name from the XML
			 * \return true on success, false on failure
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Sun Jan 11 2015
			 */
			bool                        readFromXML(const std::string& fileName, const std::string& section);

			/*!
			 * \brief gets a value based on a given token and provides a default value if the token is not present
			 * \param token
			 * \return the value
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Sun Jan 11 2015
			 */
			const std::string           getConfig(const std::string& token, const std::string = "");

			/*!
			 * \brief gets a value based on a given token and provides a default value if the token is not present
			 * \param token
			 * \return the value
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Sun Jan 11 2015
			 */
			double                      getConfig(const std::string& token, double value = 0.0);

			/*!
			 * \brief gets a value based on a given token and provides a default value if the token is not present
			 * \param token
			 * \return the value
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Sun Jan 11 2015
			 */
			int                         getConfig(const std::string& token, int value = 0);

		protected:
			Configuration() {}
			~Configuration() {}

			typedef std::map< std::string, std::string >                    ConfigMap;
			typedef std::map< std::string, std::string >::iterator          ConfigMapIterator;
			typedef std::map< std::string, std::string >::const_iterator    ConfigMapConstIterator;

			/*! \brief token based std::map of tag values */
			ConfigMap   _configuration;

		};
	} // namespace
} // openig

#endif // CONFIGURATION_H
