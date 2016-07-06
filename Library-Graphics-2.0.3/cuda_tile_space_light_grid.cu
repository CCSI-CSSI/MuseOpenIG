#ifndef CUDA_TILE_SPACE_LIGHT_GRID
#define CUDA_TILE_SPACE_LIGHT_GRID

#include "cuda_tile_space_light_grid.cuh"
#define COMPILE_FOR_CUDA 1
#include "AxisAlignedBoundingBox.h"
#include "Vector2.h"
#include "Matrix4.h"
#include "Camera.h"
#include "CameraFwdDeclare.h"
#include "ScreenRect.h"


//using namespace OpenIG::Library::Graphics;

__global__
void _update_grid_counts(
   int* m_GridOffsetsAndCountsCUDA
 , const AxisAlignedBoundingBox_64* pLightWorldAABBs
 , unsigned int viewport_width, unsigned int viewport_height
 , const double _matViewProjection[16]
 , unsigned int tileSizeX, unsigned int tileSizeY
 , unsigned int rejectAreaW, unsigned int rejectAreaH
 , unsigned int tileGridMaxDimsX, unsigned int tileGridMaxDimsY
 , size_t numLights)
{
   int lightIndex = blockDim.x * blockIdx.x + threadIdx.x;

   if (lightIndex<numLights)
   {
      Vector2_uint32 viewPortSize(viewport_width, viewport_height);
      Vector2_uint32 m_TileSize(tileSizeX, tileSizeY);
      Vector2_uint32 rejectArea(rejectAreaW, rejectAreaH);
      
      ScreenRect rect = Camera_64::GetScreenAABB(pLightWorldAABBs[lightIndex], viewPortSize, Matrix4_64(_matViewProjection));

      // Culled
      if (rect.width() < rejectArea.x && rect.height() < rejectArea.y)
      {
         return;
      }

      Vector2_uint32 tileGridMaxDims(tileGridMaxDimsX, tileGridMaxDimsY);

      Vector2_uint32 tileLowerBound = Math::Clamp(rect.vMin / m_TileSize, Vector2_uint32(0,0), tileGridMaxDims + 1);
      Vector2_uint32 tileUpperBound = Math::Clamp((rect.vMax + m_TileSize - 1) / m_TileSize, Vector2_uint32(0,0), tileGridMaxDims + 1);

      // For each light find whether its spans a tile
      // Update grid count if so
      for (uint32 y = tileLowerBound.y; y < tileUpperBound.y; ++y)
      {
         for (uint32 x = tileLowerBound.x; x < tileUpperBound.x; ++x)
         {
            int gridCountIndex = (x + y * tileGridMaxDims.x)*2;
            atomicAdd(&m_GridOffsetsAndCountsCUDA[gridCountIndex],1);
         }
      }
   }
}

__global__
void _update_grid_offsets(int* m_GridOffsetsAndCountsCUDA
, unsigned int viewport_width, unsigned int viewport_height
, unsigned int tileSizeX, unsigned int tileSizeY
, unsigned int tileGridMaxDimsX, unsigned int tileGridMaxDimsY
)
{
   Vector2_uint32 viewPortSize(viewport_width, viewport_height);
   Vector2_uint32 m_TileSize(tileSizeX, tileSizeY);
   Vector2_uint32 tileGridMaxDims(tileGridMaxDimsX, tileGridMaxDimsY);

   uint32 offset = 0;
   for (uint32 y = 0; y < tileGridMaxDims.y; ++y)
   {
      for (uint32 x = 0; x < tileGridMaxDims.x; ++x)
      {
         int index = (x + y * tileGridMaxDims.x)*2;

         uint32 count = m_GridOffsetsAndCountsCUDA[index];
         // set offset to be just past end, then decrement while filling in
         m_GridOffsetsAndCountsCUDA[index+1] = offset + count;
         offset += count;
      }
   }
}

__global__
void _update_light_index_list_and_offsets(int* m_GridOffsetsAndCountsCUDA
, int* m_TileLightIndexListsCUDA
, const AxisAlignedBoundingBox_64* pLightWorldAABBs
, unsigned int viewport_width, unsigned int viewport_height
, const double _matViewProjection[16]
, unsigned int tileSizeX, unsigned int tileSizeY
, unsigned int rejectAreaW, unsigned int rejectAreaH
, unsigned int tileGridMaxDimsX, unsigned int tileGridMaxDimsY
, int numLights
)
{
   int lightIndex = blockDim.x * blockIdx.x + threadIdx.x;

   if (lightIndex<numLights)
   {
      Vector2_uint32 viewPortSize(viewport_width, viewport_height);
      Vector2_uint32 m_TileSize(tileSizeX, tileSizeY);
      Vector2_uint32 rejectArea(rejectAreaW, rejectAreaH);

      ScreenRect rect = Camera_64::GetScreenAABB(pLightWorldAABBs[lightIndex], viewPortSize, Matrix4_64(_matViewProjection));


      // Culled
      if (rect.width() < rejectArea.x && rect.height() < rejectArea.y)
      {
         return;
      }


      Vector2_uint32 tileGridMaxDims(tileGridMaxDimsX, tileGridMaxDimsY);

      Vector2_uint32 tileLowerBound = Math::Clamp(rect.vMin / m_TileSize, Vector2_uint32(0,0), tileGridMaxDims + 1);
      Vector2_uint32 tileUpperBound = Math::Clamp((rect.vMax + m_TileSize - 1) / m_TileSize, Vector2_uint32(0,0), tileGridMaxDims + 1);

      // For all the tiles it spans
      for (uint32 y = tileLowerBound.y; y < tileUpperBound.y; ++y)
      {
         for (uint32 x = tileLowerBound.x; x < tileUpperBound.x; ++x)
         {
               uint32 gridOffsetIndex = (x + y * tileGridMaxDims.x)*2+1;
               // store reversely into next free slot

               uint32 offset = atomicSub(&m_GridOffsetsAndCountsCUDA[gridOffsetIndex],1);
               m_TileLightIndexListsCUDA[offset-1] = lightIndex;
         }
      }
   }
}


extern "C"
void update_grid_counts_offsets(
   int* m_GridOffsetsAndCountsCUDA
 , int* m_TileLightIndexListsCUDA
 , const AxisAlignedBoundingBox_64* pLightWorldAABBs
 , unsigned int viewport_width, unsigned int viewport_height
 , const double _matViewProjection[16]
 , unsigned int tileSizeX, unsigned int tileSizeY
 , unsigned int rejectAreaW, unsigned int rejectAreaH
 , unsigned int tileGridMaxDimsX, unsigned int tileGridMaxDimsY
 , size_t numLights)
{
      // PPP: How to compute this
   int threadsPerBlock = 512;
   int blocksPerGrid = (numLights + threadsPerBlock - 1)/threadsPerBlock;

   _update_grid_counts<<<blocksPerGrid, threadsPerBlock>>>
      ( m_GridOffsetsAndCountsCUDA
      , pLightWorldAABBs
      , viewport_width, viewport_height
      , _matViewProjection
      , tileSizeX, tileSizeY
      , rejectAreaW, rejectAreaH
	  , tileGridMaxDimsX, tileGridMaxDimsY
      , numLights);

   // PPP: Could actually parallize this
   _update_grid_offsets<<<1, 1>>>(m_GridOffsetsAndCountsCUDA
      , viewport_width, viewport_height
      , tileSizeX, tileSizeY
	  , tileGridMaxDimsX, tileGridMaxDimsY);

   _update_light_index_list_and_offsets<<<blocksPerGrid, threadsPerBlock>>>
      ( m_GridOffsetsAndCountsCUDA
      , m_TileLightIndexListsCUDA
	  , pLightWorldAABBs
      , viewport_width, viewport_height
	  , _matViewProjection
      , tileSizeX, tileSizeY
      , rejectAreaW, rejectAreaH
	  , tileGridMaxDimsX, tileGridMaxDimsY
      , numLights
      );
}


#endif