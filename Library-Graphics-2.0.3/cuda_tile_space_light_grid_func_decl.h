#pragma once

namespace OpenIG { namespace Library { namespace Graphics {
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
      , size_t numLights);
}}}