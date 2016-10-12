/*
-----------------------------------------------------------------------------
File:        Camera.inl
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

namespace OpenIG {
	namespace Library {
		namespace Graphics {

			template <class T>
			Camera<T>::Camera()
				: m_vPosition(Vector3<T>::ZERO)
				, m_vDirection(Vector3<T>::ZERO)
				, m_vUpVector(Vector3<T>::ZERO)
				, m_fNearPlane(0.1f)
				, m_fFarPlane(10.0f)
				, m_fFieldOfViewY(Math::PI_BY_4)
				, m_bFixedUpVector(true)
			{

			}

			template <class T>
			Camera<T>::~Camera()
			{

			}

			template <class T>
			void Camera<T>::SetNearPlane(T fNearPlane)
			{
				m_fNearPlane = fNearPlane;
				SetPerspective(GetFieldOfView(), GetAspectRatio(), GetNearPlane(), GetFarPlane());
			}

			template <class T>
			T Camera<T>::GetNearPlane(void) const
			{
				return m_fNearPlane;
			}

			template <class T>
			void Camera<T>::SetFarPlane(T fFarPlane)
			{
				m_fFarPlane = fFarPlane;
				SetPerspective(GetFieldOfView(), GetAspectRatio(), GetNearPlane(), GetFarPlane());
			}

			template <class T>
			T Camera<T>::GetFarPlane(void) const
			{
				return m_fFarPlane;
			}

			template <class T>
			void Camera<T>::SetFieldOfView(T fFOVy)
			{
				m_fFieldOfViewY = fFOVy;
				SetPerspective(GetFieldOfView(), GetAspectRatio(), GetNearPlane(), GetFarPlane());
			}

			template <class T>
			T Camera<T>::GetFieldOfView(void) const
			{
				return m_fFieldOfViewY;
			}

			template <class T>
			void Camera<T>::SetPosition(const Vector3<T>& vPosition)
			{
				LookAt(vPosition, GetDirection(), GetUpVector());
			}

			template <class T>
			Vector3<T> Camera<T>::GetPosition(void) const
			{
				return m_vPosition;
			}

			template <class T>
			void Camera<T>::SetDirection(const Vector3<T>& vDirection)
			{
				m_vDirection = vDirection;
				m_vDirection.Normalize();
			}
			template <class T>
			Vector3<T> Camera<T>::GetDirection(void) const
			{
				return m_vDirection;
			}

			template <class T>
			Vector3<T> Camera<T>::GetUpVector(void) const
			{
				return m_vUpVector;
			}

			template <class T>
			Vector3<T> Camera<T>::GetSideVector(void) const
			{
				return m_vSideVector;
			}

			template <class T>
			void Camera<T>::LookAt(const Vector3<T>& vEye, const Vector3<T>& vLook, const Vector3<T>& _vUp)
			{
				Vector3<T> vForward = vLook - vEye;
				vForward.Normalize();

				Vector3<T> vUp = _vUp;
				vUp.Normalize();

				// Get the side vector
				Vector3<T> vSide = vForward.CrossProduct(vUp);
				vSide.Normalize();

				// Compute the up vector again
				vUp = vSide.CrossProduct(vForward);
				vUp.Normalize();

				Matrix4<T> matToViewSpace(
					vSide.x, vSide.y, vSide.z, 0
					, vUp.x, vUp.y, vUp.z, 0
					, -vForward.x, -vForward.y, -vForward.z, 0
					, 0, 0, 0, 1
					);

				Matrix4<T> matTranslateToEye(
					1, 0, 0, -vEye.x
					, 0, 1, 0, -vEye.y
					, 0, 0, 1, -vEye.z
					, 0, 0, 0, 1
					);

				m_ViewMatrix = matToViewSpace*matTranslateToEye;

				m_vPosition = vEye;
				m_vDirection = vForward;
				if (GetFixedUpVector())
				{
					m_vUpVector = _vUp;
					m_vUpVector.Normalize();
				}
				else
				{
					m_vUpVector = vUp;
				}
				m_vSideVector = vSide;

				UpdateFrustumPlanes();
				UpdateViewProjectionMatrix();
			}

			template <class T>
			const Matrix4<T>& Camera<T>::GetViewMatrix(void) const
			{
				return m_ViewMatrix;
			}

			template <class T>
			void Camera<T>::SetPerspective(T fFOVy, T fAspectRatio, T fNearPlane, T fFarPlane)
			{
				ASSERT_PREDICATE_RETURN(fFarPlane > fNearPlane);

				m_fFieldOfViewY = fFOVy;

				m_fNearPlane = fNearPlane;
				m_fFarPlane = fFarPlane;

				m_fAspectRatio = fAspectRatio;

				T f = 1.0f / Math::Tan(m_fFieldOfViewY*0.5f);
				T zNear_plus_zFar = m_fNearPlane + m_fFarPlane;
				T zNear_minus_zFar = m_fNearPlane - m_fFarPlane;

				m_ProjectionMatrix[0][0] = f / m_fAspectRatio;	m_ProjectionMatrix[0][1] = 0;	m_ProjectionMatrix[0][2] = 0;									m_ProjectionMatrix[0][3] = 0;
				m_ProjectionMatrix[1][0] = 0;					m_ProjectionMatrix[1][1] = f;	m_ProjectionMatrix[1][2] = 0;									m_ProjectionMatrix[1][3] = 0;
				m_ProjectionMatrix[2][0] = 0;					m_ProjectionMatrix[2][1] = 0;	m_ProjectionMatrix[2][2] = zNear_plus_zFar / zNear_minus_zFar;	m_ProjectionMatrix[2][3] = 2 * m_fNearPlane*m_fFarPlane / zNear_minus_zFar;
				m_ProjectionMatrix[3][0] = 0;					m_ProjectionMatrix[3][1] = 0;	m_ProjectionMatrix[3][2] = -1;									m_ProjectionMatrix[3][3] = 0;

				UpdateFrustumPlanes();
				UpdateViewProjectionMatrix();
			}

			template <class T>
			const Matrix4<T>& Camera<T>::GetProjectionMatrix(void) const
			{
				return m_ProjectionMatrix;
			}

			template <class T>
			void Camera<T>::SetFixedUpVector(bool fixed)
			{
				m_bFixedUpVector = fixed;
			}
			template <class T>
			bool Camera<T>::GetFixedUpVector(void) const
			{
				return m_bFixedUpVector;
			}

			template <class T>
			T Camera<T>::GetAspectRatio(void) const
			{
				return m_fAspectRatio;
			}

			template <class T>
			void Camera<T>::UpdateFrustumPlanes(void)
			{
				Matrix4<T> combo = GetProjectionMatrix() * GetViewMatrix();

				m_FrustumPlanes[FRUSTUM_PLANE_LEFT].vNormal.x = combo[3][0] + combo[0][0];
				m_FrustumPlanes[FRUSTUM_PLANE_LEFT].vNormal.y = combo[3][1] + combo[0][1];
				m_FrustumPlanes[FRUSTUM_PLANE_LEFT].vNormal.z = combo[3][2] + combo[0][2];
				m_FrustumPlanes[FRUSTUM_PLANE_LEFT].d = combo[3][3] + combo[0][3];

				m_FrustumPlanes[FRUSTUM_PLANE_RIGHT].vNormal.x = combo[3][0] - combo[0][0];
				m_FrustumPlanes[FRUSTUM_PLANE_RIGHT].vNormal.y = combo[3][1] - combo[0][1];
				m_FrustumPlanes[FRUSTUM_PLANE_RIGHT].vNormal.z = combo[3][2] - combo[0][2];
				m_FrustumPlanes[FRUSTUM_PLANE_RIGHT].d = combo[3][3] - combo[0][3];

				m_FrustumPlanes[FRUSTUM_PLANE_TOP].vNormal.x = combo[3][0] - combo[1][0];
				m_FrustumPlanes[FRUSTUM_PLANE_TOP].vNormal.y = combo[3][1] - combo[1][1];
				m_FrustumPlanes[FRUSTUM_PLANE_TOP].vNormal.z = combo[3][2] - combo[1][2];
				m_FrustumPlanes[FRUSTUM_PLANE_TOP].d = combo[3][3] - combo[1][3];

				m_FrustumPlanes[FRUSTUM_PLANE_BOTTOM].vNormal.x = combo[3][0] + combo[1][0];
				m_FrustumPlanes[FRUSTUM_PLANE_BOTTOM].vNormal.y = combo[3][1] + combo[1][1];
				m_FrustumPlanes[FRUSTUM_PLANE_BOTTOM].vNormal.z = combo[3][2] + combo[1][2];
				m_FrustumPlanes[FRUSTUM_PLANE_BOTTOM].d = combo[3][3] + combo[1][3];

				m_FrustumPlanes[FRUSTUM_PLANE_NEAR].vNormal.x = combo[3][0] + combo[2][0];
				m_FrustumPlanes[FRUSTUM_PLANE_NEAR].vNormal.y = combo[3][1] + combo[2][1];
				m_FrustumPlanes[FRUSTUM_PLANE_NEAR].vNormal.z = combo[3][2] + combo[2][2];
				m_FrustumPlanes[FRUSTUM_PLANE_NEAR].d = combo[3][3] + combo[2][3];

				m_FrustumPlanes[FRUSTUM_PLANE_FAR].vNormal.x = combo[3][0] - combo[2][0];
				m_FrustumPlanes[FRUSTUM_PLANE_FAR].vNormal.y = combo[3][1] - combo[2][1];
				m_FrustumPlanes[FRUSTUM_PLANE_FAR].vNormal.z = combo[3][2] - combo[2][2];
				m_FrustumPlanes[FRUSTUM_PLANE_FAR].d = combo[3][3] - combo[2][3];

				// Re-normalize any normals which were not unit length
				for (int i = 0; i < 6; ++i)
				{
					T length = m_FrustumPlanes[i].vNormal.GetLength();
					m_FrustumPlanes[i].vNormal.Normalize();
					m_FrustumPlanes[i].d /= length;
				}
			}

			template <class T>
			void Camera<T>::UpdateViewProjectionMatrix(void)
			{
				m_ViewProjectionMatrix = GetProjectionMatrix()*GetViewMatrix();
			}

			template <class T>
			const Matrix4<T>& Camera<T>::GetViewProjectionMatrix(void) const
			{
				return m_ViewProjectionMatrix;
			}

			template <class T>
			bool Camera<T>::IsVisible(const AxisAlignedBoundingBox<T>& box) const
			{
				// Null boxes always invisible
				if (box.IsNull()) return false;

				// Infinite boxes always visible
				if (box.IsInfinite()) return true;

				// Get center of the box
				const Vector3<T>& vCenter = box.GetCenter();
				// Get the half-size of the box
				Vector3<T> vHalfSize = box.GetHalfSize();

				// For each plane, see if all points are on the negative side
				// If so, object is not visible
				for (int plane = 0; plane < 6; ++plane)
				{
					// Skip far plane if infinite view frustum
					if (plane == FRUSTUM_PLANE_FAR && GetFarPlane() == 0)
						continue;

					PlaneSide side = m_FrustumPlanes[plane].GetSide(vCenter, vHalfSize);
					if (side == NEGATIVE_SIDE)
					{
						// ALL corners on negative side therefore out of view
						return false;
					}
				}
				return true;
			}

			template <class T>
			VisibilityFromCamera Camera<T>::GetVisibility(const AxisAlignedBoundingBox<T>& box) const
			{
				// Null boxes always invisible
				if (box.IsNull()) return VISIBILITY_NONE;

				// Infinite boxes always visible
				if (box.IsInfinite()) return VISIBILITY_FULL;

				// Get center of the box
				Vector3<T> vCenter = box.GetCenter();
				// Get the half-size of the box
				Vector3<T> vHalfSize = box.GetHalfSize();

				bool bAllInside = true;
				// For each plane, see if all points are on the negative side
				// If so, object is not visible
				for (int plane = 0; plane < 6; ++plane)
				{
					// Skip far plane if infinite view frustum
					if (plane == FRUSTUM_PLANE_FAR && GetFarPlane() == 0)
						continue;

					PlaneSide side = m_FrustumPlanes[plane].GetSide(vCenter, vHalfSize);
					if (side == NEGATIVE_SIDE)
					{
						// ALL corners on negative side therefore out of view
						return VISIBILITY_NONE;
					}
					// We can't return now as the box could be later on the negative side of a plane.
					if (side == BOTH_SIDE)
					{
						bAllInside = false;
					}
				}
				if (bAllInside)
					return VISIBILITY_FULL;
				else
					return VISIBILITY_PARTIAL;
			}

			template<class T>
			Vector2<T> Camera<T>::Project(const Vector3<T>& vPosition3) const
			{
				Vector4<T> position = GetViewProjectionMatrix()*Vector4<T>(vPosition3, 1);

				position.w = Math::Abs(position.w);

				position.x = Math::Clamp<T>(position.x, -position.w, position.w);
				position.y = Math::Clamp<T>(position.y, -position.w, position.w);

				position /= position.w;

				position = position*T(0.5) + T(0.5);
				//position.y = 1 - position.y;

				return Vector2<T>(position.x, position.y);
			}

			template<class T>
			Vector2_uint32 Camera<T>::Project(const Vector3<T>& vPosition3, const Vector2_uint32& viewPortSize) const
			{
				Vector2<T> vClipCoords = Project(vPosition3);

				vClipCoords.x *= viewPortSize.x;
				vClipCoords.y *= viewPortSize.y;

				return Vector2_uint32(uint32(vClipCoords.x), uint32(vClipCoords.y));
			}

			template<class T>
			ScreenRect Camera<T>::GetScreenAABB(const AxisAlignedBoundingBox<T>& worldBox, const Vector2_uint32& viewPortSize)  const
			{
				ScreenRect rect;

				if (worldBox.IsInfinite())
				{
					rect.vMin = Vector2_uint32::ZERO;
					rect.vMax = viewPortSize;
					return rect;
				}
				if (worldBox.IsNull())
				{
					rect.vMin = Vector2_uint32::ZERO;
					rect.vMax = Vector2_uint32::ZERO;
					return rect;
				}

				rect.vMin = viewPortSize;
				rect.vMax = Vector2_uint32::ZERO;

				const AxisAlignedBoundingBox_64::VectorCorners& vectorAABBCorners = worldBox.GetCorners();
				ASSERT_PREDICATE(vectorAABBCorners.size() == 8);

				for (size_t i = 0; i < vectorAABBCorners.size(); ++i)
				{
					Vector2_uint32 vWindowCoords = Project(vectorAABBCorners[i], viewPortSize);

					rect.vMin.x = std::min(rect.vMin.x, vWindowCoords.x);
					rect.vMin.y = std::min(rect.vMin.y, vWindowCoords.y);

					rect.vMax.x = std::max(rect.vMax.x, vWindowCoords.x);
					rect.vMax.y = std::max(rect.vMax.y, vWindowCoords.y);
				}
				return rect;
			}

		}
	}
}
