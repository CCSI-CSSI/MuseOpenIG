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
//#*	authors		Trajce Nikolov Nick openig@compro.net
//#*				Pelle Nordqvist nordqvist@gmail.com
//#*	copyright(c)Compro Computer Services, Inc.
//#*
//#*    Please direct any questions or comments to the OpenIG Forums
//#*    Email address: openig@compro.net
//#*
//#*

#pragma once

#if defined(OPENIG_SDK)
#include <OpenIG-Networking/Buffer.h>
#include <OpenIG-Networking/Packet.h>
#include <OpenIG-Protocol/Export.h>
#include <OpenIG-Protocol/Opcodes.h>
#else
#include <Library-Networking/Buffer.h>
#include <Library-Networking/Packet.h>
#include <Library-Protocol/Export.h>
#include <Library-Protocol/Opcodes.h>
#endif

#include <osg/Vec3d>
#include <osg/Vec3>

namespace OpenIG {
	namespace Library {
		namespace Protocol {

			struct IGLIBPROTOCOL_EXPORT DeadReckonEntityState : public OpenIG::Library::Networking::Packet
			{
				DeadReckonEntityState();

				META_Packet(OPCODE_DEADRECKON_ENTITYSTATE, DeadReckonEntityState);

				virtual int write(OpenIG::Library::Networking::Buffer &buf) const;
				virtual int read(OpenIG::Library::Networking::Buffer &buf);

				int			entityID;				
				osg::Vec3	positionalVelocity;
				osg::Vec3	orientationalVelocity;
			};

		}
	}
}
