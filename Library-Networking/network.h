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

#ifndef NETWORK_H
#define NETWORK_H

#if defined(OPENIG_SDK)
	#include <OpenIG-Networking/export.h>
	#include <OpenIG-Networking/packet.h>
	#include <OpenIG-Networking/buffer.h>
	#include <OpenIG-Networking/parser.h>
	#include <OpenIG-Networking/network.h>
	#include <OpenIG-Networking/error.h>
#else
	#include <Library-Networking/export.h>
	#include <Library-Networking/packet.h>
	#include <Library-Networking/buffer.h>
	#include <Library-Networking/parser.h>
	#include <Library-Networking/network.h>
	#include <Library-Networking/error.h>
#endif

#include <boost/shared_ptr.hpp>

#include <map>
#include <sstream>

//
//Standard MTU 1500
//Standard Header size 64
//1500-64=1436 datagram size left over to use
#define BUFFER_SIZE 1436
//

namespace OpenIG {
	namespace Library {
		namespace Networking {

			class IGLIBNETWORKING_EXPORT Network
			{
			public:
				typedef std::map< Packet::Opcode, boost::shared_ptr<Packet::Callback> >		PacketCallbacks;

				Network();
				virtual ~Network();

				virtual void send(const Buffer&) = 0;
				virtual void receive(Buffer&, bool resetBuffer = true) = 0;

				void addCallback(Packet::Opcode, Packet::Callback*);
				void removeCallback(Packet::Opcode);

				void process(Packet&);
				void process();

				void setPort(unsigned int);
				void setParser(Parser*);

				Network& operator<<(const Packet&);

			protected:
				PacketCallbacks					_callbacks;
				unsigned int					_port;
				boost::shared_ptr<Parser>		_parser;

			public:
				static boost::shared_ptr<ErrorHandler>	log;
			};
		} // namespace
	} // namespace 
} // namespace

#endif // NETWORK_H
