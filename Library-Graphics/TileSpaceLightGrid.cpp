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
#include "CommonUtils.h"
#include "TileSpaceLightGrid.h"
#include "Camera.h"
#include "Light.h"

#include <iostream>

namespace OpenIG {
	namespace Library {
		namespace Graphics {

			TileSpaceLightGrid::TileSpaceLightGrid(const Vector2_uint32& tileSize)
				: m_ScreenSpaceRejectArea(Vector2_uint32::ZERO)
				, m_TileSize(tileSize)
				, m_GridOffsetsAndSizeWidth(0)
				, m_GridCounts(0)
				, m_GridOffsets(0)
				, m_pGridOffsetsAndSizesData(0)
			{

			}
			TileSpaceLightGrid::~TileSpaceLightGrid()
			{
				TearDownGridOffsetsAndCounts();
			}

			void TileSpaceLightGrid::UpdateScreenSpaceRectangles(const VectorLights& frustumVisibleLights, const Camera_64* pCamera, const Vector2_uint32& viewportSize, const Vector2_uint32& tileSize)
			{
				OIG_UNREFERENCED_VARIABLE(tileSize);
				m_ViewSpaceLights.clear();
				m_ScreenRects.clear();

				for (uint32 i = 0; i < frustumVisibleLights.size(); ++i)
				{
					const Light* pLight = frustumVisibleLights[i];
					ScreenRect rect = pCamera->GetScreenAABB(pLight->_GetWorldAABB(), viewportSize);
					ASSERT_PREDICATE(rect.vMin.x <= rect.vMax.x && rect.vMin.y <= rect.vMax.y);
					m_ScreenRects.push_back(rect);
					// save light in model space
					m_ViewSpaceLights.push_back(pLight);
				}
			}

			int TileSpaceLightGrid::GetGridCount(int x, int y, const Vector2_uint32& lightGridMaxDims)
			{
				return m_GridCounts[x + y * lightGridMaxDims.x];
			}
			int TileSpaceLightGrid::GetGridOffset(int x, int y, const Vector2_uint32& lightGridMaxDims)
			{
				return m_GridOffsets[x + y * lightGridMaxDims.x];
			}

			int& TileSpaceLightGrid::GridCount(int x, int y, const Vector2_uint32& lightGridMaxDims)
			{
				return m_GridCounts[x + y * lightGridMaxDims.x];
			}

			int& TileSpaceLightGrid::GridOffset(int x, int y, const Vector2_uint32& lightGridMaxDims)
			{
				return m_GridOffsets[x + y * lightGridMaxDims.x];
			}

			void TileSpaceLightGrid::TearDownGridOffsetsAndCounts(void)
			{
				delete[] m_GridOffsets; m_GridOffsets = 0;
				delete[] m_GridCounts; m_GridCounts = 0;
				delete[] m_pGridOffsetsAndSizesData; m_pGridOffsetsAndSizesData = 0;
			}

			Vector2_uint32 TileSpaceLightGrid::ComputeGridMaxDims(const Vector2_uint32& viewportSize) const
			{
				ASSERT_PREDICATE(m_TileSize.x > 0 && m_TileSize.y > 0);

				Vector2_uint32 vGridMaxDims;
				vGridMaxDims.x = ((viewportSize.x + m_TileSize.x - 1) / m_TileSize.x);
				vGridMaxDims.y = ((viewportSize.y + m_TileSize.y - 1) / m_TileSize.y);

				return vGridMaxDims;
			}
			void TileSpaceLightGrid::ResizeGridOffsetsAndCountsIfNecessary(const Vector2_uint32& tileGridMaxDims)
			{
				uint32 requiredWidth = Math::GetUpperPowerOfTwo(tileGridMaxDims.x*tileGridMaxDims.y);
				if (m_GridOffsetsAndSizeWidth >= requiredWidth)
				{
					return;
				}

				TearDownGridOffsetsAndCounts();

				m_GridOffsets = new int32[requiredWidth];
				m_GridCounts = new int32[requiredWidth];
				m_pGridOffsetsAndSizesData = new int32[requiredWidth * 2];

				m_GridOffsetsAndSizeWidth = requiredWidth;
			}

			void TileSpaceLightGrid::Update(const VectorLights& frustumVisibleLights, const Camera_64* pCamera, const Vector2_uint32& viewportSize)
			{
#if 0
				m_gridMinMaxZ = gridMinMaxZ;
				m_minMaxGridValid = !gridMinMaxZ.empty();

				const float2 *gridMinMaxZPtr = m_minMaxGridValid ? &m_gridMinMaxZ[0] : 0;
#endif

				Vector2_uint32 tileGridMaxDims = ComputeGridMaxDims(viewportSize);

				ResizeGridOffsetsAndCountsIfNecessary(tileGridMaxDims);

				m_MaxTileLightCount = 0;

				UpdateScreenSpaceRectangles(frustumVisibleLights, pCamera, viewportSize, m_TileSize);

				memset(m_GridOffsets, 0, m_GridOffsetsAndSizeWidth*sizeof(int32));
				memset(m_GridCounts, 0, m_GridOffsetsAndSizeWidth*sizeof(int32));

				// For each light find whether its spans a tile
				// Update grid count if so
				int totalus = 0;
				{
					for (size_t i = 0; i < m_ScreenRects.size(); ++i)
					{
						const ScreenRect& rect = m_ScreenRects[i];

						if (rect.width() < m_ScreenSpaceRejectArea.x && rect.height() < m_ScreenSpaceRejectArea.y)
						{
							continue;
						}
#if 0
						Light* light = m_viewSpaceLights[i];
#endif
						Vector2_uint32 tileLowerBound = Math::Clamp(rect.vMin / m_TileSize, Vector2_uint32::ZERO, tileGridMaxDims + 1);
						Vector2_uint32 tileUpperBound = Math::Clamp((rect.vMax + m_TileSize - 1) / m_TileSize, Vector2_uint32::ZERO, tileGridMaxDims + 1);

						for (uint32 y = tileLowerBound.y; y < tileUpperBound.y; ++y)
						{
							for (uint32 x = tileLowerBound.x; x < tileUpperBound.x; ++x)
							{
#if 0
								if (!m_minMaxGridValid || testDepthBounds(gridMinMaxZPtr[y * m_gridDim.x + x], light))
#endif
								{
									GridCount(x, y, tileGridMaxDims) += 1;
									++totalus;
								}
							}
						}
					}
				}
				m_TileLightIndexLists.resize(totalus);
#ifdef _DEBUG
				if (!m_TileLightIndexLists.empty())
				{
					memset(&m_TileLightIndexLists[0], 0, m_TileLightIndexLists.size() * sizeof(m_TileLightIndexLists[0]));
				}
#endif // _DEBUG

				// For each grid find out what its grid count is
				// and update grid offset for that tile
				uint32 offset = 0;
				{
					for (uint32 y = 0; y < tileGridMaxDims.y; ++y)
					{
						for (uint32 x = 0; x < tileGridMaxDims.x; ++x)
						{
							uint32 count = GridCount(x, y, tileGridMaxDims);
							// set offset to be just past end, then decrement while filling in
							GridOffset(x, y, tileGridMaxDims) = offset + count;
							offset += count;

							// for debug/profiling etc.
							m_MaxTileLightCount = std::max(m_MaxTileLightCount, count);
						}
					}
				}

				bool ok = m_ScreenRects.size() && !m_TileLightIndexLists.empty();
				if (!ok)
					return;

				// Update the tile light index list
				int *data = &m_TileLightIndexLists[0];

				// For each light
				for (size_t i = 0; i < m_ScreenRects.size(); ++i)
				{
					const ScreenRect& rect = m_ScreenRects[i];

					if (rect.width() < m_ScreenSpaceRejectArea.x && rect.height() < m_ScreenSpaceRejectArea.y)
					{
						continue;
					}

					Vector2_uint32 tileLowerBound = Math::Clamp(rect.vMin / m_TileSize, Vector2_uint32::ZERO, tileGridMaxDims + 1);
					Vector2_uint32 tileUpperBound = Math::Clamp((rect.vMax + m_TileSize - 1) / m_TileSize, Vector2_uint32::ZERO, tileGridMaxDims + 1);

					// For all the tiles it spans
					for (uint32 y = tileLowerBound.y; y < tileUpperBound.y; ++y)
					{
						for (uint32 x = tileLowerBound.x; x < tileUpperBound.x; ++x)
						{
#if 0
							Light* light = m_viewSpaceLights[i];
							if (!m_minMaxGridValid || testDepthBounds(gridMinMaxZPtr[y * m_gridDim.x + x], light))
#endif
							{
								// store reversely into next free slot
								uint32 offset = GridOffset(x, y, tileGridMaxDims) - 1;
								uint32 lightId = uint32(i);
								data[offset] = lightId;
								GridOffset(x, y, tileGridMaxDims) = offset;
							}
						}
					}
				}

				const int32 *counts = GetTileCountsDataPtr();
				const int32 *offsets = GetTileDataPtr();

				memset(m_pGridOffsetsAndSizesData, GetTileGridOffsetAndSizeSizeInBytes(), 0);

				for (uint32 y = 0; y < tileGridMaxDims.y; ++y)
				{
					for (uint32 x = 0; x < tileGridMaxDims.x; ++x)
					{
						size_t offset = x + y * tileGridMaxDims.x;

						m_pGridOffsetsAndSizesData[offset * 2 + 0] = counts[offset];
						m_pGridOffsetsAndSizesData[offset * 2 + 1] = offsets[offset];
					}
				}
			}

			void TileSpaceLightGrid::SetScreenAreaCullSize(const Vector2_uint32& val)
			{
				m_ScreenSpaceRejectArea = val;
			}
			const Vector2_uint32& TileSpaceLightGrid::GetScreenAreaCullSize(void) const
			{
				return m_ScreenSpaceRejectArea;
			}
		}
	}
}
