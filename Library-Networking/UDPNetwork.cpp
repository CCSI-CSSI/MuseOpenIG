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
//#*	author    Trajce Nikolov Nick openig@compro.net
//#*	copyright(c)Compro Computer Services, Inc.
//#*
//#*    Please direct any questions or comments to the OpenIG Forums
//#*    Email address: openig@compro.net
//#*
//#*
//#*    Please direct any questions or comments to the OpenIG Forums
//#*    Email address: openig@compro.net
//#*

#include <iostream>

#include <Library-Networking/UDPNetwork.h>

#include <boost/asio.hpp>
#include <boost/array.hpp>

using namespace OpenIG::Library::Networking;

//
// Some of the incoming parameters are configured in the libOpenIG-Plugin-Networking.so.xml file
//
UDPNetwork::UDPNetwork(const std::string& host, const std::string &destination, bool broadcast, bool nonBlocking)
    : Network()
    , _senderSocket(0)
    , _recieverSocket(0)
    , _host(host)
    , _destination(destination)
    , _senderSocketInitiated(false)
    , _recieverSocketInitiated(false)
    , _broadcast(broadcast)
    , _nonBlocking(nonBlocking)
{
     std::ostringstream oss;
    //*log << oss << "Networking(1): UDP incoming host: " << _host << ", broadcast set to: " << _broadcast << std::endl;
}

UDPNetwork::~UDPNetwork()
{
    if (_senderSocket) delete _senderSocket;
    if (_recieverSocket) delete _recieverSocket;
}

void UDPNetwork::send(const Buffer& buffer)
{
    if (_senderSocket == 0 && !_senderSocketInitiated)
    {
        try
        {
            _senderSocketInitiated = true;

            _senderSocket = new boost::asio::ip::udp::socket(_senderIOService);
            std::ostringstream oss;

            _senderSocket->open(boost::asio::ip::udp::v4());
            _senderSocket->bind(boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(_host), 0));
            _senderSocket->set_option(boost::asio::socket_base::reuse_address(true));
            if(_broadcast)
            {
                _senderSocket->set_option(boost::asio::socket_base::broadcast(true));
                _senderBroadcastEndpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::broadcast(), _port);
            }
            else
                _senderBroadcastEndpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(_destination), _port);

            //*log << oss << "Networking(1): UDP bound to:" << _senderSocket->local_endpoint().address().to_string() << std::endl;

        }
        catch (std::exception& e)
        {
            std::ostringstream oss;
            *log << oss << "Networking(2): UDP send(buffer) socket setup -- exception thrown: " << e.what() << std::endl;

            if (_senderSocket)
            {
                delete _senderSocket;
                _senderSocket = 0;
            }
        }
    }

    if (_senderSocket)
    {
        try
        {
            boost::system::error_code ignored_error;
            size_t status;
            std::ostringstream oss;

            status = _senderSocket->send_to(boost::asio::buffer(buffer.getData(), buffer.getWritten()), _senderBroadcastEndpoint, 0, ignored_error);
            //*log << oss << "Networking(3): UDP send_to buffer.getSize: " << buffer.getSize() << ", buffer.getWritten: " <<  buffer.getWritten() << std::endl;
            //if(ignored_error.message() != "system:0")
            //    *log << oss << "Networking(4): UDP send_to status: " << status << ", error_code: " << ignored_error << std::endl;

        }
        catch (std::exception& e)
        {
            std::ostringstream oss;
            *log << oss << "Networking(5): UDP socket send_to exception thrown: " << e.what() << std::endl;
        }
    }

}

void UDPNetwork::receive(Buffer& buffer, bool resetBuffer)
{
    boost::system::error_code errorcode;
    size_t bytes_recv=0;

    if (_recieverSocket == 0 && !_recieverSocketInitiated)
    {
        _recieverSocketInitiated = true;
        try
        {
            boost::asio::ip::udp::endpoint endpoint;
            boost::asio::ip::address_v4 ipaddr;

            ipaddr.from_string(_host, errorcode);

            endpoint.address(ipaddr);
            endpoint.port(_port);

            _recieverSocket = new boost::asio::ip::udp::socket(_recieverIOService);
            _recieverSocket->open(endpoint.protocol(), errorcode);
            _recieverSocket->set_option(boost::asio::ip::udp::socket::reuse_address(true));
            _recieverSocket->bind(endpoint, errorcode);
            if (_nonBlocking) _recieverSocket->non_blocking(true);

            std::ostringstream oss;
            //*log << oss << "Networking(6): UDP bound to:" << _recieverSocket->local_endpoint().address().to_string() << std::endl;

        }
        catch (std::exception& e)
        {
            std::ostringstream oss;
            *log << oss << "Networking(7): recieverSocket setup exception thrown: " << e.what() << ", error code reported: " << errorcode << std::endl;

            if (_recieverSocket)
            {
                delete _recieverSocket;
                _recieverSocket = 0;
            }
        }
    }

    if (_recieverSocket)
    {
        try
        {
            Buffer rbuff(BUFFER_SIZE);
            boost::asio::ip::udp::endpoint	sendersEndpoint;
            boost::asio::mutable_buffers_1	buff((void*)rbuff.getData(), rbuff.getSize());

            bytes_recv = _recieverSocket->receive_from(buff, sendersEndpoint, 0, errorcode);
            if (bytes_recv)
            {
                buffer.write(rbuff.getData(), bytes_recv);
                if (resetBuffer) buffer.reset();
            }
        }
        catch (std::exception& e)
        {
            std::ostringstream oss;
            *log << oss << "Networking(8): receive_from exception thrown: error code: " << errorcode << ", bytes received: " << bytes_recv << std::endl;
            *log << oss << "Networking(8): receive_from exception thrown: " << e.what() << std::endl;
        }
    }
}
