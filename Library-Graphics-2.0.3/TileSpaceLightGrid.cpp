/*
-----------------------------------------------------------------------------
File:        TileSpaceLightGrid.cpp
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
#include "OIGAssert.h"
#include "CommonUtils.h"
#include "TileSpaceLightGrid.h"
#include "Camera.h"
#include "Light.h"

#include <iostream>

#include "TileSpaceLightGridCPUImpl.h"
#include "TileSpaceLightGridGPUImpl.h"

namespace OpenIG {
    namespace Library {
        namespace Graphics {

            TileSpaceLightGrid::TileSpaceLightGrid(const Vector2_uint32& tileSize, GPGPUDevice* gpgpudevice, bool bUseMultipleCPUCores)
                : m_GPGPUDevice(gpgpudevice)
            {
#if VERIFY_TILE_GRID_SLN
                m_CPUImpl = new TileSpaceLightGridCPUImpl(tileSize);
#endif
#if CUDA_FOUND
                if(m_GPGPUDevice==0)
                {
                    m_Impl = new TileSpaceLightGridCPUImpl(tileSize, bUseMultipleCPUCores);
                    std::cout<<"F+ culling uses CPU"<<std::endl;
                }
                else
                {
                    m_Impl = new TileSpaceLightGridGPUImpl(tileSize, m_GPGPUDevice);
                    std::cout<<"F+ culling uses GPU (CUDA)"<<std::endl;
                }
#else
                m_Impl = new TileSpaceLightGridCPUImpl(tileSize, bUseMultipleCPUCores);
                std::cout<<"F+ culling uses CPU"<<std::endl;
#endif

            }
            TileSpaceLightGrid::~TileSpaceLightGrid()
            {
#if VERIFY_TILE_GRID_SLN
                delete m_CPUImpl;m_CPUImpl=0;
#endif
                delete m_Impl;m_Impl=0;
            }

            static bool AreEqual(std::set<int>& lhs, std::set<int>& rhs)
            {
                if (lhs.size()!=rhs.size())
                {
                    return false;
                }
                for(std::set<int>::const_iterator it = lhs.begin(); it != lhs.end(); ++it)
                {
                    if (rhs.find(*it)==rhs.end())
                        return false;
                }
                return true;
            }

            static bool SolutionsAreEqual(const TileSpaceLightGridImpl* lhs, const TileSpaceLightGridImpl* rhs)
            {
                const int* lhsGridOffsetsAndCounts = lhs->GetTileGridOffsetAndSizeDataPtr();
                const int* rhsGridOffsetsAndCounts = rhs->GetTileGridOffsetAndSizeDataPtr();

                ASSERT_PREDICATE(lhs->GetTileGridOffsetAndSizeWidth()==rhs->GetTileGridOffsetAndSizeWidth());
                if(lhs->GetTileGridOffsetAndSizeWidth()!=rhs->GetTileGridOffsetAndSizeWidth())
                {
                    return false;
                }

                size_t szGridOffsetsAndSizeWidthInInt32 = lhs->GetTileGridOffsetAndSizeWidth();

                ASSERT_PREDICATE(memcmp(lhsGridOffsetsAndCounts , rhsGridOffsetsAndCounts , szGridOffsetsAndSizeWidthInInt32*sizeof(int)*2)==0);

                if(memcmp(lhsGridOffsetsAndCounts , rhsGridOffsetsAndCounts , szGridOffsetsAndSizeWidthInInt32*sizeof(int)*2)!=0)
                {
                    return false;
                }

                const int* lhsTileLightIndexList = lhs->GetTileLightIndexListsPtr();
                const int* rhsTileLightIndexList = rhs->GetTileLightIndexListsPtr();
                ASSERT_PREDICATE(lhs->GetTotalTileLightIndexListLength()==rhs->GetTotalTileLightIndexListLength());
                if(lhs->GetTotalTileLightIndexListLength()!=rhs->GetTotalTileLightIndexListLength())
                {
                    return false;
                }

                std::set<int> lhsLightSet;
                std::set<int> rhsLightSet;

                //int entriesCompared = 0;
                for(int i = 0; i < szGridOffsetsAndSizeWidthInInt32; ++i)
                {
                    lhsLightSet.clear();rhsLightSet.clear();
                    int lightCount = lhsGridOffsetsAndCounts[i*2];
                    int lightsStartOffset = lhsGridOffsetsAndCounts[i*2+1];

                    for(size_t index = lightsStartOffset; index < lightsStartOffset+lightCount; ++index)
                    {
                        lhsLightSet.insert(lhsTileLightIndexList[index]);
                        rhsLightSet.insert(rhsTileLightIndexList[index]);
                        //    ++entriesCompared;
                    }
                    ASSERT_PREDICATE(AreEqual(lhsLightSet, rhsLightSet));
                    if(AreEqual(lhsLightSet, rhsLightSet)==false)
                    {
                        return false;
                    }
                }
                //std::cout<<"Success, Compared: "<<entriesCompared<<std::endl;
                return true;
            }

            void TileSpaceLightGrid::Update(const VectorLights& frustumVisibleLights, const Camera_64* pCamera, const Vector2_uint32& viewportSize
                , void* gridoffsetandcount_graphics_interop_buffer, size_t gridoffsetandcount_bytes
                , void* tilelightindexlist_graphics_interop_buffer, size_t tilelightindexlist_bytes)
            {
                m_Impl->Update(frustumVisibleLights, pCamera, viewportSize
                    , gridoffsetandcount_graphics_interop_buffer, gridoffsetandcount_bytes
                    , tilelightindexlist_graphics_interop_buffer, tilelightindexlist_bytes);

#if VERIFY_TILE_GRID_SLN
                m_CPUImpl->Update(frustumVisibleLights, pCamera, viewportSize, 0, 0);
                ASSERT_PREDICATE(SolutionsAreEqual(m_CPUImpl, m_Impl));
#endif
            }

            const int *TileSpaceLightGrid::GetTileLightIndexListsPtr() const
            {
                return m_Impl->GetTileLightIndexListsPtr();
            }
            uint32     TileSpaceLightGrid::GetTotalTileLightIndexListLength() const
            {
                return m_Impl->GetTotalTileLightIndexListLength();
            }

            // Rejects lights with screen space area less than this value
            void TileSpaceLightGrid::SetScreenAreaCullSize(const Vector2_uint32& val)
            {
#if VERIFY_TILE_GRID_SLN
                m_CPUImpl->SetScreenAreaCullSize(val);
#endif
                m_Impl->SetScreenAreaCullSize(val);
            }
            const Vector2_uint32& TileSpaceLightGrid::GetScreenAreaCullSize(void) const
            {
                return m_Impl->GetScreenAreaCullSize();
            }

            const int32* TileSpaceLightGrid::GetTileGridOffsetAndSizeDataPtr(void) const
            {
                return m_Impl->GetTileGridOffsetAndSizeDataPtr();
            }
            DataFormat   TileSpaceLightGrid::GetTileGridOffsetAndSizeDataFormat(void) const
            {
                return m_Impl->GetTileGridOffsetAndSizeDataFormat();
            }
            uint32       TileSpaceLightGrid::GetTileGridOffsetAndSizeWidth(void) const
            {
                return m_Impl->GetTileGridOffsetAndSizeWidth();
            }
            uint32       TileSpaceLightGrid::GetTileGridOffsetAndSizeSizeInBytes(void) const
            {
                return m_Impl->GetTileGridOffsetAndSizeSizeInBytes();
            }

            const Vector2_uint32& TileSpaceLightGrid::GetTileSize(void) const
            {
                return m_Impl->GetTileSize();
            }

            uint32 TileSpaceLightGrid::GetTileGridOffsetAndSizeWidth(const Vector2_uint32& viewportSize, const Vector2_uint32& tileSize)
            {
                return TileSpaceLightGridImpl::GetTileGridOffsetAndSizeWidth(viewportSize, tileSize);
            }
            uint32 TileSpaceLightGrid::GetTileGridOffsetAndSizeSizeInBytes(const Vector2_uint32& viewportSize, const Vector2_uint32& tileSize)
            {
                return TileSpaceLightGridImpl::GetTileGridOffsetAndSizeSizeInBytes(viewportSize, tileSize);
            }

            uint32 TileSpaceLightGrid::GetTileLightIndexListLengthUpperBound(size_t numLights, const Vector2_uint32& viewportSize, const Vector2_uint32& tileSize)
            {
                return TileSpaceLightGridImpl::GetTileLightIndexListLengthUpperBound(numLights, viewportSize, tileSize);
            }
        }
    }


}
