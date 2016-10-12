/*
-----------------------------------------------------------------------------
File:        GPGPUDevice.h
Copyright:   Copyright (C) 2007-2015 Poojan Prabhu. All rights reserved.
Created:     05/01/2016
Last edit:   05/01/2016
Author:      Poojan Prabhu
E-mail:      openig@compro.net

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
#else
#include <Library-Graphics/export.h>
#endif
#include <cstddef>

namespace OpenIG {
   namespace Library {
      namespace Graphics {

         class MapGraphicsBuffersToCUDAResources;

         enum GraphicsAPI
         {
            API_OPENGL = 0, API_DIRECTX = 1
         };

         class IGLIBGRAPHICS_EXPORT GPGPUDevice
         {
         public:
            GPGPUDevice(GraphicsAPI api);
            virtual ~GPGPUDevice();
            void Initialize(void);
            bool Initialized(void) const;

            void RegisterBuffer(void* bufferID);
            void UnregisterBuffer(void* bufferID);

            void* MapBuffer(void* bufferID, const size_t& numBytes);
            void UnmapBuffer(void* bufferID);
            
         private:
            GraphicsAPI m_GraphicsAPI;
            bool m_Initialized;
            bool m_InitializationTried;

            static int m_RefCount;

            MapGraphicsBuffersToCUDAResources* m_MapGraphicsBuffersToCUDAResources;

         };

}}}

#pragma warning( pop )
