/*
-----------------------------------------------------------------------------
File:        LightData.cpp
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
#include "LightData.h"
#include "Light.h"
#include "CommonUtils.h"
#include "VectorUtils.h"
#include <iostream>

namespace OpenIG {
	namespace Library {
		namespace Graphics {

			size_t LightDataStruct::GetSizeInFloats(void)
			{
				return sizeof(LightDataStruct) / sizeof(float32);
			}

			size_t LightDataStruct::GetSizeInBytes(void)
			{
				return sizeof(LightDataStruct);
			}

			void resize(float32*& pData, size_t numOfLights, DataFormat format, size_t& width)
			{
				if (pData)
				{
					delete[] pData;
				}

				size_t requiredSizeInBytes = LightDataStruct::GetSizeInBytes()*numOfLights;
				size_t requiredWidthInRGBAFloatFormat = requiredSizeInBytes / (DataFormatUtils::GetNumComponents(format)*sizeof(float32));
				width = Math::GetUpperPowerOfTwo((uint32)requiredWidthInRGBAFloatFormat);

				pData = new float32[width*DataFormatUtils::GetNumComponents(format)];
				if (pData == 0)
				{
					width = 0;
				}
			}

			LightData::LightData(size_t initalEstimateNumLights, DataFormat format)
				: m_Format(format)
				, m_NumPackedLights(0)
				, m_pData(0)
			{

				ASSERT_PREDICATE(DataFormatUtils::IsFloatFormat(m_Format));
				ASSERT_PREDICATE(DataFormatUtils::GetNumComponents(format) == 4);

				resize(m_pData, initalEstimateNumLights, m_Format, m_Width);
			}
			LightData::~LightData()
			{
				SAFE_DELETE_ARRAY(m_pData);
				m_pData = 0;
			}

			size_t LightData::GetWidth(void) const
			{
				return m_Width;
			}
			DataFormat LightData::GetFormat(void) const
			{
				return m_Format;
			}

			const float32* LightData::GetPackedData(void) const
			{
				return m_pData;
			}

			int LightData::GetPackedDataSizeInBytes() const
			{
				return (int)(m_NumPackedLights*LightDataStruct::GetSizeInBytes());
			}

			void LightData::PackLight(size_t offset, const Light* pLight, bool bUseVec1Vec2)
			{
				ASSERT_PREDICATE_RETURN(pLight);

				float32 fThreeFloats[3];
				Vector3_32 vVec;

				float32* pCurDataPtr = m_pData + offset;
				memcpy(pCurDataPtr, pLight->GetAmbientColor().ptr(), sizeof(float32) * 3); pCurDataPtr += 3;
				memcpy(pCurDataPtr, pLight->GetDiffuseColor().ptr(), sizeof(float32) * 3); pCurDataPtr += 3;
				memcpy(pCurDataPtr, pLight->GetSpecularColor().ptr(), sizeof(float32) * 3); pCurDataPtr += 3;

				if (pLight->GetLightType() == LT_POINT || pLight->GetLightType() == LT_SPOT)
				{
					if (bUseVec1Vec2)
					{
						memcpy(pCurDataPtr, pLight->GetVec1().ptr(), sizeof(float32) * 3); pCurDataPtr += 3;
					}
					else
					{
						vVec = VectorPrecisionConvert::ToFloat32(pLight->GetPosition());
						memcpy(pCurDataPtr, vVec.ptr(), sizeof(float32) * 3); pCurDataPtr += 3;
					}
				}
				else
				{
					memset(pCurDataPtr, 0, sizeof(float32) * 3); pCurDataPtr += 3;
				}

				if (pLight->GetLightType() == LT_DIRECTIONAL || pLight->GetLightType() == LT_SPOT)
				{
					if (bUseVec1Vec2)
					{
						memcpy(pCurDataPtr, pLight->GetVec2().ptr(), sizeof(float32) * 3); pCurDataPtr += 3;
					}
					else
					{
						vVec = VectorPrecisionConvert::ToFloat32(pLight->GetDirection());
						memcpy(pCurDataPtr, vVec.ptr(), sizeof(float32) * 3); pCurDataPtr += 3;
					}
				}
				else
				{
					memset(pCurDataPtr, 0, sizeof(float32) * 3); pCurDataPtr += 3;
				}

				if (pLight->GetLightType() == LT_SPOT)
				{
					pLight->GetSpotLightAngles(fThreeFloats[0], fThreeFloats[1]);
					fThreeFloats[0] = Math::ToRadians(fThreeFloats[0]);
					fThreeFloats[1] = Math::ToRadians(fThreeFloats[1]);

					fThreeFloats[2] = pLight->GetFalloff();

					memcpy(pCurDataPtr, fThreeFloats, sizeof(float32) * 3); pCurDataPtr += 3;
				}
				else
				{
					memset(pCurDataPtr, 0, sizeof(float32) * 3); pCurDataPtr += 3;
				}


				if (pLight->GetLightType() == LT_POINT || pLight->GetLightType() == LT_SPOT)
				{
					pLight->GetRanges(fThreeFloats[0], fThreeFloats[1]);
				}
				else
				{
					fThreeFloats[0] = 0; fThreeFloats[1] = 0;
				}
				fThreeFloats[2] = static_cast<float32>(pLight->GetLightType());
				memcpy(pCurDataPtr, fThreeFloats, sizeof(float32) * 3); pCurDataPtr += 3;

				// Pack the remaining
				pLight->GetCustomFloats(fThreeFloats);
				memcpy(pCurDataPtr, fThreeFloats, sizeof(float32)); pCurDataPtr += 3;
			}

			void LightData::PackLights(const VectorLights& lights)
			{
				ASSERT_PREDICATE(DataFormatUtils::IsFloatFormat(m_Format));

				if (lights.size() > GetNumLightsPackable())
				{
					std::cout << "Number of lights to be packed (" << lights.size() << "), exceeds the capacity of the light data store(" << GetNumLightsPackable() << ")" << std::endl;
					std::cout << "Resizing Light Data store.." << std::endl;
					resize(m_pData, lights.size(), m_Format, m_Width);
					std::cout << "Resized Light Data store, done" << std::endl;
					std::cout << "Num of Lights that can be packed" << std::endl;
					std::cout << "whilst maintaining power of 2: " << GetNumLightsPackable() << std::endl;
				}

				size_t maxIndex = GetNumLightsPackable() - 1;

				size_t currLightIndex = 0;
				m_NumPackedLights = 0;
				for (VectorLights::const_iterator it = lights.begin(); it != lights.end(); ++it)
				{
					Light* pLight = *it;

					size_t offset = currLightIndex*LightDataStruct::GetSizeInFloats();

					PackLight(offset, pLight, true);
					++m_NumPackedLights;
					if (currLightIndex == maxIndex)
						break;
					++currLightIndex;
				}
			}

			size_t LightData::GetNumLightsPackable(void)
			{
				return (DataFormatUtils::GetNumComponents(m_Format)*GetWidth()) / LightDataStruct::GetSizeInFloats();
			}

		}
	}
}
