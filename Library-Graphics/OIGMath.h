/*
-----------------------------------------------------------------------------
File:        OIGMath.h
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
#pragma once

#if defined(OPENIG_SDK)
	#include <OpenIG-Graphics/export.h>
	#include <OpenIG-Graphics/CommonTypes.h>
	#include <OpenIG-Graphics/VectorForwardDeclare.h>
	#include <OpenIG-Graphics/OIGAssert.h>
#else
	#include <Library-Graphics/export.h>
	#include <Library-Graphics/CommonTypes.h>
	#include <Library-Graphics/VectorForwardDeclare.h>
	#include <Library-Graphics/OIGAssert.h>
#endif

#include <cmath>
#include <limits>

namespace OpenIG {
	namespace Library {
		namespace Graphics {

			class IGLIBGRAPHICS_EXPORT Math
			{
			public:
				template <class T> static T Sqrt(T val) { return sqrt(val); }
				template <class T> static T Abs(T val) { return fabs(val); }
				template <class T> static T Sin(T val) { return sin(val); }
				template <class T> static T Cos(T val) { return cos(val); }
				template <class T> static T Tan(T val) { return tan(val); }
				template <class T> static T Squared(T val) { return val*val; }

				static bool IsEqual(float32 lhs, float32 rhs, float32 tolerance = std::numeric_limits<float32>::epsilon());
				static bool IsEqual(float64 lhs, float64 rhs, float64 tolerance = std::numeric_limits<float64>::epsilon());

				static float32	UnitRandom32(void);
				static float64	UnitRandom64(void);

				template <class T> static T ToDegrees(T val);
				template <class T> static T ToRadians(T val);

				static int		RangeRandom(int iLow, int iHigh);
				static float32	RangeRandom(float32 fLow, float32 fHigh);
				static float64	RangeRandom(float64 fLow, float64 fHigh);
				static bool     RandomBool(void);

				static void     ResetRandomEngine(void);

				template <class T> static T Clamp(T val, T minval, T maxval);

				template <class T> static Vector2<T> Clamp(const Vector2<T>& val, const Vector2<T>& minval, const Vector2<T>& maxval);
				template <class T> static Vector3<T> Clamp(const Vector3<T>& val, const Vector3<T>& minval, const Vector3<T>& maxval);
				template <class T> static Vector4<T> Clamp(const Vector4<T>& val, const Vector4<T>& minval, const Vector4<T>& maxval);

				template <class T> static Vector2<T> FractionalPart(const Vector2<T>& val);

				template <class T> static Vector3<T> ToCartesian(T thetaDegrees, T phiDegrees, bool flipYZ);

				static uint32 GetUpperPowerOfTwo(uint32 val);
				static uint32 GetLowerPowerOfTwo(uint32 val);
				static bool   IsPowerOfTwo(uint32 val);
				static uint32 GetPowerOfTwo(uint32 val);

				template <class T> T static Max(T lhs, T rhs);
				template <class T> T static Min(T lhs, T rhs);

				template <class T> static void Swap(T& a, T& b);

				template <class T> static T Log2(T val);

#undef max
#undef min
				template <class T> static T PositiveInfinity(void) { return std::numeric_limits<float32>::max(); }
				template <class T> static T NegativeInfinity(void) { return std::numeric_limits<float32>::min(); }

				static const float32 TWO_PI;
				static const float32 PI;
				static const float32 PI_BY_2;
				static const float32 PI_BY_3;
				static const float32 PI_BY_4;
				static const float32 PI_BY_6;
				static const float32 Gravity;
			private:
				static const float32 m_fConvertToRadians;
				static const float32 m_fConvertToDegrees;
			};

			template <class T>
			T Math::Clamp(T val, T minval, T maxval)
			{
				ASSERT_PREDICATE(minval <= maxval && "Invalid clamp range");
				return std::max(std::min(val, maxval), minval);
			}

			template <class T>
			Vector2<T> Math::Clamp(const Vector2<T>& val, const Vector2<T>& minval, const Vector2<T>& maxval)
			{
				Vector2<T> vClampedVal;
				vClampedVal.x = Math::Clamp(val.x, minval.x, maxval.x);
				vClampedVal.y = Math::Clamp(val.y, minval.y, maxval.y);
				return vClampedVal;
			}

			template <class T>
			Vector3<T> Math::Clamp(const Vector3<T>& val, const Vector3<T>& minval, const Vector3<T>& maxval)
			{
				Vector3<T> vClampedVal;
				vClampedVal.x = Math::Clamp(val.x, minval.x, maxval.x);
				vClampedVal.y = Math::Clamp(val.y, minval.y, maxval.y);
				vClampedVal.z = Math::Clamp(val.z, minval.z, maxval.z);
				return vClampedVal;
			}

			template <class T>
			Vector4<T> Math::Clamp(const Vector4<T>& val, const Vector4<T>& minval, const Vector4<T>& maxval)
			{
				Vector4<T> vClampedVal;
				vClampedVal.x = Math::Clamp(val.x, minval.x, maxval.x);
				vClampedVal.y = Math::Clamp(val.y, minval.y, maxval.y);
				vClampedVal.z = Math::Clamp(val.z, minval.z, maxval.z);
				vClampedVal.w = Math::Clamp(val.w, minval.w, maxval.w);
				return vClampedVal;
			}

			template <class T>
			Vector2<T> Math::FractionalPart(const Vector2<T>& val)
			{
				Vector2<T> vFractional;
				T dummy;

				return Vector2<T>(modf(val.x, &dummy), modf(val.y, &dummy));
			}

			template <class T>
			Vector3<T> Math::ToCartesian(T thetaDegrees, T phiDegrees, bool flipYZ)
			{
				T theta = ToRadians(thetaDegrees);
				T phi = ToRadians(phiDegrees);

				Vector3<T> vPosition;
				vPosition.x = Math::Cos(theta)*Cos(phi);
				vPosition.y = Math::Sin(theta)*Cos(phi);
				vPosition.z = Math::Sin(phi);

				if (flipYZ)
				{
					T temp = vPosition.y;
					vPosition.y = vPosition.z;
					vPosition.z = temp;
				}

				return vPosition;
			}

			template <class T>
			T Math::ToDegrees(T val)
			{
				return val*m_fConvertToDegrees;
			}

			template <class T>
			T Math::ToRadians(T val)
			{
				return val*m_fConvertToRadians;
			}

			template <class T>
			T Math::Max(T lhs, T rhs)
			{
				if (lhs > rhs)
					return lhs;
				else
					return rhs;
			}

			template <class T>
			T Math::Min(T lhs, T rhs)
			{
				if (lhs < rhs)
					return lhs;
				else
					return rhs;
			}

			template <class T> void Math::Swap(T& a, T& b)
			{
				if (&a == &b)
				{
					return;
				}
				a ^= b;
				b ^= a;
				a ^= b;
			}

			template <class T> T Math::Log2(T val)
			{
				return log(val) / log(T(2));
			}

		}
	}
}
