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

#ifndef UDPNETWORK_H
#define UDPNETWORK_H

#include <IgLib-Networking/export.h>
#include <IgLib-Networking/network.h>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/udp.hpp>


namespace iglib {
	namespace networking {

		class IGLIBNETWORKING_EXPORT UDPNetwork : public Network
		{
		public:		
			explicit UDPNetwork(const std::string& host);
			virtual ~UDPNetwork();

			virtual void send(const Buffer&);
			virtual void receive(Buffer&);

		protected:
			boost::asio::io_service			_senderIOService;
			boost::asio::ip::udp::socket*	_senderSocket;
			boost::asio::ip::udp::endpoint	_senderBroadcastEndpoint;

			boost::asio::io_service			_recieverIOService;
			boost::asio::ip::udp::socket*	_recieverSocket;

			std::string						_host;
			bool							_senderSocketInitiated;
			bool							_recieverSocketInitiated;
		};
	} // namespace
} // namespace

#endif // UDPNETWORK_H
