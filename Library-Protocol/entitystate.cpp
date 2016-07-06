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
//#*	author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
//#*	copyright(c)Compro Computer Services, Inc.

#include <Library-Protocol/entitystate.h>

using namespace OpenIG::Library::Protocol;

EntityState::EntityState()
	: entityID(0)
{
}

int EntityState::write(OpenIG::Library::Networking::Buffer &buf) const
{
	buf << (unsigned char)opcode();
	buf << entityID;
	buf << mx(0, 0) << mx(0, 1) << mx(0, 2) << mx(0, 3);
	buf << mx(1, 0) << mx(1, 1) << mx(1, 2) << mx(1, 3);
	buf << mx(2, 0) << mx(2, 1) << mx(2, 2) << mx(2, 3);
	buf << mx(3, 0) << mx(3, 1) << mx(3, 2) << mx(3, 3);

	return sizeof(unsigned char) + sizeof(entityID) + sizeof(osg::Matrixd::value_type) * 16;
}

int EntityState::read(OpenIG::Library::Networking::Buffer &buf)
{
	unsigned char op;

	buf >> op;
	buf >> entityID;
	buf >> mx(0, 0) >> mx(0, 1) >> mx(0, 2) >> mx(0, 3);
	buf >> mx(1, 0) >> mx(1, 1) >> mx(1, 2) >> mx(1, 3);
	buf >> mx(2, 0) >> mx(2, 1) >> mx(2, 2) >> mx(2, 3);
	buf >> mx(3, 0) >> mx(3, 1) >> mx(3, 2) >> mx(3, 3);

	return sizeof(unsigned char) + sizeof(entityID) + sizeof(osg::Matrixd::value_type) * 16;
}