/*
-----------------------------------------------------------------------------
File:        LightData.h
Copyright:   Copyright (C) 2007-2015 Poojan Prabhu. All rights reserved.
Created:     10/20/2015
Last edit:   12/15/2015
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
#include <Library-Graphics/export.h>

#pragma warning( push )
#pragma warning( disable : 4251 )

#if defined(OPENIG_SDK)
	#include <OpenIG-Graphics/DataFormat.h>
	#include <OpenIG-Graphics/ForwardDeclare.h>
	#include <OpenIG-Graphics/IntSize.h>
#else
	#include <Library-Graphics/DataFormat.h>
	#include <Library-Graphics/ForwardDeclare.h>
	#include <Library-Graphics/IntSize.h>
#endif

FORWARD_DECLARE(Light)

namespace OpenIG {
	namespace Library {
		namespace Graphics {

			struct LightDataStruct
			{
				float ambient[3];
				float diffuse[3];
				float specular[3];

				float position[3];
				float direction[3];

				float spotparams[3];

				float rangesandtype[3];

				float pack[3];

				static size_t GetSizeInFloats(void);
				static size_t GetSizeInBytes(void);
			};

			class IGLIBGRAPHICS_EXPORT LightData
			{
			public:
				LightData(size_t initalEstimateNumLights, DataFormat format);
				virtual ~LightData();

				// Get the total number of lights that can be packed
				size_t GetNumLightsPackable(void);

				size_t GetWidth(void) const;
				DataFormat GetFormat(void) const;

				// Pack lights into the grid
				void PackLights(const VectorLights& lights);
				// Get the light grid data
				const float32* GetPackedData(void) const;
				int            GetPackedDataSizeInBytes(void) const;
			private:
				DataFormat m_Format;
				size_t m_Width;
				float32* m_pData;
				int      m_NumPackedLights;

				void PackLight(size_t offset, const Light* pLight, bool bUseVec1Vec2);
			};

		}
	}
}

#pragma warning( pop )
