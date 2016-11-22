/*
-----------------------------------------------------------------------------
File:        Light.h
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

#if defined(OPENIG_SDK)
    #include <OpenIG-Graphics/Export.h>
    #include <OpenIG-Graphics/ColorValue.h>
    #include <OpenIG-Graphics/VectorForwardDeclare.h>
    #include <OpenIG-Graphics/Vector3.h>
    #include <OpenIG-Graphics/AxisAlignedBoundingBox.h>
    #include <OpenIG-Graphics/UserObjectBindings.h>
    #include <OpenIG-Graphics/Signal.h>
    #include <OpenIG-Graphics/ForwardDeclare.h>
#else
    #include <Library-Graphics/Export.h>
    #include <Library-Graphics/ColorValue.h>
    #include <Library-Graphics/VectorForwardDeclare.h>
    #include <Library-Graphics/Vector3.h>
    #include <Library-Graphics/AxisAlignedBoundingBox.h>
    #include <Library-Graphics/UserObjectBindings.h>
    #include <Library-Graphics/Signal.h>
    #include <Library-Graphics/ForwardDeclare.h>
#endif

FORWARD_DECLARE(Light)

namespace OpenIG {
    namespace Library {
        namespace Graphics {

            class Light;

            typedef signal1< const Light* > LightSignal;

            enum LightType
            {
                LT_DIRECTIONAL = 0
                , LT_POINT = 1
                , LT_SPOT = 2
                , LT_UNKNOWN = 3
            };

            class IGLIBGRAPHICS_EXPORT Light
            {
            public:
                Light(void);
                virtual ~Light();

                void SetOn(bool bOn);
                bool IsOn(void) const;

                // This is separate so as to enable pooling of lights
                // so that a previous light of type x in the pool
                // can be picked up and used as a light of a different
                // type y, or the same type for that matter
                void SetLightType(LightType lightType);
                LightType GetLightType(void) const;

                void SetAmbientColor(const ColorValue& color);
                void SetDiffuseColor(const ColorValue& color);
                void SetSpecularColor(const ColorValue& color);

                const ColorValue& GetAmbientColor(void) const;
                const ColorValue& GetDiffuseColor(void) const;
                const ColorValue& GetSpecularColor(void) const;

                void SetPosition(const Vector3_64& vPosition);
                const Vector3_64& GetPosition(void) const;

                void SetDirection(const Vector3_64& vDirection);
                const Vector3_64& GetDirection(void) const;

                void SetRanges(float32 fStart, float32 fEnd);
                void GetRanges(float32& fStart, float32& fEnd) const;

                void SetSpotLightAngles(float32 fInnerAngleDegrees, float32 fOuterAngleDegrees);
                void GetSpotLightAngles(float32& fInnerAngleDegrees, float32& fOuterAngleDegrees) const;

                float32 GetInnerAngle(void) const;
                float32 GetOuterAngle(void) const;

                void    SetFalloff(float fFallOff);
                float32 GetFalloff(void) const;

                UserObjectBindings& GetUserObjectBindings(void);
                const UserObjectBindings& GetUserObjectBindings(void) const;

                LightSignal signal_LightUpdated;
                LightSignal signal_LightBoundsUpdated;
                LightSignal signal_LightDestroyed;

                const AxisAlignedBoundingBox_64& _GetWorldAABB(void) const;

                float64 fTempValue;

                void SetVec3_32_1(const Vector3_32& vec){ m_Vec1 = vec; }
                void SetVec3_32_2(const Vector3_32& vec){ m_Vec2 = vec; }
                const Vector3_32& GetVec1(void) const { return m_Vec1; }
                const Vector3_32& GetVec2(void) const { return m_Vec2; }

                // Up to 3 custom floats
                void SetCustomFloats(float vals[3]);
                void GetCustomFloats(float vals[3]) const;
            private:

                Vector3_32 m_Vec1;
                Vector3_32 m_Vec2;

                LightType m_LightType;

                ColorValue m_AmbientColor;
                ColorValue m_DiffuseColor;
                ColorValue m_SpecularColor;

                // Applicable only to point lights and spot lights
                Vector3_64 m_vPosition;
                // Applicable only to directional lights
                Vector3_64 m_vDirection;

                float32 m_fStartRange;
                float32 m_fEndRange;

                // Angle of a spotlight's inner cone - that is, the fully illuminated spotlight cone.
                float32 m_fInnerAngle;
                // Angle defining the outer edge of the spotlight's outer cone. Points outside this cone are not lit by the spotlight. This value must be between 0 and pi.
                float32 m_fOuterAngle;
                // Decrease in illumination between a spotlight's inner cone and the outer edge of the outer cone.
                float32 m_fFallOff;

                bool m_bIsOn;

                UserObjectBindings m_UserObjectBindings;

                void UpdateBounds(void);
                AxisAlignedBoundingBox_64 m_WorldAABB;

                float m_fCustomFloats[3];

                bool m_UpdateSignalEnabled;
            };

        }
    }
}
