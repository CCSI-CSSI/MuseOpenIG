/*
-----------------------------------------------------------------------------
File:        Vector2.h
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
	#include <OpenIG-Graphics/OIGMath.h>
#else
	#include <Library-Graphics/CommonTypes.h>
	#include <Library-Graphics/OIGMath.h>
#endif

#include <list>
#include <vector>

namespace OpenIG {
	namespace Library {
		namespace Graphics {

			template <class T>
			class Vector2
			{
			public:
				Vector2(){}
				Vector2(T _x, T _y) : x(_x), y(_y){}

				const void* ptr(void) const{ return &x; }

				T operator[](size_t index) const;

				T GetLength(void) const;

				bool IsEqual(const Vector2& rhs, T tolerance) const;

				Vector2 operator+(const Vector2& rhs) const;
				Vector2 operator-(const Vector2& rhs) const;
				Vector2 operator*(const Vector2& rhs) const;
				Vector2 operator/(const Vector2& rhs) const;

				Vector2 operator+(T fScalar) const;
				Vector2 operator-(T fScalar) const;
				Vector2 operator*(T fScalar) const;
				Vector2 operator/(T fScalar) const;

				Vector2 NormalizedCopy(void) const;

				T       DotProduct(const Vector2& rhs) const;

				T x, y;

				static Vector2 ZERO;
				static Vector2 UNIT_SCALE;

				static const uint32 sizeInBytes;

				bool operator==(const Vector2& rhs){ return x == rhs.x&&y == rhs.y; }

			};

			template <class T>
			Vector2<T> Vector2<T>::ZERO = Vector2<T>(0, 0);
			template <class T>
			Vector2<T> Vector2<T>::UNIT_SCALE = Vector2<T>(1, 1);

			template <class T>
			Vector2<T> Vector2<T>::operator+(const Vector2<T>& rhs) const
			{
				return Vector2<T>(x + rhs.x, y + rhs.y);
			}
			template <class T>
			Vector2<T> Vector2<T>::operator-(const Vector2<T>& rhs) const
			{
				return Vector2<T>(x - rhs.x, y - rhs.y);
			}

			template <class T>
			Vector2<T> Vector2<T>::operator+(T fScalar) const
			{
				return Vector2<T>(x + fScalar, y + fScalar);
			}

			template <class T>
			Vector2<T> Vector2<T>::operator-(T fScalar) const
			{
				return Vector2<T>(x - fScalar, y - fScalar);
			}

			template <class T>
			Vector2<T> Vector2<T>::operator*(T fScalar) const
			{
				return Vector2<T>(x*fScalar, y*fScalar);
			}

			template <class T>
			Vector2<T> Vector2<T>::operator/(T fScalar) const
			{
				T OneByfScalar = T(1.0) / fScalar;
				return Vector2<T>(x*OneByfScalar, y*OneByfScalar);
			}

			template <class T>
			Vector2<T> Vector2<T>::operator*(const Vector2<T>& rhs) const
			{
				return Vector2<T>(x*rhs.x, y*rhs.y);
			}

			template <class T>
			Vector2<T> Vector2<T>::operator/(const Vector2<T>& rhs) const
			{
				return Vector2<T>(x / rhs.x, y / rhs.y);
			}

			template <class T>
			bool Vector2<T>::IsEqual(const Vector2<T>& rhs, T tolerance) const
			{
				return Math::IsEqual(this->x, rhs.x, tolerance)
					&& Math::IsEqual(this->y, rhs.y, tolerance)
					;
			}


			template <class T>
			T Vector2<T>::DotProduct(const Vector2<T>& rhs) const
			{
				return x*rhs.x + y*rhs.y;
			}

			template <class T>
			Vector2<T> Vector2<T>::NormalizedCopy(void) const
			{
				T length = GetLength();
				if (length < std::numeric_limits<T>::epsilon())
					return Vector2::ZERO;

				T one_by_length = T(1.0) / length;
				return (Vector2<T>(x, y)*one_by_length);
			}

			template <class T>
			T Vector2<T>::operator[](size_t index) const
			{
				ASSERT_PREDICATE_RETURN_ZERO(index <= 1);
				const T* ptrT = static_cast<const T*>(ptr());
				return ptrT[index];
			}

			template <class T>
			T Vector2<T>::GetLength(void) const
			{
				return Math::Sqrt(x*x + y*y);
			}

		}
	}
}
