/*
-----------------------------------------------------------------------------
File:        Plane.inl
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

			template<class T>
			Plane<T>::Plane()
			{

			}

			template<class T>
			Plane<T>::~Plane()
			{

			}

			template<class T>
			PlaneSide Plane<T>::GetSide(const AxisAlignedBoundingBox<T>& box) const
			{
				if (box.IsNull())
					return NO_SIDE;
				if (box.IsInfinite())
					return BOTH_SIDE;

				return GetSide(box.GetCenter(), box.GetHalfSize());
			}

			template<class T>
			PlaneSide Plane<T>::GetSide(const Vector3<T>& centre, const Vector3<T>& halfSize) const
			{
				// Calculate the distance between box centre and the plane
				T dist = GetDistance(centre);

				// Calculate the maximise allows absolute distance for
				// the distance between box centre and plane
				T maxAbsDist = vNormal.AbsDotProduct(halfSize);

				if (dist < -maxAbsDist)
					return NEGATIVE_SIDE;

				if (dist > +maxAbsDist)
					return POSITIVE_SIDE;

				return BOTH_SIDE;
			}

			template<class T>
			T Plane<T>::GetDistance(const Vector3<T>& rhs) const
			{
				return vNormal.DotProduct(rhs) + d;
			}

		}
	}
}
