/*
-----------------------------------------------------------------------------
File:        Light.cpp
Copyright:   Copyright (C) 2007-2015 Poojan Prabhu. All rights reserved.
Created:     10/20/2015
Last edit:   12/15/2015
Author:      Poojan Prabhu
E-mail:      poojanprabhu@gmail.com

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
#include "CommonTypes.h"
#include "Light.h"

using namespace OpenIG::Library::Graphics;

Light::Light()
    : m_LightType(LT_POINT)
    , m_AmbientColor(ColorValue::WHITE)
    , m_DiffuseColor(ColorValue::WHITE)
    , m_SpecularColor(ColorValue::WHITE)
    , m_vPosition(Vector3_64::ZERO)
    , m_vDirection(Vector3_64::Y_AXIS)
    , m_fStartRange(sqrtf(std::numeric_limits<float32>::max()))
    , m_fEndRange(sqrtf(std::numeric_limits<float32>::max()))
    , m_fInnerAngle(30.0f)
    , m_fOuterAngle(60.0f)
    , m_fFallOff(1.0f)
    , m_bIsOn(true)
	, m_UpdateSignalEnabled(false)
{
    UpdateBounds();

	m_fCustomFloats[0] = m_fCustomFloats[1] = m_fCustomFloats[2] = 0;
}
Light::~Light()
{
    signal_LightDestroyed(this);
}

void Light::SetLightType(LightType lightType)
{
    if (m_LightType==lightType)
    {
        return;
    }
    m_LightType = lightType;
    UpdateBounds();
	if (m_UpdateSignalEnabled)
	{
		signal_LightUpdated(this);
	}
}

LightType Light::GetLightType(void) const
{
    return m_LightType;
}

void Light::SetOn(bool bOn)
{
    m_bIsOn = bOn;
	if (m_UpdateSignalEnabled)
	{
		signal_LightUpdated(this);
	}
}
bool Light::IsOn(void) const
{
    return m_bIsOn;
}

void Light::SetAmbientColor(const ColorValue& color)
{
    if (memcmp(m_AmbientColor.ptr(), color.ptr(), ColorValue::sizeInBytes)==0)
    {
        return;
    }
    m_AmbientColor = color;
	if (m_UpdateSignalEnabled)
	{
		signal_LightUpdated(this);
	}
}
void Light::SetDiffuseColor(const ColorValue& color)
{
    if (memcmp(m_DiffuseColor.ptr(), color.ptr(), ColorValue::sizeInBytes)==0)
    {
        return;
    }
    m_DiffuseColor = color;
	if (m_UpdateSignalEnabled)
	{
		signal_LightUpdated(this);
	}
}
void Light::SetSpecularColor(const ColorValue& color)
{
    if (memcmp(m_SpecularColor.ptr(), color.ptr(), ColorValue::sizeInBytes)==0)
    {
        return;
    }
    m_SpecularColor = color;
	if (m_UpdateSignalEnabled)
	{
		signal_LightUpdated(this);
	}
}

const ColorValue& Light::GetAmbientColor(void) const
{
    return m_AmbientColor;
}
const ColorValue& Light::GetDiffuseColor(void) const
{
    return m_DiffuseColor;
}
const ColorValue& Light::GetSpecularColor(void) const
{
    return m_SpecularColor;
}

void Light::SetPosition(const Vector3_64& vPosition)
{
    ASSERT_PREDICATE(m_LightType==LT_POINT||m_LightType==LT_SPOT);
    if (memcmp(m_vPosition.ptr(), vPosition.ptr(), sizeof(Vector3_64))==0)
    {
        return;
    }
    m_vPosition = vPosition;
    UpdateBounds();
	if (m_UpdateSignalEnabled)
	{
		signal_LightUpdated(this);
	}
}
const Vector3_64& Light::GetPosition(void) const
{
    ASSERT_PREDICATE(m_LightType==LT_POINT||m_LightType==LT_SPOT);
    return m_vPosition;
}
void Light::SetDirection(const Vector3_64& vDirection)
{
    ASSERT_PREDICATE(m_LightType==LT_SPOT||m_LightType==LT_DIRECTIONAL);
    if (memcmp(m_vDirection.ptr(), vDirection.ptr(), sizeof(Vector3_64))==0)
    {
        return;
    }
    m_vDirection = vDirection;
    m_vDirection.Normalize();
	if (m_UpdateSignalEnabled)
	{
		signal_LightUpdated(this);
	}
}
const Vector3_64& Light::GetDirection(void) const
{
    ASSERT_PREDICATE(m_LightType==LT_SPOT||m_LightType==LT_DIRECTIONAL);
    return m_vDirection;
}


void Light::SetRanges(float32 fStart, float32 fEnd)
{
    ASSERT_PREDICATE(m_LightType!=LT_DIRECTIONAL);
    if (memcmp(&m_fStartRange, &fStart, sizeof(float32))==0 && memcmp(&m_fEndRange, &fEnd, sizeof(float32))==0 )
    {
        return;
    }
    m_fStartRange = fStart;
    m_fEndRange   = fEnd;
    UpdateBounds();
	if (m_UpdateSignalEnabled)
	{
		signal_LightUpdated(this);
	}
}
void Light::GetRanges(float32& fStart, float32& fEnd) const
{
    fStart = m_fStartRange;
    fEnd   = m_fEndRange;
}

void Light::SetSpotLightAngles(float32 fInnerAngleDegrees, float32 fOuterAngleDegrees)
{
    ASSERT_PREDICATE(m_LightType==LT_SPOT);
    ASSERT_PREDICATE(0.0f<=fInnerAngleDegrees&&fInnerAngleDegrees<=fOuterAngleDegrees);
    ASSERT_PREDICATE(fOuterAngleDegrees<=180.0f);

    if (memcmp(&m_fStartRange, &fInnerAngleDegrees, sizeof(float32))==0 
        && memcmp(&m_fEndRange, &fOuterAngleDegrees, sizeof(float32))==0
        )
    {
        return;
    }

    m_fInnerAngle = fInnerAngleDegrees;
    m_fOuterAngle = fOuterAngleDegrees;
	if (m_UpdateSignalEnabled)
	{
		signal_LightUpdated(this);
	}
}

void Light::SetFalloff(float fFallOff)
{
    ASSERT_PREDICATE(m_LightType==LT_SPOT);
    if (memcmp(&m_fEndRange, &fFallOff, sizeof(float32))==0)
    {
        return;
    }
	if (m_UpdateSignalEnabled)
	{
		signal_LightUpdated(this);
	}
}
void Light::GetSpotLightAngles(float32& fInnerAngleDegrees, float32& fOuterAngleDegrees) const
{
    fInnerAngleDegrees = GetInnerAngle();
    fOuterAngleDegrees = GetOuterAngle();
}

float32 Light::GetInnerAngle(void) const
{
    // Only valid for spot light
    ASSERT_PREDICATE(m_LightType==LT_SPOT);
    return m_fInnerAngle;
}
float32 Light::GetOuterAngle(void) const
{
    // Only valid for spot light
    ASSERT_PREDICATE(m_LightType==LT_SPOT);
    return m_fOuterAngle;
}
float32 Light::GetFalloff(void) const
{
    // Only valid for spot light
    ASSERT_PREDICATE(m_LightType==LT_SPOT);
    return m_fFallOff;
}

UserObjectBindings& Light::GetUserObjectBindings(void)
{
    return m_UserObjectBindings;
}

const UserObjectBindings& Light::GetUserObjectBindings(void) const
{
    return m_UserObjectBindings;
}

void Light::UpdateBounds(void)
{
    if (GetLightType()==LT_DIRECTIONAL)
    {
        m_WorldAABB.SetInfinite();
    }
    else
    {
        Vector3_64 vCenter = GetPosition();
        Vector3_64 vMin = vCenter - m_fEndRange;
        Vector3_64 vMax = vCenter + m_fEndRange;

        m_WorldAABB.SetMinMax(vMin, vMax);
    }
    signal_LightBoundsUpdated(this);
}

const AxisAlignedBoundingBox_64& Light::_GetWorldAABB(void) const
{
    return m_WorldAABB;
}

void Light::SetCustomFloats(float vals[3])
{
	memcpy(m_fCustomFloats, vals, 3*sizeof(float32));
}
void Light::GetCustomFloats(float vals[3]) const
{
	memcpy(vals, m_fCustomFloats, 3*sizeof(float32));
}
