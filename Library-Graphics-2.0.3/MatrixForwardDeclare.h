/*
-----------------------------------------------------------------------------
File:        MatrixForwardDeclare.h
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
#ifndef MATRIXFORWARDDECLARE_H
#define MATRIXFORWARDDECLARE_H

namespace OpenIG {
	namespace Library {
		namespace Graphics {

#if defined(OPENIG_SDK)
	#include <OpenIG-Graphics/CommonTypes.h>
#else
	#include <Library-Graphics/CommonTypes.h>
#endif

			template <class T> class Matrix3;
			template <class T> class Matrix4;

			typedef Matrix3<float>	Matrix3_32;
			typedef Matrix3<double> Matrix3_64;

			typedef Matrix4<float>	Matrix4_32;
			typedef Matrix4<double> Matrix4_64;

		}
	}
}

#endif
