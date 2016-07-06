/*
-----------------------------------------------------------------------------
File:        CommonTypes.h
Copyright:   Copyright (C) 2007-2015 Poojan Prabhu. All rights reserved.
Created:     10/20/2015
Last edit:   12/15/2015
Author:      Poojan Prabhu
E-mail:      poojanprabhu@gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#pragma once
#include <vector>
#include <set>
#include <map>
#include <string>

namespace OpenIG {
	namespace Library {
		namespace Graphics {

			typedef std::vector<std::string> StringVector;
			typedef std::set<std::string> StringSet;
			typedef std::map<std::string, std::string> MapStringToString;

			typedef unsigned short      uint16;
			typedef unsigned int        uint32;

			typedef short               int16;
			typedef int                 int32;

			typedef float   float32;
			typedef double  float64;

			typedef unsigned char byte;

			typedef std::vector<int32>  VecInt32s;
			typedef std::vector<uint32> VecUnsignedInt32s;

		}
	}
}
