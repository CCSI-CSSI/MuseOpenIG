/*
-----------------------------------------------------------------------------
File:        VectorForwardDeclare.h
Copyright:   Copyright (C) 2007-2015 Poojan Prabhu. All rights reserved.
Created:     10/20/2015
Last edit:   12/15/2015
Author:      Poojan Prabhu
E-mail:      openig@compro.net

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
#ifndef VECTORFORWARDDECLARE_H
#define VECTORFORWARDDECLARE_H

#if defined(OPENIG_SDK)
	#include <OpenIG-Graphics/CommonTypes.h>
#else
	#include <Library-Graphics/CommonTypes.h>
#endif

#include <list>
#include <vector>

namespace OpenIG {
	namespace Library {
		namespace Graphics {

			template <class T> class Vector2;
			template <class T> class Vector3;
			template <class T> class Vector4;

			// 32 bit
			typedef Vector2<float32> Vector2_32;
			typedef std::list<Vector2_32> Vector2_32_List;
			typedef std::vector<Vector2_32> Vector2_32_Vector;

			typedef Vector2<int> Vector2_int;
			typedef Vector2<uint32> Vector2_uint32;

			typedef Vector3<float32> Vector3_32;
			typedef std::list<Vector3_32> Vector3_32_List;
			typedef std::vector<Vector3_32> Vector3_32_Vector;

			typedef Vector4<float32> Vector4_32;
			typedef std::list<Vector4_32> Vector4_32_List;
			typedef std::vector<Vector4_32> Vector4_32_Vector;

			// 64 bit
			typedef Vector2<float64> Vector2_64;
			typedef std::list<Vector2_64> Vector2_64_List;
			typedef std::vector<Vector2_64> Vector2_64_Vector;

			typedef Vector3<float64> Vector3_64;
			typedef std::list<Vector3_64> Vector3_64_List;
			typedef std::vector<Vector3_64> Vector3_64_Vector;

			typedef Vector4<float64> Vector4_64;
			typedef std::list<Vector4_64> Vector4_64_List;
			typedef std::vector<Vector4_64> Vector4_64_Vector;
		}
	}
}

#endif
