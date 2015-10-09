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

#ifndef PACKET_H
#define PACKET_H

#include <IgLib-Networking/export.h>
#include <IgLib-Networking/buffer.h>

namespace iglib {
	namespace networking {

		class IGLIBNETWORKING_EXPORT Packet
		{
        public:
            Packet() {}
            virtual ~Packet() {}

			typedef unsigned int Opcode;

			virtual Opcode	opcode() const = 0;
			virtual int		write(Buffer &buf) const = 0;			
			virtual int		read(Buffer &buf) = 0;				

			Buffer& operator<<(Buffer&);

			class IGLIBNETWORKING_EXPORT Callback
			{
			public:
				virtual void process(Packet&) = 0;
			};
			
		};
	} // namespace
} // namespace

#endif // PACKET_H
