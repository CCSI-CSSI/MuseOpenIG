/*
-----------------------------------------------------------------------------
File:        Matrix3.h
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
	#include <OpenIG-Graphics/Vector3.h>
	#include <OpenIG-Graphics/OIGMath.h>
#else
	#include <Library-Graphics/Vector3.h>
	#include <Library-Graphics/OIGMath.h>
#endif

#include <iostream>

#if GRAPHICS_LIBRARY_HAS_NAMESPACE
namespace OpenIG {
	namespace Library {
		namespace Graphics {
#endif

			template<class T>
			class Matrix3
			{
			public:
				Matrix3();
				Matrix3(const T rhs[3][3]);
				Matrix3(T m00, T m01, T m02
					, T m10, T m11, T m12
					, T m20, T m21, T m22);
				const void* ptr(void) const;
				virtual ~Matrix3();

				Matrix3 operator*(const Matrix3& rhs) const;

				Vector3<T> operator*(const Vector3<T>& rhs) const;

				void SetZero(void);
				void SetIdentity(void);

				Matrix3 GetTranspose(void) const;
				bool    GetInverse(Matrix3& rkInverse, T fTolerance) const;

				bool IsEqual(const Matrix3& rhs, T tolerance) const;

				const T* operator [](size_t iRow) const;
				T* operator [](size_t iRow);

				static Matrix3 IDENTITY;
			private:
				union
				{
					T _m[9];
					T  m[3][3];
				};
			};

			template <class T>
			Matrix3<T> Matrix3<T>::IDENTITY = Matrix3<T>(1, 0, 0
				, 0, 1, 0
				, 0, 0, 1);

			template<class T>
			Matrix3<T>::Matrix3()
			{

			}

			template<class T>
			Matrix3<T>::Matrix3(const T rhs[3][3])
			{
				memcpy(_m, rhs, 9 * sizeof(T));
			}

			template<class T>
			Matrix3<T>::Matrix3(T m00, T m01, T m02
				, T m10, T m11, T m12
				, T m20, T m21, T m22)
			{
				m[0][0] = m00; m[0][1] = m01; m[0][2] = m02;
				m[1][0] = m10; m[1][1] = m11; m[1][2] = m12;
				m[2][0] = m20; m[2][1] = m21; m[2][2] = m22;
			}
			template<class T>
			Matrix3<T>::~Matrix3()
			{

			}
			template<class T>
			const void* Matrix3<T>::ptr(void) const
			{
				return _m;
			}

			template<class T>
			const T* Matrix3<T>::operator [](size_t iRow) const
			{
				ASSERT_PREDICATE_RETURN_ZERO(iRow < 3);
				return m[iRow];
			}
			template<class T>
			T* Matrix3<T>::operator [](size_t iRow)
			{
				ASSERT_PREDICATE_RETURN_ZERO(iRow < 3);
				return m[iRow];
			}

			typedef Matrix3<float>	Matrix3_32;
			typedef Matrix3<double> Matrix3_64;

			template<class T>
			Matrix3<T> Matrix3<T>::operator*(const Matrix3<T>& rhs) const
			{
				/*
					| lhs.00,  lhs.01,  lhs.02 |  | rhs.00,  rhs.01,  rhs.02 |
					| lhs.10,  lhs.11,  lhs.12 |  | rhs.10,  rhs.11,  rhs.12 |
					| lhs.20,  lhs.21,  lhs.22 |  | rhs.20,  rhs.21,  rhs.22 |
					*/
				const Matrix3& lhs = *this;

				Matrix3 result;

				result.m[0][0] = lhs.m[0][0] * rhs.m[0][0] + lhs.m[0][1] * rhs.m[1][0] + lhs.m[0][2] * rhs.m[2][0];
				result.m[0][1] = lhs.m[0][0] * rhs.m[0][1] + lhs.m[0][1] * rhs.m[1][1] + lhs.m[0][2] * rhs.m[2][1];
				result.m[0][2] = lhs.m[0][0] * rhs.m[0][2] + lhs.m[0][1] * rhs.m[1][2] + lhs.m[0][2] * rhs.m[2][2];

				result.m[1][0] = lhs.m[1][0] * rhs.m[0][0] + lhs.m[1][1] * rhs.m[1][0] + lhs.m[1][2] * rhs.m[2][0];
				result.m[1][1] = lhs.m[1][0] * rhs.m[0][1] + lhs.m[1][1] * rhs.m[1][1] + lhs.m[1][2] * rhs.m[2][1];
				result.m[1][2] = lhs.m[1][0] * rhs.m[0][2] + lhs.m[1][1] * rhs.m[1][2] + lhs.m[1][2] * rhs.m[2][2];

				result.m[2][0] = lhs.m[2][0] * rhs.m[0][0] + lhs.m[2][1] * rhs.m[1][0] + lhs.m[2][2] * rhs.m[2][0];
				result.m[2][1] = lhs.m[2][0] * rhs.m[0][1] + lhs.m[2][1] * rhs.m[1][1] + lhs.m[2][2] * rhs.m[2][1];
				result.m[2][2] = lhs.m[2][0] * rhs.m[0][2] + lhs.m[2][1] * rhs.m[1][2] + lhs.m[2][2] * rhs.m[2][2];

				return result;
			}

			template<class T>
			Vector3<T> Matrix3<T>::operator*(const Vector3<T>& rhs) const
			{
				/*
				| lhs.00,  lhs.01,  lhs.02 |  | rhs.0 |
				| lhs.10,  lhs.11,  lhs.12 |  | rhs.1 |
				| lhs.20,  lhs.21,  lhs.22 |  | rhs.2 |
				*/
				Vector3<T> vResult;

				const Matrix3& lhs = *this;

				vResult.x = lhs.m[0][0] * rhs.x + lhs.m[0][1] * rhs.y + lhs.m[0][2] * rhs.z;
				vResult.y = lhs.m[1][0] * rhs.x + lhs.m[1][1] * rhs.y + lhs.m[1][2] * rhs.z;
				vResult.z = lhs.m[2][0] * rhs.x + lhs.m[2][1] * rhs.y + lhs.m[2][2] * rhs.z;

				return vResult;
			}

			template<class T>
			void Matrix3<T>::SetZero(void)
			{
				memset(_m, 0, 9 * sizeof(T));
			}

			template<class T>
			void Matrix3<T>::SetIdentity(void)
			{
				SetZero();
				m[0][0] = m[1][1] = m[2][2] = 1;
			}

			template<class T>
			Matrix3<T> Matrix3<T>::GetTranspose(void) const
			{
				return Matrix3(
					m[0][0], m[1][0], m[2][0]
					, m[0][1], m[1][1], m[2][1]
					, m[0][2], m[1][2], m[2][2]
					);
			}

			template<class T>
			bool Matrix3<T>::GetInverse(Matrix3& rkInverse, T fTolerance) const
			{

				rkInverse[0][0] = m[1][1] * m[2][2] - m[1][2] * m[2][1];
				rkInverse[0][1] = m[0][2] * m[2][1] - m[0][1] * m[2][2];
				rkInverse[0][2] = m[0][1] * m[1][2] - m[0][2] * m[1][1];

				rkInverse[1][0] = m[1][2] * m[2][0] - m[1][0] * m[2][2];
				rkInverse[1][1] = m[0][0] * m[2][2] - m[0][2] * m[2][0];
				rkInverse[1][2] = m[0][2] * m[1][0] - m[0][0] * m[1][2];

				rkInverse[2][0] = m[1][0] * m[2][1] - m[1][1] * m[2][0];
				rkInverse[2][1] = m[0][1] * m[2][0] - m[0][0] * m[2][1];
				rkInverse[2][2] = m[0][0] * m[1][1] - m[0][1] * m[1][0];

				T fDet =
					m[0][0] * rkInverse[0][0] +
					m[0][1] * rkInverse[1][0] +
					m[0][2] * rkInverse[2][0];

				if (Math::Abs(fDet) <= fTolerance)
					return false;

				T fInvDet = 1.0f / fDet;
				for (size_t iRow = 0; iRow < 3; iRow++)
				{
					for (size_t iCol = 0; iCol < 3; iCol++)
						rkInverse[iRow][iCol] *= fInvDet;
				}

				return true;
			}

			template<class T>
			bool Matrix3<T>::IsEqual(const Matrix3<T>& rhs, T tolerance) const
			{
				const Matrix3& lhs = *this;
				for (size_t i = 0; i < 3; ++i)
				{
					for (size_t j = 0; j < 3; ++j)
					{
						if (!Math::IsEqual(lhs[i][j], rhs[i][j], tolerance))
							return false;
					}
				}
				return true;
			}

			template<class T>
			inline std::ostream& operator <<
				(std::ostream& o, const Matrix3<T>& mat)
			{
				for (size_t i = 0; i < 3; ++i)
				{
					o << mat[i][0] << " " << mat[i][1] << " " << mat[i][2] << std::endl;
				}
				return o;
			}

#if GRAPHICS_LIBRARY_HAS_NAMESPACE
		}
	}
}
#endif
