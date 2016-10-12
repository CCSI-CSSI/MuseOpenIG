/*
-----------------------------------------------------------------------------
File:        OctreeNode.inl
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
			OctreeNode<T>::OctreeNode()
				: m_Octant(0)
			{

			}
			template<class T>
			OctreeNode<T>::~OctreeNode()
			{

			}

			template <class T>
			void OctreeNode<T>::SetOctant(Octree<T>* octree)
			{
				m_Octant = octree;
			}

			template <class T>
			Octree<T>* OctreeNode<T>::GetOctant(void)
			{
				return m_Octant;
			}


			template <class T>
			bool OctreeNode<T>::IsIn(const AxisAlignedBoundingBox<T>& box)
			{
				// Always fail if not in the scene graph or box is null    
				if (/*!mIsInSceneGraph || */box.IsNull()) return false;

				// Always succeed if AABB is infinite
				if (box.IsInfinite())
					return true;

				Vector3<T> center = m_WorldAABB.GetMax().MidPoint(m_WorldAABB.GetMin());

				Vector3<T> bmin = box.GetMin();
				Vector3<T> bmax = box.GetMax();

				bool centre = (bmax > center && bmin < center);
				if (!centre)
					return false;

				// Even if covering the centre line, need to make sure this BB is not large
				// enough to require being moved up into parent. When added, bboxes would
				// end up in parent due to cascade but when updating need to deal with
				// bbox growing too large for this child
				Vector3<T> octreeSize = bmax - bmin;
				Vector3<T> nodeSize = m_WorldAABB.GetMax() - m_WorldAABB.GetMin();
				return nodeSize < octreeSize;
			}

			template <class T>
			const AxisAlignedBoundingBox<T>& OctreeNode<T>::_GetWorldAABB(void) const
			{
				return m_WorldAABB;
			}

			template <class T>
			UserObjectBindings& OctreeNode<T>::GetUserObjectBindings(void)
			{
				return m_UserObjectBindings;
			}

			template <class T>
			const UserObjectBindings& OctreeNode<T>::GetUserObjectBindings(void) const
			{
				return m_UserObjectBindings;
			}

		}
	}
}
