/*
-----------------------------------------------------------------------------
File:        Octree.h
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

#if defined(OPENIG_SDK)
	#include <OpenIG-Graphics/AxisAlignedBoundingBox.h>
#else
	#include <Library-Graphics/AxisAlignedBoundingBox.h>
#endif

#include <list>

/** Octree datastructure for managing scene nodes.
@remarks
This is a loose octree implementation, meaning that each
octant child of the octree actually overlaps it's siblings by a factor
of .5.  This guarantees that any thing that is half the size of the parent will
fit completely into a child, with no splitting necessary.
*/

#include "OctreeNodeFwdDeclare.h"
#include "CameraFwdDeclare.h"

namespace OpenIG {
	namespace Library {
		namespace Graphics {

			template<class T>
			class Octree
			{
			public:
				Octree(Octree * parent, Octree* treeRoot, size_t maxDepth);
				~Octree();

				/** Adds an Octree scene node to this octree level.
				@remarks
				This is called by the OctreeSceneManager after
				it has determined the correct Octree to insert the node into.
				*/
				void _AddNode(OctreeNode<T> *);

				/** Removes an Octree scene node to this octree level.
				*/
				void _RemoveNode(OctreeNode<T> *);

				/** Returns the number of scene nodes attached to this octree
				*/
				int NumNodes()
				{
					return m_NumNodes;
				};

				/** The bounding box of the octree
				@remarks
				This is used for octant index determination and rendering, but not culling
				*/
				AxisAlignedBoundingBox<T> m_Box;

				/** Vector containing the dimensions of this octree / 2
				*/
				Vector3<T> m_HalfSize;

				/** 3D array of children of this octree.
				@remarks
				Children are dynamically created as needed when nodes are inserted in the Octree.
				If, later, all the nodes are removed from the child, it is still kept around.
				*/
				Octree * m_Children[2][2][2];

				/** Determines if this octree is twice as big as the given box.
				@remarks
				This method is used by the OctreeSceneManager to determine if the given
				box will fit into a child of this octree.
				*/
				bool _IsTwiceSize(const AxisAlignedBoundingBox<T> &box) const;

				/**  Returns the appropriate indexes for the child of this octree into which the box will fit.
				@remarks
				This is used by the OctreeSceneManager to determine which child to traverse next when
				finding the appropriate octree to insert the box.  Since it is a loose octree, only the
				center of the box is checked to determine the octant.
				*/
				void _GetChildIndexes(const AxisAlignedBoundingBox<T> &, int *x, int *y, int *z) const;

				/** Creates the AxisAlignedBoundingBox<T> used for culling this octree.
				@remarks
				Since it's a loose octree, the culling bounds can be different than the actual bounds of the octree.
				*/
				void _GetCullBounds(AxisAlignedBoundingBox<T> *) const;


				typedef std::list< OctreeNode<T> * > NodeList;
				/** Public list of SceneNodes attached to this particular octree
				*/
				NodeList m_Nodes;

				void UpdateOctreeNode(OctreeNode<T>* pOctreeNode);
				void AddOctreeNode(OctreeNode<T>* pOctreeNode, Octree<T>* pOctree, size_t depth = 0);
				void RemoveOctreeNode(OctreeNode<T>* pOctreeNode);

				void FindVisibleObjects(NodeList& visibleNodes, Octree* pOctree, const Camera<T>* pCamera, bool foundVisible);
			protected:

				/** Increments the overall node count of this octree and all its parents
				*/
				inline void _Ref()
				{
					m_NumNodes++;

					if (m_Parent != 0) m_Parent->_Ref();
				};

				/** Decrements the overall node count of this octree and all its parents
				*/
				inline void _UnRef()
				{
					m_NumNodes--;

					if (m_Parent != 0) m_Parent->_UnRef();
				};

				///number of SceneNodes in this octree and all its children.
				int m_NumNodes;

				///parent octree
				Octree * m_Parent;

				///tree root
				Octree * m_Octree;

				size_t m_MaxDepth;
			};

		}
	}
}

#include "Octree.inl"
