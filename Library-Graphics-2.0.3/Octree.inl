/*
-----------------------------------------------------------------------------
File:        Octree.inl
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
	#include <OpenIG-Graphics/OctreeNode.h>
	#include <OpenIG-Graphics/Camera.h>
#else
	#include <Library-Graphics/OctreeNode.h>
	#include <Library-Graphics/Camera.h>
#endif

namespace OpenIG {
	namespace Library {
		namespace Graphics {

			/** Returns true is the box will fit in a child.
			*/
			template<class T>
			bool Octree<T>::_IsTwiceSize(const AxisAlignedBoundingBox<T> &box) const
			{
				// infinite boxes never fit in a child - always root node
				if (box.IsInfinite())
					return false;

				Vector3<T> halfm_BoxSize = m_Box.GetHalfSize();
				Vector3<T> boxSize = box.GetSize();
				return ((boxSize.x <= halfm_BoxSize.x) && (boxSize.y <= halfm_BoxSize.y) && (boxSize.z <= halfm_BoxSize.z));

			}

			/** It's assumed the the given box has already been proven to fit into
			* a child.  Since it's a loose octree, only the centers need to be
			* compared to find the appropriate node.
			*/
			template<class T>
			void Octree<T>::_GetChildIndexes(const AxisAlignedBoundingBox<T> &box, int *x, int *y, int *z) const
			{
				Vector3<T> max = m_Box.GetMax();
				Vector3<T> min = box.GetMin();

				Vector3<T> center = m_Box.GetMax().MidPoint(m_Box.GetMin());

				Vector3<T> ncenter = box.GetMax().MidPoint(box.GetMin());

				if (ncenter.x > center.x)
					* x = 1;
				else
					*x = 0;

				if (ncenter.y > center.y)
					* y = 1;
				else
					*y = 0;

				if (ncenter.z > center.z)
					* z = 1;
				else
					*z = 0;

			}

			template<class T>
			Octree<T>::Octree(Octree * parent, Octree* treeRoot, size_t maxDepth)
				: m_HalfSize(0, 0, 0)
			{
				//initialize all children to null.
				for (int i = 0; i < 2; i++)
				{
					for (int j = 0; j < 2; j++)
					{
						for (int k = 0; k < 2; k++)
						{
							m_Children[i][j][k] = 0;
						}
					}
				}

				if (parent == 0 && treeRoot == 0)
				{
					m_Octree = this;
				}
				else
				{
					ASSERT_PREDICATE(parent && treeRoot);
					m_Octree = treeRoot;
				}

				m_MaxDepth = maxDepth;

				m_Parent = parent;
				m_NumNodes = 0;
			}

			template<class T>
			Octree<T>::~Octree()
			{
				//initialize all children to null.
				for (int i = 0; i < 2; i++)
				{
					for (int j = 0; j < 2; j++)
					{
						for (int k = 0; k < 2; k++)
						{
							if (m_Children[i][j][k] != 0)
								SAFE_DELETE(m_Children[i][j][k]);
						}
					}
				}
				m_Parent = 0;
			}

			template<class T>
			void Octree<T>::_AddNode(OctreeNode<T> * n)
			{
				m_Nodes.push_back(n);
				n->SetOctant(this);

				//update total counts.
				_Ref();

			}

			template<class T>
			void Octree<T>::_RemoveNode(OctreeNode<T> * n)
			{
				m_Nodes.erase(std::find(m_Nodes.begin(), m_Nodes.end(), n));
				n->SetOctant(0);

				//update total counts.
				_UnRef();
			}

			template<class T>
			void Octree<T>::_GetCullBounds(AxisAlignedBoundingBox<T> *b) const
			{
				b->SetMinMax(m_Box.GetMin() - m_HalfSize, m_Box.GetMax() + m_HalfSize);
			}

			template<class T>
			void Octree<T>::UpdateOctreeNode(OctreeNode<T>* pOctreeNode)
			{
				const AxisAlignedBoundingBox<T>& box = pOctreeNode->m_WorldAABB;
				if (box.IsNull())
					return;

				// octree deleted (shutdown condition)
				if (m_Octree == 0)
					return;

				if (pOctreeNode->GetOctant() == 0)
				{
					//if outside the octree, force into the root node.
					if (!pOctreeNode->IsIn(m_Octree->m_Box))
						m_Octree->_AddNode(pOctreeNode);
					else
						AddOctreeNode(pOctreeNode, m_Octree);
					return;
				}

				if (!pOctreeNode->IsIn(pOctreeNode->GetOctant()->m_Box))
				{
					RemoveOctreeNode(pOctreeNode);

					//if outside the octree, force into the root node.
					if (!pOctreeNode->IsIn(m_Octree->m_Box))
						m_Octree->_AddNode(pOctreeNode);
					else
						AddOctreeNode(pOctreeNode, m_Octree);
				}
			}
			template<class T>
			void Octree<T>::AddOctreeNode(OctreeNode<T>* pOctreeNode, Octree<T>* octant, size_t depth)
			{
				// octree deleted (shutdown condition)
				if (m_Octree == 0)
					return;

				const AxisAlignedBoundingBox<T>& box = pOctreeNode->m_WorldAABB;

				//if the octree is twice as big as the scene node,
				//we will add it to a child.
				if ((depth < m_MaxDepth) && octant->_IsTwiceSize(box))
				{
					int x, y, z;
					octant->_GetChildIndexes(box, &x, &y, &z);

					if (octant->m_Children[x][y][z] == 0)
					{
						octant->m_Children[x][y][z] = new Octree<T>(octant, m_Octree, m_MaxDepth);
						const Vector3<T>& octantMin = octant->m_Box.GetMin();
						const Vector3<T>& octantMax = octant->m_Box.GetMax();
						Vector3<T> min, max;

						if (x == 0)
						{
							min.x = octantMin.x;
							max.x = (octantMin.x + octantMax.x) / 2;
						}

						else
						{
							min.x = (octantMin.x + octantMax.x) / 2;
							max.x = octantMax.x;
						}

						if (y == 0)
						{
							min.y = octantMin.y;
							max.y = (octantMin.y + octantMax.y) / 2;
						}

						else
						{
							min.y = (octantMin.y + octantMax.y) / 2;
							max.y = octantMax.y;
						}

						if (z == 0)
						{
							min.z = octantMin.z;
							max.z = (octantMin.z + octantMax.z) / 2;
						}

						else
						{
							min.z = (octantMin.z + octantMax.z) / 2;
							max.z = octantMax.z;
						}

						octant->m_Children[x][y][z]->m_Box.SetMinMax(min, max);
						octant->m_Children[x][y][z]->m_HalfSize = (max - min) / 2;
					}

					AddOctreeNode(pOctreeNode, octant->m_Children[x][y][z], ++depth);
				}
				else
				{
					octant->_AddNode(pOctreeNode);
				}
			}
			template<class T>
			void Octree<T>::RemoveOctreeNode(OctreeNode<T>* pOctreeNode)
			{
				// octree deleted (shutdown condition)
				if (m_Octree == 0)
					return;

				Octree<T> * oct = pOctreeNode->GetOctant();

				if (oct)
				{
					oct->_RemoveNode(pOctreeNode);
				}

				pOctreeNode->SetOctant(0);
			}

			template<class T>
			void Octree<T>::FindVisibleObjects(NodeList& visibleNodes, Octree<T>* octant, const Camera<T>* camera, bool foundvisible)
			{
				//return immediately if nothing is in the node.
				if (octant->NumNodes() == 0)
					return;

				VisibilityFromCamera v = VISIBILITY_NONE;

				if (foundvisible)
				{
					v = VISIBILITY_FULL;
				}

				else if (octant == m_Octree)
				{
					v = VISIBILITY_PARTIAL;
				}

				else
				{
					AxisAlignedBoundingBox<T> box;
					octant->_GetCullBounds(&box);
					v = camera->GetVisibility(box);
				}

				// if the octant is visible, or if it's the root node...
				if (v != VISIBILITY_NONE)
				{

					//Add stuff to be rendered;
					typename Octree<T>::NodeList::iterator it = octant->m_Nodes.begin();

#if 0
					if ( mShowBoxes )
					{
						mBoxes.push_back( octant->getWireBoundingBox() );
					}
#endif

					bool vis = true;

					while (it != octant->m_Nodes.end())
					{
						OctreeNode<T> * sn = *it;

						// if this octree is partially visible, manually cull all
						// scene nodes attached directly to this level.

						if (v == VISIBILITY_PARTIAL)
							vis = camera->IsVisible(sn->_GetWorldAABB());

						if (vis)
						{

#if 0
							mNumObjects++;
							sn -> _addToRenderQueue(camera, queue, onlyShadowCasters );
#endif

							visibleNodes.push_back(sn);

#if 0
							if ( mDisplayNodes )
								queue -> addRenderable( sn->getDebugRenderable() );

							// check if the scene manager or this node wants the bounding box shown.
							if (sn->getShowBoundingBox() || mShowBoundingBoxes)
								sn->_addBoundingBoxToQueue(queue);
#endif
						}

						++it;
					}

					Octree<T>* child;
					bool childfoundvisible = (v == VISIBILITY_FULL);
					if ((child = octant->m_Children[0][0][0]) != 0)
						FindVisibleObjects(visibleNodes, child, camera, childfoundvisible);

					if ((child = octant->m_Children[1][0][0]) != 0)
						FindVisibleObjects(visibleNodes, child, camera, childfoundvisible);

					if ((child = octant->m_Children[0][1][0]) != 0)
						FindVisibleObjects(visibleNodes, child, camera, childfoundvisible);

					if ((child = octant->m_Children[1][1][0]) != 0)
						FindVisibleObjects(visibleNodes, child, camera, childfoundvisible);

					if ((child = octant->m_Children[0][0][1]) != 0)
						FindVisibleObjects(visibleNodes, child, camera, childfoundvisible);

					if ((child = octant->m_Children[1][0][1]) != 0)
						FindVisibleObjects(visibleNodes, child, camera, childfoundvisible);

					if ((child = octant->m_Children[0][1][1]) != 0)
						FindVisibleObjects(visibleNodes, child, camera, childfoundvisible);

					if ((child = octant->m_Children[1][1][1]) != 0)
						FindVisibleObjects(visibleNodes, child, camera, childfoundvisible);
				}
			}

		}
	}
}
