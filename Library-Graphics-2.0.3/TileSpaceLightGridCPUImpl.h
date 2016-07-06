/*
-----------------------------------------------------------------------------
File:        TileSpaceLightGrid.h
Copyright:   Copyright (C) 2007-2015 Poojan Prabhu. All rights reserved.
Created:     05/01/2016
Last edit:   05/01/2016
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
#pragma warning( push )
#pragma warning( disable : 4251 )

#include "TileSpaceLightGridImpl.h"

namespace OpenIG {
   namespace Library {
      namespace Graphics {

      class TileSpaceLightGridCPUImpl : public TileSpaceLightGridImpl
      {
      public:
         TileSpaceLightGridCPUImpl(const Vector2_uint32& tileSize, bool bUseMultipleCPUCores);
         virtual ~TileSpaceLightGridCPUImpl();

         virtual void Update(const VectorLights& frustumVisibleLights, const Camera_64* pCamera, const Vector2_uint32& viewportSize
            , void* gridoffsetandcount_graphics_interop_buffer, size_t gridoffsetandcount_bytes
            , void* tilelightindexlist_graphics_interop_buffer, size_t tilelightindexlist_bytes);

         virtual const int *GetTileLightIndexListsPtr() const { return &m_TileLightIndexLists[0]; }
         virtual uint32     GetTotalTileLightIndexListLength() const { return uint32(m_TileLightIndexLists.size()); }

         virtual const int32* GetTileGridOffsetAndSizeDataPtr(void) const { return m_pGridOffsetsAndSizesData; }
         virtual DataFormat   GetTileGridOffsetAndSizeDataFormat(void) const { return FORMAT_R32G32_SINT; }
         virtual uint32       GetTileGridOffsetAndSizeWidth(void) const{ return m_GridOffsetsAndSizeWidthInInt32; }
         virtual uint32       GetTileGridOffsetAndSizeSizeInBytes(void) const{ return GetTileGridOffsetAndSizeWidth() * 2 * sizeof(int32); }

      private:
         void UpdateScreenSpaceRectangles(const VectorLights& frustumVisibleLights, const Camera_64* pCamera, const Vector2_uint32& viewportSize, const Vector2_uint32& tileSize);
         void UpdateScreenSpaceRectangles(const VectorLights& frustumVisibleLights, const Camera_64* pCamera, const Vector2_uint32& viewportSize, const Vector2_uint32& tileSize, size_t start, size_t num);

#if 0
         const Vector2_32 getTileMinMax(uint32 x, uint32 y) const { return m_minMaxGridValid ? m_gridMinMaxZ[x + y * m_gridDim.x] : chag::make_vector(0.0f, 0.0f); }
         std::vector<chag::float2> m_gridMinMaxZ;
#endif
         int32* m_GridOffsets;
         int32* m_GridCounts;
         int32* m_pGridOffsetsAndSizesData;

         uint32 m_GridOffsetsAndSizeWidthInInt32;

         void TearDownGridOffsetsAndCounts(void);
         void ResizeGridOffsetsAndCountsIfNecessary(const Vector2_uint32& tileGridMaxDims);

         std::vector<int> m_TileLightIndexLists;

         uint32 m_MaxTileLightCount;
         bool m_MinMaxGridValid;

         VectorScreenRectangles m_ScreenRects;

         int GetGridOffset(int x, int y, const Vector2_uint32& lightGridMaxDims);
         int GetGridCount(int x, int y, const Vector2_uint32& lightGridMaxDims);
         int& GridOffset(int x, int y, const Vector2_uint32& lightGridMaxDims);
         int& GridCount(int x, int y, const Vector2_uint32& lightGridMaxDims);

         // Tile accessor functions. Returns tile data for an individual tile
         int GeTileLightCount(uint32 x, uint32 y, const Vector2_uint32& tileGridMaxDims) const
         {
            return m_GridCounts[x + y * tileGridMaxDims.x];
         }

         const bool m_bUseMultipleCPUCores;

      };


}}}
