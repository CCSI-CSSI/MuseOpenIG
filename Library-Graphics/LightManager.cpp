/*
-----------------------------------------------------------------------------
File:        LightManager.cpp
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
#include "CommonUtils.h"
#include "STLUtilities.h"
#include "LightManager.h"
#include "Light.h"
#include "Octree.h"
#include "OctreeNode.h"

#include <iterator>

namespace OpenIG {
	namespace Library {
		namespace Graphics {

			const std::string LightManager::m_StrOctreeNodeKey = "OctreeNode";
			const std::string LightManager::m_StrLightKey = "Light";

			void LightManager::InitOctree(const AxisAlignedBoundingBox_64& box)
			{
				m_Octree->m_Box = box;
				m_Octree->m_HalfSize = ((box.GetMax() - box.GetMin()))*0.5f;
			}

			LightManager::LightManager(/*const AxisAlignedBoundingBox_64& box, size_t depth*/)
				: m_bVectorLightsdirty(false)
			{
				// PPP: TO DO
				m_Octree = 0;
				//      m_Octree = new Octree<float64>(0, 0, depth);
				//      InitOctree(box);
			}
			LightManager::~LightManager()
			{
				SAFE_DELETE(m_Octree);
				destroy_all_from_list(m_Lights);
			}

			Light*  LightManager::CreateLight(LightType lightType)
			{
				Light* pLight = new Light();
				pLight->SetLightType(lightType);
				m_Lights.push_back(pLight);
				ConnectSignals(pLight);
				CreateOctreeNode(pLight);
				m_bVectorLightsdirty = true;
				pLight->signal_LightUpdated(pLight);
				pLight->signal_LightBoundsUpdated(pLight);

				return pLight;
			}
			void    LightManager::DestroyLight(Light* pLight)
			{
				destroy_from_container(m_Lights, pLight);
				m_bVectorLightsdirty = true;
			}

			size_t  LightManager::GetNumLights(void) const
			{
				return m_Lights.size();
			}
			Light* LightManager::GetLight(size_t index)
			{
				return get_nth_item(m_Lights, index);
			}

			struct LightCompare
			{
				LightCompare(const Vector3_64& vPosition) : m_vPosition(vPosition){}
				bool operator()(const Light* pLHS, const Light* pRHS)
				{
					ASSERT_PREDICATE_RETURN_FALSE(pLHS&&pRHS);
					return pLHS->fTempValue < pRHS->fTempValue;
				}
				Vector3_64 m_vPosition;
			};

			const VectorLights& LightManager::GetAllLights(void) const
			{
				if (m_bVectorLightsdirty)
				{
					m_VectorLights.clear();
					std::copy(m_Lights.begin(), m_Lights.end(), std::back_inserter(m_VectorLights));

					m_bVectorLightsdirty = false;
				}
				return m_VectorLights;
			}

			void LightManager::CreateOctreeNode(Light* pLight)
			{
				ASSERT_PREDICATE_RETURN(pLight->GetUserObjectBindings().HasKey(m_StrOctreeNodeKey) == false);
				OctreeNode<float64>* pOctreeNode = new OctreeNode_64();
				pLight->GetUserObjectBindings().SetUserAny(m_StrOctreeNodeKey, pOctreeNode);
				// Store a back pointer
				pOctreeNode->GetUserObjectBindings().SetUserAny(m_StrLightKey, pLight);
			}

			void LightManager::DestroyOctreeNode(Light* pLight)
			{
				OctreeNode<float64>* pOctreeNode = GetOctreeNode(pLight);
				ASSERT_PREDICATE(pOctreeNode);
				SAFE_DELETE(pOctreeNode);
				pLight->GetUserObjectBindings().EraseKey(m_StrOctreeNodeKey);
			}

			OctreeNode_64* LightManager::GetOctreeNode(const Light* pLight) const
			{
				ASSERT_PREDICATE_RETURN_ZERO(pLight && pLight->GetUserObjectBindings().HasKey(m_StrOctreeNodeKey) == true);
				UserObjectBindings& userObjectBindings = const_cast<UserObjectBindings&>(pLight->GetUserObjectBindings());
				OctreeNode_64* pOctreeNode = boost::any_cast<OctreeNode_64*>(userObjectBindings.GetUserAny(m_StrOctreeNodeKey));
				ASSERT_PREDICATE(pOctreeNode);
				return pOctreeNode;
			}

			void LightManager::LightUpdated(const Light* pLight)
			{
				OIG_UNREFERENCED_VARIABLE(pLight);
			}
			void LightManager::LightBoundsUpdated(const Light* pLight)
			{
				ASSERT_PREDICATE_RETURN(pLight);
				OctreeNode_64* pOctreeNode = GetOctreeNode(pLight);
				pOctreeNode->m_WorldAABB = pLight->_GetWorldAABB();
				if (m_Octree) m_Octree->UpdateOctreeNode(pOctreeNode);
			}
			void LightManager::LightDestroyed(const Light* pLight)
			{
				ASSERT_PREDICATE_RETURN(pLight);
				// shutdown condition has m_Octree = 0
				if (m_Octree)
				{
					m_Octree->RemoveOctreeNode(GetOctreeNode(pLight));
				}
				DestroyOctreeNode(const_cast<Light*>(pLight));
				DisconnectSignals(pLight);
			}

			void LightManager::ConnectSignals(Light* pLight)
			{
				Light* pLightNonConst = const_cast<Light*>(pLight);

				//  pLightNonConst->signal_LightUpdated.connect(boost::bind(&LightManager::LightUpdated, this, _1));
				pLightNonConst->signal_LightBoundsUpdated.connect(&LightManager::LightBoundsUpdated, this);
				pLightNonConst->signal_LightDestroyed.connect(&LightManager::LightDestroyed, this);
			}
			void LightManager::DisconnectSignals(const Light* pLight)
			{
				Light* pLightNonConst = const_cast<Light*>(pLight);

				//  pLightNonConst->signal_LightUpdated.disconnect(boost::bind(&LightManager::LightUpdated, this, _1));
				pLightNonConst->signal_LightBoundsUpdated.disconnect(&LightManager::LightBoundsUpdated, this);
				pLightNonConst->signal_LightDestroyed.disconnect(&LightManager::LightDestroyed, this);
			}

			void LightManager::Update(Camera_64* pCamera)
			{
				ASSERT_PREDICATE_RETURN(pCamera);

				// Find the lights affecting the frustum
				FindVisibleObjects(pCamera);
			}

			void LightManager::FindVisibleObjects(Camera_64* pCamera)
			{
				m_VisibleOctreeNodes.clear();
				if (m_Octree) m_Octree->FindVisibleObjects(m_VisibleOctreeNodes, m_Octree, pCamera, false);


				m_DequeLightsScratchPad.clear();

				// PPP: Fix this
#if 0
				BOOST_FOREACH(OctreeNode_64* pOctreeNode, m_VisibleOctreeNodes)
				{
					ASSERT_PREDICATE(pOctreeNode->GetUserObjectBindings().HasKey(m_StrLightKey));
					Light* pLight = boost::any_cast<Light*>
						(pOctreeNode->GetUserObjectBindings().GetUserAny(m_StrLightKey));

					m_DequeLightsScratchPad.push_back(pLight);
				}
#endif
				for (ListLights::const_iterator it = m_Lights.begin(); it != m_Lights.end(); ++it)
				{
					Light* pLight = *it;
					if (pLight->IsOn() == false)
					{
						continue;
					}

					if (pCamera->IsVisible(pLight->_GetWorldAABB()))
					{
						if (pLight->GetLightType()==LT_DIRECTIONAL)
						{
							m_DequeLightsScratchPad.push_front(pLight);
						}
						else
						{
							m_DequeLightsScratchPad.push_back(pLight);
						}	
					}
				}

				m_FrustumAffectingLights.clear();
				for(unsigned int i = 0; i < m_DequeLightsScratchPad.size(); ++i)
				{
					m_FrustumAffectingLights.push_back(m_DequeLightsScratchPad[i]);
				}
			}

			const VectorLights& LightManager::GetFrustumAffectingLights(void) const
			{
				return m_FrustumAffectingLights;
			}

		}
	}
}
