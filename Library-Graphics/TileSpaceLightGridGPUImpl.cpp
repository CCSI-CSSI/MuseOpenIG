#define OIG_ENABLE_ASSERTS 1
#include "OIGAssert.h"
#include "TileSpaceLightGridGPUImpl.h"
#include "GPGPUDevice.h"

#include "Camera.h"
#include "Light.h"

#include <cuda_runtime.h>
#include "cuda_tile_space_light_grid_func_decl.h"

namespace OpenIG {
	namespace Library {
		namespace Graphics {

			TileSpaceLightGridGPUImpl::TileSpaceLightGridGPUImpl(const Vector2_uint32& tileSize, GPGPUDevice* gpgupdevice)
				: TileSpaceLightGridImpl(tileSize)
				, m_GridOffsetsAndSizeWidthInInt32(0)

				, m_GridOffsetsAndCountsCUDA(0)
				, m_GridOffsetsAndCountsCUDAToCPU(0)

				, m_pMatViewProjectionCUDA(0)
				, m_pLightWorldAABBsCUDA(0)
				, m_szLightWorldAABBsCUDA(0)


				, m_TileLightIndexListsCUDA(0)
				, m_szTileLightIndexListsCUDA(0)
				, m_TileLightIndexListsCUDAToCPU(0)

				, m_pTotalUSCuda(0)
				, m_GPGPUDevice(gpgupdevice)
			{

			}
			TileSpaceLightGridGPUImpl::~TileSpaceLightGridGPUImpl()
			{
				TearDownGridOffsetsAndCounts();
			}

			void TileSpaceLightGridGPUImpl::resizeLightAABBsCUDAIfNecessary(size_t numLights)
			{
				size_t szRequired = numLights*sizeof(AxisAlignedBoundingBox_64);
				if (m_szLightWorldAABBsCUDA<szRequired)
				{
					if (m_pLightWorldAABBsCUDA!=0) 
					{
						cudaFree(m_pLightWorldAABBsCUDA);
						m_pLightWorldAABBsCUDA = 0;
					}
				}

				if (m_pLightWorldAABBsCUDA==0)
				{
					cudaError_t err;
					err = cudaMalloc((void**)&m_pLightWorldAABBsCUDA, szRequired);
					ASSERT_PREDICATE(err==cudaSuccess);
					m_szLightWorldAABBsCUDA = szRequired;
				}
				if (m_VecLightWorldAABBs.capacity()<numLights)
				{
					m_VecLightWorldAABBs.reserve(Math::GetUpperPowerOfTwo(numLights));
				}
				m_VecLightWorldAABBs.resize(numLights);
			}

			void TileSpaceLightGridGPUImpl::allocateCudaConstantsIfNecessary(void)
			{
				cudaError_t err = cudaSuccess;

				if (m_pMatViewProjectionCUDA==0)
				{
					err = cudaMalloc((void**)&m_pMatViewProjectionCUDA, 16*sizeof(double));
					ASSERT_PREDICATE(err==cudaSuccess);
				}
				if (m_pTotalUSCuda==0)
				{
					err = cudaMalloc((void**)&m_pTotalUSCuda, sizeof(int));
					ASSERT_PREDICATE(err==cudaSuccess);
				}
			}

			static void fillWorldAABBs(std::vector<AxisAlignedBoundingBox_64>& vec, const VectorLights& lights, size_t start, size_t num)
			{
				for(size_t i = start; i < start + num; ++i)
				{
					const AxisAlignedBoundingBox_64& srcBox = lights[i]->_GetWorldAABB();
					AxisAlignedBoundingBox_64& dstBox = vec[i];
					memcpy(&dstBox, srcBox.GetMin().ptr(), sizeof(AxisAlignedBoundingBox_64));
				}
			}

			void TileSpaceLightGridGPUImpl::updateInputsToCuda(const VectorLights& frustumVisibleLights, const Camera_64* pCamera, const Vector2_uint32& viewportSize)
			{
				if (frustumVisibleLights.size()==0)
				{
					return;
				}
				
				fillWorldAABBs(m_VecLightWorldAABBs, frustumVisibleLights, 0, frustumVisibleLights.size());

				cudaError_t err;

				err = cudaMemcpyAsync(m_pLightWorldAABBsCUDA, &m_VecLightWorldAABBs.front(), sizeof(AxisAlignedBoundingBox_64)*frustumVisibleLights.size(), cudaMemcpyHostToDevice);
				ASSERT_PREDICATE(err==cudaSuccess);

				err = cudaMemcpyAsync(m_pMatViewProjectionCUDA, pCamera->GetViewProjectionMatrix().ptr(), 16*sizeof(float64), cudaMemcpyHostToDevice);
				ASSERT_PREDICATE(err==cudaSuccess);
			}

			void TileSpaceLightGridGPUImpl::TearDownGridOffsetsAndCounts(void)
			{
				delete [] m_GridOffsetsAndCountsCUDAToCPU; m_GridOffsetsAndCountsCUDAToCPU = 0;

				cudaFree(m_GridOffsetsAndCountsCUDA); m_GridOffsetsAndCountsCUDA = 0;
			}

			void TileSpaceLightGridGPUImpl::ResizeGridOffsetsAndCountsIfNecessary(const Vector2_uint32& tileGridMaxDims)
			{
				uint32 requiredWidth = Math::GetUpperPowerOfTwo(tileGridMaxDims.x*tileGridMaxDims.y);
				if (m_GridOffsetsAndSizeWidthInInt32 >= requiredWidth)
				{
					return;
				}

				TearDownGridOffsetsAndCounts();

				cudaError_t err;

				err = cudaMalloc((void**)&m_GridOffsetsAndCountsCUDA, requiredWidth*sizeof(int32)*2);
				ASSERT_PREDICATE(err==cudaSuccess);

				m_GridOffsetsAndCountsCUDAToCPU = new int32[requiredWidth * 2];

				m_GridOffsetsAndSizeWidthInInt32 = requiredWidth;
			}

			void TileSpaceLightGridGPUImpl::Update(const VectorLights& frustumVisibleLights, const Camera_64* pCamera, const Vector2_uint32& viewportSize
				, void* gridoffsetandcount_graphics_interop_buffer, size_t gridoffsetandcount_bytes
				, void* tilelightindexlist_graphics_interop_buffer, size_t tilelightindexlist_bytes)
			{
				resizeLightAABBsCUDAIfNecessary(frustumVisibleLights.size());
				allocateCudaConstantsIfNecessary();
				updateInputsToCuda(frustumVisibleLights, pCamera, viewportSize);

				Vector2_uint32 tileGridMaxDims = ComputeGridMaxDims(viewportSize, m_TileSize);

				if (gridoffsetandcount_graphics_interop_buffer==0)
				{
					ResizeGridOffsetsAndCountsIfNecessary(tileGridMaxDims);
				}

#if 0
				m_TileLightIndexLists.resize(totalus);

				bool ok = m_ScreenRects.size() && !m_TileLightIndexLists.empty();
				if (!ok)
					return;

				// Update the tile light index list
				int *data = &m_TileLightIndexLists[0];
#endif

				cudaError_t err;

				err = cudaMemsetAsync(m_pTotalUSCuda, 0, sizeof(int32));
				ASSERT_PREDICATE(err==cudaSuccess);

				if (gridoffsetandcount_graphics_interop_buffer==0&&tilelightindexlist_graphics_interop_buffer==0)
				{
					err = cudaMemsetAsync(m_GridOffsetsAndCountsCUDA, 0, m_GridOffsetsAndSizeWidthInInt32*sizeof(int32)*2);
					ASSERT_PREDICATE(err==cudaSuccess);

					// This is actually a rough estimate
					// The actual size is totaluscuda*sizeof(int)
					size_t szTileLightIndexListsCUDARequired = Math::GetUpperPowerOfTwo(
						TileSpaceLightGridImpl::GetTileLightIndexListLengthUpperBound(frustumVisibleLights.size(), viewportSize, m_TileSize)
						*sizeof(int));
					if (m_szTileLightIndexListsCUDA<szTileLightIndexListsCUDARequired)
					{
						cudaFree(m_TileLightIndexListsCUDA);m_TileLightIndexListsCUDA= 0;
						delete [] m_TileLightIndexListsCUDAToCPU;m_TileLightIndexListsCUDAToCPU=0;
					}
					if (m_TileLightIndexListsCUDA==0)
					{
						cudaError_t err = cudaMalloc((void**)&m_TileLightIndexListsCUDA, szTileLightIndexListsCUDARequired);   
						ASSERT_PREDICATE(err==cudaSuccess);
						m_TileLightIndexListsCUDAToCPU = new int[szTileLightIndexListsCUDARequired];
						m_szTileLightIndexListsCUDA = szTileLightIndexListsCUDARequired;
					}

					update_grid_counts_offsets(
						m_GridOffsetsAndCountsCUDA
						, m_TileLightIndexListsCUDA
						, m_pLightWorldAABBsCUDA
						, viewportSize.x, viewportSize.y
						, m_pMatViewProjectionCUDA
						, m_TileSize.x, m_TileSize.y
						, m_ScreenSpaceRejectArea.x, m_ScreenSpaceRejectArea.y
						, tileGridMaxDims.x, tileGridMaxDims.y
						, frustumVisibleLights.size());
				}
				else if (gridoffsetandcount_graphics_interop_buffer&&tilelightindexlist_graphics_interop_buffer)
				{
					ASSERT_PREDICATE(m_GPGPUDevice);

					void * poffsetcountBuffer = m_GPGPUDevice->MapBuffer(gridoffsetandcount_graphics_interop_buffer, gridoffsetandcount_bytes);;
					void * ptileindexBuffer = m_GPGPUDevice->MapBuffer(tilelightindexlist_graphics_interop_buffer, tilelightindexlist_bytes);;

					err = cudaMemsetAsync(poffsetcountBuffer, 0, gridoffsetandcount_bytes);
					ASSERT_PREDICATE(err==cudaSuccess);

					update_grid_counts_offsets(
						(int*)(poffsetcountBuffer)
						, (int*)(ptileindexBuffer)
						, m_pLightWorldAABBsCUDA
						, viewportSize.x, viewportSize.y
						, m_pMatViewProjectionCUDA
						, m_TileSize.x, m_TileSize.y
						, m_ScreenSpaceRejectArea.x, m_ScreenSpaceRejectArea.y
						, tileGridMaxDims.x, tileGridMaxDims.y
						, frustumVisibleLights.size());

					m_GPGPUDevice->UnmapBuffer(gridoffsetandcount_graphics_interop_buffer);
					m_GPGPUDevice->UnmapBuffer(tilelightindexlist_graphics_interop_buffer);
				}
				else
				{
					ASSERT_PREDICATE(false);
				}

				if (gridoffsetandcount_graphics_interop_buffer==0&&tilelightindexlist_graphics_interop_buffer==0)
				{
					int totalusGPU = 0;
					err = cudaMemcpyAsync(&totalusGPU, m_pTotalUSCuda, sizeof(int32), cudaMemcpyDeviceToHost);
					ASSERT_PREDICATE(err==cudaSuccess);
					//ASSERT_PREDICATE(totalusGPU==totalus);

					m_szTileLightIndexListsCUDAActual = totalusGPU;

					err = cudaMemcpyAsync(m_GridOffsetsAndCountsCUDAToCPU, m_GridOffsetsAndCountsCUDA, m_GridOffsetsAndSizeWidthInInt32*sizeof(int32)*2, cudaMemcpyDeviceToHost);
					ASSERT_PREDICATE(err==cudaSuccess);

					err = cudaMemcpyAsync(m_TileLightIndexListsCUDAToCPU, m_TileLightIndexListsCUDA, m_szTileLightIndexListsCUDAActual*sizeof(int32), cudaMemcpyDeviceToHost);
					ASSERT_PREDICATE(err==cudaSuccess);
				}
			}

			const int *TileSpaceLightGridGPUImpl::GetTileLightIndexListsPtr() const 
			{ 
				ASSERT_PREDICATE(false);
				return &m_TileLightIndexListsCUDAToCPU[0]; 
			}
			uint32 TileSpaceLightGridGPUImpl::GetTotalTileLightIndexListLength() const 
			{ 
				ASSERT_PREDICATE(false);
				return uint32(m_szTileLightIndexListsCUDAActual); 
			}

		}}}