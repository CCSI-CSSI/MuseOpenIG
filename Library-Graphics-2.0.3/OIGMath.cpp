/*
-----------------------------------------------------------------------------
File:        OIGMath.cpp
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
#include "OIGMath.h"
#include "OIGAssert.h"

#include <cstdlib>

namespace OpenIG {
	namespace Library {
		namespace Graphics {

			const float32 Math::PI = 4 * atan(1.0f);
			const float32 Math::TWO_PI = 2 * Math::PI;
			const float32 Math::PI_BY_2 = Math::PI*0.5f;
			const float32 Math::PI_BY_3 = Math::PI*(1.0f / 3.0f);
			const float32 Math::PI_BY_4 = Math::PI*0.25f;
			const float32 Math::PI_BY_6 = Math::PI*(1.0f / 6.0f);
			const float32 Math::Gravity = 9.81f;

			const float32 Math::m_fConvertToDegrees = 180.0f / Math::PI;
			const float32 Math::m_fConvertToRadians = Math::PI / 180.0f;

			bool Math::IsEqual(float32 lhs, float32 rhs, float32 tolerance)
			{
				return (fabs(rhs - lhs) <= tolerance);
			}
			bool Math::IsEqual(float64 lhs, float64 rhs, float64 tolerance)
			{
				return (fabs(rhs - lhs) <= tolerance);
			}

			float32	 Math::UnitRandom32(void)
			{
				return float32(rand()) / float32(RAND_MAX);
			}
			float64	 Math::UnitRandom64(void)
			{
				return float64(rand()) / float64(RAND_MAX);
			}

			int	Math::RangeRandom(int iLow, int iHigh)
			{
				return static_cast<int>((iHigh - iLow)*UnitRandom32() + iLow);
			}
			float32	Math::RangeRandom(float32 fLow, float32 fHigh)
			{
				return (fHigh - fLow)*UnitRandom32() + fLow;
			}
			float64	Math::RangeRandom(float64 fLow, float64 fHigh)
			{
				return (fHigh - fLow)*UnitRandom32() + fLow;
			}
			bool Math::RandomBool(void)
			{
				float32 fRandomVal = Math::RangeRandom(0.0f, 1.0f);
				return fRandomVal < 0.5f;
			}

			void Math::ResetRandomEngine(void)
			{
				srand(1);
			}

			uint32 Math::GetUpperPowerOfTwo(uint32 val)
			{
				val--;
				val |= val >> 1;
				val |= val >> 2;
				val |= val >> 4;
				val |= val >> 8;
				val |= val >> 16;
				val++;
				return val;
			}

			uint32 Math::GetLowerPowerOfTwo(uint32 val)
			{
				if (val == 0 || val == 1)
					return 1;

				return GetUpperPowerOfTwo(val) / (uint32)2;
			}

			uint32 Math::GetPowerOfTwo(uint32 val)
			{
				ASSERT_PREDICATE_RETURN_ZERO(IsPowerOfTwo(val));
				uint32 n = 0;
				while (((val & 1) == 0) && val > 1) /* While x is even and > 1 */
				{
					val >>= 1;
					++n;
				}
				return n;
			}

			bool Math::IsPowerOfTwo(uint32 val)
			{
				return ((val != 0) && !(val & (val - 1)));
			}

		}
	}
}
