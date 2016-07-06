#include "OIGAssert.h"
#include "GPGPUDevice.h"

#if CUDA_FOUND
#if _WIN32
#include <windows.h>
#endif
#include <cuda_gl_interop.h>
#include <cuda_runtime.h>
#include "cuda_helper.h"
#endif

#include <boost/unordered_map.hpp>
#include <iostream>

namespace OpenIG {
   namespace Library {
      namespace Graphics {

         int GPGPUDevice::m_RefCount = 0;

         class MapGraphicsBuffersToCUDAResources
         {
         public:
            MapGraphicsBuffersToCUDAResources(){}
            virtual ~MapGraphicsBuffersToCUDAResources(){}

            virtual void RegisterBuffer(void* bufferID) = 0;
            virtual void UnregisterBuffer(void* bufferID) = 0;

            virtual void* MapBuffer(void* bufferID, const size_t& numBytes) = 0;
            virtual void UnmapBuffer(void* bufferID) = 0;

         private:
         };
#if CUDA_FOUND
         class MapGraphicsBuffersToCUDAResourcesGL : public MapGraphicsBuffersToCUDAResources
         {
         public:
            MapGraphicsBuffersToCUDAResourcesGL();
            virtual ~MapGraphicsBuffersToCUDAResourcesGL();

            void RegisterBuffer(void* bufferID);
            void UnregisterBuffer(void* bufferID);

            void* MapBuffer(void* bufferID, const size_t& numBytes);
            void UnmapBuffer(void* bufferID);
         private:

            typedef boost::unordered_map<GLuint, cudaGraphicsResource*> MapGLToCUDA;
            MapGLToCUDA m_MapGLToCUDA;
         };

         MapGraphicsBuffersToCUDAResourcesGL::MapGraphicsBuffersToCUDAResourcesGL()
         {

         }
         MapGraphicsBuffersToCUDAResourcesGL::~MapGraphicsBuffersToCUDAResourcesGL()
         {

         }

         void MapGraphicsBuffersToCUDAResourcesGL::RegisterBuffer(void* bufferID)
         {
#if CUDA_FOUND
               GLuint _bufferID = *(GLuint*)(bufferID);
               ASSERT_PREDICATE(m_MapGLToCUDA.find(_bufferID)==m_MapGLToCUDA.end());
               if (m_MapGLToCUDA.find(_bufferID)!=m_MapGLToCUDA.end())
               {
                  std::cout<<"Resource already registered with CUDA"<<std::endl;
                  return;
               }
               cudaGraphicsResource *cuda_buffer_resource = 0;
               cudaError_t err = cudaGraphicsGLRegisterBuffer(&cuda_buffer_resource, _bufferID, cudaGraphicsMapFlagsWriteDiscard);
               ASSERT_PREDICATE(err==cudaSuccess);
               if (err==cudaSuccess)
               {
                  m_MapGLToCUDA.insert(std::make_pair(_bufferID, cuda_buffer_resource));
               }
               else
               {
                  std::cout<<"Could not register resource"<<std::endl;
               }
#endif
         }
         void MapGraphicsBuffersToCUDAResourcesGL::UnregisterBuffer(void* bufferID)
         {
#if CUDA_FOUND
            GLuint _bufferID = *(GLuint*)(bufferID);
            MapGLToCUDA::const_iterator it = m_MapGLToCUDA.find(_bufferID);
            ASSERT_PREDICATE(it!=m_MapGLToCUDA.end());
            if (it==m_MapGLToCUDA.end())
            {
               std::cout<<"Resource not found/registered with CUDA"<<std::endl;
               return;
            }

            cudaGraphicsResource *cuda_buffer_resource = it->second;
            cudaError_t err = cudaGraphicsUnregisterResource(cuda_buffer_resource);
            ASSERT_PREDICATE(err==cudaSuccess);
            m_MapGLToCUDA.erase(it);
#endif
         }

         void* MapGraphicsBuffersToCUDAResourcesGL::MapBuffer(void* bufferID, const size_t& numBytes)
         {
#if CUDA_FOUND
            GLuint _bufferID = *(GLuint*)(bufferID);
            MapGLToCUDA::const_iterator it = m_MapGLToCUDA.find(_bufferID);
            ASSERT_PREDICATE(it!=m_MapGLToCUDA.end());
            if (it==m_MapGLToCUDA.end())
            {
               std::cout<<"Resource not found/registered with CUDA"<<std::endl;
               return 0;
            }

            cudaGraphicsResource *cuda_buffer_resource = it->second;

            cudaError_t err;
            err = cudaGraphicsMapResources(1, &cuda_buffer_resource, 0);
            ASSERT_PREDICATE(err==cudaSuccess);
            int* pCudaBuffer = 0;
            size_t numBytesLocked = 0;
            err = cudaGraphicsResourceGetMappedPointer((void**)&pCudaBuffer, &numBytesLocked,cuda_buffer_resource);
            ASSERT_PREDICATE(numBytesLocked==numBytes);
            ASSERT_PREDICATE(err==cudaSuccess);
#endif
            return pCudaBuffer;
         }
         void MapGraphicsBuffersToCUDAResourcesGL::UnmapBuffer(void* bufferID)
         {
#if CUDA_FOUND
            GLuint _bufferID = *(GLuint*)(bufferID);
            MapGLToCUDA::const_iterator it = m_MapGLToCUDA.find(_bufferID);
            ASSERT_PREDICATE(it!=m_MapGLToCUDA.end());
            if (it==m_MapGLToCUDA.end())
            {
               std::cout<<"Resource not found/registered with CUDA"<<std::endl;
               return;
            }

            cudaGraphicsResource *cuda_buffer_resource = it->second;

            cudaError_t err;
            err = cudaGraphicsUnmapResources(1, &cuda_buffer_resource, 0);
            ASSERT_PREDICATE(err==cudaSuccess);
#endif
         }

#endif

         GPGPUDevice::GPGPUDevice(GraphicsAPI api)
            : m_Initialized(false)
            , m_GraphicsAPI(api)
            , m_InitializationTried(false)
            , m_MapGraphicsBuffersToCUDAResources(0)
         {
            ++m_RefCount;
            ASSERT_PREDICATE(m_RefCount==1);

#if CUDA_FOUND
            if (m_GraphicsAPI==API_OPENGL)
            {
               m_MapGraphicsBuffersToCUDAResources = new MapGraphicsBuffersToCUDAResourcesGL;
            }
#endif
         }
         GPGPUDevice::~GPGPUDevice()
         {
            --m_RefCount;
            ASSERT_PREDICATE(m_RefCount==0);

            delete m_MapGraphicsBuffersToCUDAResources;m_MapGraphicsBuffersToCUDAResources=0;
         }

         void GPGPUDevice::RegisterBuffer(void* bufferID)
         {
            m_MapGraphicsBuffersToCUDAResources->RegisterBuffer(bufferID);
         }
         void GPGPUDevice::UnregisterBuffer(void* bufferID)
         {
            m_MapGraphicsBuffersToCUDAResources->UnregisterBuffer(bufferID);
         }

         void* GPGPUDevice::MapBuffer(void* bufferID, const size_t& numBytes)
         {
            return m_MapGraphicsBuffersToCUDAResources->MapBuffer(bufferID, numBytes);
         }
         void GPGPUDevice::UnmapBuffer(void* bufferID)
         {
            m_MapGraphicsBuffersToCUDAResources->UnmapBuffer(bufferID);
         }

         bool GPGPUDevice::Initialized(void) const
         {
            return m_Initialized;
         }
         void GPGPUDevice::Initialize(void)
         {
            if (m_Initialized || m_InitializationTried)
            {
               return;
            }
#if CUDA_FOUND
            if (m_GraphicsAPI==API_OPENGL)
            {
               m_InitializationTried = true;
               cudaError_t err = cudaSuccess;
               ASSERT_PREDICATE(err==cudaSuccess);
               if (err==cudaSuccess)
               {
                  m_Initialized = true;
               }
               else
               {
                  m_Initialized = false;
               }

            }
#endif
         }

      }}}