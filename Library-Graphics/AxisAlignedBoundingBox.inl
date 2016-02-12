/*
-----------------------------------------------------------------------------
File:        AxisAlignedBoundingBox.inl
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
	#include <OpenIG-Graphics/AxisAlignedBoundingBox.h>
#else
	#include <Library-Graphics/AxisAlignedBoundingBox.h>
#endif

namespace OpenIG {
	namespace Library {
		namespace Graphics {

			template <class T>
			AxisAlignedBoundingBox<T>::AxisAlignedBoundingBox()
			{
				m_VectorCorners.resize(8);
				SetNull();
			}

			template <class T>
			AxisAlignedBoundingBox<T>::AxisAlignedBoundingBox(const Vector3<T>& vMin, const Vector3<T>& vMax)
			{
				m_VectorCorners.resize(8);
				SetMinMax(vMin, vMax);
			}
			template <class T>
			AxisAlignedBoundingBox<T>::~AxisAlignedBoundingBox()
			{

			}

			template <class T>
			const Vector3<T>& AxisAlignedBoundingBox<T>::GetMin(void) const
			{
				return m_vMin;
			}

			template <class T>
			const Vector3<T>& AxisAlignedBoundingBox<T>::GetMax(void) const
			{
				return m_vMax;
			}

			template <class T>
			const Vector3<T>& AxisAlignedBoundingBox<T>::GetCenter(void) const
			{
				return m_vCenter;
			}

			template <class T>
			T AxisAlignedBoundingBox<T>::GetBoundingSphereRadius(void) const
			{
				return (m_vMax - m_vMin).GetLength()*0.5f;
			}

			template <class T>
			void AxisAlignedBoundingBox<T>::SetMinMax(const Vector3<T>& vMin, const Vector3<T>& vMax)
			{
				ASSERT_PREDICATE(vMin.x <= vMax.x && vMin.y <= vMax.y && vMin.z <= vMax.z);

				m_vMin = vMin;
				m_vMax = vMax;
				m_vCenter = (m_vMin + m_vMax)*0.5f;
				m_Extent = EXTENT_FINITE;

				UpdateCorners();
			}

			template <class T>
			void AxisAlignedBoundingBox<T>::SetNull(void)
			{
				m_vMin = Vector3<T>::ZERO;
				m_vMax = Vector3<T>::ZERO;
				m_vCenter = Vector3<T>::ZERO;
				m_Extent = EXTENT_NULL;

				UpdateCorners();
			}
			template <class T>
			void AxisAlignedBoundingBox<T>::SetInfinite(void)
			{
				m_vMin = Vector3<T>::ZERO;
				m_vMax = Vector3<T>::ZERO;
				m_vCenter = Vector3<T>::ZERO;
				m_Extent = EXTENT_INFINITE;

				UpdateCorners();
			}

			template <class T>
			void AxisAlignedBoundingBox<T>::Merge(const AxisAlignedBoundingBox<T>& rhs)
			{
				const AxisAlignedBoundingBox<T>& lhs = *this;
				if (rhs.m_Extent == EXTENT_NULL || lhs.m_Extent == EXTENT_INFINITE)
					return;
				if (rhs.m_Extent == EXTENT_INFINITE)
				{
					m_Extent = EXTENT_INFINITE;
					return;
				}
				if (m_Extent == EXTENT_NULL)
				{
					SetMinMax(rhs.GetMin(), rhs.GetMax());
					return;
				}
				Vector3<T> vMin = GetMin();
				vMin.MakeFloor(rhs.GetMin());

				Vector3<T> vMax = GetMax();
				vMax.MakeFloor(rhs.GetMax());

				SetMinMax(vMin, vMax);
			}

			template <class T>
			Vector3<T> AxisAlignedBoundingBox<T>::GetHalfSize(void) const
			{
				SWITCH_BEGIN(GetExtent())
					CASE_RETURN(EXTENT_NULL, Vector3<T>::ZERO);
				CASE_RETURN(EXTENT_FINITE, (GetMax() - GetMin())*static_cast<T>(0.5));
				CASE_RETURN(EXTENT_INFINITE, Vector3<T>::INFINITE_VAL_VECTOR);
				SWITCH_END(GetExtent())

					ASSERT_PREDICATE(false);
				return -Vector3<T>::UNIT_SCALE;
			}

			template <class T>
			Vector3<T> AxisAlignedBoundingBox<T>::GetSize(void) const
			{
				SWITCH_BEGIN(GetExtent())
					CASE_RETURN(EXTENT_NULL, Vector3<T>::ZERO);
				CASE_RETURN(EXTENT_FINITE, (GetMax() - GetMin()));
				CASE_RETURN(EXTENT_INFINITE, Vector3<T>::INFINITE_VAL_VECTOR);
				SWITCH_END(GetExtent())

					ASSERT_PREDICATE(false);
				return -Vector3<T>::UNIT_SCALE;
			}


			template <class T>
			Extent AxisAlignedBoundingBox<T>::GetExtent(void) const
			{
				return m_Extent;
			}

			template <class T>
			void AxisAlignedBoundingBox<T>::TransformAffine(const Matrix4<T>& m)
			{
				ASSERT_PREDICATE(m.IsAffine());

				if (GetExtent() != EXTENT_FINITE)
				{
					return;
				}

				Vector3<T> center = GetCenter();
				Vector3<T> vHalfSize = GetHalfSize();

				Vector3<T> vNewCenter = m.TransformAffine(center);

				Vector3<T> vNewHalfSize = Vector3<T>(
					Math::Abs(m[0][0]) * vHalfSize.x + Math::Abs(m[0][1]) * vHalfSize.y + Math::Abs(m[0][2]) * vHalfSize.z
					, Math::Abs(m[1][0]) * vHalfSize.x + Math::Abs(m[1][1]) * vHalfSize.y + Math::Abs(m[1][2]) * vHalfSize.z
					, Math::Abs(m[2][0]) * vHalfSize.x + Math::Abs(m[2][1]) * vHalfSize.y + Math::Abs(m[2][2]) * vHalfSize.z
					);

				SetMinMax(vNewCenter - vNewHalfSize, vNewCenter + vNewHalfSize);
			}

			template <class T>
			bool AxisAlignedBoundingBox<T>::IsInfinite(void) const
			{
				return GetExtent() == EXTENT_INFINITE;
			}
			template <class T>
			bool AxisAlignedBoundingBox<T>::IsFinite(void) const
			{
				return GetExtent() == EXTENT_FINITE;
			}
			template <class T>
			bool AxisAlignedBoundingBox<T>::IsNull(void) const
			{
				return GetExtent() == EXTENT_NULL;
			}

			template <class T>
			const typename AxisAlignedBoundingBox<T>::VectorCorners& AxisAlignedBoundingBox<T>::GetCorners(void) const
			{
				return m_VectorCorners;
			}

			template <class T>
			void AxisAlignedBoundingBox<T>::UpdateCorners(void)
			{
				m_VectorCorners[0] = m_vMin;
				m_VectorCorners[1].x = m_vMin.x; m_VectorCorners[1].y = m_vMax.y; m_VectorCorners[1].z = m_vMin.z;
				m_VectorCorners[2].x = m_vMax.x; m_VectorCorners[2].y = m_vMax.y; m_VectorCorners[2].z = m_vMin.z;
				m_VectorCorners[3].x = m_vMax.x; m_VectorCorners[3].y = m_vMin.y; m_VectorCorners[3].z = m_vMin.z;

				m_VectorCorners[4] = m_vMax;
				m_VectorCorners[5].x = m_vMin.x; m_VectorCorners[5].y = m_vMax.y; m_VectorCorners[5].z = m_vMax.z;
				m_VectorCorners[6].x = m_vMin.x; m_VectorCorners[6].y = m_vMin.y; m_VectorCorners[6].z = m_vMax.z;
				m_VectorCorners[7].x = m_vMax.x; m_VectorCorners[7].y = m_vMin.y; m_VectorCorners[7].z = m_vMax.z;
			}

		}
	}
}
