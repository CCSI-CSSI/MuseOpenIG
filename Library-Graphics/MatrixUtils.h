/*
-----------------------------------------------------------------------------
File:        MatrixUtils.h
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
#ifndef MATRIXUTILS_H
#define MATRIXUTILS_H

#if defined(OPENIG_SDK)
	#include <OpenIG-Graphics/Matrix3.h>
	#include <OpenIG-Graphics/Matrix4.h>
#else
	#include <Library-Graphics/Matrix3.h>
	#include <Library-Graphics/Matrix4.h>
#endif

namespace OpenIG {
	namespace Library {
		namespace Graphics {

			template<class T>
			class MatrixUtils
			{
			public:
				static Matrix4<T> MatrixRotationX(T angle);
				static Matrix4<T> MatrixRotationY(T angle);
				static Matrix4<T> MatrixRotationZ(T angle);
				static Matrix4<T> MatrixRotation_4x4(const Vector3<T>& axis, T angle);
				static Matrix3<T> MatrixRotation_3x3(const Vector3<T>& axis, T angle);
			};

			template<class T>
			Matrix4<T> MatrixUtils<T>::MatrixRotationX(T angle)
			{
				Matrix4<T> result;

				result.SetIdentity();

				result[1][1] = Math::Cos(angle); result[1][2] = -Math::Sin(angle);
				result[2][1] = Math::Sin(angle); result[2][2] = Math::Cos(angle);

				return result;
			}

			template<class T>
			Matrix4<T> MatrixUtils<T>::MatrixRotationY(T angle)
			{
				Matrix4<T> result;

				result.SetIdentity();

				result[0][0] = Math::Cos(angle); result[0][2] = Math::Sin(angle);
				result[2][0] = -Math::Sin(angle); result[2][2] = Math::Cos(angle);

				return result;
			}

			template<class T>
			Matrix4<T> MatrixUtils<T>::MatrixRotationZ(T angle)
			{
				Matrix4<T> result;

				result.SetIdentity();

				result[0][0] = Math::Cos(angle); result[0][1] = -Math::Sin(angle);
				result[1][0] = Math::Sin(angle); result[1][1] = Math::Cos(angle);

				return result;
			}

			template<class T>
			Matrix3<T> MatrixUtils<T>::MatrixRotation_3x3(const Vector3<T>& axis, T theta)
			{
				Matrix3<T> result;

				T sin_theta = Math::Sin(theta);
				T cos_theta = Math::Cos(theta);
				T one_minus_cos_theta = 1 - Math::Cos(theta);

				result[0][0] = cos_theta + Math::Squared(axis.x)*(one_minus_cos_theta);
				result[0][1] = axis.x*axis.y*one_minus_cos_theta - axis.z*sin_theta;
				result[0][2] = axis.x*axis.z*one_minus_cos_theta + axis.y*sin_theta;

				result[1][0] = axis.y*axis.x*one_minus_cos_theta + axis.z*sin_theta;
				result[1][1] = cos_theta + Math::Squared(axis.y)*one_minus_cos_theta;
				result[1][2] = axis.y*axis.z*one_minus_cos_theta - axis.x*sin_theta;

				result[2][0] = axis.z*axis.x*one_minus_cos_theta - axis.y*sin_theta;
				result[2][1] = axis.z*axis.y*one_minus_cos_theta + axis.x*sin_theta;
				result[2][2] = cos_theta + Math::Squared(axis.z)*one_minus_cos_theta;

				return result;
			}
			template<class T>
			Matrix4<T> MatrixUtils<T>::MatrixRotation_4x4(const Vector3<T>& axis, T theta)
			{
				return Matrix4<T>(MatrixRotation_3x3(axis, theta));
			}

			class MatrixPrecisionConvert
			{
			public:
				static Matrix3_32 ToFloat32(const Matrix3_64& rhs)
				{
					Matrix3_32 result;
					for (size_t i = 0; i < 3; ++i)
					{
						for (size_t j = 0; j < 3; ++j)
						{
							result[i][j] = static_cast<float32>(rhs[i][j]);
						}
					}
					return result;
				}

				static Matrix4_32 ToFloat32(const Matrix4_64& rhs)
				{
					Matrix4_32 result;
					for (size_t i = 0; i < 4; ++i)
					{
						for (size_t j = 0; j < 4; ++j)
						{
							result[i][j] = static_cast<float32>(rhs[i][j]);
						}
					}
					return result;
				}
			};

		}
	}
}

#endif
