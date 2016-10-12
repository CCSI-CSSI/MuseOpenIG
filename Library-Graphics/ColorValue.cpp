/*
-----------------------------------------------------------------------------
File:        ColorValue.cpp
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
#include "ColorValue.h"
#include "OIGMath.h"

namespace OpenIG {
	namespace Library {
		namespace Graphics {

			ColorValue ColorValue::ZERO = ColorValue(0, 0, 0, 0);

			//alpha is one
			ColorValue ColorValue::BLACK = ColorValue(0, 0, 0, 1);
			ColorValue ColorValue::WHITE = ColorValue(1, 1, 1, 1);

			ColorValue ColorValue::RED = ColorValue(1, 0, 0, 1);
			ColorValue ColorValue::GREEN = ColorValue(0, 1, 0, 1);
			ColorValue ColorValue::BLUE = ColorValue(0, 0, 1, 1);

			ColorValue ColorValue::YELLOW = ColorValue(1, 1, 0, 1);
			ColorValue ColorValue::MAGENTA = ColorValue(1, 0, 1, 1);
			ColorValue ColorValue::CYAN = ColorValue(0, 1, 1, 1);

			uint32 const ColorValue::sizeInBytes = 4 * sizeof(float32);

			ColorValue::ColorValue()
				: r(0), g(0), b(0), a(0)
			{
			}
			ColorValue::ColorValue(float32 _r, float32 _g, float32 _b, float32 _a)
				: r(_r), g(_g), b(_b), a(_a)
			{

			}

			void* ColorValue::ptr(void)
			{
				return &r;
			}
			const void* ColorValue::ptr(void) const
			{
				return &r;
			}

			bool ColorValue::operator == (const ColorValue& rhs) const
			{
				return Math::IsEqual(r, rhs.r)
					&& Math::IsEqual(g, rhs.g)
					&& Math::IsEqual(b, rhs.b)
					&& Math::IsEqual(a, rhs.a);
			}

			bool ColorValue::operator != (const ColorValue& rhs) const
			{
				return !this->operator ==(rhs);
			}

		}
	}
}
