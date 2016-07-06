/*
-----------------------------------------------------------------------------
File:        IntSize.h
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

#if defined(OPENIG_SDK)
	#include <OpenIG-Graphics/CommonTypes.h>
    #include <OpenIG-Graphics/OIGCudaPreamble.h>
#else
	#include <Library-Graphics/CommonTypes.h>
    #include <Library-Graphics/OIGCudaPreamble.h>
#endif

#if GRAPHICS_LIBRARY_HAS_NAMESPACE

namespace OpenIG {
	namespace Library {
		namespace Graphics {
#endif

			template<class T>
			class IntSize
			{
			public:
				IntSize(){}
				IntSize(T _width, T _height)
					: width(_width), height(_height)
				{

				}
				bool operator<(const IntSize& rhs) const
				{
					if (width < rhs.width)
						return true;
					if (width == rhs.width && height < rhs.height)
						return true;
					return false;
				}
				T width;
				T height;
			private:
			};

			typedef IntSize<int32>  IntSize_int32;
			typedef IntSize<uint32> IntSize_uint32;
#if GRAPHICS_LIBRARY_HAS_NAMESPACE
		}
	}
}
#endif
