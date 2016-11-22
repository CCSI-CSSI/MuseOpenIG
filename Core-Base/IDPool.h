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


#ifndef IDPOOL_H
#define IDPOOL_H

#if defined(OPENOG_SDK)
	#include <OpenIG-Base/Export.h>
#else
	#include <Core-Base/Export.h>
#endif

#include <string>
#include <map>
#include <vector>

namespace OpenIG {
	namespace Base {

		/*! The inherits of \ref OpenIG::Base::ImageGenerator are exepcted to implement ID
		 *  based scene management of \ref OpenIG::Base::ImageGenerator::Entity, Lights .. etc.
		 *  There are situation when one will need an automated ID generations, for example
		 *  when reading a model definition from a file containing ID based constructs like
		 *  Lights. As a reference see the \ref ModelCompositionPlugin and \ref LightingPlugin
		 *  implementations/
		 * \brief Handy singleton class for ID numbers management
		 * \author    Trajce Nikolov Nick openig@compro.net
		 * \copyright (c)Compro Computer Services, Inc.
		 * \date      Sun Jan 11 2015
		 */
		class IGCORE_EXPORT IDPool
		{
		public:
			/*!
			 * \brief The singleton
			 * \return The singleton
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Sun Jan 11 2015
			 */
			static IDPool*  instance();

			/*!
			 * \brief Inits a ID group by a name, base ID and the size of this group
			 * \param The name of this ID group. You can have multiple groups identified by a name
			 * \param The base ID. All consequent IDs are from this one and on
			 * \param The size of the vector. If 10, then only 10 different IDs will be available
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Sun Jan 11 2015
			 */
			void initIdGroup(const std::string& group, unsigned int base, unsigned int size = 0);

			/*!
			 * \brief Gets the next ID from a ID group
			 * \param The name of the group ID
			 * \param The next ID available
			 * \return true on success, false if no more available IDs
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Sun Jan 11 2015
			 */
			bool getNextId(const std::string& group, unsigned int& id);

			/*! Sets list of IDs to be reused. Let explain it through an example. There is a
			 * plugin available, see \ref RunwayLights that creates real Lights for a runway model
			 * Once this is paged out in a paged visual database, the IDs of these lights can be
			 * reused for another runway sitting on another tile
			 * \brief Sets list of IDs to be reused
			 * \param The ID group name
			 * \param List of available IDs to be reused
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Sun Jan 11 2015
			 */
			void setAvailableIds(const std::string& group, const std::vector<unsigned int>& ids);

		protected:
			IDPool();
			~IDPool();

			/*!
			 * \brief The IdGroup struct. Internal for ID management
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Sun Jan 11 2015
			 */
			struct IdGroup
			{
				typedef std::vector<unsigned int>   IDs;

				unsigned int _base;
				IDs          _ids;

				IdGroup() : _base(0) {}
			};

			typedef std::map< std::string, IdGroup >    IdGroupMap;

			/*! \brief name based std::map of \ref IdGroup */
			IdGroupMap  _groups;
		};
	} // namespace
} // namespace

#endif // IDPOOL_H

