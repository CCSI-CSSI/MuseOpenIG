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

#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <IgLib-Networking/export.h>
#include <IgLib-Networking/network.h>

#include <iostream>
#include <string>
#include <map>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

namespace iglib {
	namespace networking {

		class IGLIBNETWORKING_EXPORT TCPServer : public Network
		{
		public:
			explicit TCPServer(const std::string& host, unsigned port);
			virtual ~TCPServer();

			virtual void send(const Buffer&);
			virtual void receive(Buffer&);

			void getConnectedClients(std::vector<std::string>&);

		protected:

			class Connection : public boost::enable_shared_from_this<Connection>
			{
			public:
				typedef boost::shared_ptr<Connection> pointer;

				static pointer create(boost::asio::io_service& io_service)
				{
					return pointer(new Connection(io_service));
				}

				boost::asio::ip::tcp::socket& socket()
				{
					return _socket;
				}

				void setupSocket(boost::asio::ip::tcp::socket& socket);

				void send(const Buffer& buffer)
				{
					try
					{
						if (_socket.is_open())
						{
							if (!_socket_setup)
							{
								setupSocket(_socket);
							}

							boost::system::error_code ignore_error;
							size_t len = boost::asio::write(
								_socket, 
								boost::asio::buffer(buffer.getData(), buffer.getSize()),
								boost::asio::transfer_all(),
								ignore_error
							);

							if (!len)
							{
								throw std::runtime_error("Socket closed");
							}
                        }
					}
					catch (std::exception& e)
					{
						std::ostringstream oss;
						*Network::log << oss << "Networking: tcp server connection exception: " << e.what() << std::endl;

						_exception_thrown = true;
					}
				}

				bool exception_thrown() const
				{
					return _exception_thrown;
				}

			private:
				Connection(boost::asio::io_service& io_service)
					: _socket(io_service)
					, _exception_thrown(false)
					, _socket_setup(false)
				{
				}

				boost::asio::ip::tcp::socket		_socket;
				bool								_exception_thrown;
				bool								_socket_setup;

			};

			boost::asio::io_service								_io_service;
			boost::shared_ptr<boost::asio::ip::tcp::acceptor>	_acceptor;
			std::multimap<std::string,Connection::pointer>		_connections;
			boost::mutex										_mutex;
			boost::shared_ptr<boost::thread>					_thread;

			void run();
			void accept();
			void handle(Connection::pointer connection, const boost::system::error_code& error);

			void init();
			bool												_serverInitiated;
			std::string											_host;
		};
	} // namespace
} // namespace

#endif 
