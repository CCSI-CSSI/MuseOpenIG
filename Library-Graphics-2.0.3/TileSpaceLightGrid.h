/*
-----------------------------------------------------------------------------
File:        TileSpaceLightGrid.h
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
#pragma warning( push )
#pragma warning( disable : 4251 )

#if defined(OPENIG_SDK)
#include <OpenIG-Graphics/export.h>
#include <OpenIG-Graphics/CameraFwdDeclare.h>
#include <OpenIG-Graphics/AxisAlignedBoundingBox.h>
#include <OpenIG-Graphics/ForwardDeclare.h>
#include <OpenIG-Graphics/Vector2.h>
#include <OpenIG-Graphics/ScreenRect.h>
#include <OpenIG-Graphics/DataFormat.h>
#else
#include <Library-Graphics/export.h>
#include <Library-Graphics/CameraFwdDeclare.h>
#include <Library-Graphics/AxisAlignedBoundingBox.h>
#include <Library-Graphics/ForwardDeclare.h>
#include <Library-Graphics/Vector2.h>
#include <Library-Graphics/ScreenRect.h>
#include <Library-Graphics/DataFormat.h>
#endif

FORWARD_DECLARE(Light)


#define VERIFY_TILE_GRID_SLN 0

	namespace OpenIG {
		namespace Library {
			namespace Graphics {

				class TileSpaceLightGridImpl;
				class GPGPUDevice;

				class IGLIBGRAPHICS_EXPORT TileSpaceLightGrid
				{
				public:
					TileSpaceLightGrid(const Vector2_uint32& tileSize, GPGPUDevice* gpgpudevice, bool bUseMultipleCPUCores);
					virtual ~TileSpaceLightGrid();

					void Update(const VectorLights& frustumVisibleLights, const Camera_64* pCamera, const Vector2_uint32& viewportSize
						, void* gridoffsetandcount_graphics_interop_buffer, size_t gridoffsetandcount_bytes
						, void* tilelightindexlist_graphics_interop_buffer, size_t tilelightindexlist_bytes);

					const int *GetTileLightIndexListsPtr() const;
					uint32     GetTotalTileLightIndexListLength() const;

					// Rejects lights with screen space area less than this value
					void SetScreenAreaCullSize(const Vector2_uint32& val);
					const Vector2_uint32& GetScreenAreaCullSize(void) const;

					const int32* GetTileGridOffsetAndSizeDataPtr(void) const;
					DataFormat   GetTileGridOffsetAndSizeDataFormat(void) const;
					uint32       GetTileGridOffsetAndSizeWidth(void) const;
					uint32       GetTileGridOffsetAndSizeSizeInBytes(void) const;

					static uint32       GetTileGridOffsetAndSizeWidth(const Vector2_uint32& viewportSize, const Vector2_uint32& tileSize);
					static uint32       GetTileGridOffsetAndSizeSizeInBytes(const Vector2_uint32& viewportSize, const Vector2_uint32& tileSize);
					static uint32       GetTileLightIndexListLengthUpperBound(size_t numLights, const Vector2_uint32& viewportSize, const Vector2_uint32& tileSize);

					const Vector2_uint32& GetTileSize(void) const;
				private:

					TileSpaceLightGridImpl* m_Impl;

					GPGPUDevice* m_GPGPUDevice;

#if VERIFY_TILE_GRID_SLN
					TileSpaceLightGridImpl* m_CPUImpl;
					TileSpaceLightGridImpl* m_GPUImpl;
#endif
				};

			}
		}
}

#pragma warning(pop)
