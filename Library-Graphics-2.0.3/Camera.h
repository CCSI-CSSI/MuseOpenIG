/*
-----------------------------------------------------------------------------
File:        Camera.h
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
#ifndef CAMERA_H
#define CAMERA_H

#if defined(OPENIG_SDK)
#include <OpenIG-Graphics/OIGCudaPreamble.h>
#include <OpenIG-Graphics/CommonTypes.h>
#include <OpenIG-Graphics/OIGAssert.h>
#include <OpenIG-Graphics/Vector.h>
#include <OpenIG-Graphics/MatrixUtils.h>
#include <OpenIG-Graphics/AxisAlignedBoundingBox.h>
#include <OpenIG-Graphics/Plane.h>
#include <OpenIG-Graphics/ScreenRect.h>
#include <OpenIG-Graphics/Vector2.h>
#include <OpenIG-Graphics/IntSize.h>
#else
#include <Library-Graphics/OIGCudaPreamble.h>
#include <Library-Graphics/CommonTypes.h>
#include <Library-Graphics/OIGAssert.h>
#include <Library-Graphics/Vector.h>
#include <Library-Graphics/MatrixUtils.h>
#include <Library-Graphics/AxisAlignedBoundingBox.h>
#include <Library-Graphics/Plane.h>
#include <Library-Graphics/ScreenRect.h>
#include <Library-Graphics/Vector2.h>
#include <Library-Graphics/IntSize.h>
#endif

#if GRAPHICS_LIBRARY_HAS_NAMESPACE
namespace OpenIG {
	namespace Library {
		namespace Graphics {
#endif

			enum FrustumPlane
			{
				FRUSTUM_PLANE_NEAR = 0,
				FRUSTUM_PLANE_FAR = 1,
				FRUSTUM_PLANE_LEFT = 2,
				FRUSTUM_PLANE_RIGHT = 3,
				FRUSTUM_PLANE_TOP = 4,
				FRUSTUM_PLANE_BOTTOM = 5
			};

			enum VisibilityFromCamera
			{
				VISIBILITY_NONE = 0, VISIBILITY_PARTIAL = 1, VISIBILITY_FULL = 2
			};

			template <class T>
			class Camera
			{
			public:
				Camera();
				virtual ~Camera();

				void SetNearPlane(T fNearPlane);
				T    GetNearPlane(void) const;

				void SetFarPlane(T fFarPlane);
				T    GetFarPlane(void) const;

				// This is the complete field of view in the Y direction. Not half
				void SetFieldOfView(T fFOVy);
				T GetFieldOfView(void) const;

				void SetPosition(const Vector3<T>& vPosition);
				Vector3<T> GetPosition(void) const;

				void SetDirection(const Vector3<T>& vDirection);
				Vector3<T> GetDirection(void) const;

				Vector3<T> GetUpVector(void) const;
				Vector3<T> GetSideVector(void) const;

				void LookAt(const Vector3<T>& vEye, const Vector3<T>& vLook, const Vector3<T>& vUp);
				const Matrix4<T>& GetViewMatrix(void) const;

				void SetPerspective(T fFOVy, T fAspectRatio, T fNearPlane, T fFarPlane);
				const Matrix4<T>& GetProjectionMatrix(void) const;

				const Matrix4<T>& GetViewProjectionMatrix(void) const;

				void SetFixedUpVector(bool fixed);
				bool GetFixedUpVector(void) const;

				T GetAspectRatio(void) const;

				bool IsVisible(const AxisAlignedBoundingBox<T>& box) const;
				VisibilityFromCamera GetVisibility(const AxisAlignedBoundingBox<T>& box) const;

				// Normalized Device Coordinates (-1,1)
				GPU_PREFIX static Vector2<T> Project(const Vector3<T>& vPosition, const Matrix4<T>& view_projection_matrix);

				// Window Coordinates
				// OpenGL:  Lower left of window is 0,0. Upper  right is w,h. We use OpenGL convention
				// DirectX: Upper left of window is 0,0. Bottom right is w,h
				GPU_PREFIX static Vector2_uint32 Project(const Vector3<T>& vPosition, const Vector2_uint32& viewPortSize, const Matrix4<T>& view_projection_matrix);

				GPU_PREFIX static ScreenRect GetScreenAABB(const AxisAlignedBoundingBox<T>& worldBox, const Vector2_uint32& viewPortSize, const Matrix4<T>& view_projection_matrix);

				// Project box into screen space and compute its bounds
				ScreenRect GetScreenAABB(const AxisAlignedBoundingBox<T>& worldBox, const Vector2_uint32& viewPortSize) const;
			private:
				Vector3<T> m_vPosition;
				Vector3<T> m_vDirection;
				Vector3<T> m_vUpVector;
				Vector3<T> m_vSideVector;

				T m_fNearPlane;
				T m_fFarPlane;
				T m_fFieldOfViewY;
				T m_fAspectRatio;

				Matrix4<T> m_ViewMatrix;
				Matrix4<T> m_ProjectionMatrix;
				Matrix4<T> m_ViewProjectionMatrix;

				bool m_bFixedUpVector;

				Plane<T> m_FrustumPlanes[6];
				void UpdateFrustumPlanes(void);
				void UpdateViewProjectionMatrix(void);
			};
#if GRAPHICS_LIBRARY_HAS_NAMESPACE
		}
	}
}
#endif

#include "Camera.inl"

#endif
