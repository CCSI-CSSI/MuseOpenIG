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

#include <IgLib-Networking/tcpserver.h>

#include <boost/asio.hpp>
#include <boost/array.hpp>

using namespace iglib::networking;

TCPServer::TCPServer(const std::string& host, unsigned port)
	: Network()
	, _serverInitiated(false)
	, _host(host)
{
	setPort(port);
}

TCPServer::~TCPServer()
{
	_io_service.stop();

	_mutex.lock();
	_connections.clear();
	_mutex.unlock();

	if (_thread) _thread->join();
}

void TCPServer::init()
{
	if (!_serverInitiated)
	{
		_serverInitiated = true;

		try
		{
			_acceptor = boost::shared_ptr<boost::asio::ip::tcp::acceptor>(
				new boost::asio::ip::tcp::acceptor(_io_service,
                    boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(_host), _port) )
                );

			accept();

			_thread = boost::shared_ptr<boost::thread>(new boost::thread(&TCPServer::run, this));
		}
		catch (std::exception& e)
		{
            if (_acceptor.get())
			{
                _acceptor.reset();
			}
			std::ostringstream oss;
			*log << oss << "Networking: tcp server exception: " << e.what() << std::endl;
		}
	}
}

void TCPServer::send(const Buffer& buffer)
{
	init();

	_mutex.lock();

	unsigned erased = 0;
	std::multimap<std::string, Connection::pointer>::iterator itr = _connections.begin();
	while (itr != _connections.end())
	{
		itr->second->send(buffer);
		if (itr->second->exception_thrown())
		{
			++erased;

			std::multimap<std::string, Connection::pointer>::iterator savedItr = itr;
			++savedItr;
			_connections.erase(itr);
			itr = savedItr;

			std::ostringstream oss;
			*log << oss << "TCPServer::send() -- Exception thrown" << std::endl;
		}
		else
		{
			++itr;
		}
	}
	if (erased)
	{
		std::ostringstream oss;
		*log << oss << "Networking: tcp server #connections: " << _connections.size() - 1 << std::endl;
	}

	_mutex.unlock();
}
void TCPServer::receive(Buffer& buffer)
{

	boost::asio::mutable_buffers_1	buff((void*)buffer.getData(), buffer.getSize());
	boost::system::error_code error;
	try
	{
		_mutex.lock();

		std::multimap<std::string, Connection::pointer>::iterator itr = _connections.begin();
		for (; itr != _connections.end(); ++itr)
		{
			Connection::pointer connection = itr->second;

			Buffer cbuff;
			boost::asio::mutable_buffers_1	buff((void*)cbuff.getData(), cbuff.getSize());
			boost::system::error_code error;

			size_t len = connection->socket().read_some(buff, error);

			buffer << cbuff;

			if (error)
				throw boost::system::system_error(error);

		}
		_mutex.unlock();
		
		
	}
	catch (std::exception& e)
	{
		_mutex.unlock();

		std::ostringstream oss;
		*log << oss << "Networking: tcp server exception: " << e.what() << std::endl;
	}

}

void TCPServer::run()
{
	try
	{
		{
			std::ostringstream oss;
			*log << oss << "Networking: tcp server started" << std::endl;
		}
		_io_service.run();
		{
			std::ostringstream oss;
			*log << oss << "Networking: tcp server shutdown" << std::endl;
		}
	}
	catch (std::exception& e)
	{
		std::ostringstream oss;
		*log << oss << "Networking: tcp server networking exception:" << e.what() << std::endl;
	}
}
void TCPServer::accept()
{
	Connection::pointer connection = Connection::create(_io_service);

	_mutex.lock();
	_connections.insert(std::pair<std::string,Connection::pointer>("",connection));

	std::ostringstream oss;
	*log << oss << "Networking: tcp server #connections: " << _connections.size() - 1 << std::endl;

	_mutex.unlock();

	if (_acceptor.get())
	{
		_acceptor->async_accept(connection->socket(),
			boost::bind(&TCPServer::handle, this, connection,
			boost::asio::placeholders::error));
	}
}

void TCPServer::handle(TCPServer::Connection::pointer connection, const boost::system::error_code& error)
{
	if (!error)
	{
		std::multimap<std::string, Connection::pointer>::iterator itr = _connections.find("");

		_connections.insert(
			std::pair<std::string, Connection::pointer>(
				connection->socket().remote_endpoint().address().to_string(), connection
			)
		);
		_connections.erase(itr);

		accept();
	}
	else
	{
		std::ostringstream oss;
		*log << oss << "Networking: tcp server error: " << error << std::endl;
	}
}

void TCPServer::getConnectedClients(std::vector<std::string>& clients)
{
	clients.clear();

	_mutex.lock();
	
	std::multimap<std::string, Connection::pointer>::iterator itr = _connections.begin();
	for (; itr != _connections.end(); ++itr)
	{
		if (itr->first.length()) clients.push_back(itr->first);
	}

	_mutex.unlock();
}
void TCPServer::Connection::setupSocket(boost::asio::ip::tcp::socket& socket)
{
	_socket_setup = true;
	_socket.set_option(boost::asio::ip::tcp::no_delay(true));
}

