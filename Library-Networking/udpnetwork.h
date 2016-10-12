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

#ifndef UDPNETWORK_H
#define UDPNETWORK_H

#if defined(OPENIG_SDK)
    #include <OpenIG-Networking/export.h>
    #include <OpenIG-Networking/network.h>
#else
    #include <Library-Networking/export.h>
    #include <Library-Networking/network.h>
#endif

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/udp.hpp>


namespace OpenIG {
    namespace Library {
        namespace Networking {

            class IGLIBNETWORKING_EXPORT UDPNetwork : public Network
            {
            public:
                explicit UDPNetwork(const std::string& host, const std::string& destination="", bool broadcast=true);
                virtual ~UDPNetwork();

                virtual void send(const Buffer&);
                virtual void receive(Buffer&, bool resetBuffer = true);

            protected:
                boost::asio::io_service			_senderIOService;
                boost::asio::ip::udp::socket*	_senderSocket;
                boost::asio::ip::udp::endpoint	_senderBroadcastEndpoint;

                boost::asio::io_service			_recieverIOService;
                boost::asio::ip::udp::socket*	_recieverSocket;

                std::string						_host;
                std::string                     _destination;
                bool							_senderSocketInitiated;
                bool							_recieverSocketInitiated;
                bool                            _broadcast;
            };
        } // namespace
    } // namespace
} // namespace

#endif // UDPNETWORK_H
