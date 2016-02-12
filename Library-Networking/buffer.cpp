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

// This work is based on Karel's work <karel.vaigl@gmail.com> from 2004
// slightly modified. Nick <trajce.nikolov.nick@gmail.com>

#if !defined(_WIN32)
 #ifndef __APPLE__
  #include <endian.h>
 #else
  #include <machine/endian.h>
 #endif
#endif

#include <string>
#include <iostream>
#include <cassert>

#include <Library-Networking/buffer.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

using namespace OpenIG::Library::Networking;

template<class T>
T revert_bytes(const T arg)
{
    union
    {
        T t;
        char c[sizeof(T)];
    } r;

    r.t = arg;

    int l = sizeof(T);

    for (int i = 0; i < l / 2; i++)
    {
        char a = r.c[i];
        r.c[i] = r.c[l - i - 1];
        r.c[l - i - 1] = a;
    }

    return r.t;
};

Buffer::Buffer(int size, bool swapBytes, bool defaultToNetworkByteOrder)
{
	_data = NULL;

	if (size > 0)
		resize(size);
	else
		_len = 0;

	_pos = 0;
	_written = 0;

	if (defaultToNetworkByteOrder)
		_swapBytes = needToSwapBytesForNetworkByteOrder();
	else
		_swapBytes = swapBytes;
}

Buffer::Buffer(const Buffer &other) 
{
	_len = other._len;
	_written = other._written;
	_pos = other._pos;
	_swapBytes = other._swapBytes;
	_data = NULL;

	if (other._data) {
		resize(_len);
		memcpy(_data, other._data, _len);
	}
}

Buffer::~Buffer()
{
	if (_data)
		free(_data);
}

// Macro taken from here, slightly modified
// http://stackoverflow.com/questions/2100331/c-macro-definition-to-determine-big-endian-or-little-endian-machine
// Nick
bool Buffer::needToSwapBytesForNetworkByteOrder()
{
	static uint32_t endianness = 0xdeadbeef;
	enum Endianness { BIG, LITTLE, UNKNOWN };

	Endianness endiannessOnThisMachine = *(const char *)&endianness == 0xef ? LITTLE : *(const char *)&endianness == 0xde ? BIG : UNKNOWN;

	return endiannessOnThisMachine != BIG;
}

void Buffer::setSwapBytes(bool swap)
{
	_swapBytes = swap;
}

int Buffer::write(const void *src, int length) 
{
	if (isShort(length))
		resize(_pos + length);
	if (_data && src) {
		memcpy(_data + _pos, src, length);
		_pos += length;
		_written += length;
		return length;
	}
	return 0;
}

int Buffer::read(void *dst, int length) 
{
	if (isShort(length))
		return -1;
	if (_data && dst) {
		memcpy(dst, _data + _pos, length);
		_pos += length;
		return length;
	}
	return 0;
}

const unsigned char* Buffer::fetch() const
{
	return _data + _pos;
}

bool Buffer::isShort(int length) const
{
	if (_pos + length > _len)
		return true;
	else
		return false;
}

int Buffer::getSize() const
{
	return _len;
}

void Buffer::resize(int a_Size)
{
	if (_data)
		_data = reinterpret_cast<unsigned char *>(realloc(_data, a_Size));
	else
		_data = reinterpret_cast<unsigned char *>(malloc(a_Size));
	_len = a_Size;
}

void Buffer::reset() 
{
	_pos = 0;
}

void Buffer::rewrite()
{
	_pos = 0;
	_written = 0;
}

void Buffer::setData(const char *buf, int len)
{
	resize(len);
	if (_data) {
		memcpy(_data, buf, len);
		_written = len;
	}
}

const char *Buffer::getData() const
{
	return reinterpret_cast<char *>(_data);
}

int Buffer::getRest() const
{
	return _written - _pos;
}

int Buffer::getWritten() const 
{
	return _written;
}

void Buffer::print() const
{
	std::string tmp = "0123456789abcdef";
	for (int i = 0; i < _len; i++)
	{
		std::cout << (tmp[_data[i] >> 4]) << (tmp[_data[i] & 0x0f]);
		if ((i + 1) % 16 == 0)
			std::cout << std::endl;
		else
			std::cout << " ";
	}
	std::cout << std::endl;
}

Buffer &Buffer::operator=(const Buffer &other)
{
	_len = other._len;
	_written = other._written;
	_pos = other._pos;
	_data = NULL;
	_swapBytes = other._swapBytes;

	if (other._data) {
		resize(_len);
		memcpy(_data, other._data, _len);
	}

	return *this;
}

Buffer &Buffer::operator<<(float f) 
{
	float rf = _swapBytes ? revert_bytes(f) : f;
	write(&rf, sizeof(float));
	return *this;
}

Buffer &Buffer::operator<<(double d)
{
	double rd = _swapBytes ? revert_bytes(d) : d;
	write(&rd, sizeof(double));
	return *this;
}

Buffer &Buffer::operator<<(unsigned int i)
{
	unsigned int ri = _swapBytes ? revert_bytes(i) : i;
	write(&ri, sizeof(unsigned int));
	return *this;
}

Buffer &Buffer::operator<<(int i) 
{
	int ri = _swapBytes ? revert_bytes(i) : i;
	write(&ri, sizeof(int));
	return *this;
}

Buffer &Buffer::operator<<(long long i)
{
	long long ri = _swapBytes ? revert_bytes(i) : i;
	write(&ri, sizeof(long long));
	return *this;
}

Buffer &Buffer::operator<<(unsigned short int i)
{
	unsigned short int ri = _swapBytes ? revert_bytes(i) : i;
	write(&ri, sizeof(unsigned short int));
	return *this;
}

Buffer &Buffer::operator<<(const Buffer &buf) 
{
	write(buf.getData(), buf.getWritten());
	return *this;
}

Buffer &Buffer::operator<<(unsigned char c)
{
	write(&c, sizeof(char));
	return *this;
}

Buffer &Buffer::operator>>(float &f)
{
	float rf;
	read(&rf, sizeof(float));
	f = _swapBytes ? revert_bytes(rf) : rf;
	return *this;
}

Buffer &Buffer::operator>>(double &d)
{
	double rd;
	read(&rd, sizeof(double));
	d = _swapBytes ? revert_bytes(rd) : rd;
	return *this;
}

Buffer &Buffer::operator>>(unsigned int &i)
{
	unsigned int ri;
	read(&ri, sizeof(unsigned int));
	i = _swapBytes ? revert_bytes(ri) : ri;
	return *this;
}

Buffer &Buffer::operator>>(int &i)
{
	int ri;
	read(&ri, sizeof(int));
	i = _swapBytes ? revert_bytes(ri) : ri;
	return *this;
}

Buffer &Buffer::operator>>(long long &i)
{
	long long ri;
	read(&ri, sizeof(long long));
	i = _swapBytes ? revert_bytes(ri) : ri;
	return *this;
}

Buffer &Buffer::operator>>(unsigned short int &i)
{
	unsigned short int ri;
	read(&ri, sizeof(unsigned short int));
	i = _swapBytes ? revert_bytes(ri) : ri;
	return *this;
}

Buffer &Buffer::operator>>(unsigned char &c)
{
	read(&c, sizeof(char));
	return *this;
}

Buffer &Buffer::operator>>(Buffer &buf)
{
	if (_data)
		buf.setData(reinterpret_cast<char *>(_data)+_pos, getWritten() - _pos);
	return *this;
}

