#include "OIGAssert.h"
#include "TileSpaceLightGridCPUImpl.h"

#include "Camera.h"
#include "Light.h"

#if TBB_FOUND
#include "TBBFunctional.h"
#endif


namespace OpenIG {
   namespace Library {
      namespace Graphics {

         TileSpaceLightGridCPUImpl::TileSpaceLightGridCPUImpl(const Vector2_uint32& tileSize, bool bUseMultipleCPUCores)
            : TileSpaceLightGridImpl(tileSize)

            , m_GridOffsetsAndSizeWidthInInt32(0)
            , m_GridCounts(0)
            , m_GridOffsets(0)
            , m_pGridOffsetsAndSizesData(0)
			, m_bUseMultipleCPUCores(bUseMultipleCPUCores)
         {

         }
         TileSpaceLightGridCPUImpl::~TileSpaceLightGridCPUImpl()
         {
            TearDownGridOffsetsAndCounts();
         }

		 void TileSpaceLightGridCPUImpl::UpdateScreenSpaceRectangles(const VectorLights& frustumVisibleLights, const Camera_64* pCamera, const Vector2_uint32& viewportSize, const Vector2_uint32& tileSize, size_t start, size_t numLights)
		 {
			 for (uint32 i = start; i < start + numLights; ++i)
			 {
				 //ASSERT_PREDICATE(rect.vMin.x <= rect.vMax.x && rect.vMin.y <= rect.vMax.y);
				 m_ScreenRects[i] = pCamera->GetScreenAABB(frustumVisibleLights[i]->_GetWorldAABB(), viewportSize);
			 }
		 }

         void TileSpaceLightGridCPUImpl::UpdateScreenSpaceRectangles(const VectorLights& frustumVisibleLights, const Camera_64* pCamera, const Vector2_uint32& viewportSize, const Vector2_uint32& tileSize)
         {
            OIG_UNREFERENCED_VARIABLE(tileSize);
	
			if (m_ScreenRects.size()<frustumVisibleLights.size())
			{
				m_ScreenRects.resize(frustumVisibleLights.size());
			}
            
#if TBB_FOUND
			if (m_bUseMultipleCPUCores)
			{
				TBBFunctional::Func f = boost::bind(&TileSpaceLightGridCPUImpl::UpdateScreenSpaceRectangles, this, frustumVisibleLights, pCamera, viewportSize, tileSize, _1, _2);
				tbb::parallel_for(tbb::blocked_range<size_t>(0, frustumVisibleLights.size()), TBBFunctional(f)); 
			}
			else
			{
				UpdateScreenSpaceRectangles(frustumVisibleLights, pCamera, viewportSize, tileSize, 0, frustumVisibleLights.size());
			}
#else
			UpdateScreenSpaceRectangles(frustumVisibleLights, pCamera, viewportSize, tileSize, 0, frustumVisibleLights.size());
#endif
         }

         int TileSpaceLightGridCPUImpl::GetGridCount(int x, int y, const Vector2_uint32& lightGridMaxDims)
         {
            return m_GridCounts[x + y * lightGridMaxDims.x];
         }
         int TileSpaceLightGridCPUImpl::GetGridOffset(int x, int y, const Vector2_uint32& lightGridMaxDims)
         {
            return m_GridOffsets[x + y * lightGridMaxDims.x];
         }

         int& TileSpaceLightGridCPUImpl::GridCount(int x, int y, const Vector2_uint32& lightGridMaxDims)
         {
            return m_GridCounts[x + y * lightGridMaxDims.x];
         }

         int& TileSpaceLightGridCPUImpl::GridOffset(int x, int y, const Vector2_uint32& lightGridMaxDims)
         {
            return m_GridOffsets[x + y * lightGridMaxDims.x];
         }

         void TileSpaceLightGridCPUImpl::TearDownGridOffsetsAndCounts(void)
         {
            delete[] m_GridOffsets; m_GridOffsets = 0;
            delete[] m_GridCounts; m_GridCounts = 0;
            delete[] m_pGridOffsetsAndSizesData; m_pGridOffsetsAndSizesData = 0;
         }


         void TileSpaceLightGridCPUImpl::ResizeGridOffsetsAndCountsIfNecessary(const Vector2_uint32& tileGridMaxDims)
         {
            uint32 requiredWidth = Math::GetUpperPowerOfTwo(tileGridMaxDims.x*tileGridMaxDims.y);
            if (m_GridOffsetsAndSizeWidthInInt32 >= requiredWidth)
            {
               return;
            }

            TearDownGridOffsetsAndCounts();

            m_GridOffsets = new int32[requiredWidth];
            m_GridCounts = new int32[requiredWidth];
            m_pGridOffsetsAndSizesData = new int32[requiredWidth * 2];

            m_GridOffsetsAndSizeWidthInInt32 = requiredWidth;
         }

         void TileSpaceLightGridCPUImpl::Update(const VectorLights& frustumVisibleLights, const Camera_64* pCamera, const Vector2_uint32& viewportSize
            , void* gridoffsetandcount_graphics_interop_buffer, size_t gridoffsetandcount_bytes
            , void* tilelightindexlist_graphics_interop_buffer, size_t tilelightindexlist_bytes)
         {
#if 0
            m_gridMinMaxZ = gridMinMaxZ;
            m_minMaxGridValid = !gridMinMaxZ.empty();

            const float2 *gridMinMaxZPtr = m_minMaxGridValid ? &m_gridMinMaxZ[0] : 0;
#endif
            OIG_UNREFERENCED_VARIABLE(gridoffsetandcount_graphics_interop_buffer);
            OIG_UNREFERENCED_VARIABLE(gridoffsetandcount_bytes);
            OIG_UNREFERENCED_VARIABLE(tilelightindexlist_graphics_interop_buffer);
            OIG_UNREFERENCED_VARIABLE(tilelightindexlist_bytes);

            Vector2_uint32 tileGridMaxDims = ComputeGridMaxDims(viewportSize, m_TileSize);

            ResizeGridOffsetsAndCountsIfNecessary(tileGridMaxDims);

            m_MaxTileLightCount = 0;

            UpdateScreenSpaceRectangles(frustumVisibleLights, pCamera, viewportSize, m_TileSize);

            memset(m_GridOffsets, 0, m_GridOffsetsAndSizeWidthInInt32*sizeof(int32));
            memset(m_GridCounts, 0, m_GridOffsetsAndSizeWidthInInt32*sizeof(int32));

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


#ifdef _DEBUG
            if (!m_TileLightIndexLists.empty())
            {
               memset(&m_TileLightIndexLists[0], 0, m_TileLightIndexLists.size() * sizeof(m_TileLightIndexLists[0]));
            }
#endif // _DEBUG

            m_TileLightIndexLists.resize(totalus);

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

            memset(m_pGridOffsetsAndSizesData, GetTileGridOffsetAndSizeSizeInBytes(), 0);

            for (uint32 y = 0; y < tileGridMaxDims.y; ++y)
            {
               for (uint32 x = 0; x < tileGridMaxDims.x; ++x)
               {
                  size_t offset = x + y * tileGridMaxDims.x;

                  m_pGridOffsetsAndSizesData[offset * 2 + 0] = m_GridCounts[offset];
                  m_pGridOffsetsAndSizesData[offset * 2 + 1] = m_GridOffsets[offset];
               }
            }
         }
      }
   }


}