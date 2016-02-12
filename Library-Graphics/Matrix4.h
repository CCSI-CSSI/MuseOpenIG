/*
-----------------------------------------------------------------------------
File:        Matrix4.h
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
	#include <OpenIG-Graphics/OIGAssert.h>
	#include <OpenIG-Graphics/Vector.h>
	#include <OpenIG-Graphics/Matrix3.h>
	#include <OpenIG-Graphics/Vector.h>
#else
	#include <Library-Graphics/OIGAssert.h>
	#include <Library-Graphics/Vector.h>
	#include <Library-Graphics/Matrix3.h>
	#include <Library-Graphics/Vector.h>
#endif

#include <iostream>

namespace OpenIG {
	namespace Library {
		namespace Graphics {

			template<class T>
			class Matrix4
			{
			public:
				Matrix4();
				Matrix4(const T rhs[4][4]);
				Matrix4(T m00, T m01, T m02, T m03
					, T m10, T m11, T m12, T m13
					, T m20, T m21, T m22, T m23
					, T m30, T m31, T m32, T m33);

				//Creates a standard 4x4 transformation matrix with a zero translation part from a rotation/scaling 3x3 matrix.
				Matrix4(const Matrix3<T>& rhs);
				void operator=(const Matrix3<T>& rhs);

				Matrix4 operator*(const Matrix4& rhs) const;

				Vector4<T> operator*(const Vector4<T>& rhs) const;

				void SetZero(void);
				void SetIdentity(void);

				Matrix4 GetTranspose(void) const;

				Matrix4 convert_depth_gl_to_dx() const;

				bool IsEqual(const Matrix4& rhs, T tolerance) const;

				virtual ~Matrix4();

				const T* operator [](size_t iRow) const;
				T* operator [](size_t iRow);

				const void* ptr(void) const;

				static Matrix4 IDENTITY;

				// Return whether the last row is (0,0,0,1);
				bool IsAffine(void) const;
				Vector3<T> TransformAffine(const Vector3<T>& v) const;
				Vector4<T> TransformAffine(const Vector4<T>& v) const;

				Matrix4<T> GetInverse(void) const;
				// Returns the inverse of this matrix. This matrix must be affine to 
				// begin with
				Matrix4<T> GetInverseAffine(void) const;
			private:
				union
				{
					T _m[16];
					T  m[4][4];
				};
			};

			template <class T>
			Matrix4<T> Matrix4<T>::IDENTITY = Matrix4<T>(1, 0, 0, 0
				, 0, 1, 0, 0
				, 0, 0, 1, 0
				, 0, 0, 0, 1);

			typedef Matrix4<float>	Matrix4_32;
			typedef Matrix4<double> Matrix4_64;

			template<class T>
			Matrix4<T>::Matrix4()
			{

			}

			template<class T>
			Matrix4<T>::Matrix4(const T rhs[4][4])
			{
				memcpy(_m, rhs, 16 * sizeof(T));
			}

			template<class T>
			Matrix4<T>::Matrix4(T m00, T m01, T m02, T m03
				, T m10, T m11, T m12, T m13
				, T m20, T m21, T m22, T m23
				, T m30, T m31, T m32, T m33)
			{
				m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03;
				m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13;
				m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23;
				m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33;
			}

			template<class T>
			Matrix4<T>::~Matrix4()
			{

			}
			template<class T>
			const void* Matrix4<T>::ptr(void) const
			{
				return _m;
			}

			template<class T>
			const T* Matrix4<T>::operator [](size_t iRow) const
			{
				ASSERT_PREDICATE_RETURN_ZERO(iRow < 4);
				return m[iRow];
			}
			template<class T>
			T* Matrix4<T>::operator [](size_t iRow)
			{
				ASSERT_PREDICATE_RETURN_ZERO(iRow < 4);
				return m[iRow];
			}

			template<class T>
			Matrix4<T> Matrix4<T>::operator*(const Matrix4<T>& rhs) const
			{
				/*
					| lhs.00,  lhs.01,  lhs.02,  lhs.03 |  | rhs.00,  rhs.01,  rhs.02,  rhs.03 |
					| lhs.10,  lhs.11,  lhs.12,  lhs.13 |  | rhs.10,  rhs.11,  rhs.12,  rhs.13 |
					| lhs.20,  lhs.21,  lhs.22,  lhs.23 |  | rhs.20,  rhs.21,  rhs.22,  rhs.23 |
					| lhs.30,  lhs.31,  lhs.32,  lhs.33 |  | rhs.30,  rhs.31,  rhs.32,  rhs.33 |
					*/
				const Matrix4& lhs = *this;

				Matrix4 result;

				result.m[0][0] = lhs.m[0][0] * rhs.m[0][0] + lhs.m[0][1] * rhs.m[1][0] + lhs.m[0][2] * rhs.m[2][0] + lhs.m[0][3] * rhs.m[3][0];
				result.m[0][1] = lhs.m[0][0] * rhs.m[0][1] + lhs.m[0][1] * rhs.m[1][1] + lhs.m[0][2] * rhs.m[2][1] + lhs.m[0][3] * rhs.m[3][1];
				result.m[0][2] = lhs.m[0][0] * rhs.m[0][2] + lhs.m[0][1] * rhs.m[1][2] + lhs.m[0][2] * rhs.m[2][2] + lhs.m[0][3] * rhs.m[3][2];
				result.m[0][3] = lhs.m[0][0] * rhs.m[0][3] + lhs.m[0][1] * rhs.m[1][3] + lhs.m[0][2] * rhs.m[2][3] + lhs.m[0][3] * rhs.m[3][3];

				result.m[1][0] = lhs.m[1][0] * rhs.m[0][0] + lhs.m[1][1] * rhs.m[1][0] + lhs.m[1][2] * rhs.m[2][0] + lhs.m[1][3] * rhs.m[3][0];
				result.m[1][1] = lhs.m[1][0] * rhs.m[0][1] + lhs.m[1][1] * rhs.m[1][1] + lhs.m[1][2] * rhs.m[2][1] + lhs.m[1][3] * rhs.m[3][1];
				result.m[1][2] = lhs.m[1][0] * rhs.m[0][2] + lhs.m[1][1] * rhs.m[1][2] + lhs.m[1][2] * rhs.m[2][2] + lhs.m[1][3] * rhs.m[3][2];
				result.m[1][3] = lhs.m[1][0] * rhs.m[0][3] + lhs.m[1][1] * rhs.m[1][3] + lhs.m[1][2] * rhs.m[2][3] + lhs.m[1][3] * rhs.m[3][3];

				result.m[2][0] = lhs.m[2][0] * rhs.m[0][0] + lhs.m[2][1] * rhs.m[1][0] + lhs.m[2][2] * rhs.m[2][0] + lhs.m[2][3] * rhs.m[3][0];
				result.m[2][1] = lhs.m[2][0] * rhs.m[0][1] + lhs.m[2][1] * rhs.m[1][1] + lhs.m[2][2] * rhs.m[2][1] + lhs.m[2][3] * rhs.m[3][1];
				result.m[2][2] = lhs.m[2][0] * rhs.m[0][2] + lhs.m[2][1] * rhs.m[1][2] + lhs.m[2][2] * rhs.m[2][2] + lhs.m[2][3] * rhs.m[3][2];
				result.m[2][3] = lhs.m[2][0] * rhs.m[0][3] + lhs.m[2][1] * rhs.m[1][3] + lhs.m[2][2] * rhs.m[2][3] + lhs.m[2][3] * rhs.m[3][3];

				result.m[3][0] = lhs.m[3][0] * rhs.m[0][0] + lhs.m[3][1] * rhs.m[1][0] + lhs.m[3][2] * rhs.m[2][0] + lhs.m[3][3] * rhs.m[3][0];
				result.m[3][1] = lhs.m[3][0] * rhs.m[0][1] + lhs.m[3][1] * rhs.m[1][1] + lhs.m[3][2] * rhs.m[2][1] + lhs.m[3][3] * rhs.m[3][1];
				result.m[3][2] = lhs.m[3][0] * rhs.m[0][2] + lhs.m[3][1] * rhs.m[1][2] + lhs.m[3][2] * rhs.m[2][2] + lhs.m[3][3] * rhs.m[3][2];
				result.m[3][3] = lhs.m[3][0] * rhs.m[0][3] + lhs.m[3][1] * rhs.m[1][3] + lhs.m[3][2] * rhs.m[2][3] + lhs.m[3][3] * rhs.m[3][3];

				return result;
			}


			template<class T>
			Vector4<T> Matrix4<T>::operator*(const Vector4<T>& rhs) const
			{
				/*
				| lhs.00,  lhs.01,  lhs.02,  lhs.03 |  | rhs.0 |
				| lhs.10,  lhs.11,  lhs.12,  lhs.13 |  | rhs.1 |
				| lhs.20,  lhs.21,  lhs.22,  lhs.23 |  | rhs.2 |
				| lhs.30,  lhs.31,  lhs.32,  lhs.33 |  | rhs.3 |
				*/
				Vector4<T> vResult;

				const Matrix4& lhs = *this;

				vResult.x = lhs.m[0][0] * rhs.x + lhs.m[0][1] * rhs.y + lhs.m[0][2] * rhs.z + lhs.m[0][3] * rhs.w;
				vResult.y = lhs.m[1][0] * rhs.x + lhs.m[1][1] * rhs.y + lhs.m[1][2] * rhs.z + lhs.m[1][3] * rhs.w;
				vResult.z = lhs.m[2][0] * rhs.x + lhs.m[2][1] * rhs.y + lhs.m[2][2] * rhs.z + lhs.m[2][3] * rhs.w;
				vResult.w = lhs.m[3][0] * rhs.x + lhs.m[3][1] * rhs.y + lhs.m[3][2] * rhs.z + lhs.m[3][3] * rhs.w;

				return vResult;
			}

			template<class T>
			void Matrix4<T>::SetZero(void)
			{
				memset(_m, 0, 16 * sizeof(T));
			}

			template<class T>
			void Matrix4<T>::SetIdentity(void)
			{
				SetZero();
				m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1;
			}

			template<class T>
			Matrix4<T> Matrix4<T>::GetTranspose(void) const
			{
				return Matrix4<T>(
					m[0][0], m[1][0], m[2][0], m[3][0]
					, m[0][1], m[1][1], m[2][1], m[3][1]
					, m[0][2], m[1][2], m[2][2], m[3][2]
					, m[0][3], m[1][3], m[2][3], m[3][3]
					);
			}

			template<class T>
			Matrix4<T> Matrix4<T>::convert_depth_gl_to_dx() const
			{
				Matrix4_32 result = *this;
				result[2][0] = (result[2][0] + result[3][0])*0.5f;
				result[2][1] = (result[2][1] + result[3][1])*0.5f;
				result[2][2] = (result[2][2] + result[3][2])*0.5f;
				result[2][3] = (result[2][3] + result[3][3])*0.5f;
				return result;
			}

			template<class T>
			bool Matrix4<T>::IsEqual(const Matrix4<T>& rhs, T tolerance) const
			{
				const Matrix4& lhs = *this;
				for (size_t i = 0; i < 4; ++i)
				{
					for (size_t j = 0; j < 4; ++j)
					{
						if (!Math::IsEqual(lhs[i][j], rhs[i][j], tolerance))
							return false;
					}
				}
				return true;
			}

			template<class T>
			Matrix4<T>::Matrix4(const Matrix3<T>& rhs)
			{
				operator=(rhs);
			}
			template<class T>
			void Matrix4<T>::operator=(const Matrix3<T>& rhs)
			{
				SetIdentity();

				m[0][0] = rhs[0][0];	m[0][1] = rhs[0][1];	m[0][2] = rhs[0][2];
				m[1][0] = rhs[1][0];	m[1][1] = rhs[1][1];	m[1][2] = rhs[1][2];
				m[2][0] = rhs[2][0];	m[2][1] = rhs[2][1];	m[2][2] = rhs[2][2];
			}

			template<class T>
			bool Matrix4<T>::IsAffine(void) const
			{
				return m[3][0] == 0 && m[3][1] == 0 && m[3][2] == 0 && m[3][3] == 1;
			}

			template<class T>
			Vector3<T> Matrix4<T>::TransformAffine(const Vector3<T>& v) const
			{
				ASSERT_PREDICATE(IsAffine());
				if (IsAffine() == false)
					return Vector3<T>::ZERO;

				return Vector3<T>(
					m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + +m[0][3]
					, m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + +m[1][3]
					, m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + +m[2][3]
					);
			}

			template<class T>
			Vector4<T> Matrix4<T>::TransformAffine(const Vector4<T>& v) const
			{
				ASSERT_PREDICATE(IsAffine());
				if (IsAffine() == false)
					return Vector4<T>::ZERO;

				return Vector4<T>(
					m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + +m[0][3]
					, m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + +m[1][3]
					, m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + +m[2][3]
					, v.w
					);
			}

			template<class T>
			Matrix4<T> Matrix4<T>::GetInverse(void) const
			{
				T m00 = m[0][0], m01 = m[0][1], m02 = m[0][2], m03 = m[0][3];
				T m10 = m[1][0], m11 = m[1][1], m12 = m[1][2], m13 = m[1][3];
				T m20 = m[2][0], m21 = m[2][1], m22 = m[2][2], m23 = m[2][3];
				T m30 = m[3][0], m31 = m[3][1], m32 = m[3][2], m33 = m[3][3];

				T v0 = m20 * m31 - m21 * m30;
				T v1 = m20 * m32 - m22 * m30;
				T v2 = m20 * m33 - m23 * m30;
				T v3 = m21 * m32 - m22 * m31;
				T v4 = m21 * m33 - m23 * m31;
				T v5 = m22 * m33 - m23 * m32;

				T t00 = +(v5 * m11 - v4 * m12 + v3 * m13);
				T t10 = -(v5 * m10 - v2 * m12 + v1 * m13);
				T t20 = +(v4 * m10 - v2 * m11 + v0 * m13);
				T t30 = -(v3 * m10 - v1 * m11 + v0 * m12);

				T invDet = 1 / (t00 * m00 + t10 * m01 + t20 * m02 + t30 * m03);

				T d00 = t00 * invDet;
				T d10 = t10 * invDet;
				T d20 = t20 * invDet;
				T d30 = t30 * invDet;

				T d01 = -(v5 * m01 - v4 * m02 + v3 * m03) * invDet;
				T d11 = +(v5 * m00 - v2 * m02 + v1 * m03) * invDet;
				T d21 = -(v4 * m00 - v2 * m01 + v0 * m03) * invDet;
				T d31 = +(v3 * m00 - v1 * m01 + v0 * m02) * invDet;

				v0 = m10 * m31 - m11 * m30;
				v1 = m10 * m32 - m12 * m30;
				v2 = m10 * m33 - m13 * m30;
				v3 = m11 * m32 - m12 * m31;
				v4 = m11 * m33 - m13 * m31;
				v5 = m12 * m33 - m13 * m32;

				T d02 = +(v5 * m01 - v4 * m02 + v3 * m03) * invDet;
				T d12 = -(v5 * m00 - v2 * m02 + v1 * m03) * invDet;
				T d22 = +(v4 * m00 - v2 * m01 + v0 * m03) * invDet;
				T d32 = -(v3 * m00 - v1 * m01 + v0 * m02) * invDet;

				v0 = m21 * m10 - m20 * m11;
				v1 = m22 * m10 - m20 * m12;
				v2 = m23 * m10 - m20 * m13;
				v3 = m22 * m11 - m21 * m12;
				v4 = m23 * m11 - m21 * m13;
				v5 = m23 * m12 - m22 * m13;

				T d03 = -(v5 * m01 - v4 * m02 + v3 * m03) * invDet;
				T d13 = +(v5 * m00 - v2 * m02 + v1 * m03) * invDet;
				T d23 = -(v4 * m00 - v2 * m01 + v0 * m03) * invDet;
				T d33 = +(v3 * m00 - v1 * m01 + v0 * m02) * invDet;

				return Matrix4(
					d00, d01, d02, d03,
					d10, d11, d12, d13,
					d20, d21, d22, d23,
					d30, d31, d32, d33);
			}

			template<class T>
			Matrix4<T> Matrix4<T>::GetInverseAffine(void) const
			{
				ASSERT_PREDICATE(IsAffine());

				T m10 = m[1][0], m11 = m[1][1], m12 = m[1][2];
				T m20 = m[2][0], m21 = m[2][1], m22 = m[2][2];

				T t00 = m22 * m11 - m21 * m12;
				T t10 = m20 * m12 - m22 * m10;
				T t20 = m21 * m10 - m20 * m11;

				T m00 = m[0][0], m01 = m[0][1], m02 = m[0][2];

				T invDet = 1 / (m00 * t00 + m01 * t10 + m02 * t20);

				t00 *= invDet; t10 *= invDet; t20 *= invDet;

				m00 *= invDet; m01 *= invDet; m02 *= invDet;

				T r00 = t00;
				T r01 = m02 * m21 - m01 * m22;
				T r02 = m01 * m12 - m02 * m11;

				T r10 = t10;
				T r11 = m00 * m22 - m02 * m20;
				T r12 = m02 * m10 - m00 * m12;

				T r20 = t20;
				T r21 = m01 * m20 - m00 * m21;
				T r22 = m00 * m11 - m01 * m10;

				T m03 = m[0][3], m13 = m[1][3], m23 = m[2][3];

				T r03 = -(r00 * m03 + r01 * m13 + r02 * m23);
				T r13 = -(r10 * m03 + r11 * m13 + r12 * m23);
				T r23 = -(r20 * m03 + r21 * m13 + r22 * m23);

				return Matrix4<T>(
					r00, r01, r02, r03,
					r10, r11, r12, r13,
					r20, r21, r22, r23,
					0, 0, 0, 1);
			}

			template<class T>
			inline std::ostream& operator <<
				(std::ostream& o, const Matrix4<T>& mat)
			{
				for (size_t i = 0; i < 4; ++i)
				{
					o << mat[i][0] << " " << mat[i][1] << " " << mat[i][2] << " " << mat[i][3] << std::endl;
				}
				return o;
			}

		}
	}
}
