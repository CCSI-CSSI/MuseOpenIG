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
#include <IgPluginCore/plugin.h>
#include <IgPluginCore/plugincontext.h>

#include <IgCore/imagegenerator.h>
#include <IgCore/stringutils.h>
#include <IgCore/mathematics.h>
#include <IgCore/animation.h>
#include <IgCore/configuration.h>
#include <IgCore/globalidgenerator.h>

#include <osg/Version>
#include <osg/ValueObject>
#include <osg/Image>
#include <osg/Texture2D>
#include <osg/TextureCubeMap>
#include <osg/Point>
#include <osg/BlendFunc>
#include <osg/Material>

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osgDB/XmlParser>

#include <osgSim/LightPointNode>
#include <osgSim/LightPoint>

#include "lightpointnode.h"

#include <sstream>

namespace igplugins
{

class ModelCompositionPlugin : public igplugincore::Plugin
{
public:

    ModelCompositionPlugin()
        : _subentityId(10000)
        , _environmentalMapSlot(0)
        , _diffuseSlot(0)
        , _environmentalFactor(0.f)
        , _autoLightId(0)
    {

    }

    virtual std::string getName() { return "ModelComposition"; }

    virtual std::string getDescription( ) { return "Compose model based on xml defintion"; }

    virtual std::string getVersion() { return "1.0.1"; }

    virtual std::string getAuthor() { return "ComPro, Nick"; }

    virtual void config(const std::string& fileName)
    {
        osgDB::XmlNode* root = osgDB::readXmlFile(fileName);
        if (root == 0) return;

        if (root->children.size() == 0) return;

        osgDB::XmlNode* config = root->children.at(0);
        if (config->name != "OpenIG-Plugin-Config") return;

        osgDB::XmlNode::Children::iterator itr = config->children.begin();
        for ( ; itr != config->children.end(); ++itr)
        {
            osgDB::XmlNode* child = *itr;

            if (child->name == "Auto-Light-Id-Start")
            {
                _autoLightId = atoi(child->contents.c_str());
            }
        }
    }

    struct UpdateEnvironmentalFactorUniformCallback : public osg::Uniform::Callback
    {
        UpdateEnvironmentalFactorUniformCallback(float& factor)
            : _factor(factor)
        {

        }

        virtual void operator () (osg::Uniform* u, osg::NodeVisitor* )
        {
            u->set(_factor);
        }

        float&  _factor;
    };

    virtual void update(igplugincore::PluginContext& context)
    {
        osg::ref_ptr<osg::Referenced> ref = context.getAttribute("TOD");
        igplugincore::PluginContext::Attribute<igcore::TimeOfDayAttributes> *attr = dynamic_cast<igplugincore::PluginContext::Attribute<igcore::TimeOfDayAttributes> *>(ref.get());
        if (attr && _dayMaterial.valid() && _nightMaterial.valid() && _runtimeMaterial.valid())
        {
            float t = 0.f;
            float hour = attr->getValue().getHour();

            if ( hour >= 1.0 && hour <= 12.0)
            {
                t = hour / 12;

                osg::Vec4 nightAmbient = _nightMaterial->getAmbient(osg::Material::FRONT_AND_BACK);
                osg::Vec4 dayAmbient = _dayMaterial->getAmbient(osg::Material::FRONT_AND_BACK);
                osg::Vec4 ambient = nightAmbient*(1.f-t) + dayAmbient*t;

                osg::Vec4 nightDiffuse = _nightMaterial->getDiffuse(osg::Material::FRONT_AND_BACK);
                osg::Vec4 dayDiffuse = _dayMaterial->getDiffuse(osg::Material::FRONT_AND_BACK);
                osg::Vec4 diffuse = nightDiffuse*(1.f-t) + dayDiffuse*t;

                osg::Vec4 nightSpecular = _nightMaterial->getSpecular(osg::Material::FRONT_AND_BACK);
                osg::Vec4 daySpecular= _dayMaterial->getSpecular(osg::Material::FRONT_AND_BACK);
                osg::Vec4 specular = nightSpecular*(1.f-t) + daySpecular*t;

                float nightShininess = _nightMaterial->getShininess(osg::Material::FRONT_AND_BACK);
                float dayShininess = _dayMaterial->getShininess(osg::Material::FRONT_AND_BACK);
                float shininess = nightShininess*(1.f-t) + dayShininess*t;

                _runtimeMaterial->setAmbient(osg::Material::FRONT_AND_BACK, ambient);
                _runtimeMaterial->setDiffuse(osg::Material::FRONT_AND_BACK, diffuse);
                _runtimeMaterial->setSpecular(osg::Material::FRONT_AND_BACK, specular);
                _runtimeMaterial->setShininess(osg::Material::FRONT_AND_BACK, shininess);
            }
            else
            if ( (hour >= 12.0 && hour <= 24.0) || hour < 1.0)
            {                
                t = (hour-12.f) / 12;

                osg::Vec4 nightAmbient = _nightMaterial->getAmbient(osg::Material::FRONT_AND_BACK);
                osg::Vec4 dayAmbient = _dayMaterial->getAmbient(osg::Material::FRONT_AND_BACK);
                osg::Vec4 ambient = nightAmbient*t + dayAmbient*(1.f-t);

                osg::Vec4 nightDiffuse = _nightMaterial->getDiffuse(osg::Material::FRONT_AND_BACK);
                osg::Vec4 dayDiffuse = _dayMaterial->getDiffuse(osg::Material::FRONT_AND_BACK);
                osg::Vec4 diffuse = nightDiffuse*t + dayDiffuse*(1.f-t);

                osg::Vec4 nightSpecular= _nightMaterial->getSpecular(osg::Material::FRONT_AND_BACK);
                osg::Vec4 daySpecular= _dayMaterial->getSpecular(osg::Material::FRONT_AND_BACK);
                osg::Vec4 specular = nightSpecular*t + daySpecular*(1.f-t);

                float nightShininess = _nightMaterial->getShininess(osg::Material::FRONT_AND_BACK);
                float dayShininess = _dayMaterial->getShininess(osg::Material::FRONT_AND_BACK);
                float shininess = nightShininess*t + dayShininess*(1.f-t);

                _runtimeMaterial->setAmbient(osg::Material::FRONT_AND_BACK, ambient);
                _runtimeMaterial->setDiffuse(osg::Material::FRONT_AND_BACK, diffuse);
                _runtimeMaterial->setSpecular(osg::Material::FRONT_AND_BACK, specular);
                _runtimeMaterial->setShininess(osg::Material::FRONT_AND_BACK, shininess);
            }


        }
    }

    class UpdateCameraPosUniformCallback : public osg::Uniform::Callback
    {
    public:
        UpdateCameraPosUniformCallback(osg::Camera* camera)
            : _camera(camera)
        {
        }

        virtual void operator () (osg::Uniform* u, osg::NodeVisitor*)
        {
            osg::Vec3 eye;
            osg::Vec3 center;
            osg::Vec3 up;
            _camera->getViewMatrixAsLookAt(eye,center,up);

            u->set(eye);
        }
    protected:
        osg::Camera* _camera;
    };

    virtual void entityAdded(igplugincore::PluginContext& context, unsigned int id, osg::Node& entity, const std::string& fileName)
    {
        if (!osgDB::fileExists(fileName+".xml")) return;

        _entity = &entity;
        _ss = new osg::StateSet;

        osgDB::XmlNode* root = osgDB::readXmlFile(fileName+".xml");
        if (!root) return;
        if (!root->children.size()) return;
        if (root->children.at(0)->name != "OpenIg-Model-Composition")  return;

        _path = osgDB::getFilePath(fileName);

        SubModelMap smm;
        int         aoSlot = -1;
        bool        environmentalMapping = false;

        osg::ref_ptr<osg::TextureCubeMap> envTexture;

        osgDB::XmlNode::Children::iterator itr = root->children.at(0)->children.begin();
        for ( ; itr != root->children.at(0)->children.end(); ++itr )
        {
            if ((**itr).name == "Light")
            {
                readLight(*itr,entity,context,id);
            }
            if ((**itr).name == "Material")
            {
                readMaterial(*itr);
            }
            if ((**itr).name == "Sub-model")
            {                
                readSubmodel(*itr,context,id,smm);
            }
            if ((**itr).name == "Animation")
            {
                _entityWithSubmodels[id] = smm;

                readAnimation(*itr,context,id);
            }
            if ((**itr).name == "Diffuse-Slot")
            {
                _diffuseSlot = atoi((**itr).contents.c_str());
            }
            if ((**itr).name == "Diffuse-Texture")
            {
                _diffuseTextureName = _path + "/" + (**itr).contents;
            }
            if ((**itr).name == "Pre-Baked-Ambient-Occlusion-Texture-Slot")
            {
                aoSlot = atoi((**itr).contents.c_str());
            }
            if ((**itr).name == "Environmental-Slot")
            {
                _environmentalMapSlot = atoi((**itr).contents.c_str());

                _ss->addUniform(
#if 0
                    new osg::Uniform(osg::Uniform::SAMPLER_CUBE,"environmentalMapTexture",(int)_environmentalMapSlot),
#else
                    new osg::Uniform("environmentalMapTexture",(int)_environmentalMapSlot),
#endif
                    osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE
                );
            }
            if ((**itr).name == "Environmental-Texture-Back")
            {
                if (!envTexture.valid()) envTexture = new osg::TextureCubeMap;
                std::string textureName = _path + "/" + (**itr).contents;
                osg::ref_ptr<osg::Image> image = osgDB::readImageFile(textureName);
                envTexture->setImage(osg::TextureCubeMap::POSITIVE_Z,image);

                if (image.valid()) envTexture->setTextureSize(image->s(),image->t());

            }
            if ((**itr).name == "Environmental-Texture-Front")
            {
                if (!envTexture.valid()) envTexture = new osg::TextureCubeMap;
                std::string textureName = _path + "/" + (**itr).contents;
                osg::ref_ptr<osg::Image> image = osgDB::readImageFile(textureName);
                envTexture->setImage(osg::TextureCubeMap::NEGATIVE_Z,image);

                if (image.valid()) envTexture->setTextureSize(image->s(),image->t());

            }
            if ((**itr).name == "Environmental-Texture-Left")
            {
                if (!envTexture.valid()) envTexture = new osg::TextureCubeMap;
                std::string textureName = _path + "/" + (**itr).contents;
                osg::ref_ptr<osg::Image> image = osgDB::readImageFile(textureName);
                envTexture->setImage(osg::TextureCubeMap::NEGATIVE_X,image);

                if (image.valid()) envTexture->setTextureSize(image->s(),image->t());

            }
            if ((**itr).name == "Environmental-Texture-Right")
            {
                if (!envTexture.valid()) envTexture = new osg::TextureCubeMap;
                std::string textureName = _path + "/" + (**itr).contents;
                osg::ref_ptr<osg::Image> image = osgDB::readImageFile(textureName);
                envTexture->setImage(osg::TextureCubeMap::POSITIVE_X,image);

                if (image.valid()) envTexture->setTextureSize(image->s(),image->t());

            }
            if ((**itr).name == "Environmental-Texture-Top")
            {
                if (!envTexture.valid()) envTexture = new osg::TextureCubeMap;
                std::string textureName = _path + "/" + (**itr).contents;
                osg::ref_ptr<osg::Image> image = osgDB::readImageFile(textureName);
                envTexture->setImage(osg::TextureCubeMap::NEGATIVE_Y,image);

                if (image.valid()) envTexture->setTextureSize(image->s(),image->t());

            }
            if ((**itr).name == "Environmental-Texture-Bottom")
            {
                if (!envTexture.valid()) envTexture = new osg::TextureCubeMap;
                std::string textureName = _path + "/" + (**itr).contents;
                osg::ref_ptr<osg::Image> image = osgDB::readImageFile(textureName);
                envTexture->setImage(osg::TextureCubeMap::POSITIVE_Y,image);

                if (image.valid()) envTexture->setTextureSize(image->s(),image->t());

            }
            if ((**itr).name == "Pre-Baked-Ambient-Occlusion-Texture")
            {
                std::string textureName = _path + "/" + (**itr).contents;

                osg::ref_ptr<osg::Image> image = osgDB::readImageFile(textureName);
                if (image.valid())
                {
                    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
                    texture->setImage(image);
                    texture->setTextureSize(image->s(),image->t());

                    _ss->setTextureAttributeAndModes(aoSlot,texture,osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE);
                    _ss->addUniform(new osg::Uniform("ambientOcclusionTexture",(int)aoSlot),osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE|osg::StateAttribute::PROTECTED);

                    osg::notify(osg::NOTICE) << "Model Composition: AO mapped to " << aoSlot << std::endl;
                }
            }
            if ((**itr).name == "Ambient-Occlusion")
            {
                if ((**itr).contents == "yes")
                {                    
#if OSG_VERSION_GREATER_OR_EQUAL(3,3,7)
                    _ss->setDefine("AO");
#endif
                }
            }
            if ((**itr).name == "Ambient-Occlusion-Factor")
            {
                float factor = atof((**itr).contents.c_str());
                _ss->addUniform(new osg::Uniform("ambientOcclusionFactor",(float)factor),osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE|osg::StateAttribute::PROTECTED);
            }
            if ((**itr).name == "Shadow")
            {
                int ReceivesShadowTraversalMask = 0x1;
                int CastsShadowTraversalMask = 0x2;

                if ((**itr).contents == "CAST")
                {
                    entity.setNodeMask(CastsShadowTraversalMask);
                }
                if ((**itr).contents == "RECEIVE")
                {
                    entity.setNodeMask(ReceivesShadowTraversalMask);
                }
                if ((**itr).contents == "CAST-AND-RECEIVE")
                {
                    entity.setNodeMask(CastsShadowTraversalMask|ReceivesShadowTraversalMask);
                }
            }
            if ((**itr).name == "Environmental")
            {
                if ((**itr).contents == "yes")
                {
                    environmentalMapping = true;
                }

                {
                    osg::Uniform* u = new osg::Uniform("cameraPos",osg::Vec3());
                    u->setUpdateCallback( new UpdateCameraPosUniformCallback(context.getImageGenerator()->getViewer()->getView(0)->getCamera()) );
                    _ss->addUniform( u );
                }
            }

            if ((**itr).name == "Environmental-Factor")
            {
                _ss->setDefine("ENVIRONMENTAL_FACTOR",(**itr).contents);
            }
            if ((**itr).name == "Shadowing")
            {
                bool shadowing = (**itr).contents == "yes";

                osg::notify(osg::NOTICE) << "Model Composition: Shadowing " << shadowing << std::endl;
                _ss->addUniform(new osg::Uniform("shadowingEnabled",(bool)shadowing),osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE|osg::StateAttribute::PROTECTED);

                if (shadowing)
                {
#if OSG_VERSION_GREATER_OR_EQUAL(3,3,7)
                    _ss->setDefine("SHADOWING",osg::StateAttribute::ON);
#endif
                }
                else
                {
#if OSG_VERSION_GREATER_OR_EQUAL(3,3,7)
                    _ss->setDefine("SHADOWING",osg::StateAttribute::OFF);
#endif
                }
            }
            if ((**itr).name == "Lighting")
            {
                bool lighting = (**itr).contents == "yes";

                if (lighting)
                {
#if OSG_VERSION_GREATER_OR_EQUAL(3,3,7)
                    _ss->setDefine("SIMPLELIGHTING",osg::StateAttribute::ON);
                    _ss->setDefine("LIGHTING",osg::StateAttribute::ON);
#endif
                }
                else
                {
#if OSG_VERSION_GREATER_OR_EQUAL(3,3,7)
                    _ss->setDefine("SIMPLELIGHTING",osg::StateAttribute::OFF);
                    _ss->setDefine("LIGHTING",osg::StateAttribute::OFF);
#endif
                }
            }
        }

        osg::ref_ptr<osg::Image> image = osgDB::readImageFile(_diffuseTextureName);
        if (image.valid())
        {
            osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
            texture->setImage(image);

            _ss->setTextureAttributeAndModes(_diffuseSlot,texture,osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE);
            _ss->addUniform(new osg::Uniform("baseTexture",(int)_diffuseSlot),osg::StateAttribute::ON|osg::StateAttribute::PROTECTED|osg::StateAttribute::OVERRIDE);

        }

        if (envTexture.valid() && environmentalMapping)
        {            
            osg::notify(osg::NOTICE) << "Model Composition: setting environmental texture " << _environmentalMapSlot << std::endl;
            _ss->setTextureAttributeAndModes(_environmentalMapSlot,envTexture,osg::StateAttribute::ON);

            envTexture->setInternalFormat(GL_RGB);
            envTexture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
            envTexture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
            envTexture->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);
            envTexture->setFilter(osg::TextureCubeMap::MIN_FILTER,osg::TextureCubeMap::LINEAR);
            envTexture->setFilter(osg::TextureCubeMap::MAG_FILTER,osg::TextureCubeMap::LINEAR);

#if OSG_VERSION_GREATER_OR_EQUAL(3,3,7)
            _ss->setDefine("ENVIRONMENTAL");
#endif
        }

        entity.asGroup()->getChild(0)->setStateSet(_ss);

        if (_dayMaterial.valid() && _nightMaterial.valid())
        {
            _runtimeMaterial = new osg::Material(*_dayMaterial);

            entity.getOrCreateStateSet()->setAttributeAndModes(_runtimeMaterial,osg::StateAttribute::ON|osg::StateAttribute::PROTECTED|osg::StateAttribute::OVERRIDE);
        }
    }

protected:
    unsigned int                                        _subentityId;
    std::string                                         _path;
    osg::observer_ptr<osg::Node>                        _entity;
    unsigned int                                        _environmentalMapSlot;
    std::string                                         _diffuseTextureName;
    unsigned int                                        _diffuseSlot;
    bool                                                _envMapping;
    bool                                                _aoOcclustion;
    osg::ref_ptr<osg::StateSet>                         _ss;
    osg::ref_ptr<osg::Material>                         _dayMaterial;
    osg::ref_ptr<osg::Material>                         _nightMaterial;
    osg::ref_ptr<osg::Material>                         _runtimeMaterial;
    float                                               _environmentalFactor;
    unsigned int                                        _autoLightId;

    struct SubModelEntry
    {
        std::string     _name;
        std::string     _fileName;
        std::string     _position;
        std::string     _orientation;
        std::string     _limits;
        unsigned int    _id;
        osg::Vec3       _originalOrientation;
        osg::Vec3       _originalPosition;

        bool isValid() const
        {
            return _name.size() && _fileName.size()!=0 && _position.size()!=0 && _orientation.size()!=0 && _limits.size()!=0;
        }
    };

    typedef std::map< std::string, SubModelEntry >                      SubModelMap;
    typedef std::map< std::string, SubModelEntry >::iterator            SubModelMapIterator;
    typedef std::map< std::string, SubModelEntry >::const_iterator      SubModelMapConstIterator;

    typedef std::map< unsigned int, SubModelMap >                       EntityWithSubmodelsMap;
    typedef std::map< unsigned int, SubModelMap >::iterator             EntityWithSubmodelsMapIterator;
    typedef std::map< unsigned int, SubModelMap >::const_iterator       EntityWithSubmodelsMapConstIterator;

    EntityWithSubmodelsMap  _entityWithSubmodels;

    struct LightAnimationPulses
    {
        double      _duration;
        osg::Vec4   _color;

        LightAnimationPulses()
            : _duration(0.0)
        {

        }
    };
    typedef std::vector<LightAnimationPulses>       LightAnimationPulsesList;

    struct LightAttribs
    {
        osg::Vec3                   _position;
        osg::Vec4                   _color;
        osg::Vec3                   _orientation;
        osg::Vec3                   _offset;
        bool                        _animated;
        LightAnimationPulsesList    _pulses;
        bool                        _real;
        double                      _minSize;
        double                      _maxSize;
        double                      _brightness;
        double                      _constAttenuation;


        LightAttribs()
            : _animated(false)
            , _real(false)
            , _minSize(2)
            , _maxSize(5)
            , _brightness(1)
            , _constAttenuation(5)
        {

        }

    };

    struct LightBlinkUpdateCallback : public osg::NodeCallback
    {
        LightBlinkUpdateCallback(const LightAttribs& light, igcore::ImageGenerator* ig)
            : _firstFrame(true)
            , _light(light)
            , _ig(ig)
            , _currentPulse(0)
        {

        }

        virtual void operator()(osg::Node* node, osg::NodeVisitor*)
        {
            if (_light._pulses.size() == 0) return;

            osg::Timer_t now = osg::Timer::instance()->tick();
            if (_firstFrame)
            {
                _lastUpdateTime = now;
                _firstFrame = false;
            }
            double dt = osg::Timer::instance()->delta_s(_lastUpdateTime,now);
            if (dt >= _light._pulses.at(_currentPulse)._duration)
            {
                unsigned int lightId = 0;
                node->getUserValue("lightId",lightId);

                if (lightId == 0) return;

                bool on = true;
                node->getUserValue("lightOn",on);

                on = !on;
                _ig->enableLight(lightId,on);
                node->setUserValue("lightOn",(bool)on);

                osgSim::LightPointNode* lpn = dynamic_cast<osgSim::LightPointNode*>(node);
                if (lpn)
                {
                    osgSim::LightPoint& lp = lpn->getLightPoint(0);
                    lp._on = on;
                }

                _lastUpdateTime = now;
                _currentPulse = (_currentPulse+1) % _light._pulses.size();
            }
        }

        bool                    _firstFrame;
        osg::Timer_t            _lastUpdateTime;
        LightAttribs            _light;
        igcore::ImageGenerator* _ig;
        unsigned int            _currentPulse;
    };

    void readLight(osgDB::XmlNode* node, osg::Node& entity, igplugincore::PluginContext& context, unsigned int entityId)
    {
        LightAttribs light;

        osgDB::XmlNode::Children::iterator itr = node->children.begin();
        for ( unsigned int i = 0; itr != node->children.end(); ++itr, ++i )
        {
            if ((**itr).name == "Color")
            {
                igcore::StringUtils::Tokens tokens = igcore::StringUtils::instance()->tokenize((**itr).contents);
                if (tokens.size() == 4)
                {
                    float r = atof(tokens.at(0).c_str());
                    float g = atof(tokens.at(1).c_str());
                    float b = atof(tokens.at(2).c_str());
                    float a = atof(tokens.at(3).c_str());

                    light._color = osg::Vec4(r,g,b,a);
                }
            }
            if ((**itr).name == "Position")
            {
                igcore::StringUtils::Tokens tokens = igcore::StringUtils::instance()->tokenize((**itr).contents);
                if (tokens.size() == 3)
                {
                    float x = atof(tokens.at(0).c_str());
                    float y = atof(tokens.at(1).c_str());
                    float z = atof(tokens.at(2).c_str());

                    light._position = osg::Vec3(x,y,z);
                }
            }
            if ((**itr).name == "Animation-Pulses")
            {
                igcore::StringUtils::Tokens tokens = igcore::StringUtils::instance()->tokenize((**itr).contents,",");
                for (size_t i=0; i<tokens.size(); ++i)
                {
                    igcore::StringUtils::Tokens ltokens = igcore::StringUtils::instance()->tokenize(tokens.at(i));
                    if (ltokens.size() == 5)
                    {
                        LightAnimationPulses pulse;

                        pulse._duration = atof(ltokens.at(0).c_str());

                        float r = atof(ltokens.at(1).c_str());
                        float g = atof(ltokens.at(2).c_str());
                        float b = atof(ltokens.at(3).c_str());
                        float a = atof(ltokens.at(4).c_str());

                        pulse._color = osg::Vec4(r,g,b,a);
                        light._pulses.push_back(pulse);
                    }
                }

                light._animated = light._pulses.size() != 0;
            }
            if ((**itr).name == "Use-Real-Light")
            {
                light._real = (**itr).contents == "yes";
            }
            if ((**itr).name == "Brightness")
            {
                light._brightness = atof((**itr).contents.c_str());
            }
            if ((**itr).name == "Orientation")
            {
                igcore::StringUtils::Tokens tokens = igcore::StringUtils::instance()->tokenize((**itr).contents);
                if (tokens.size() == 3)
                {
                    float x = atof(tokens.at(0).c_str());
                    float y = atof(tokens.at(1).c_str());
                    float z = atof(tokens.at(2).c_str());

                    light._orientation = osg::Vec3(x,y,z);
                }
            }
            if ((**itr).name == "MinSize")
            {
                light._minSize = atof((**itr).contents.c_str());
            }
            if ((**itr).name == "MaxSize")
            {
                light._maxSize = atof((**itr).contents.c_str());
            }
            if ((**itr).name == "ConstAttenuation")
            {
                light._constAttenuation = atof((**itr).contents.c_str());
            }
            if ((**itr).name == "Real-Light-Offset")
            {
                igcore::StringUtils::Tokens tokens = igcore::StringUtils::instance()->tokenize((**itr).contents);
                if (tokens.size() == 3)
                {
                    float x = atof(tokens.at(0).c_str());
                    float y = atof(tokens.at(1).c_str());
                    float z = atof(tokens.at(2).c_str());

                    light._offset = osg::Vec3(x,y,z);
                }
            }
        }


        {
            osg::ref_ptr<LightPointNode> lpn = new LightPointNode;

            //lpn->setPointSprite();
            osg::ref_ptr<osg::Texture2D> spriteTexture = new osg::Texture2D;
            osg::ref_ptr<osg::Image> spriteImage = osgDB::readImageFile("/usr/local/3rdparty/OpenSceneGraph-Data-3.0.0/Images/particle.rgb");
            spriteTexture->setImage(spriteImage);

            osgSim::LightPoint lp(true,light._position,light._color);
            lp._blendingMode = osgSim::LightPoint::ADDITIVE;

            osg::StateSet* stateSet = lpn->getOrCreateStateSet();
            stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF|osg::StateAttribute::PROTECTED|osg::StateAttribute::OVERRIDE);
            stateSet->setAttributeAndModes(new osg::Program,osg::StateAttribute::ON|osg::StateAttribute::PROTECTED|osg::StateAttribute::OVERRIDE);

            if (light._animated)
            {
#if 0
                osgSim::BlinkSequence* blinkSequence = new osgSim::BlinkSequence;
                blinkSequence->addPulse(light._duration, light._color);
                blinkSequence->addPulse(light._duration, osg::Vec4(0,0,0,1));
                lp._blinkSequence = blinkSequence;
#endif

                lpn->setUpdateCallback(new LightBlinkUpdateCallback(light,context.getImageGenerator()));
            }

            lpn->setMaxPixelSize(light._maxSize);
            lpn->setMinPixelSize(light._minSize);
            lpn->addLightPoint(lp);

            entity.asGroup()->addChild(lpn);

            igcore::GlobalIdGenerator::instance()->getNextId("Real-Lights",_autoLightId);

            lpn->setUserValue("lightId",(unsigned int)_autoLightId);
            lpn->setUserValue("lightOn",(bool)true);


            if (light._real)
            {
                igcore::LightAttributes la;
                la._diffuse = light._color;
                la._ambient = osg::Vec4(0,0,0,1);
                la._specular = osg::Vec4(0,0,0,1);
                la._constantAttenuation = light._constAttenuation;
                la._brightness = light._brightness;
                la._spotCutoff = 20;
                la._dirtyMask = igcore::LightAttributes::ALL;

                context.getImageGenerator()->addLight(_autoLightId,
                    igcore::Math::instance()->toMatrix(
                        light._position.x()+light._offset.x(),
                        light._position.y()+light._offset.y(),
                        light._position.z()+light._offset.z(),
                        light._orientation.x(),
                        light._orientation.y(),
                        light._orientation.z())
                );
                context.getImageGenerator()->updateLightAttributes(_autoLightId,la);
                context.getImageGenerator()->bindLightToEntity(_autoLightId,entityId);
            }

        }
    }

    void readMaterial(osgDB::XmlNode* node)
    {
        osg::ref_ptr<osg::Material> material;

        osgDB::XmlNode::Children::iterator itr = node->children.begin();
        for ( ; itr != node->children.end(); ++itr)
        {
            if ((**itr).name == "Name")
            {
                if ((**itr).contents == "Day")
                {
                    _dayMaterial = new osg::Material;
                    material = _dayMaterial;
                }
                else
                if ((**itr).contents == "Night")
                {
                    _nightMaterial = new osg::Material;
                    material = _nightMaterial;
                }
            }
            if ((**itr).name == "Ambient")
            {
                igcore::StringUtils::Tokens tokens = igcore::StringUtils::instance()->tokenize((**itr).contents);
                if (tokens.size() == 4)
                {
                    float r = atof(tokens.at(0).c_str());
                    float g = atof(tokens.at(1).c_str());
                    float b = atof(tokens.at(2).c_str());
                    float a = atof(tokens.at(3).c_str());

                    if (material.valid()) material->setAmbient(osg::Material::FRONT_AND_BACK,osg::Vec4(r,g,b,a));
                }
            }
            if ((**itr).name == "Diffuse")
            {
                igcore::StringUtils::Tokens tokens = igcore::StringUtils::instance()->tokenize((**itr).contents);
                if (tokens.size() == 4)
                {
                    float r = atof(tokens.at(0).c_str());
                    float g = atof(tokens.at(1).c_str());
                    float b = atof(tokens.at(2).c_str());
                    float a = atof(tokens.at(3).c_str());

                    if (material.valid()) material->setDiffuse(osg::Material::FRONT_AND_BACK,osg::Vec4(r,g,b,a));
                }
            }
            if ((**itr).name == "Specular")
            {
                igcore::StringUtils::Tokens tokens = igcore::StringUtils::instance()->tokenize((**itr).contents);
                if (tokens.size() == 4)
                {
                    float r = atof(tokens.at(0).c_str());
                    float g = atof(tokens.at(1).c_str());
                    float b = atof(tokens.at(2).c_str());
                    float a = atof(tokens.at(3).c_str());

                    if (material.valid()) material->setSpecular(osg::Material::FRONT_AND_BACK,osg::Vec4(r,g,b,a));
                }
            }
            if ((**itr).name == "Shininess")
            {
                if (material.valid()) material->setShininess(osg::Material::FRONT_AND_BACK,atof((**itr).contents.c_str()));
            }
        }
    }

    void readAnimationSequence(osgDB::XmlNode* node,igcore::Animations::Animation::Sequence* sequence, unsigned int entityId)
    {
        for (size_t i=0; i<node->children.size(); ++i)
        {
            osgDB::XmlNode* child = node->children.at(i);

            if (child->name == "Name")
            {
                sequence->_name = child->contents;
            }
            if (child->name == "Player")
            {
                std::string player = child->contents;

                SubModelMap& smm = _entityWithSubmodels[entityId];
                SubModelMapIterator itr = smm.find(player);
                if (itr != smm.end())
                {
                    sequence->_player = child->contents;
                    sequence->_playerId = itr->second._id;
                    sequence->_playerOriginalOrientation = itr->second._originalOrientation;
                    sequence->_playerOriginalPosition = itr->second._originalPosition;
                }
            }
            if (child->name == "Time-Frame")
            {
                igcore::StringUtils::Tokens tokens = igcore::StringUtils::instance()->tokenize(child->contents);
                if (tokens.size()==2)
                {
                    double from = atof(tokens.at(0).c_str());
                    double to = atof(tokens.at(1).c_str());

                    sequence->_timeFrame = std::pair<double,double>(from,to);
                }
            }
            if (child->name == "Orientation-Update-Vector")
            {
                igcore::StringUtils::Tokens tokens = igcore::StringUtils::instance()->tokenize(child->contents,",");
                if (tokens.size()==3)
                {
                    float x = atof(tokens.at(0).c_str());
                    float y = atof(tokens.at(1).c_str());
                    float z = atof(tokens.at(2).c_str());

                    sequence->_operationVector = osg::Vec3(x,y,z);
                }
            }
            if (child->name == "Orientation-Update")
            {
                igcore::StringUtils::Tokens tokens = igcore::StringUtils::instance()->tokenize(child->contents);
                if (tokens.size()==2)
                {
                    double from = atof(tokens.at(0).c_str());
                    double to = atof(tokens.at(1).c_str());

                    sequence->_rotationUpdate = std::pair<double,double>(from,to);
                }

            }
            if (child->name == "Position-Update")
            {
                igcore::StringUtils::Tokens tokens = igcore::StringUtils::instance()->tokenize(child->contents);
                if (tokens.size()==6)
                {
                    float x1 = atof(tokens.at(0).c_str());
                    float y1 = atof(tokens.at(1).c_str());
                    float z1 = atof(tokens.at(2).c_str());

                    float x2 = atof(tokens.at(3).c_str());
                    float y2 = atof(tokens.at(4).c_str());
                    float z2 = atof(tokens.at(5).c_str());

                    sequence->_positionalUpdate = std::pair<osg::Vec3,osg::Vec3>(osg::Vec3(x1,y1,z1),osg::Vec3(x2,y2,z2));
                }
            }

            if (child->name == "Swap-Pitch-Roll")
            {
                sequence->_swapPitchRoll = child->contents == "yes";
            }
        }
    }

    void readAnimation(osgDB::XmlNode* node, igplugincore::PluginContext& context, unsigned int entityId)
    {
        osg::ref_ptr<igcore::Animations::Animation> animation = new igcore::Animations::Animation;

        for (size_t i=0; i<node->children.size(); ++i)
        {
            osgDB::XmlNode* child = node->children.at(i);

            if (child->name == "Name")
            {
                animation->_name = child->contents;
            }
            if (child->name == "Duration-In-Seconds")
            {
                animation->_duration = atoi(child->contents.c_str());
            }
            if (child->name == "Sequence")
            {
                osg::ref_ptr<igcore::Animations::Animation::Sequence> seq = new igcore::Animations::Animation::Sequence;
                readAnimationSequence(child,seq.get(),entityId);

                animation->_sequences[seq->_name] = seq;
            }
        }

        igcore::ImageGenerator::Entity& entity = context.getImageGenerator()->getEntityMap()[entityId];\
        if (entity.valid())
        {
            igcore::Animations::AnimationContainer* ac = dynamic_cast<igcore::Animations::AnimationContainer*>(entity->getUserData());
            if (!ac)
            {
                ac  = new igcore::Animations::AnimationContainer;
                entity->setUserData(ac);
            }

            (*ac)[animation->_name] = animation;
        }
    }

    void readSubmodel(osgDB::XmlNode* node, igplugincore::PluginContext& context, unsigned int entityId, SubModelMap& smm)
    {
        SubModelEntry submodel;
        submodel._id = _subentityId++;

        bool submodelAdded = false;

        if (node && node->children.size())
        for (size_t i=0; i<node->children.size(); ++i)
        {
            osgDB::XmlNode* child = node->children.at(i);

            if (child->name == "Name")
            {
                submodel._name = child->contents;
            }
            if (child->name == "File")
            {
                submodel._fileName = child->contents;
            }
            if (child->name == "Position")
            {
                submodel._position = child->contents;

                igcore::StringUtils::Tokens tokens = igcore::StringUtils::instance()->tokenize(submodel._position);
                if (tokens.size() == 3)
                {
                    float x = atof(tokens.at(0).c_str());
                    float y = atof(tokens.at(1).c_str());
                    float z = atof(tokens.at(2).c_str());

                    submodel._originalPosition = osg::Vec3(x,y,z);
                }
            }
            if (child->name == "Orientation")
            {
                submodel._orientation = child->contents;

                igcore::StringUtils::Tokens tokens = igcore::StringUtils::instance()->tokenize(submodel._orientation);
                if (tokens.size() == 3)
                {
                    float x = atof(tokens.at(0).c_str());
                    float y = atof(tokens.at(1).c_str());
                    float z = atof(tokens.at(2).c_str());

                    submodel._originalOrientation = osg::Vec3(x,y,z);
                }
            }
            if (child->name == "Limits")
            {
                submodel._limits = child->contents;
            }
            if (child->name == "Sub-model")
            {
                if (submodel.isValid())
                {
                    if (!submodelAdded)
                    {
                        addSubmodel(submodel,context,entityId,smm);
                        submodelAdded = true;                        
                    }
                    readSubmodel(child,context,submodel._id,smm);
                }
            }
        }

        if (!submodelAdded)
        {
            addSubmodel(submodel,context,entityId,smm);
        }

    }

    void addSubmodel(const SubModelEntry& submodel,igplugincore::PluginContext& context,unsigned int entityId, SubModelMap& smm)
    {
        igcore::StringUtils::Tokens tokensPosition = igcore::StringUtils::instance()->tokenize(submodel._position);
        igcore::StringUtils::Tokens tokensOrientation = igcore::StringUtils::instance()->tokenize(submodel._orientation);

        osg::Vec3d pos;
        osg::Vec3d ori;

        if (tokensPosition.size()==3)
        {
            pos.x() = atof(tokensPosition.at(0).c_str());
            pos.y() = atof(tokensPosition.at(1).c_str());
            pos.z() = atof(tokensPosition.at(2).c_str());
        }

        if (tokensOrientation.size()==3)
        {
            ori.x() = atof(tokensOrientation.at(0).c_str());
            ori.y() = atof(tokensOrientation.at(1).c_str());
            ori.z() = atof(tokensOrientation.at(2).c_str());
        }

        osg::Matrixd mx = igcore::Math::instance()->toMatrix(pos.x(),pos.y(),pos.z(),ori.x(),ori.y(),ori.z());

        std::string subModelFileName = _path + "/" + submodel._fileName;
        osg::notify(osg::NOTICE) << "Model Composition: Loading submodel: (" << submodel._id << ") " << subModelFileName << std::endl;

        context.getImageGenerator()->addEntity(submodel._id,subModelFileName,mx);
        context.getImageGenerator()->bindToEntity(submodel._id,entityId);
        context.getImageGenerator()->setEntityName(submodel._id,submodel._name);

        igcore::ImageGenerator::Entity& subentity = context.getImageGenerator()->getEntityMap()[submodel._id];
        subentity->setStateSet(_ss);

        smm[submodel._name] = submodel;

    }

};

} // namespace

#if defined(_MSC_VER) || defined(__MINGW32__)
    //  Microsoft
    #define EXPORT __declspec(dllexport)
    #define IMPORT __declspec(dllimport)
#elif defined(__GNUG__)
    //  GCC
    #define EXPORT __attribute__((visibility("default")))
    #define IMPORT
#else
    //  do nothing and hope for the best?
    #define EXPORT
    #define IMPORT
    #pragma warning Unknown dynamic link import/export semantics.
#endif

extern "C" EXPORT igplugincore::Plugin* CreatePlugin()
{
    return new igplugins::ModelCompositionPlugin;
}

extern "C" EXPORT void DeletePlugin(igplugincore::Plugin* plugin)
{
    osg::ref_ptr<igplugincore::Plugin> p(plugin);
}
