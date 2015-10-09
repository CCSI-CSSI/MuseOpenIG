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

#include <iostream>

#include <IgLib-Networking/tcpclient.h>

#include <boost/asio.hpp>
#include <boost/array.hpp>

using namespace iglib::networking;

TCPClient::TCPClient(const std::string& host, const std::string& server)
	: Network()
	, _server(server)
	, _socket(0)
	, _setup_socket(false)
	, _host(host)
{
}

TCPClient::~TCPClient()
{
	if (_socket) delete _socket;
}

void TCPClient::send(const Buffer& buffer)
{
	if (_socket == 0)
	{
		createSocket();
	}

	if (_socket)
	{
		boost::asio::mutable_buffers_1	buff((void*)buffer.getData(), buffer.getSize());
		boost::system::error_code error;

		try
		{
			_socket->write_some(buff);
		}
		catch (std::exception& e)
		{
			std::ostringstream oss;
			*log << oss << "Networking: tcp client exception: " << e.what() << std::endl;
		}
	}
}

void TCPClient::receive(Buffer& buffer)
{
	if (_socket == 0)
	{
		createSocket();
	}	

	if (_socket)
	{
		const boost::asio::mutable_buffers_1	buff((void*)buffer.getData(), buffer.getSize());
		boost::system::error_code error;

		try
		{
			setupSocket(*_socket);
			size_t len = _socket->read_some(buff, error);

			if (error)
				throw boost::system::system_error(error); 
		}
		catch (std::exception& e)
		{
			std::ostringstream oss;
			*log << oss << "Networking: tcp client exception: " << e.what() << std::endl;
		}
	}
}

void TCPClient::createSocket()
{
	try
	{
		boost::asio::ip::tcp::resolver resolver(_io_service);

		std::ostringstream oss;
		oss << _port;

		_socket = new boost::asio::ip::tcp::socket(_io_service);

		setupSocket(*_socket);

		_socket->open(boost::asio::ip::tcp::v4());
		_socket->bind(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(_host), 0));
		_socket->connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(_server), atoi(oss.str().c_str())));
	}
	catch (std::exception& e)
	{
		if (_socket)
		{
			delete _socket;
			_socket = 0;
		}
		std::ostringstream oss;
		*log << oss << "Networking: tcp client exception: " << e.what() << std::endl;
	}
}

void TCPClient::setupSocket(boost::asio::ip::tcp::socket& socket)
{
	if (!_setup_socket)
	{
		_setup_socket = true;
		socket.set_option(boost::asio::ip::tcp::no_delay(true));
	}
}
