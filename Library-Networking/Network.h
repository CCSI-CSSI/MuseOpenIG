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

#ifndef NETWORK_H
#define NETWORK_H

#if defined(OPENIG_SDK)
    #include <OpenIG-Networking/Export.h>
    #include <OpenIG-Networking/Packet.h>
    #include <OpenIG-Networking/Buffer.h>
    #include <OpenIG-Networking/Parser.h>
    #include <OpenIG-Networking/Network.h>
    #include <OpenIG-Networking/Error.h>
#else
    #include <Library-Networking/Export.h>
    #include <Library-Networking/Packet.h>
    #include <Library-Networking/Buffer.h>
    #include <Library-Networking/Parser.h>
    #include <Library-Networking/Network.h>
    #include <Library-Networking/Error.h>
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
                static boost::shared_ptr<ErrorHandler> log;
            };
        } // namespace
    } // namespace
} // namespace

#endif // NETWORK_H
