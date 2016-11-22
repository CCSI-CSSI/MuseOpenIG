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

#ifndef FACTORY_H
#define FACTORY_H

#if defined(OPENIG_SDK)
    #include <OpenIG-Networking/Export.h>
    #include <OpenIG-Networking/Packet.h>
#else
    #include <Library-Networking/Export.h>
    #include <Library-Networking/Packet.h>
#endif

#include <boost/shared_ptr.hpp>

#include <map>

namespace OpenIG {
    namespace Library {
        namespace Networking {

            class IGLIBNETWORKING_EXPORT Factory
            {
            public:
                static Factory* instance();

                Packet* packet(Packet::Opcode opcode);
                void	addTemplate(Packet* packet);

            protected:
                Factory() {}
                ~Factory() {}

                typedef boost::shared_ptr<Packet>				PacketPointer;
                typedef std::map<Packet::Opcode, PacketPointer>	Packets;

                Packets	_packets;
            };
        }
    }
}

#endif
