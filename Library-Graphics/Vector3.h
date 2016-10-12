/*
-----------------------------------------------------------------------------
File:        Vector3.h
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
	#include <OpenIG-Graphics/CommonTypes.h>
	#include <OpenIG-Graphics/OIGAssert.h>
	#include <OpenIG-Graphics/OIGMath.h>
#else
	#include <Library-Graphics/CommonTypes.h>
	#include <Library-Graphics/OIGAssert.h>
	#include <Library-Graphics/OIGMath.h>
#endif

#include <list>
#include <vector>
#include <iostream>

namespace OpenIG {
	namespace Library {
		namespace Graphics {

			template <class T>
			class Vector3
			{
			public:
				Vector3(){}
				Vector3(T _x, T _y, T _z) : x(_x), y(_y), z(_z){}

				const void* ptr(void) const{ return &x; }

				Vector3 operator-(void) const { return Vector3(-x, -y, -z); }

				bool IsEqual(const Vector3& rhs, T tolerance) const;

				Vector3 operator+(const Vector3& rhs) const;
				Vector3 operator-(const Vector3& rhs) const;
				Vector3 operator*(const Vector3& rhs) const;
				Vector3 operator/(const Vector3& rhs) const;

				Vector3 operator+(T fScalar) const;
				Vector3 operator-(T fScalar) const;
				Vector3 operator*(T fScalar) const;
				Vector3 operator/(T fScalar) const;

				Vector3& operator+=(T fScalar);
				Vector3& operator-=(T fScalar);
				Vector3& operator*=(T fScalar);
				Vector3& operator/=(T fScalar);

				// Return true if each component greater than the corresponding component of rhs
				bool operator>(const Vector3& rhs) const;
				// Return true if each component lessser than the corresponding component of rhs
				bool operator<(const Vector3& rhs) const;

				void Normalize(void);
				T GetLength(void) const;
				T GetSquaredLength(void) const;
				T GetDistance(const Vector3& rhs) const;

				Vector3 CrossProduct(const Vector3& rhs) const;
				T       DotProduct(const Vector3& rhs) const;
				T       AbsDotProduct(const Vector3& rhs) const;

				Vector3 MidPoint(const Vector3& rhs) const;

				// Each component = max(component, rhs.component)
				void MakeCeil(const Vector3& rhs);

				// Each component = min(component, rhs.component)
				void MakeFloor(const Vector3& rhs);

				T operator[](size_t index) const;
				T& operator[](size_t index);

				T x, y, z;

				static Vector3 ZERO;
				static Vector3 UNIT_SCALE;
				static Vector3 X_AXIS;
				static Vector3 Y_AXIS;
				static Vector3 Z_AXIS;

				static Vector3 INFINITE_VAL_VECTOR;
			};

			typedef Vector3<float32> Vector3_32;
			typedef std::list<Vector3_32> Vector3_32_List;
			typedef std::vector<Vector3_32> Vector3_32_Vector;

			template <class T>
			Vector3<T> Vector3<T>::ZERO = Vector3<T>(0, 0, 0);
			template <class T>
			Vector3<T> Vector3<T>::UNIT_SCALE = Vector3<T>(1, 1, 1);
			template <class T>
			Vector3<T> Vector3<T>::X_AXIS = Vector3<T>(1, 0, 0);
			template <class T>
			Vector3<T> Vector3<T>::Y_AXIS = Vector3<T>(0, 1, 0);
			template <class T>
			Vector3<T> Vector3<T>::Z_AXIS = Vector3<T>(0, 0, 1);

			template <class T>
			Vector3<T> Vector3<T>::INFINITE_VAL_VECTOR = Vector3<T>(Math::PositiveInfinity<T>()
				, Math::PositiveInfinity<T>()
				, Math::PositiveInfinity<T>());

			template <class T>
			void Vector3<T>::Normalize(void)
			{
				T fLength = Math::Sqrt(x*x + y*y + z*z);
				if (fLength > 0)
				{
					T fInvLength = T(1.0) / fLength;
					x *= fInvLength;
					y *= fInvLength;
					z *= fInvLength;
				}
			}

			template <class T>
			T Vector3<T>::GetLength(void) const
			{
				return Math::Sqrt(x*x + y*y + z*z);
			}

			template <class T>
			T Vector3<T>::GetSquaredLength(void) const
			{
				return x*x + y*y + z*z;
			}


			template <class T>
			T Vector3<T>::GetDistance(const Vector3& rhs) const
			{
				return (*this - rhs).GetLength();
			}

			template <class T>
			Vector3<T> Vector3<T>::operator+(const Vector3<T>& rhs) const
			{
				return Vector3<T>(x + rhs.x, y + rhs.y, z + rhs.z);
			}
			template <class T>
			Vector3<T> Vector3<T>::operator-(const Vector3<T>& rhs) const
			{
				return Vector3<T>(x - rhs.x, y - rhs.y, z - rhs.z);
			}

			template <class T>
			Vector3<T> Vector3<T>::operator*(const Vector3<T>& rhs) const
			{
				return Vector3<T>(x*rhs.x, y*rhs.y, z*rhs.z);
			}
			template <class T>
			Vector3<T> Vector3<T>::operator/(const Vector3<T>& rhs) const
			{
				return Vector3<T>(x / rhs.x, y / rhs.y, z / rhs.z);
			}

			template <class T>
			Vector3<T> Vector3<T>::operator+(T fScalar) const
			{
				return Vector3<T>(x + fScalar, y + fScalar, z + fScalar);
			}
			template <class T>
			Vector3<T> Vector3<T>::operator-(T fScalar) const
			{
				return Vector3<T>(x - fScalar, y - fScalar, z - fScalar);
			}

			template <class T>
			Vector3<T> Vector3<T>::operator*(T fScalar) const
			{
				return Vector3<T>(x*fScalar, y*fScalar, z*fScalar);
			}

			template <class T>
			Vector3<T> Vector3<T>::operator/(T fScalar) const
			{
				T OneByfScalar = T(1.0) / fScalar;
				return Vector3<T>(x*OneByfScalar, y*OneByfScalar, z*OneByfScalar);
			}

			template <class T>
			bool Vector3<T>::operator>(const Vector3<T>& rhs) const
			{
				return x > rhs.x && y > rhs.y && z > rhs.z;
			}

			template <class T>
			bool Vector3<T>::operator<(const Vector3<T>& rhs) const
			{
				return x < rhs.x && y < rhs.y && z < rhs.z;
			}

			template <class T>
			bool Vector3<T>::IsEqual(const Vector3<T>& rhs, T tolerance) const
			{
				return Math::IsEqual(this->x, rhs.x, tolerance)
					&& Math::IsEqual(this->y, rhs.y, tolerance)
					&& Math::IsEqual(this->z, rhs.z, tolerance)
					;
			}

			template <class T>
			Vector3<T> Vector3<T>::CrossProduct(const Vector3<T>& rhs) const
			{
				//    i          j            k
				//    x          y            z
				//    rhs.x      rhs.y        rhs.z
				return Vector3<T>(
					y*rhs.z - z*rhs.y
					, z*rhs.x - x*rhs.z
					, x*rhs.y - y*rhs.x
					);
			}

			template <class T>
			T Vector3<T>::DotProduct(const Vector3<T>& rhs) const
			{
				return x*rhs.x + y*rhs.y + z*rhs.z;
			}

			template <class T>
			T Vector3<T>::AbsDotProduct(const Vector3<T>& rhs) const
			{
				return Math::Abs(x*rhs.x) + Math::Abs(y*rhs.y) + Math::Abs(z*rhs.z);
			}

			template <class T>
			Vector3<T> Vector3<T>::MidPoint(const Vector3& rhs) const
			{
				return Vector3<T>((x + rhs.x)*static_cast<T>(0.5)
					, (y + rhs.y)*static_cast<T>(0.5)
					, (z + rhs.z)*static_cast<T>(0.5)
					);

			}

			template <class T>
			T Vector3<T>::operator[](size_t index) const
			{
				ASSERT_PREDICATE_RETURN_ZERO(index <= 2);
				const T* ptrT = static_cast<const T*>(ptr());
				return ptrT[index];
			}

			template <class T>
			T& Vector3<T>::operator[](size_t index)
			{
				ASSERT_PREDICATE(index <= 2);
				T* ptrT = &x;
				return ptrT[index];
			}

			template <class T>
			void Vector3<T>::MakeCeil(const Vector3<T>& rhs)
			{
				if (rhs.x > x) x = rhs.x;
				if (rhs.y > y) y = rhs.y;
				if (rhs.z > z) z = rhs.z;
			}
			template <class T>
			void Vector3<T>::MakeFloor(const Vector3<T>& rhs)
			{
				if (rhs.x < x) x = rhs.x;
				if (rhs.y < y) y = rhs.y;
				if (rhs.z < z) z = rhs.z;
			}

			template <class T>
			Vector3<T>& Vector3<T>::operator+=(T fScalar)
			{
				x += fScalar;
				y += fScalar;
				z += fScalar;
				return *this;
			}

			template <class T>
			Vector3<T>& Vector3<T>::operator-=(T fScalar)
			{
				x -= fScalar;
				y -= fScalar;
				z -= fScalar;
				return *this;
			}

			template <class T>
			Vector3<T>& Vector3<T>::operator*=(T fScalar)
			{
				x *= fScalar;
				y *= fScalar;
				z *= fScalar;
				return *this;
			}

			template <class T>
			Vector3<T>& Vector3<T>::operator/=(T fScalar)
			{
				return this->operator*=(T(1.0) / fScalar);
			}

			template<class T>
			inline std::ostream& operator <<
				(std::ostream& o, const Vector3<T>& v)
			{
				o << "Vector3(" << v.x << ", " << v.y << ", " << v.z << ")";
				return o;
			}
		}
	}
}
