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
//#*    Please direct any questions or comments to the OpenIG Forums
//#*    Email address: openig@compro.net
//#*
//#*
//#*****************************************************************************
#ifndef ATTRIBUTES_H
#define ATTRIBUTES_H

#include <osg/Vec4>
#include <osg/Referenced>
#include <osg/Group>
#include <osg/ValueObject>

namespace OpenIG {
    namespace Base {

        /*! This struct is used to pass data to the available plugins by using it with
        * \ref igplugincore::PluginContext::Attribute
        * \brief The MultiSwitchAttributes struct
        * \author    Curtis Rubel openig@compro.net
        * \copyright (c)Compro Computer Services, Inc.
        * \date      Tuesday June 28 2016
        */
        struct MultiSwitchAttributes
        {
            std::string msName;
            int         msIndex;

            MultiSwitchAttributes(std::string multiSwitchName, int multiSwitchIndex)
                : msName(multiSwitchName)
                , msIndex(multiSwitchIndex)
            {

            }

            void setMultiSwitchName(std::string multiSwitchName)
            {
                msName = multiSwitchName;
            }

            std::string getMultiSwitchName() const
            {
                return msName;
            }

            void setMultiSwitchIndex(int multiSwitchIndex) { msIndex = multiSwitchIndex; }
            int getMultiSwitchIndex() { return msIndex; }

        };

        /*! This struct is used to pass data to the available plugins by using it with
         * \ref igplugincore::PluginContext::Attribute
         * \brief The FogAttributes struct
         * \author    Trajce Nikolov Nick openig@compro.net
         * \copyright (c)Compro Computer Services, Inc.
         * \date      Sun Jan 11 2015
         */
        struct FogAttributes
        {
            double		visibility;
            osg::Vec3	fogColor;

            FogAttributes(double visibilityIn = 1000, osg::Vec3 colorIn = osg::Vec3(1, 1, 1))
                : visibility(visibilityIn)
                , fogColor(colorIn)
            {

            }

            void setVisibility(double visibilityIn)
            {
                visibility = visibilityIn;
            }

            double getVisibility() const
            {
                return visibility;
            }

            void setFogColor(osg::Vec3 color) { fogColor = color; }
            osg::Vec3 getFogColor() { return fogColor; }

        };

        /*! This struct is used to pass data to the available plugins by using it with
         * \ref igplugincore::PluginContext::Attribute
         * \brief The TimeOfDayAttributes struct
         * \author    Trajce Nikolov Nick openig@compro.net
         * \copyright (c)Compro Computer Services, Inc.
         * \date      Sun Jan 11 2015
         */
        struct TimeOfDayAttributes
        {
            unsigned int    hour;
            unsigned int    minutes;

            TimeOfDayAttributes(unsigned int hourIn, unsigned int minutesIn)
                : hour(hourIn)
                , minutes(minutesIn)
            {

            }

            void setHour(unsigned int hourIn)
            {
                hour = hourIn;
            }
            void setMinutes(unsigned int minutesIn)
            {
                minutes = minutesIn;
            }
            unsigned int getHour() const
            {
                return hour;
            }
            unsigned int getMinutes() const
            {
                return minutes;
            }
        };

        /*! This struct is used to pass data to the available plugins by using it with
         * \ref igplugincore::PluginContext::Attribute
         * \brief The DateAttributes struct
         * \author Curtis G Rubel openig@compro.net
         * \copyright (c)Compro Computer Services, Inc.
         * \date     Wed May 25 2016
         */
        struct DateAttributes
        {
            int    month;
            int    day;
            int    year;

            DateAttributes(int monthIn, int dayIn, int yearIn)
                : month(monthIn)
                , day(dayIn)
                , year(yearIn)
            {

            }

            void setMonth(int monthIn)
            {
                month = monthIn;
            }
            void setDay(int dayIn)
            {
                day = dayIn;
            }
            void setYear(int yearIn)
            {
                year = yearIn;
            }
            int getMonth() const
            {
                return month;
            }
            int getDay() const
            {
                return day;
            }
            int getYear() const
            {
                return year;
            }
        };

        /*! This struct is used to pass data to the available plugins by using it with
         * \ref igplugincore::PluginContext::Attribute
         * \brief The AtmosphereAttributes struct
         * \author    Trajce Nikolov Nick openig@compro.net
         * \copyright (c)Compro Computer Services, Inc.
         * \date      Sun Jan 11 2015
         */
        struct AtmosphereAttributes
        {
            void *atmosphere;

            AtmosphereAttributes(void *atmosphereIn)
                : atmosphere(atmosphereIn)
            {

            }

            void setAtmosphere(void *atmosphereIn)
            {
                atmosphere = atmosphere;
            }

            void *getAtmosphere() const
            {
                return atmosphere;
            }

        };


        /*! This struct is used to pass data to the available plugins by using it with
         * \ref igplugincore::PluginContext::Attribute
         * \brief The WindAttributes struct
         * \author    Trajce Nikolov Nick openig@compro.net
         * \copyright (c)Compro Computer Services, Inc.
         * \date      Sun Jan 11 2015
         */
        struct WindAttributes
        {
            float   speed;
            float   direction;

            WindAttributes(float speedIn, float directionIn)
                : speed(speedIn)
                , direction(directionIn)
            {

            }
        };

        /*! This struct is used to pass data to the available plugins by using it with
         * \ref igplugincore::PluginContext::Attribute
         * \brief The RainSnowAttributes struct
         * \author    Trajce Nikolov Nick openig@compro.net
         * \copyright (c)Compro Computer Services, Inc.
         * \date      Sun Jan 11 2015
         */
        struct RainSnowAttributes
        {
            float   factor;

            RainSnowAttributes(float factorIn = 0.f)
                : factor(factorIn)
            {

            }

            void setFactor(float factorIn)
            {
                factor = factorIn;
            }
            float getFactor() const
            {
                return factor;
            }
        };

        /*! This struct is used to pass data to the available plugins by using it with
         * \ref igplugincore::PluginContext::Attribute
         * \brief The CLoudLayerAttributes struct
         * \author    Trajce Nikolov Nick openig@compro.net
         * \copyright (c)Compro Computer Services, Inc.
         * \date      Sun Jan 11 2015
         */
        struct CLoudLayerAttributes
        {
            int             id;
            int             type;
            int             museLayer;
            double          altitude;
            double          density;
            double          thickness;
            double          width;
            double          length;

            bool            infinite;
            bool            add;
            bool            remove;
            bool            dirty;

            CLoudLayerAttributes()
                : id(-1)
                , type(-1)
                , museLayer(-1)
                , altitude(0.0)
                , density(0.0)
                , thickness(0.0)
                , width(50000)
                , length(50000)
                , infinite(true)
                , add(false)
                , remove(false)
                , dirty(false)
            {

            }

            void setId(int idIn)
            {
                id = idIn;
            }
            void setType(int typeIn)
            {
                type = typeIn;
            }
            void setMuseLayer(int layerIn)
            {
                museLayer = layerIn;
            }
            void setAltitude(double altitudeIn)
            {
                altitude = altitudeIn;
            }
            void setDensity(double densityIn)
            {
                density = densityIn;
            }
            void setThickness(double thicknessIn)
            {
                thickness = thicknessIn;
            }
            void setWidth(double widthIn)
            {
                width = widthIn;
            }
            void setLength(double lengthIn)
            {
                length = lengthIn;
            }
            void setInfinite(bool infiniteIn)
            {
                infinite = infiniteIn;
            }
            void setFlags(bool addIn = false, bool removeIn = false)
            {
                add = addIn;
                remove = removeIn;
            }
            void setIsDirty(bool dirtyIn = true)
            {
                dirty = dirtyIn;
            }

            int getId() const
            {
                return id;
            }
            int getType() const
            {
                return type;
            }
            int getMuseLayer() const
            {
                return museLayer;
            }
            double getDensity() const
            {
                return density;
            }
            double getThickness() const
            {
                return thickness;
            }
            double getAltitude() const
            {
                return altitude;
            }

            bool getAddFlag() const
            {
                return add;
            }
            bool getRemoveFlag() const
            {
                return remove;
            }
            bool isDirty() const
            {
                return dirty;
            }
        };

        /*! This struct is used to pass data to the available plugins by using it with
         * \ref igplugincore::PluginContext::Attribute
         * \brief The CLoudLayerFileAttributes struct
         * \author    Curtis Rubel
         * \copyright (c)Compro Computer Services, Inc.
         * \date      Weds 20 July 2016
         */
        struct CLoudLayerFileAttributes
        {
            int                   id;
            int                 type;
            std::string     filename;
            bool               dirty;
            bool          saveToFile;

            CLoudLayerFileAttributes()
                : id(-1)
                , type(-1)
                , dirty(false)
                , saveToFile(false)
            {
                filename.clear();
            }

            void setId(int idIn)
            {
                id = idIn;
            }
            void setType(int typeIn)
            {
                type = typeIn;
            }
            void setFilename(std::string fileName)
            {
                filename = fileName;
            }
            void setIsDirty(bool dirtyIn = true)
            {
                dirty = dirtyIn;
            }

            void setSaveToFile(bool saveToFileIn = false)
            {
                saveToFile = saveToFileIn;
            }

            int getId() const
            {
                return id;
            }
            int getType() const
            {
                return type;
            }
            std::string getFilename()
            {
                return filename;
            }
            bool isDirty() const
            {
                return dirty;
            }

            bool getSaveToFile()
            {
                return saveToFile;
            }
        };

        enum LightType
        {
            LT_DIRECTIONAL = 0
            , LT_POINT = 1
            , LT_SPOT = 2
            , LT_UNKNOWN = 3
        };

        /*! This struct is used to pass data to the available plugins that are
         * provinding lighting implementation. See \ref OpenIG::Base::ImageGenerator::addLight
         * \ref OpenIG::Base::ImageGenerator::setLightImplementationCallback
         * \brief The LightAttributes struct
         * \author    Trajce Nikolov Nick openig@compro.net
         * \copyright (c)Compro Computer Services, Inc.
         * \date      Sun Jan 11 2015
         */
        struct LightAttributes
        {
            osg::Vec4       ambient;
            osg::Vec4       diffuse;
            osg::Vec4       specular;
            float           brightness;
            float           constantAttenuation;
            float           spotCutoff;
            bool            enabled;
            float			cloudBrightness;
            float			waterBrightness;
            double			lod;
            double			realLightLOD;
            unsigned int    dirtyMask;

            // PPP: I added these 4 additional parameters
            // Valid for point lights and spot lights
            float           fStartRange;
            float           fEndRange;
            // Valid for spot lights
            float           fSpotInnerAngle;
            float           fSpotOuterAngle;

            LightType       lightType;

            osg::Object::DataVariance	dataVariance;
            bool						cullingActive;

            enum Mask
            {
                AMBIENT = 1,
                DIFFUSE = 2,
                SPECULAR = 4,
                BRIGHTNESS = 8,
                CONSTANTATTENUATION = 16,
                SPOTCUTOFF = 32,
                CLOUDBRIGHTNESS = 64,
                WATERBRIGHTNESS = 128,
                LOD = 256,
                REALLIGHTLOD = 512,
                ENABLED = 1024,
                RANGES = 2048,
                ANGLES = 4096,
                ALL = AMBIENT | DIFFUSE | SPECULAR | BRIGHTNESS
                | CLOUDBRIGHTNESS | WATERBRIGHTNESS | CONSTANTATTENUATION
                | ENABLED | SPOTCUTOFF | LOD | REALLIGHTLOD | RANGES | ANGLES
            };

            LightAttributes()
                : brightness(1.f)
                , constantAttenuation(400.f)
                , spotCutoff(20.f)
                , enabled(true)
                , cloudBrightness(1.f)
                , waterBrightness(1.f)
                , dirtyMask(0)
                , lod(0.0)
                , realLightLOD(0.0)
                // PPP: Parameters I added
                // ------------------------
                , fStartRange(0)
                , fEndRange(0)
                , fSpotInnerAngle(0)
                , fSpotOuterAngle(0)
                , lightType(LT_UNKNOWN)
                // -----------------------
                , dataVariance(osg::Object::DYNAMIC)
                , cullingActive(true)
            {

            }
        };

        /*! This struct is used to pass data to the available plugins by using it with
         * \ref igplugincore::PluginContext::Attribute
         * \brief The AnimationAttributes struct
         * \author    Trajce Nikolov Nick openig@compro.net
         * \copyright (c)Compro Computer Services, Inc.
         * \date      Sun Jan 11 2015
         */
        struct AnimationAttributes
        {
            unsigned int                    entityId;
            bool                            playback;
            bool                            reset;
            bool							pause;
            bool							restore;
            std::string                     animationName;
            osg::ref_ptr<osg::Referenced>   sequenceCallbacks;


            AnimationAttributes() : entityId(0), playback(true), reset(false), pause(false), restore(false)  {}
        };

        /*! This struct is used to pass data to the available plugins by using it with
         * \ref igplugincore::PluginContext::Attribute
         * \brief The EnvironmentalMapAttributes struct
         * \author    Trajce Nikolov Nick openig@compro.net
         * \copyright (c)Compro Computer Services, Inc.
         * \date      Sun Jan 11 2015
         */
        struct EnvironmentalMapAttributes
        {
            int envMapId;

            EnvironmentalMapAttributes() : envMapId(-1) {}
        };

        /*! This struct is introduced a bit later, and it is intended to be used
        *	for passing custom data through setUserValue. It is handy and general
        *	enough then the previous defined specialized structures
        * \brief	Struct for passing custom data to plugins via setUserValue
        * \author   Trajce Nikolov Nick openig@compro.net
        * \copyright (c)Compro Computer Services, Inc.
        * \date     Sun Jun 14 2015
        */
        struct GenericAttribute : public osg::ValueObject
        {

        };

        /*! This class is general purpose class for creating and managing
        *	custom implementation of entities. As an example can be Effect
        *	Entity that is implemented in a plugin.
        * \brief	This class is general purpose class for creating and managing custom implementation of entities
        * \author   Trajce Nikolov Nick openig@compro.net
        * \copyright (c)Compro Computer Services, Inc.
        * \date     Sun Jun 14 2015
        */
        class GenericImplementationCallback : public osg::Referenced
        {
        public:
            virtual osg::Node* create(unsigned int id, const std::string& name, OpenIG::Base::GenericAttribute* attributes = 0) = 0;
            virtual void destroy(unsigned int id) = 0;
            virtual void update(unsigned int id, OpenIG::Base::GenericAttribute* attributes) = 0;
        };

        /*! See \ref OpenIG::Base::ImageGenerator::setLightImplementationCallback for explanation.
         * Plugins that are implementing lighting to the IG should have inherit and implement
         * from this class
         * \brief The LightImplementationCallback class
         * \author    Trajce Nikolov Nick openig@compro.net
         * \copyright (c)Compro Computer Services, Inc.
         * \date      Sun Jan 11 2015
         */
        class LightImplementationCallback : public osg::Referenced
        {
        public:

            /*!
             * \brief Method that is called for light creation
             * \param id            The id of the light
             * \param attribs       The light attributes
             * \param lightsGroup   The osg::Group this light is attached to
             * \return              Custom light implmentation.
             * \author    Trajce Nikolov Nick openig@compro.net
             * \copyright (c)Compro Computer Services, Inc.
             * \date      Sun Jan 11 2015
             */
            virtual osg::Referenced* createLight(
                unsigned int id,
                const LightAttributes& attribs = LightAttributes(),
                osg::Group* lightsGroup = 0) = 0;

            /*!
            * \brief Method that is called for light deletion
            * \param id            The id of the light
            * \author    Trajce Nikolov Nick openig@compro.net
            * \copyright (c)Compro Computer Services, Inc.
            * \date      Sun Jun 1 2016
            */
            virtual void deleteLight(unsigned int id) {}

            /*!
             * \brief Method that is called to update the light by new attributes
             * \param id            The id of the light
             * \param attribs       New light attributes
             * \author    Trajce Nikolov Nick openig@compro.net
             * \copyright (c)Compro Computer Services, Inc.
             * \date      Sun Jan 11 2015
             */
            virtual void             updateLight(unsigned int id, const LightAttributes& attribs) = 0;

            /*!
            * \brief Method that is called to set user data to the light
            * \param id				The id of the light
            * \param data			The user data
            * \author    Trajce Nikolov Nick openig@compro.net
            * \copyright (c)Compro Computer Services, Inc.
            * \date      Tue May 17 2016
            */
            virtual void setLightUserData(unsigned int id, osg::Referenced* data) {}
        };
    } // namespace
} // namespace

#endif // ATTRIBUTES_H
