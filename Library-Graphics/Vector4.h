/*
-----------------------------------------------------------------------------
File:        Vector4.h
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
	#include <OpenIG-Graphics/OIGAssert.h>
	#include <OpenIG-Graphics/Vector3.h>
#else
	#include <Library-Graphics/CommonTypes.h>
	#include <Library-Graphics/OIGAssert.h>
	#include <Library-Graphics/Vector3.h>
#endif

#include <list>
#include <vector>
#include <iostream>


namespace OpenIG {
	namespace Library {
		namespace Graphics {

			template <class T>
			class Vector4
			{
			public:
				Vector4(){}
				Vector4(T _x, T _y, T _z, T _w) : x(_x), y(_y), z(_z), w(_w){}
				Vector4(const Vector3<T>& rhs, T _w) : x(rhs.x), y(rhs.y), z(rhs.z), w(_w){}

				const void* ptr(void) const{ return &x; }

				Vector4 operator-(void) const { return Vector4(-x, -y, -z, -w); }

				Vector4 operator+(T fScalar) const;
				Vector4 operator-(T fScalar) const;
				Vector4 operator*(T fScalar) const;
				Vector4 operator/(T fScalar) const;

				Vector4& operator+=(T fScalar);
				Vector4& operator-=(T fScalar);
				Vector4& operator*=(T fScalar);
				Vector4& operator/=(T fScalar);

				Vector4& operator+=(const Vector4& rhs);
				Vector4& operator-=(const Vector4& rhs);
				Vector4& operator*=(const Vector4& rhs);
				Vector4& operator/=(const Vector4& rhs);

				bool IsEqual(const Vector4& rhs, T tolerance) const;

				T operator[](size_t index) const;
				T& operator[](size_t index);

				void Normalize(void);

				Vector4 NormalizedCopy() const;

				T x, y, z, w;

				static Vector4 ZERO;
				static Vector4 UNIT;
			};

			typedef Vector4<float32> Vector4_32;
			typedef std::list<Vector4_32> Vector4_32_List;
			typedef std::vector<Vector4_32> Vector4_32_Vector;

			template <class T>
			Vector4<T> Vector4<T>::ZERO = Vector4<T>(0, 0, 0, 0);
			template <class T>
			Vector4<T> Vector4<T>::UNIT = Vector4<T>(1, 1, 1, 1);

			template <class T>
			Vector4<T> Vector4<T>::operator+(T fScalar) const
			{
				return Vector4<T>(x + fScalar, y + fScalar, z + fScalar, w + fScalar);
			}
			template <class T>
			Vector4<T> Vector4<T>::operator-(T fScalar) const
			{
				return Vector4<T>(x - fScalar, y - fScalar, z - fScalar, w - fScalar);
			}

			template <class T>
			Vector4<T> Vector4<T>::operator*(T fScalar) const
			{
				return Vector4<T>(x*fScalar, y*fScalar, z*fScalar, w*fScalar);
			}

			template <class T>
			Vector4<T> Vector4<T>::operator/(T fScalar) const
			{
				T OneByfScalar = T(1.0) / fScalar;
				return Vector4<T>(x*OneByfScalar, y*OneByfScalar, z*OneByfScalar, w*OneByfScalar);
			}


			template <class T>
			bool Vector4<T>::IsEqual(const Vector4<T>& rhs, T tolerance) const
			{
				return Math::IsEqual(this->x, rhs.x, tolerance)
					&& Math::IsEqual(this->y, rhs.y, tolerance)
					&& Math::IsEqual(this->z, rhs.z, tolerance)
					;
				// PPP: What about the w component???
			}

			template <class T>
			void Vector4<T>::Normalize(void)
			{
				T fLength = Math::Sqrt(x*x + y*y + z*z + w*w);
				fLength = std::max(0.000001f, fLength);

				T fInvLength = 1.0f / fLength;

				x *= fInvLength;
				y *= fInvLength;
				z *= fInvLength;
				w *= fInvLength;
			}

			template <class T>
			Vector4<T> Vector4<T>::NormalizedCopy(void) const
			{
				T fLength = Math::Sqrt(x*x + y*y + z*z + w*w);
				fLength = std::max(0.000001f, fLength);

				T fInvLength = 1.0f / fLength;

				return Vector4(x*fInvLength
					, y*fInvLength
					, z*fInvLength
					, w* fInvLength
					);

			}

			template <class T>
			T Vector4<T>::operator[](size_t index) const
			{
				ASSERT_PREDICATE_RETURN_ZERO(index <= 3);
				const T* ptrT = static_cast<const T*>(ptr());
				return ptrT[index];
			}

			template <class T>
			T& Vector4<T>::operator[](size_t index)
			{
				ASSERT_PREDICATE(index <= 3);
				T* ptrT = &x;
				return ptrT[index];
			}

			template <class T>
			Vector4<T>& Vector4<T>::operator+=(T fScalar)
			{
				x += fScalar;
				y += fScalar;
				z += fScalar;
				w += fScalar;
				return *this;
			}

			template <class T>
			Vector4<T>& Vector4<T>::operator-=(T fScalar)
			{
				x -= fScalar;
				y -= fScalar;
				z -= fScalar;
				w -= fScalar;
				return *this;
			}

			template <class T>
			Vector4<T>& Vector4<T>::operator*=(T fScalar)
			{
				x *= fScalar;
				y *= fScalar;
				z *= fScalar;
				w *= fScalar;
				return *this;
			}

			template <class T>
			Vector4<T>& Vector4<T>::operator/=(T fScalar)
			{
				return this->operator*=(T(1.0) / fScalar);
			}

			template <class T>
			Vector4<T>& Vector4<T>::operator+=(const Vector4<T>& rhs)
			{
				x += rhs.x;
				y += rhs.y;
				z += rhs.z;
				w += rhs.w;
				return *this;
			}

			template <class T>
			Vector4<T>& Vector4<T>::operator-=(const Vector4<T>& rhs)
			{
				x -= rhs.x;
				y -= rhs.y;
				z -= rhs.z;
				w -= rhs.w;
				return *this;
			}

			template <class T>
			Vector4<T>& Vector4<T>::operator*=(const Vector4<T>& rhs)
			{
				x *= rhs.x;
				y *= rhs.y;
				z *= rhs.z;
				w *= rhs.w;
				return *this;
			}

			template <class T>
			Vector4<T>& Vector4<T>::operator/=(const Vector4<T>& rhs)
			{
				x /= rhs.x;
				y /= rhs.y;
				z /= rhs.z;
				w /= rhs.w;
				return *this;
			}

			template<class T>
			inline std::ostream& operator <<
				(std::ostream& o, const Vector4<T>& v)
			{
				o << "Vector4(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
				return o;
			}

		}
	}
}
