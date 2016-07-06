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


#include <Library-Protocol/tod.h>

using namespace OpenIG::Library::Protocol;

TOD::TOD()
	: hour(0)
	, minutes(0)
{
}

int TOD::write(OpenIG::Library::Networking::Buffer &buf) const
{
	buf << (unsigned char)opcode();
	buf << hour;
	buf << minutes;

	return sizeof(unsigned char) + 2 * sizeof(unsigned int);
}

int TOD::read(OpenIG::Library::Networking::Buffer &buf)
{
	unsigned char op;

	buf >> op;
	buf >> hour;
	buf >> minutes;

	return sizeof(unsigned char) + 2*sizeof(unsigned int);
}