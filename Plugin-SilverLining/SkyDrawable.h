// Copyright (c) 2008-2012 Sundog Software, LLC. All rights reserved worldwide.

//#******************************************************************************
//#*
//#*      Copyright (C) 2015  Compro Computer Services
//#*      http://openig.compro.net
//#*
//#*      Source available at: https://github.com/CCSI-CSSI/MuseOpenIG
//#*
//#*      This software is released under the LGPL.
//#*
//#*   This software is free software; you can redistribute it and/or modify
//#*   it under the terms of the GNU Lesser General Public License as published
//#*   by the Free Software Foundation; either version 2.1 of the License, or
//#*   (at your option) any later version.
//#*
//#*   This software is distributed in the hope that it will be useful,
//#*   but WITHOUT ANY WARRANTY; without even the implied warranty of
//#*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
//#*   the GNU Lesser General Public License for more details.
//#*
//#*   You should have received a copy of the GNU Lesser General Public License
//#*   along with this library; if not, write to the Free Software
//#*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//#*
//#*****************************************************************************

#pragma once

#include <osg/Drawable>
#include <osg/Light>
#include <osg/Fog>
#include <osgViewer/View>
#include <osg/TexGen>

#include <SilverLining.h>

#include <Core-Base/imagegenerator.h>

#include <Core-OpenIG/openig.h>

namespace OpenIG {
    namespace Plugins {

        class AtmosphereReference;
        class SkyDrawable;

        // Grab the camera and projection matrices for SilverLining. Note we do not use Atmosphere::CullObjects(), since it will
        // interfere with the environment cube maps generated from the render thread.
        struct SilverLiningCullCallback : public osg::Drawable::CullCallback
        {
            SilverLiningCullCallback() : atmosphere(0) {}

            virtual bool cull(osg::NodeVisitor* nv, osg::Drawable* drawable, osg::RenderInfo* renderInfo) const
            {
                if (atmosphere)  {
                    osgUtil::CullVisitor* cv = static_cast<osgUtil::CullVisitor*>(nv);
                    atmosphere->SetCameraMatrix(cv->getModelViewMatrix()->ptr());
                    atmosphere->SetProjectionMatrix(cv->getProjectionMatrix()->ptr());
                }
                return false;
            }

            SilverLining::Atmosphere *atmosphere;
        };

        // Our update callback just marks our bounds dirty each frame (since they move with the camera.)
        struct SilverLiningUpdateCallback : public osg::Drawable::UpdateCallback
        {
            typedef std::vector<osg::Camera*>			Cameras;
            Cameras										cameras;

            SilverLiningUpdateCallback(const Cameras& cs) : cameras(cs) {}

            virtual void update(osg::NodeVisitor*, osg::Drawable* drawable);
        };

        // We also hook in with a bounding box callback to tell OSG how big our skybox is, plus the
        // atmospheric limb if applicable.
        struct SilverLiningSkyComputeBoundingBoxCallback : public osg::Drawable::ComputeBoundingBoxCallback
        {
            typedef std::vector<osg::Camera*>			Cameras;
            Cameras										cameras;

            SilverLiningSkyComputeBoundingBoxCallback(const Cameras& cs) : cameras(cs) {}

            virtual osg::BoundingBox computeBound(const osg::Drawable&) const;
        };

        // The SkyDrawable wraps SilverLining to handle updates, culling, and drawing of the skybox - and works together with a CloudsDrawable
        // to draw the clouds toward the end of the scene.


        class SkyDrawable : public osg::Drawable
        {
        public:
            SkyDrawable();
            SkyDrawable(const std::string& path, osgViewer::CompositeViewer* viewer, osg::Light* light, osg::Fog* fog, bool geocentric = false);

            virtual bool isSameKindAs(const Object* obj) const {
                return dynamic_cast<const SkyDrawable*>(obj) != NULL;
            }
            virtual Object* cloneType() const {
                return new SkyDrawable();
            }
            virtual Object* clone(const osg::CopyOp& copyop) const {
                return new SkyDrawable();
            }

            void            setSkyboxSize(double size) { _skyboxSize = size; }
            double          getSkyboxSize() const { return _skyboxSize; }
            virtual void    drawImplementation(osg::RenderInfo& renderInfo) const;
            void            setTimeOfDay(unsigned int hour, unsigned int minutes);
            void            setVisibility(double visibility);
            void            setRain(double factor);
            void            setSnow(double factor);
            void            setDate(int month, int day, int year);
            void            setTimezone(int tz);
            void            setWind(float speed, float direction);
            void            addCloudLayer(int id, int type, double altitude, double thickness, double density);
            void            removeCloudLayer(int id);
            void            updateCloudLayer(int id, double altitude, double thickness, double density);
            void            removeAllCloudLayers();
            void			setGeocentric(bool geocentric);
            void			setLightingParams(OpenIG::Engine* ig);
            void			setIG(OpenIG::Engine* ig) { _openIG = ig; }
            void            setLogDepthUniforms(SilverLining::Atmosphere *atmosphere, const osg::State* state, float fCoef) const;
            void			setSunMoonBrightness(float day, float night);

            struct SilverLiningParams
            {
                double			cloudsBaseLength;
                double			cloudsBaseWidth;
                bool			generateEnvironmentalMap;

                enum Mask
                {
                    CLOUDS_BASE_LENGTH = 0x1,
                    CLOUDS_BASE_WIDTH = 0x2,
                    ENVIRONMENTAL_MAP = 0x4,
                    ALL = CLOUDS_BASE_LENGTH | CLOUDS_BASE_WIDTH | ENVIRONMENTAL_MAP
                };

                SilverLiningParams()
                    : cloudsBaseLength(100000)
                    , cloudsBaseWidth(100000)
                    , generateEnvironmentalMap(false)
                {
                }

            };

            void setSilverLiningParams(const SilverLiningParams& params, unsigned int mask = OpenIG::Plugins::SkyDrawable::SilverLiningParams::ALL)
            {
                if ((mask & SilverLiningParams::CLOUDS_BASE_LENGTH) == SilverLiningParams::CLOUDS_BASE_LENGTH)
                {
                    _silverLiningParams.cloudsBaseLength = params.cloudsBaseLength;
                    //osg::notify(osg::NOTICE) << "setSilverLiningParams() cloud layer len: " << _silverLiningParams.cloudsBaseLength << std::endl;
                }
                if ((mask & SilverLiningParams::CLOUDS_BASE_WIDTH) == SilverLiningParams::CLOUDS_BASE_WIDTH)
                {
                    _silverLiningParams.cloudsBaseWidth = params.cloudsBaseWidth;
                    //osg::notify(osg::NOTICE) << "setSilverLiningParams() cloud layer wid: " << _silverLiningParams.cloudsBaseWidth << std::endl;
                }
                if ((mask & SilverLiningParams::ENVIRONMENTAL_MAP) == SilverLiningParams::ENVIRONMENTAL_MAP)
                {
                    _silverLiningParams.generateEnvironmentalMap = params.generateEnvironmentalMap;
                    //osg::notify(osg::NOTICE) << "setSilverLiningParams() cloud layer map: " << _silverLiningParams.generateEnvironmentalMap << std::endl;
                }

            }

            void setLight(osg::LightSource* light)
            {
                if (light) _light = light->getLight();
            }

            void setFog(osg::Fog* fog)
            {
                _fog = fog;
            }

            static void setGamma(double gamma, bool use = false);
            static void setLocation(double lat, double lon);
            static void setUTC(int hour, int minutes);
			static void setSkyModel(const std::string& skyModel);

        protected:
            void setLighting(SilverLining::Atmosphere *atm) const;
			void resetLighting(SilverLining::Atmosphere *atmosphere) const;
            void setShadow(SilverLining::Atmosphere *atm, osg::RenderInfo & renderInfo);
            void initializeSilverLining(AtmosphereReference *ar, osg::GLExtensions* ext) const;
            void initializeDrawable();
            void initializeShadow();

            osg::ref_ptr<osgViewer::CompositeViewer>   _viewer;
            double                          _skyboxSize;
            osg::ref_ptr<osg::Light>        _light;
            osg::ref_ptr<osg::Fog>          _fog;
            std::string                     _path;

            unsigned int                    _todHour;
            unsigned int                    _todMinutes;
            bool                            _todDirty;

            float                           _windSpeed;
            float                           _windDirection;
            bool                            _windDirty;
            int                             _windVolumeHandle;

            osg::ref_ptr< osg::TexGen >         _cloudShadowTexgen;
            osg::ref_ptr< osg::Texture2D >      _cloudShadowTextureWhiteSubstitute;
            int                                 _cloudShadowTextureStage;
            int                                 _cloudShadowTexgenStage;
            osg::ref_ptr< osg::Uniform >        _cloudShadowCoordMatrixUniform;
            osg::ref_ptr< osg::Texture2D >      _texture;
            void*                               _shadowTexHandle;
            SilverLining::Matrix4               _lightMVP;
            SilverLining::Matrix4               _worldToShadowMapTexCoord;
            bool                                _cloudShadowsEnabled;
            bool                                _init_shadows_once;
            bool                                _needsShadowUpdate;
            bool                                _cloudReflections;

            SilverLiningCullCallback *                  cullCallback;
            SilverLiningUpdateCallback *                updateCallback;
            SilverLiningSkyComputeBoundingBoxCallback * computeBoundingBoxCallback;

            int                                 _month;
            int                                 _day;
            int                                 _year;
            int                                 _tz;

            double                              _rainFactor;
            double                              _snowFactor;
            bool                                _removeAllCloudLayers;
            bool                                _enableCloudShadows;
            bool								_geocentric;

            SilverLining::Atmosphere*			_slatmosphere;
            OpenIG::Engine*						_openIG;
            mutable float						_inCloudsFogDensity;

            float								_sunMoonBrightnessDay;
            float								_sunMoonBrightnessNight;

            static double						_gamma;
			static bool							_userDefinedGamma;

            static double						_latitude;
            static double						_longitude;

            static bool							_utcSet;
            static int							_utcHour;
            static int							_utcMinutes;

			static std::string					_skyModel;

			osg::Vec4							_originalInsideFogSettings;
			

            struct CloudLayerInfo
            {
                int     _id;
                int     _type;
                int     _handle;
                double  _altitude;
                double  _density;
                double	_thickness;
                double _width;
                double _length;
                bool   _infinite;
                bool    _dirty;
                bool    _needReseed;

                CloudLayerInfo()
                    : _type(-1)
                    , _handle(-1)
                    , _altitude(0.0)
                    , _density(0.0)
                    , _thickness(0.0)
                    , _width(50000)
                    , _length(50000)
                    , _infinite(true)
                    , _id(-1)
                    , _dirty(false)
                    , _needReseed(false)
                {

                }
            };
            typedef std::map< int, CloudLayerInfo >                     CloudLayers;
            typedef std::map< int, CloudLayerInfo >::iterator           CloudLayersIterator;
            typedef std::map< int, CloudLayerInfo >::const_iterator     CloudLayersConstIterator;

            CloudLayers         _clouds;

            typedef std::vector< CloudLayerInfo >                       CloudLayersQueue;
            typedef std::vector< CloudLayerInfo >::iterator             CloudLayersQueueIterator;
            typedef std::vector< CloudLayerInfo >::const_iterator       CloudLayersQueueConstIterator;

            CloudLayersQueue    _cloudsQueueToAdd;
            CloudLayersQueue    _cloudsQueueToRemove;

            void addClouds(SilverLining::Atmosphere *atmosphere, const osg::Vec3d& position);
            void removeClouds(SilverLining::Atmosphere *atmosphere);
            void updateClouds(SilverLining::Atmosphere *atmosphere);

            SilverLiningParams		_silverLiningParams;

        };
    } // namespace
} // namespace
