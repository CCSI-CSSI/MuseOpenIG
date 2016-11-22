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

// This work is based on Karel's work <karel.vaigl@gmail.com> from 2001
// slightly modified. Nick openig@compro.net>

#ifndef BUFFER_H
#define BUFFER_H

#if defined(OPENIG_SDK)
    #include <OpenIG-Networking/Export.h>
#else
    #include <Library-Networking/Export.h>
#endif

namespace OpenIG {
    namespace Library {
        namespace Networking {

            class IGLIBNETWORKING_EXPORT Buffer
            {
            public:
                Buffer(int size = 0, bool swapBytes = false, bool defaultToNetworkByteOrder = true);
                Buffer(const Buffer &);
                virtual ~Buffer();

                int write(const void *, int);
                int read(void *, int);

                void reset();
                void rewrite();

                void setData(const char *, int);
                void setSwapBytes(bool);
                void setSize(int);
                bool needToSwapBytesForNetworkByteOrder();

                const char*          getData() const;
                int                  getWritten() const;
                int                  getRest() const;
                int                  getSize() const;
                const unsigned char* fetch() const;

                virtual void print() const;

                Buffer &operator=(const Buffer &);

                Buffer &operator<<(float);
                Buffer &operator<<(double);
                Buffer &operator<<(unsigned int);
                Buffer &operator<<(int);
                Buffer &operator<<(long long);
                Buffer &operator<<(unsigned short int);
                Buffer &operator<<(unsigned char);
                Buffer &operator<<(const Buffer &);

                Buffer &operator>>(float &);
                Buffer &operator>>(double &);
                Buffer &operator>>(unsigned int &);
                Buffer &operator>>(int &);
                Buffer &operator>>(long long &);
                Buffer &operator>>(unsigned short int &);
                Buffer &operator>>(unsigned char &);
                Buffer &operator>>(Buffer &);

            protected:
                bool isShort(int) const;
                void resize(int);

                unsigned char*	_data;
                int				_len;
                int				_pos;
                int				_written;
                bool			_swapBytes;
            };
        } // namespace
    } // namespace
} // namespace

#define WRITE_BUFFER(b)		(*(const_cast<iglib::networking::Buffer *>(&b)))

#endif // BUFFER_H
