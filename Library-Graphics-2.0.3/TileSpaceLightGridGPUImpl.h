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

#define VERIFY_UPDATE_GRID_COUNTS 1

namespace OpenIG {
   namespace Library {
      namespace Graphics {

         class GPGPUDevice;

         class TileSpaceLightGridGPUImpl : public TileSpaceLightGridImpl
         {
         public:
            TileSpaceLightGridGPUImpl(const Vector2_uint32& tileSize, GPGPUDevice* gpgupdevice);
            virtual ~TileSpaceLightGridGPUImpl();

            virtual void Update(const VectorLights& frustumVisibleLights, const Camera_64* pCamera, const Vector2_uint32& viewportSize
               , void* gridoffsetandcount_graphics_interop_buffer, size_t gridoffsetandcount_bytes
               , void* tilelightindexlist_graphics_interop_buffer, size_t tilelightindexlist_bytes);

            const int *GetTileLightIndexListsPtr() const;
            uint32     GetTotalTileLightIndexListLength() const;

            const int32* GetTileGridOffsetAndSizeDataPtr(void) const { return m_GridOffsetsAndCountsCUDAToCPU; }
            DataFormat   GetTileGridOffsetAndSizeDataFormat(void) const { return FORMAT_R32G32_SINT; }
            uint32       GetTileGridOffsetAndSizeWidth(void) const{ return m_GridOffsetsAndSizeWidthInInt32; }
            uint32       GetTileGridOffsetAndSizeSizeInBytes(void) const{ return GetTileGridOffsetAndSizeWidth() * 2 * sizeof(int32); }

         private:

            int32* m_GridOffsetsAndCountsCUDA;

            int32* m_GridOffsetsAndCountsCUDAToCPU;

            int32* m_pTotalUSCuda;

            double* m_pMatViewProjectionCUDA;

			std::vector<AxisAlignedBoundingBox_64> m_VecLightWorldAABBs;
            AxisAlignedBoundingBox_64* m_pLightWorldAABBsCUDA;

            size_t m_szLightWorldAABBsCUDA;

            uint32 m_GridOffsetsAndSizeWidthInInt32;

            void TearDownGridOffsetsAndCounts(void);
            void ResizeGridOffsetsAndCountsIfNecessary(const Vector2_uint32& tileGridMaxDims);

            int* m_TileLightIndexListsCUDA;
            size_t m_szTileLightIndexListsCUDA;
            size_t m_szTileLightIndexListsCUDAActual;

            int* m_TileLightIndexListsCUDAToCPU;

            void resizeLightAABBsCUDAIfNecessary(size_t numLights);

            void allocateCudaConstantsIfNecessary(void);

            void updateInputsToCuda(const VectorLights& frustumVisibleLights, const Camera_64* pCamera, const Vector2_uint32& viewportSize);

            GPGPUDevice* m_GPGPUDevice;
         };

}}}