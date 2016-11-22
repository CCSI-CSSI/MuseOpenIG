#pragma once

#if defined(OPENIG_SDK)
#include <OpenIG-Graphics/Export.h>
#include <OpenIG-Graphics/CameraFwdDeclare.h>
#include <OpenIG-Graphics/AxisAlignedBoundingBox.h>
#include <OpenIG-Graphics/ForwardDeclare.h>
#include <OpenIG-Graphics/Vector2.h>
#include <OpenIG-Graphics/ScreenRect.h>
#include <OpenIG-Graphics/DataFormat.h>
#else
#include <Library-Graphics/Export.h>
#include <Library-Graphics/CameraFwdDeclare.h>
#include <Library-Graphics/AxisAlignedBoundingBox.h>
#include <Library-Graphics/ForwardDeclare.h>
#include <Library-Graphics/Vector2.h>
#include <Library-Graphics/ScreenRect.h>
#include <Library-Graphics/DataFormat.h>
#endif

FORWARD_DECLARE(Light)

namespace OpenIG {
   namespace Library {
      namespace Graphics {

         class TileSpaceLightGridImpl
         {
         public:
            TileSpaceLightGridImpl(const Vector2_uint32& tileSize);
            virtual ~TileSpaceLightGridImpl();
            virtual void Update(const VectorLights& frustumVisibleLights, const Camera_64* pCamera, const Vector2_uint32& viewportSize
               , void* gridoffsetandcount_graphics_interop_buffer, size_t gridoffsetandcount_bytes
               , void* tilelightindexlist_graphics_interop_buffer, size_t tilelightindexlist_bytes)=0;

            // Rejects lights with screen space area less than this value
            void SetScreenAreaCullSize(const Vector2_uint32& val);
            const Vector2_uint32& GetScreenAreaCullSize(void) const;

            virtual const int * GetTileLightIndexListsPtr() const = 0;
            virtual uint32      GetTotalTileLightIndexListLength() const = 0;

            // These are valid only *after* update is called
            virtual const int32* GetTileGridOffsetAndSizeDataPtr(void) const = 0;
            virtual uint32       GetTileGridOffsetAndSizeWidth(void) const = 0;
            virtual uint32       GetTileGridOffsetAndSizeSizeInBytes(void) const = 0;
            virtual DataFormat   GetTileGridOffsetAndSizeDataFormat(void) const = 0;

            // These are valid at any point in time
            static uint32       GetTileGridOffsetAndSizeWidth(const Vector2_uint32& viewportSize, const Vector2_uint32& tileSize);
            static uint32       GetTileGridOffsetAndSizeSizeInBytes(const Vector2_uint32& viewportSize, const Vector2_uint32& tileSize);
            static uint32       GetTileLightIndexListLengthUpperBound(size_t numLights, const Vector2_uint32& viewportSize, const Vector2_uint32& tileSize);
            

            const Vector2_uint32& GetTileSize(void) const{ return m_TileSize; }
         protected:
            Vector2_uint32 m_TileSize;
            Vector2_uint32 m_ScreenSpaceRejectArea;

            static Vector2_uint32 ComputeGridMaxDims(const Vector2_uint32& viewportSize, const Vector2_uint32& tileSize);
         };

}}}
