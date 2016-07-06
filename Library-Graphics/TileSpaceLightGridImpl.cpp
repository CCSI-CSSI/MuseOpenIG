#include "TileSpaceLightGridImpl.h"


namespace OpenIG {
   namespace Library {
      namespace Graphics {

         TileSpaceLightGridImpl::TileSpaceLightGridImpl(const Vector2_uint32& tileSize)
            : m_TileSize(tileSize)
            , m_ScreenSpaceRejectArea(Vector2_uint32::ZERO)
         {

         }

         TileSpaceLightGridImpl::~TileSpaceLightGridImpl()
         {

         }

         void TileSpaceLightGridImpl::SetScreenAreaCullSize(const Vector2_uint32& val)
         {
            m_ScreenSpaceRejectArea = val;
         }
         const Vector2_uint32& TileSpaceLightGridImpl::GetScreenAreaCullSize(void) const
         {
            return m_ScreenSpaceRejectArea;
         }

         Vector2_uint32 TileSpaceLightGridImpl::ComputeGridMaxDims(const Vector2_uint32& viewportSize, const Vector2_uint32& tileSize)
         {
            ASSERT_PREDICATE(tileSize.x > 0 && tileSize.y > 0);

            Vector2_uint32 vGridMaxDims;
            vGridMaxDims.x = ((viewportSize.x + tileSize.x - 1) / tileSize.x);
            vGridMaxDims.y = ((viewportSize.y + tileSize.y - 1) / tileSize.y);

            return vGridMaxDims;
         }

         uint32 TileSpaceLightGridImpl::GetTileGridOffsetAndSizeWidth(const Vector2_uint32& viewportSize, const Vector2_uint32& tileSize)
         {
            Vector2_uint32 tileGridMaxDims = ComputeGridMaxDims(viewportSize, tileSize);
            return Math::GetUpperPowerOfTwo(tileGridMaxDims.x*tileGridMaxDims.y);

         }
         uint32 TileSpaceLightGridImpl::GetTileGridOffsetAndSizeSizeInBytes(const Vector2_uint32& viewportSize, const Vector2_uint32& tileSize)
         {
            return GetTileGridOffsetAndSizeWidth(viewportSize, tileSize)*2*sizeof(int32);
         }

         uint32 TileSpaceLightGridImpl::GetTileLightIndexListLengthUpperBound(size_t numLights, const Vector2_uint32& viewportSize, const Vector2_uint32& tileSize)
         {
            Vector2_uint32 tileGridMaxDims = ComputeGridMaxDims(viewportSize, tileSize);
            return (numLights*tileGridMaxDims.x*tileGridMaxDims.y);
         }
}}}