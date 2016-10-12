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
//#*	author    Trajce Nikolov Nick openig@compro.net
//#*	copyright(c)Compro Computer Services, Inc.
//#*
//#*    Please direct any questions or comments to the OpenIG Forums
//#*    Email address: openig@compro.net
//#*

#include <Library-Networking/network.h>

using namespace OpenIG::Library::Networking;

Network::Network()
{

}

Network::~Network()
{

}

void Network::addCallback(Packet::Opcode opcode, Packet::Callback* callback)
{
	_callbacks[opcode] = boost::shared_ptr<Packet::Callback>(callback);
}

void Network::removeCallback(Packet::Opcode opcode)
{
	PacketCallbacks::iterator itr = _callbacks.find(opcode);
	if (itr != _callbacks.end())
	{
		_callbacks.erase(itr);
	}
}

void Network::process(Packet& packet)
{
	PacketCallbacks::iterator itr = _callbacks.find(packet.opcode());
	if (itr != _callbacks.end())
	{
		itr->second->process(packet);
	}
}

Network &Network::operator<<(const Packet& packet)
{
	Buffer buffer;
	packet.write(buffer);

	send(buffer);

	return *this;
}

void Network::setPort(unsigned int port)
{
	_port = port;
}

void Network::setParser(Parser* parser)
{
	_parser = boost::shared_ptr<Parser>(parser);
}

void Network::process()
{
	if (_parser.get() == 0) return;

	Buffer buffer(BUFFER_SIZE);
	receive(buffer);
	if (buffer.getWritten() == 0) return;

	Packet* packet = 0;
    while ((packet = _parser->parse(buffer)))
	{
		process(*packet);
		delete packet;
	}
}


boost::shared_ptr<ErrorHandler>	Network::log( new COUTErrorHandler );
