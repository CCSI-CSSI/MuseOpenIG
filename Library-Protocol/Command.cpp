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


#include <Library-Protocol/Command.h>

using namespace OpenIG::Library::Protocol;

Command::Command()
{
}

int Command::write(OpenIG::Library::Networking::Buffer &buf) const
{
    buf << (unsigned char)opcode();

    unsigned int len = command.length();
    buf << len;

    if (!command.empty())
    {
        buf.write(command.c_str(), command.length());
    }
    return sizeof(unsigned char) + sizeof(unsigned int) + len;
}

int Command::read(OpenIG::Library::Networking::Buffer &buf)
{
    unsigned char op;

    buf >> op;

    unsigned int len = 0;
    buf >> len;

    if (len)
    {
#if 0
        command.resize(len);
        // Older STL is not supporting this
        buf.read(&command.front(), len);
#else
        // Do it the old C style
        char* str = new char[len+1];
        buf.read(str, len);
        str[len] = 0;

        command = std::string(str);
        delete[] str;
#endif
    }

    return sizeof(unsigned char) + sizeof(unsigned int) + len;
}
