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
#ifndef ATTRIBUTES_H
#define ATTRIBUTES_H

#include <osg/Vec4>
#include <osg/Referenced>
#include <osg/Group>
#include <osg/ValueObject>

namespace igcore
{

/*! This struct is used to pass data to the available plugins by using it with
 * \ref igplugincore::PluginContext::Attribute
 * \brief The FogAttributes struct
 * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
 * \copyright (c)Compro Computer Services, Inc.
 * \date      Sun Jan 11 2015
 */
struct FogAttributes
{
    double _visibility;
    osg::Vec3 _fogColor;

    FogAttributes(double visibility = 1000, osg::Vec3 color = osg::Vec3(1,1,1))
        : _visibility(visibility)
        , _fogColor(color)
    {

    }

    void setVisibility(double visibility)
    {
        _visibility = visibility;
    }

    double getVisibility() const
    {
        return _visibility;
    }

    void setFogColor(osg::Vec3 color) { _fogColor = color;}
    osg::Vec3 getFogColor() { return _fogColor;}

};

/*! This struct is used to pass data to the available plugins by using it with
 * \ref igplugincore::PluginContext::Attribute
 * \brief The TimeOfDayAttributes struct
 * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
 * \copyright (c)Compro Computer Services, Inc.
 * \date      Sun Jan 11 2015
 */
struct TimeOfDayAttributes
{
    unsigned int    _hour;
    unsigned int    _minutes;

    TimeOfDayAttributes(unsigned int hour, unsigned int minutes)
        : _hour(hour)
        , _minutes(minutes)
    {

    }

    void setHour(unsigned int hour)
    {
        _hour = hour;
    }
    void setMinutes(unsigned int minutes)
    {
        _minutes = minutes;
    }
    unsigned int getHour() const
    {
        return _hour;
    }
    unsigned int getMinutes() const
    {
        return _minutes;
    }
};

/*! This struct is used to pass data to the available plugins by using it with
 * \ref igplugincore::PluginContext::Attribute
 * \brief The AtmosphereAttributes struct
 * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
 * \copyright (c)Compro Computer Services, Inc.
 * \date      Sun Jan 11 2015
 */
struct AtmosphereAttributes
{
    void *_atmosphere;

    AtmosphereAttributes(void *atmosphere)
        : _atmosphere(atmosphere)
    {

    }

    void setAtmosphere(void *atmosphere )
    {
        _atmosphere = atmosphere;
    }

    void *getAtmosphere() const
    {
        return _atmosphere;
    }

};


/*! This struct is used to pass data to the available plugins by using it with
 * \ref igplugincore::PluginContext::Attribute
 * \brief The WindAttributes struct
 * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
 * \copyright (c)Compro Computer Services, Inc.
 * \date      Sun Jan 11 2015
 */
struct WindAttributes
{
    float   _speed;
    float   _direction;

    WindAttributes(float speed, float direction)
        : _speed(speed)
        , _direction(direction)
    {

    }
};

/*! This struct is used to pass data to the available plugins by using it with
 * \ref igplugincore::PluginContext::Attribute
 * \brief The RainSnowAttributes struct
 * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
 * \copyright (c)Compro Computer Services, Inc.
 * \date      Sun Jan 11 2015
 */
struct RainSnowAttributes
{
    float   _factor;

    RainSnowAttributes(float factor = 0.f)
        : _factor(factor)
    {

    }

    void setFactor(float factor)
    {
        _factor = factor;
    }
    float getFactor() const
    {
        return _factor;
    }
};

/*! This struct is used to pass data to the available plugins by using it with
 * \ref igplugincore::PluginContext::Attribute
 * \brief The CLoudLayerAttributes struct
 * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
 * \copyright (c)Compro Computer Services, Inc.
 * \date      Sun Jan 11 2015
 */
struct CLoudLayerAttributes
{
    int             _id;
    int             _type;
    double          _altitude;
    double          _density;
    double          _thickness;
    double          _width;
    double          _length;

    bool            _infinite;
    bool            _add;
    bool            _remove;
    bool            _dirty;

    CLoudLayerAttributes()
        : _id(-1)
        , _type(-1)
        , _altitude(0.0)
        , _density(0.0)
        , _thickness(0.0)
        , _width(50000)
        , _length(50000)
        , _infinite(true)
        , _add(false)
        , _remove(false)
        , _dirty(false)
    {

    }

    void setId(int id)
    {
        _id = id;
    }
    void setType(int type)
    {
        _type = type;
    }
    void setAltitude(double altitude)
    {
        _altitude = altitude;
    }
    void setDensity(double density)
    {
        _density = density;
    }
    void setThickness(double thickness)
    {
        _thickness = thickness;
    }
    void setWidth(double width)
    {
        _width = width;
    }
    void setLength(double length)
    {
        _length = length;
    }
    void setInfinite(bool infinite)
    {
        _infinite = infinite;
    }
    void setFlags(bool add = false, bool remove = false)
    {
        _add = add;
        _remove = remove;
    }
    void setIsDirty(bool dirty = true)
    {
        _dirty = dirty;
    }

    int getId() const
    {
        return _id;
    }
    int getType() const
    {
        return _type;
    }
    double getDensity() const
    {
        return _density;
    }
    double getThickness() const
    {
        return _thickness;
    }
    double getAltitude() const
    {
        return _altitude;
    }

    bool getAddFlag() const
    {
        return _add;
    }
    bool getRemoveFlag() const
    {
        return _remove;
    }
    bool isDirty() const
    {
        return _dirty;
    }
};

/*! This struct is used to pass data to the available plugins that are
 * provinding lighting implementation. See \ref igcore::ImageGenerator::addLight
 * \ref igcore::ImageGenerator::setLightImplementationCallback
 * \brief The LightAttributes struct
 * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
 * \copyright (c)Compro Computer Services, Inc.
 * \date      Sun Jan 11 2015
 */
struct LightAttributes
{
    osg::Vec4       _ambient;
    osg::Vec4       _diffuse;
    osg::Vec4       _specular;
    float           _brightness;
    float           _constantAttenuation;
    float           _spotCutoff;
    bool            _enabled;
	float			_cloudBrightness;
	float			_waterBrightness;
	double			_lod;
	double			_realLightLOD;
    unsigned int    _dirtyMask;

	enum Mask
	{
		AMBIENT					= 1,
		DIFFUSE					= 2,
		SPECULAR				= 4,
		BRIGHTNESS				= 8,
		CONSTANTATTENUATION		= 16,
		SPOTCUTOFF				= 32,
		CLOUDBRIGHTNESS			= 64,
		WATERBRIGHTNESS			= 128,
		LOD						= 256,
		REALLIGHTLOD			= 512,
		ENABLED					= 1024,
		ALL = AMBIENT | DIFFUSE | SPECULAR | BRIGHTNESS 
			| CLOUDBRIGHTNESS | WATERBRIGHTNESS | CONSTANTATTENUATION 
			| ENABLED | SPOTCUTOFF | LOD | REALLIGHTLOD
    };

    LightAttributes()
        : _brightness(1.f)
        , _constantAttenuation(400.f)
        , _spotCutoff(20.f)
        , _enabled(true)
		, _cloudBrightness(1.f)
		, _waterBrightness(1.f)
        , _dirtyMask(0)
		, _lod(0.0)
		, _realLightLOD(0.0)
    {

    }
};

/*! This struct is used to pass data to the available plugins by using it with
 * \ref igplugincore::PluginContext::Attribute
 * \brief The AnimationAttributes struct
 * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
 * \copyright (c)Compro Computer Services, Inc.
 * \date      Sun Jan 11 2015
 */
struct AnimationAttributes
{
    unsigned int                    _entityId;
    bool                            _playback;
    bool                            _reset;
    std::string                     _animationName;
    osg::ref_ptr<osg::Referenced>   _sequenceCallbacks;


    AnimationAttributes() : _entityId(0), _playback(true), _reset(false) {}
};

/*! This struct is used to pass data to the available plugins by using it with
 * \ref igplugincore::PluginContext::Attribute
 * \brief The EnvironmentalMapAttributes struct
 * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
 * \copyright (c)Compro Computer Services, Inc.
 * \date      Sun Jan 11 2015
 */
struct EnvironmentalMapAttributes
{
    int _envMapId;

    EnvironmentalMapAttributes() : _envMapId(-1) {}
};

/*! This struct is introduced a bit later, and it is intended to be used
*	for passing custom data through setUserValue. It is handy and general
*	enough then the previous defined specialized structures
* \brief	Struct for passing custom data to plugins via setUserValue
* \author   Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
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
* \author   Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
* \copyright (c)Compro Computer Services, Inc.
* \date     Sun Jun 14 2015
*/
class GenericImplementationCallback : public osg::Referenced
{
public:
	virtual osg::Node* create(unsigned int id, const std::string& name, igcore::GenericAttribute* attributes = 0) = 0;
	virtual void destroy(unsigned int id) = 0;
	virtual void update(unsigned int id, igcore::GenericAttribute* attributes) = 0;
};

/*! See \ref igcore::ImageGenerator::setLightImplementationCallback for explanation.
 * Plugins that are implementing lighting to the IG should have inherit and implement
 * from this class
 * \brief The LightImplementationCallback class
 * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
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
     * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
     * \copyright (c)Compro Computer Services, Inc.
     * \date      Sun Jan 11 2015
     */
    virtual osg::Referenced* createLight(
        unsigned int id,
        const LightAttributes& attribs = LightAttributes(),
        osg::Group* lightsGroup = 0) = 0;

    /*!
     * \brief Method that is called to update the light by new attributes
     * \param id            The id of the light
     * \param attribs       New light attributes
     * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
     * \copyright (c)Compro Computer Services, Inc.
     * \date      Sun Jan 11 2015
     */
    virtual void             updateLight(unsigned int id, const LightAttributes& attribs) = 0;
};

} // namespace

#endif // ATTRIBUTES_H
