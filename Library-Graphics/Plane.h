/*
-----------------------------------------------------------------------------
File:        Plane.h
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

namespace OpenIG {
	namespace Library {
		namespace Graphics {

			enum PlaneSide
			{
				NO_SIDE,
				POSITIVE_SIDE,
				NEGATIVE_SIDE,
				BOTH_SIDE
			};

			template<class T>
			class Plane
			{
			public:

				Plane();
				virtual ~Plane();

				Vector3<T> vNormal;
				T d;

				// Returns the side where the alignedBox is. The flag BOTH_SIDE indicates an intersecting box.
				// One corner ON the plane is sufficient to consider the box and the plane intersecting.
				PlaneSide GetSide(const AxisAlignedBoundingBox<T>& box) const;

				// Returns which side of the plane that the given box lies on.
				// The box is defined as centre/half-size pairs for effectively.
				// POSITIVE_SIDE if the box complete lies on the "positive side" of the plane,
				// NEGATIVE_SIDE if the box complete lies on the "negative side" of the plane,
				// and BOTH_SIDE if the box intersects the plane.
				PlaneSide GetSide(const Vector3<T>& centre, const Vector3<T>& halfSize) const;

				//This is a pseudodistance. The sign of the return value is
				//positive if the point is on the positive side of the plane,
				//negative if the point is on the negative side, and zero if the
				//point is on the plane.
				//@par
				//The absolute value of the return value is the true distance only
				//when the plane normal is a unit length vector.
				T GetDistance(const Vector3<T>& rhs) const;
			};

		}
	}
}

#include "Plane.inl"
