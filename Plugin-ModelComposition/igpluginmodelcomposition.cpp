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
#include <Core-PluginBase/plugin.h>
#include <Core-PluginBase/plugincontext.h>

#include <Core-Base/imagegenerator.h>
#include <Core-Base/stringutils.h>
#include <Core-Base/mathematics.h>
#include <Core-Base/animation.h>
#include <Core-Base/configuration.h>
#include <Core-Base/globalidgenerator.h>
#include <Core-Base/filesystem.h>

#include <Core-Utils/texturecache.h>

#include <Core-OpenIG/renderbins.h>
#include <Core-OpenIG/openig.h>

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

#include <boost/thread.hpp>

#include <sstream>

using namespace osg;

namespace OpenIG { namespace Plugins {

class ModelCompositionPlugin : public OpenIG::PluginBase::Plugin
{
public:

    ModelCompositionPlugin()
        : _subentityId(10000)
        , _environmentalMapSlot(2)
        , _diffuseSlot(0)
        , _aoSlot(3)
        , _autoLightId(1000)
        , _todBasedEnvironmentalLightingFactor(1.0)
    {
        std::string resourcePath = OpenIG::Base::FileSystem::path(OpenIG::Base::FileSystem::Resources, "../resources");
        _textureCache.addPath(resourcePath);
        _textureCubeMapCache.addPath(resourcePath);
    }

    virtual std::string getName() { return "ModelComposition"; }

    virtual std::string getDescription( ) { return "Compose model based on xml defintion"; }

    virtual std::string getVersion() { return "2.0.0"; }

    virtual std::string getAuthor() { return "ComPro, Nick"; }

    virtual void update(OpenIG::PluginBase::PluginContext& context)
    {
        if (_dayMaterial.valid() && _nightMaterial.valid() && _runtimeMaterial.valid())
        {

            osg::Vec4 day_ambient;
            osg::Vec4 day_diffuse;
            osg::Vec4 day_specular;
            float day_shininess;

            osg::Vec4 night_ambient;
            osg::Vec4 night_diffuse;
            osg::Vec4 night_specular;
            float night_shininess;

            osg::Vec4 ambient;
            osg::Vec4 diffuse;
            osg::Vec4 specular;
            float shininess;


            _todBasedEnvironmentalLightingFactor = 1.0;

            day_ambient = _dayMaterial->getAmbient(osg::Material::FRONT_AND_BACK);
            day_diffuse = _dayMaterial->getDiffuse(osg::Material::FRONT_AND_BACK);
            day_specular = _dayMaterial->getSpecular(osg::Material::FRONT_AND_BACK);
            day_shininess = _dayMaterial->getShininess(osg::Material::FRONT_AND_BACK);

            night_ambient = _nightMaterial->getAmbient(osg::Material::FRONT_AND_BACK);
            night_diffuse = _nightMaterial->getDiffuse(osg::Material::FRONT_AND_BACK);
            night_specular = _nightMaterial->getSpecular(osg::Material::FRONT_AND_BACK);
            night_shininess = _nightMaterial->getShininess(osg::Material::FRONT_AND_BACK);

            float factor = context.getImageGenerator()->getSunOrMoonLight()->getLight()->getAmbient().g();
            ambient = day_ambient*factor + night_ambient*(1.f - factor);
            diffuse = day_diffuse*factor + night_diffuse*(1.f - factor);
            specular = day_specular*factor + night_specular*(1.f - factor);
            shininess = day_shininess*factor + night_shininess*(1.f - factor);

            _runtimeMaterial->setAmbient(osg::Material::FRONT_AND_BACK, ambient);
            _runtimeMaterial->setDiffuse(osg::Material::FRONT_AND_BACK, diffuse);
            _runtimeMaterial->setSpecular(osg::Material::FRONT_AND_BACK, specular);
            _runtimeMaterial->setShininess(osg::Material::FRONT_AND_BACK, shininess);
            _runtimeMaterial->setEmission(osg::Material::FRONT_AND_BACK,osg::Vec4(0,0,0,1));
        }
    }

    // Some models does not come with a texture, only material
    // we need to know this and not let override the global
    // model material and set a flag (define) for the shader
    class PreserveMaterialNodeVisitor : public osg::NodeVisitor
    {
    public:
        PreserveMaterialNodeVisitor() : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}
        virtual void apply(osg::Node& node)
        {
            osg::ref_ptr<osg::StateSet> ss = node.getOrCreateStateSet();
            osg::ref_ptr<osg::StateAttribute> attr = ss->getAttribute(osg::StateAttribute::MATERIAL);
            if (attr.valid())
            {
                ss->setAttributeAndModes(attr, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
            }
            attr = ss->getTextureAttribute(0,osg::StateAttribute::TEXTURE);
            if (attr.valid())
            {
                // Here we protect the found materil
                // and set the flag for the shader
                ss->setTextureAttributeAndModes(0, attr, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
                ss->setDefine("TEXTURING", osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
            }

            traverse(node);
        }
    };

    virtual void databaseRead(const std::string& fileName, osg::Node* node, const osgDB::Options*)
    {
        // Check or model definition if exists
        // if not silently quit
        if (!osgDB::fileExists(fileName + ".xml")) return;

        // say it lod we have a XML file
        osg::notify(osg::NOTICE) << "ModelComposition: (" << fileName << ") reading XML config:" << fileName + ".xml" << std::endl;

        // we have some xml definition file and we process it
        osgDB::XmlNode* root = osgDB::readXmlFile(fileName + ".xml");
        if (!root || !root->children.size() || root->children.at(0)->name != "OpenIg-Model-Composition") return;

        // a flag we read from the XML
        bool preserveMaterial = false;

        osgDB::XmlNode::Children::iterator itr = root->children.at(0)->children.begin();
        for (; itr != root->children.at(0)->children.end(); ++itr)
        {
            if ((**itr).name == "PreserveMaterial")
            {
                preserveMaterial = (**itr).contents == "yes";
            }
        }

        // Ok. We then process the model to
        // set it all to use its materials
        // without texture
        if (preserveMaterial)
        {
            PreserveMaterialNodeVisitor nv;
            node->accept(nv);
        }
    }

    // we process lots of different tags here,
    // so let keep them organizied into a map
    struct TagValue
    {
        std::string			value;
        osgDB::XmlNode*		node;
        TagValue() : node(0) {}
    };
    typedef std::map< const std::string, TagValue >				TagValueMap;
    typedef std::multimap< const std::string, TagValue >		TagValueMultiMap;

    // Handy method that will fill a tagvalue map
    // for an XmlNode
    void readXmlNode(osgDB::XmlNode& node, TagValueMap& tags, TagValueMultiMap& mmtags)
    {
        osgDB::XmlNode::Children::iterator itr = node.children.begin();
        for (; itr != node.children.end(); ++itr)
        {
            TagValue value;
            value.value = (**itr).contents;
            value.node = *itr;

            tags[(**itr).name] = value;
            mmtags.insert(std::pair<std::string, TagValue>((**itr).name, value));
        }
    }
    void readXmlNode(osgDB::XmlNode& node, TagValueMap& tags)
    {
        osgDB::XmlNode::Children::iterator itr = node.children.begin();
        for (; itr != node.children.end(); ++itr)
        {
            TagValue value;
            value.value = (**itr).contents;
            value.node = *itr;

            tags[(**itr).name] = value;
        }
    }
    void readXmlNode(osgDB::XmlNode& node, TagValueMultiMap& tags)
    {
        osgDB::XmlNode::Children::iterator itr = node.children.begin();
        for (; itr != node.children.end(); ++itr)
        {
            TagValue value;
            value.value = (**itr).contents;
            value.node = *itr;

            tags.insert(std::pair<std::string,TagValue>((**itr).name,value));
        }
    }

    // We triger the ENVIRONMENTAL_FACTOR for
    // the environmental mapping by the
    // amount of light from the sun/moon to get
    // better visual at darker times
    struct UpdateEnvironmentalFactor : public osg::StateSet::Callback
    {
        UpdateEnvironmentalFactor(OpenIG::Base::ImageGenerator* ig, float factor)
            : _ig(ig), _factor(factor)
        {
        }

        virtual void operator() (osg::StateSet* ss, osg::NodeVisitor*)
        {
            std::ostringstream oss;
            oss << _factor * _ig->getSunOrMoonLight()->getLight()->getDiffuse().r();

            ss->setDefine("ENVIRONMENTAL_FACTOR", oss.str());
        }
    protected:
        OpenIG::Base::ImageGenerator*	_ig;
        float					_factor;
    };

    // Handy callback for update float uniforms
    struct UpdateFloatUniformCallback : public osg::UniformCallback
    {
        UpdateFloatUniformCallback(float& value)
            : _value(value)
        {
        }

        virtual void operator () (osg::Uniform* u, osg::NodeVisitor*)
        {
            u->set(_value);
        }

    protected:
        float& _value;
    };

    // a hook when an entity is added
    // called from the IG
    virtual void entityAdded(OpenIG::PluginBase::PluginContext& context, unsigned int id, osg::Node& entity, const std::string& fileName)
    {
        // entity was added. If the file does not exists
        // we quit silently
        if (!osgDB::fileExists(fileName + ".xml")) return;

        // Say it loud, we have an XML model definition
        osg::notify(osg::NOTICE) << "ModelComposition: Parsing XML for " << fileName << std::endl;

        // We save these
        _entity = &entity;
        _ss = new osg::StateSet;

        // Here a bit of paranoia
        osgDB::XmlNode* root = osgDB::readXmlFile(fileName + ".xml");
        if (!root || !root->children.size() || root->children.at(0)->name != "OpenIg-Model-Composition") return;

        // we get the path and save
        // it for texture lookup from the cache
        _path = osgDB::getFilePath(fileName);
        _textureCache.addPath(_path);
        _textureCubeMapCache.addPath(_path);

        // A sub-entities map. We need
        // to keep track of all the
        // submodels added
        SubModelMap smm;

        // Our tags with values
        TagValueMultiMap	mmtags;
        TagValueMap			tags;

        // Read all the children
        readXmlNode(*root->children.at(0), tags, mmtags);

        // Iterate over lights
        TagValueMultiMap::iterator itr = mmtags.find("Light");
        while (itr != mmtags.end())
        {
            readLight((itr++)->second.node, entity, context, id);
            if (itr == mmtags.end() || itr->first != "Light") break;
        }

        // Read the material
        itr = mmtags.find("Material");
        while (itr != mmtags.end())
        {
            readMaterial((itr++)->second.node);
            if (itr == mmtags.end() || itr->first != "Material") break;
        }

        // Read the submodels
        itr = mmtags.find("Sub-model");
        while (itr != mmtags.end())
        {
            readSubmodel((itr++)->second.node, context, id, smm);
            if (itr == mmtags.end() || itr->first != "Sub-model") break;
        }

        // Read animations
        _entityWithSubmodels[id] = smm;
        itr = mmtags.find("Animation");
        while (itr != mmtags.end())
        {
            readAnimation((itr++)->second.node, context, id);
            if (itr == mmtags.end() || itr->first != "Animation") break;
        }

        // Some other
        _diffuseSlot			= atoi(tags["Diffuse-Slot"].value.c_str());
        _aoSlot					= atoi(tags["Pre-Baked-Ambient-Occlusion-Texture-Slot"].value.c_str());
        _environmentalMapSlot	= atoi(tags["Environmental-Slot"].value.c_str());
        _diffuseTextureName		= tags["Diffuse-Texture"].value;

        // setup ambiento occlusion
        bool ao = tags["Ambient-Occlusion"].value == "yes";
        if (ao)
        {
            std::string	aotexture = tags["Pre-Baked-Ambient-Occlusion-Texture"].value;
            float factor = atof(tags["Ambient-Occlusion-Factor"].value.c_str());

            Texture2DPointer texture = _textureCache.get(aotexture);
            if (texture.valid())
            {
                osg::notify(osg::NOTICE) << "ModelComposition: (" << fileName << ")" << " ao texture:" << aotexture << ", slot:" << _aoSlot << std::endl;

                _ss->setDefine("AO");
                _ss->setTextureAttributeAndModes(_aoSlot, texture, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
                _ss->addUniform(new osg::Uniform("ambientOcclusionTexture", (int)_aoSlot), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);
                _ss->addUniform(new osg::Uniform("ambientOcclusionFactor", factor), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);
            }
        }

        // setup environmental mapping
        bool envmapping = tags["Environmental"].value == "yes";
        if (envmapping)
        {
            // Read the 6 images of the env map
            OpenIG::Base::StringUtils::StringList environmentalMaps;
            environmentalMaps.push_back(tags["Environmental-Texture-Right"].value);
            environmentalMaps.push_back(tags["Environmental-Texture-Left"].value);
            environmentalMaps.push_back(tags["Environmental-Texture-Bottom"].value);
            environmentalMaps.push_back(tags["Environmental-Texture-Top"].value);
            environmentalMaps.push_back(tags["Environmental-Texture-Back"].value);
            environmentalMaps.push_back(tags["Environmental-Texture-Front"].value);

            TextureCubeMapPointer texture = _textureCubeMapCache.get(environmentalMaps);
            if (texture.valid())
            {
                _ss->addUniform(new osg::Uniform("environmentalMapTexture", (int)_environmentalMapSlot), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);
                _ss->setTextureAttributeAndModes(_environmentalMapSlot, texture, osg::StateAttribute::ON);
                _ss->setDefine("ENVIRONMENTAL");
                _ss->setDefine("ENVIRONMENTAL_FACTOR", tags["Environmental-Factor"].value);

                _ss->setUpdateCallback(new UpdateEnvironmentalFactor(context.getImageGenerator(), atof(tags["Environmental-Factor"].value.c_str())));

                osg::Uniform* u = new osg::Uniform(osg::Uniform::FLOAT, "todBasedEnvironmentalLightingFactor", 1.f);
                u->setUpdateCallback(new UpdateFloatUniformCallback(_todBasedEnvironmentalLightingFactor));
                _ss->addUniform(u);

                texture->setInternalFormat(GL_RGB);
                texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
                texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
                texture->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);
                texture->setFilter(osg::TextureCubeMap::MIN_FILTER, osg::TextureCubeMap::LINEAR);
                texture->setFilter(osg::TextureCubeMap::MAG_FILTER, osg::TextureCubeMap::LINEAR);
            }
        }

        // setup diffuse
        Texture2DPointer diffuseTexture = _textureCache.get(_diffuseTextureName);
        if (diffuseTexture.valid())
        {
            osg::notify(osg::NOTICE) << "ModelComposition: (" << fileName << ")" << " diffuse texture:" << _diffuseTextureName << ", slot:" << _diffuseSlot << std::endl;

            _ss->setTextureAttributeAndModes(_diffuseSlot, diffuseTexture, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
            _ss->addUniform(new osg::Uniform("baseTexture", (int)_diffuseSlot), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
        }

        TagValueMap::const_iterator iterNormalMap = tags.find("NormalMap");
        TagValueMap::const_iterator iterNormalMapSlot = tags.find("NormalMapSlot");
        if (iterNormalMap!=tags.end()&&iterNormalMapSlot!=tags.end())
        {
            const std::string& normalMap = iterNormalMap->second.value;
            int normalMapSlot = atoi(iterNormalMapSlot->second.value.c_str());
            if (normalMap!="")
            {
                Texture2DPointer normalMapTexture = _textureCache.get(normalMap);
                if (normalMapTexture.valid())
                {
                    osg::notify(osg::NOTICE) << "ModelComposition: (" << fileName << ")" << " normal map texture:" << normalMap << ", slot:" << normalMapSlot << std::endl;

                    _ss->setTextureAttributeAndModes(normalMapSlot, normalMapTexture, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
                    _ss->addUniform(new osg::Uniform("normalMapSampler", (int)normalMapSlot), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);

                    _ss->setDefine("HAS_NORMAL_MAP");
                }
            }
        }

        // shadowing, set deffault to casting
        int ReceivesShadowTraversalMask = 0x1;
        int CastsShadowTraversalMask = 0x2;

        entity.setNodeMask(CastsShadowTraversalMask);

        bool shadowing = tags["Shadowing"].value == "yes";
        if (shadowing)
        {
            std::string shadow = tags["Shadow"].value;
            if (shadow == "CAST")
                entity.setNodeMask(CastsShadowTraversalMask);
            if (shadow == "RECEIVE")
                entity.setNodeMask(ReceivesShadowTraversalMask);
            if (shadow == "CAST-AND-RECEIVE")
                entity.setNodeMask(CastsShadowTraversalMask | ReceivesShadowTraversalMask);

            _ss->setDefine("SHADOWING", osg::StateAttribute::ON);
        }
        else
            _ss->setDefine("SHADOWING", osg::StateAttribute::OFF);


        // setup the material if valid
        if (_dayMaterial.valid() && _nightMaterial.valid())
        {
            _runtimeMaterial = new osg::Material(*_dayMaterial);

            entity.getOrCreateStateSet()->setAttributeAndModes(_runtimeMaterial,osg::StateAttribute::ON|osg::StateAttribute::PROTECTED|osg::StateAttribute::OVERRIDE);
        }

        // set our stateset
        entity.asGroup()->getChild(0)->setStateSet(_ss);
    }

protected:
    unsigned int                                        _subentityId;
    std::string                                         _path;
    osg::observer_ptr<osg::Node>                        _entity;

    osg::ref_ptr<osg::StateSet>                         _ss;
    osg::ref_ptr<osg::Material>                         _dayMaterial;
    osg::ref_ptr<osg::Material>                         _nightMaterial;
    osg::ref_ptr<osg::Material>                         _runtimeMaterial;

    osg::ref_ptr<osg::Program>                          _spriteProgram;
    osg::ref_ptr<osg::Program>                          _lightPointProgram;
    unsigned int										_autoLightId;

    std::string                                         _diffuseTextureName;
    unsigned int                                        _diffuseSlot;

    bool                                                _aoOcclustion;
    unsigned int										_aoSlot;

    float                                               _environmentalFactor;
    unsigned int                                        _environmentalMapSlot;

    bool												_normalmapslot;

    float												_todBasedEnvironmentalLightingFactor;

    // This is struct we keep for sub-entity definition
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
        bool			_environmental;
        std::string		_diffuse;
        std::string		_ambient;
        std::string		_specular;
        std::string		_shininess;
        bool			_environmentalSet;
        bool			_materialSet;
        osg::ref_ptr<osg::Material>	_material;

        SubModelEntry() : _id(0), _environmental(true), _environmentalSet(false), _materialSet(false) {}

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

    // Struct for animation blinking pulses
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

    // Light attributes
    struct LightAttribs
    {
        osg::Vec3                   _position;
        osg::Vec4                   _color;
        osg::Vec3                   _orientation;
        osg::Vec3                   _offset;
        bool                        _animated;
        LightAnimationPulsesList    _pulses;
        bool                        _real;
        double                      _minPixelSize;
        double                      _maxPixelSize;
        double                      _minPixelSizeMultiplierForSprites;
        float                       _radius;
        double                      _brightness;
        double                      _constAttenuation;
        double                      _fStartRange;
        double                      _fEndRange;
        double                      _fSpotInnerAngle;
        double                      _fSpotOuterAngle;
        std::string                 _spriteTexture;

        LightAttribs()
            : _animated(false)
            , _real(false)
            , _minPixelSize(2)
            , _minPixelSizeMultiplierForSprites(1)
            , _maxPixelSize(5)
            , _brightness(1)
            , _constAttenuation(5)
            , _radius(1)
        {

        }

    };

    // Callback for Lights blinking
    struct LightBlinkUpdateCallback : public osg::NodeCallback
    {
        LightBlinkUpdateCallback(const LightAttribs& light, OpenIG::Base::ImageGenerator* ig)
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
        OpenIG::Base::ImageGenerator* _ig;
        unsigned int            _currentPulse;
    };

    void setUpSpriteStateSetProgram()
    {
        static bool checkedProgram = false;
        if (checkedProgram)
        {
            return;
        }
        checkedProgram = true;

#if defined(_WIN32)
            std::string resourcesPath = OpenIG::Base::FileSystem::path(OpenIG::Base::FileSystem::Resources, "../resources");
#else
            std::string resourcesPath = OpenIG::Base::FileSystem::path(OpenIG::Base::FileSystem::Resources, "../../openig/resources");
#endif

        std::string strVS = OpenIG::Base::FileSystem::readFileIntoString(resourcesPath + "/shaders/sprite_bb_vs.glsl");
        std::string strGS = OpenIG::Base::FileSystem::readFileIntoString(resourcesPath + "/shaders/sprite_bb_gs.glsl");
        std::string strPS = OpenIG::Base::FileSystem::readFileIntoString(resourcesPath + "/shaders/sprite_bb_ps.glsl");
        if (strVS!=""&&strPS!=""&&strGS!="")
        {
            osg::notify(osg::NOTICE)<<"Model Composition: Loaded sprite programs (vs, gs, ps)"<<std::endl;
            _spriteProgram = new osg::Program;
            _spriteProgram->addShader(new osg::Shader(osg::Shader::VERTEX  , strVS));
            _spriteProgram->addShader(new osg::Shader(osg::Shader::GEOMETRY, strGS));
            _spriteProgram->addShader(new osg::Shader(osg::Shader::FRAGMENT, strPS));
        }
        else
        {
            osg::notify(osg::NOTICE)<<"Model Composition: Error: could not load sprite programs (vs, gs, ps)"<<std::endl;
        }
    }

    TextureCache			_textureCache;
    TextureCubeMapCache		_textureCubeMapCache;

    static bool useLogZDepthBuffer(void)
    {
        std::string strLogZDepthBuffer = OpenIG::Base::Configuration::instance()->getConfig("LogZDepthBuffer","yes");
        if (strLogZDepthBuffer.compare(0, 3, "yes") == 0)
            return true;
        else
            return false;
    }

    void setUpSpriteStateSet(osgSim::LightPointNode& lpn, const LightAttribs& def, OpenIG::Base::ImageGenerator* ig)
    {
        if (def._spriteTexture.empty())
        {
            osg::notify(osg::NOTICE)<<"Empty sprite texture!"<<std::endl;
            return;
        }

        setUpSpriteStateSetProgram();
        if (_spriteProgram.valid()==false)
        {
            osg::notify(osg::NOTICE)<<"Empty sprite program!"<<std::endl;
            return;
        }

        osg::ref_ptr<osg::Texture2D> spriteTexture = _textureCache.get(def._spriteTexture);

        osg::StateSet* stateSet = new osg::StateSet();
        stateSet->setRenderBinDetails(MOVING_SPRITE_LIGHT_POINTS_RENDER_BIN, "DepthSortedBin");

        // Turn off our lighting. We will use our own shader, primarily because we use logarithmic depth buffer,
        // but also because we could have an optional sprite texture
        stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF|osg::StateAttribute::PROTECTED|osg::StateAttribute::OVERRIDE);

        osg::StateAttribute::OverrideValue val = osg::StateAttribute::ON|osg::StateAttribute::PROTECTED;

        stateSet->setTextureAttributeAndModes(0, spriteTexture, val);
        stateSet->addUniform(new osg::Uniform("spriteTexture", 0), val);

        if (useLogZDepthBuffer())
        {
            stateSet->setDefine("USE_LOG_DEPTH_BUFFER");
        }

        static const float radiusMultiplier = 0.25f;
        osg::Vec4f vSpriteDimensions(def._minPixelSize*def._minPixelSizeMultiplierForSprites, def._maxPixelSize, def._radius*radiusMultiplier, 0);
        stateSet->addUniform(new osg::Uniform("spriteDimensions", vSpriteDimensions), val);

        stateSet->setAttributeAndModes(_spriteProgram, val);

        // Override the state set because the LightPoint node uses an internal singleton stateset
        lpn.setStateSet(stateSet);
    }

    void setUpLightPointStateSetProgram()
    {
        static bool checkedProgram = false;
        if (checkedProgram)
        {
            return;
        }
        checkedProgram = true;

#if defined(_WIN32)
            std::string resourcesPath = OpenIG::Base::FileSystem::path(OpenIG::Base::FileSystem::Resources, "../resources");
#else
            std::string resourcesPath = OpenIG::Base::FileSystem::path(OpenIG::Base::FileSystem::Resources, "../../openig/resources");
#endif

        std::string strVS = OpenIG::Base::FileSystem::readFileIntoString(resourcesPath+"/shaders/lightpoint_vs.glsl");
        std::string strPS = OpenIG::Base::FileSystem::readFileIntoString(resourcesPath+"/shaders/lightpoint_ps.glsl");

        if (strVS!=""&&strPS!="")
        {
            osg::notify(osg::NOTICE)<<"Model Composition: Loaded light point (fallback) programs"<<std::endl;
            _lightPointProgram = new osg::Program;
            _lightPointProgram->addShader(new osg::Shader(osg::Shader::VERTEX  , strVS));
            _lightPointProgram->addShader(new osg::Shader(osg::Shader::FRAGMENT, strPS));
        }
        else
        {
            osg::notify(osg::NOTICE)<<"Model Composition: Error: could not load light point (fallback) programs"<<std::endl;
        }
    }

    void setUpLightPointStateSet(osgSim::LightPointNode& lpn, const LightAttribs& def, OpenIG::Base::ImageGenerator* ig)
    {
        setUpLightPointStateSetProgram();
        if (_lightPointProgram.valid()==false)
        {
            std::cerr<<"Empty light point program!"<<std::endl;
            return;
        }

        osg::StateSet* stateSet = lpn.getOrCreateStateSet();
        stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF|osg::StateAttribute::PROTECTED|osg::StateAttribute::OVERRIDE);
        stateSet->setAttributeAndModes(_lightPointProgram, osg::StateAttribute::ON|osg::StateAttribute::PROTECTED|osg::StateAttribute::OVERRIDE);
        if (useLogZDepthBuffer())
        {
            stateSet->setDefine("USE_LOG_DEPTH_BUFFER");
        }
    }

    void readLight(osgDB::XmlNode* node, osg::Node& entity, OpenIG::PluginBase::PluginContext& context, unsigned int entityId)
    {
        LightAttribs light;

        OpenIG::Base::LightType lightType = OpenIG::Base::LT_POINT;

        TagValueMap tags;
        readXmlNode(*node, tags);

        // Read the color
        std::string color = tags["Color"].value;
        OpenIG::Base::StringUtils::Tokens tokens = OpenIG::Base::StringUtils::instance()->tokenize(color);
        if (tokens.size() == 4)
        {
            float r = atof(tokens.at(0).c_str());
            float g = atof(tokens.at(1).c_str());
            float b = atof(tokens.at(2).c_str());
            float a = atof(tokens.at(3).c_str());

            light._color = osg::Vec4(r, g, b, a);
        }

        // POsition
        std::string position = tags["Position"].value;
        tokens = OpenIG::Base::StringUtils::instance()->tokenize(position);
        if (tokens.size() == 3)
        {
            float x = atof(tokens.at(0).c_str());
            float y = atof(tokens.at(1).c_str());
            float z = atof(tokens.at(2).c_str());

            light._position = osg::Vec3(x, y, z);
        }

        // Animation pulses
        std::string pulses = tags["Animation-Pulses"].value;
        tokens = OpenIG::Base::StringUtils::instance()->tokenize(pulses, ",");
        for (size_t i = 0; i < tokens.size(); ++i)
        {
            OpenIG::Base::StringUtils::Tokens ltokens = OpenIG::Base::StringUtils::instance()->tokenize(tokens.at(i));
            if (ltokens.size() == 5)
            {
                LightAnimationPulses pulse;

                pulse._duration = atof(ltokens.at(0).c_str());

                float r = atof(ltokens.at(1).c_str());
                float g = atof(ltokens.at(2).c_str());
                float b = atof(ltokens.at(3).c_str());
                float a = atof(ltokens.at(4).c_str());

                pulse._color = osg::Vec4(r, g, b, a);
                light._pulses.push_back(pulse);
            }
            light._animated = light._pulses.size() != 0;
        }

        // Orientation
        tokens = OpenIG::Base::StringUtils::instance()->tokenize(tags["Orientation"].value);
        if (tokens.size() == 3)
        {
            float x = atof(tokens.at(0).c_str());
            float y = atof(tokens.at(1).c_str());
            float z = atof(tokens.at(2).c_str());

            light._orientation = osg::Vec3(x, y, z);

            lightType = OpenIG::Base::LT_SPOT;
        }

        // Offset for the OpenIG light
        tokens = OpenIG::Base::StringUtils::instance()->tokenize(tags["Real-Light-Offset"].value);
        if (tokens.size() == 3)
        {
            float x = atof(tokens.at(0).c_str());
            float y = atof(tokens.at(1).c_str());
            float z = atof(tokens.at(2).c_str());

            light._offset = osg::Vec3(x, y, z);
        }

        // The sprite texture with its world size
        tokens = OpenIG::Base::StringUtils::instance()->tokenize(tags["SpriteTexture"].value);
        if (tokens.size() == 1)
        {
            light._spriteTexture = tokens.at(0);
            //std::cerr<<"Sprite Texture: "<<light._spriteTexture<<std::endl;
        }

        tokens = OpenIG::Base::StringUtils::instance()->tokenize(tags["Radius"].value);
        if (tokens.size() == 1)
        {
            light._radius = atof(tokens.at(0).c_str());
        }

        // Some other tags
        light._real								= tags["Use-Real-Light"].value == "yes";
        light._brightness						= atof(tags["Brightness"].value.c_str());
        light._minPixelSize						= osg::maximum(atof(tags["MinPixelSize"].value.c_str()),1.0);
        light._maxPixelSize						= osg::maximum(atof(tags["MaxPixelSize"].value.c_str()), 1.0);
        light._minPixelSizeMultiplierForSprites = osg::maximum(atof(tags["MinPixelSizeMultiplierForSprites"].value.c_str()),1.0);
        light._constAttenuation					= atof(tags["ConstAttenuation"].value.c_str());
        light._fStartRange						= atof(tags["StartRange"].value.c_str());
        light._fEndRange						= atof(tags["EndRange"].value.c_str());
        light._fSpotInnerAngle					= atof(tags["SpotInnerAngle"].value.c_str());
        light._fSpotOuterAngle					= atof(tags["SpotOuterAngle"].value.c_str());

        // Setup the light point
        osg::ref_ptr<osgSim::LightPointNode> lpn = new osgSim::LightPointNode;
		lpn->setCullingActive(false);

        // Let find if we have foward+ available
        // if not, no sprites either, till fixed
        bool forwardPlusPluginAvailable = false;

        // Look up for the F+ plugin
        OpenIG::Engine* openIG = dynamic_cast<OpenIG::Engine*>(context.getImageGenerator());
        if (openIG)
        {
            const OpenIG::PluginBase::PluginHost::PluginsMap& plugins = openIG->getPlugins();
            OpenIG::PluginBase::PluginHost::PluginsMap::const_iterator itr = plugins.begin();
            for (; itr != plugins.end(); ++itr)
            {
                if (itr->second->getName() == "ForwardPlusLighting")
                {
                    forwardPlusPluginAvailable = true;
                    break;
                }
            }
        }

        //lpn->setPointSprite();
        //if (light._spriteTexture != std::string("") && forwardPlusPluginAvailable)
		if (light._spriteTexture != std::string(""))
        {
            setUpSpriteStateSet(*lpn, light, context.getImageGenerator());
        }
        else
        {
            setUpLightPointStateSet(*lpn, light, context.getImageGenerator());
        }

        if (light._animated)
        {
            lpn->setUpdateCallback(new LightBlinkUpdateCallback(light,context.getImageGenerator()));
        }

        lpn->setMinPixelSize(light._minPixelSize);
        lpn->setMaxPixelSize(light._maxPixelSize);

        osgSim::LightPoint lp(true,light._position,light._color);
        lp._blendingMode = osgSim::LightPoint::ADDITIVE;
        lpn->addLightPoint(lp);

        entity.asGroup()->addChild(lpn);

        OpenIG::Base::GlobalIdGenerator::instance()->getNextId("Real-Lights",_autoLightId);

        lpn->setUserValue("lightId",(unsigned int)_autoLightId);
        lpn->setUserValue("lightOn",(bool)true);


        if (light._real)
        {
            OpenIG::Base::LightAttributes la;
            la.diffuse = light._color;
            la.ambient = osg::Vec4(0,0,0,1);
            la.specular = osg::Vec4(0,0,0,1);
            la.constantAttenuation = light._constAttenuation;
            la.brightness = light._brightness;
            la.spotCutoff = 20;

            // PPP:
            la.fStartRange		= light._fStartRange;
            la.fEndRange		= light._fEndRange;
            la.fSpotInnerAngle	= light._fSpotInnerAngle;
            la.fSpotOuterAngle	= light._fSpotOuterAngle;

            la.lightType		= lightType;
            la.cullingActive	= false;

            la.dirtyMask = OpenIG::Base::LightAttributes::ALL;

            std::string name = tags["Name"].value;
            osg::notify(osg::NOTICE) << "ModelComposition: Added light: " << name << ", " << _autoLightId << std::endl;

            context.getImageGenerator()->addLight(_autoLightId
                , la
                , OpenIG::Base::Math::instance()->toMatrix(
                    light._position.x()+light._offset.x(),
                    light._position.y()+light._offset.y(),
                    light._position.z()+light._offset.z(),
                    light._orientation.x(),
                    light._orientation.y(),
                    light._orientation.z())
            );
            context.getImageGenerator()->updateLightAttributes(_autoLightId,la);
            context.getImageGenerator()->setLightUserData(_autoLightId, lpn);
            context.getImageGenerator()->bindLightToEntity(_autoLightId++,entityId);
        }
    }

    void readMaterial(osgDB::XmlNode* node)
    {
        TagValueMap tags;
        readXmlNode(*node, tags);

        osg::ref_ptr<osg::Material> material;

        // read the correct material we are setting
        if (tags["Name"].value == "Day")
        {
            _dayMaterial = new osg::Material;
            material = _dayMaterial;
        }
        else
        if (tags["Name"].value == "Night")
        {
            _nightMaterial = new osg::Material;
            material = _nightMaterial;
        }

        // Ambient
        OpenIG::Base::StringUtils::Tokens tokens = OpenIG::Base::StringUtils::instance()->tokenize(tags["Ambient"].value);
        if (tokens.size() == 4)
        {
            float r = atof(tokens.at(0).c_str());
            float g = atof(tokens.at(1).c_str());
            float b = atof(tokens.at(2).c_str());
            float a = atof(tokens.at(3).c_str());

            if (material.valid()) material->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(r, g, b, a));
        }

        // Diffuse
        tokens = OpenIG::Base::StringUtils::instance()->tokenize(tags["Diffuse"].value);
        if (tokens.size() == 4)
        {
            float r = atof(tokens.at(0).c_str());
            float g = atof(tokens.at(1).c_str());
            float b = atof(tokens.at(2).c_str());
            float a = atof(tokens.at(3).c_str());

            if (material.valid()) material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(r, g, b, a));
        }

        // Specular
        tokens = OpenIG::Base::StringUtils::instance()->tokenize(tags["Specular"].value);
        if (tokens.size() == 4)
        {
            float r = atof(tokens.at(0).c_str());
            float g = atof(tokens.at(1).c_str());
            float b = atof(tokens.at(2).c_str());
            float a = atof(tokens.at(3).c_str());

            if (material.valid()) material->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(r, g, b, a));
        }

        // Shininess
        if (material.valid()) material->setShininess(osg::Material::FRONT_AND_BACK, atof(tags["Shininess"].value.c_str()));
    }

    void readAnimationSequence(osgDB::XmlNode* node,OpenIG::Base::Animations::Animation::Sequence* sequence, unsigned int entityId)
    {
        TagValueMap tags;
        readXmlNode(*node, tags);

        // Name
        sequence->_name = tags["Name"].value;

        // Player
        SubModelMap& smm = _entityWithSubmodels[entityId];
        SubModelMapIterator itr = smm.find(tags["Player"].value);
        if (itr != smm.end())
        {
            sequence->_player = tags["Player"].value;
            sequence->_playerId = itr->second._id;
            sequence->_playerOriginalOrientation = itr->second._originalOrientation;
            sequence->_playerOriginalPosition = itr->second._originalPosition;
        }

        // Time frame
        OpenIG::Base::StringUtils::Tokens tokens = OpenIG::Base::StringUtils::instance()->tokenize(tags["Time-Frame"].value);
        if (tokens.size() == 2)
        {
            double from = atof(tokens.at(0).c_str());
            double to = atof(tokens.at(1).c_str());

            sequence->_timeFrame = std::pair<double, double>(from, to);
        }

        // Orientation update vector
        tokens = OpenIG::Base::StringUtils::instance()->tokenize(tags["Orientation-Update-Vector"].value, ",");
        if (tokens.size() == 3)
        {
            float x = atof(tokens.at(0).c_str());
            float y = atof(tokens.at(1).c_str());
            float z = atof(tokens.at(2).c_str());

            sequence->_operationVector = osg::Vec3(x, y, z);
        }

        // Positional update vector
        tokens = OpenIG::Base::StringUtils::instance()->tokenize(tags["Position-Update-Vector"].value, ",");
        if (tokens.size() == 3)
        {
            float x = atof(tokens.at(0).c_str());
            float y = atof(tokens.at(1).c_str());
            float z = atof(tokens.at(2).c_str());

            sequence->_positionalOperationVector = osg::Vec3(x, y, z);
        }

        // Orientation update
        tokens = OpenIG::Base::StringUtils::instance()->tokenize(tags["Orientation-Update"].value);
        if (tokens.size() == 2)
        {
            double from = atof(tokens.at(0).c_str());
            double to = atof(tokens.at(1).c_str());

            sequence->_rotationUpdate = std::pair<double, double>(from, to);
        }

        // Positional update
        tokens = OpenIG::Base::StringUtils::instance()->tokenize(tags["Position-Update"].value);
        if (tokens.size() == 6)
        {
            float x1 = atof(tokens.at(0).c_str());
            float y1 = atof(tokens.at(1).c_str());
            float z1 = atof(tokens.at(2).c_str());

            float x2 = atof(tokens.at(3).c_str());
            float y2 = atof(tokens.at(4).c_str());
            float z2 = atof(tokens.at(5).c_str());

            sequence->_positionalUpdate = std::pair<osg::Vec3, osg::Vec3>(osg::Vec3(x1, y1, z1), osg::Vec3(x2, y2, z2));
        }

        // Swap pitch roll
        sequence->_swapPitchRoll = tags["Swap-Pitch-Roll"].value == "yes";
    }

    void readAnimation(osgDB::XmlNode* node, OpenIG::PluginBase::PluginContext& context, unsigned int entityId)
    {
        osg::ref_ptr<OpenIG::Base::Animations::Animation> animation = new OpenIG::Base::Animations::Animation;

        TagValueMap			tags;
        TagValueMultiMap	mmtags;
        readXmlNode(*node, tags, mmtags);

        //Name and duration
        animation->_name		= tags["Name"].value;
        animation->_duration	= atoi(tags["Duration-In-Seconds"].value.c_str());

        // Sequence
        TagValueMultiMap::iterator itr = mmtags.find("Sequence");
        while (itr != mmtags.end())
        {
            osg::ref_ptr<OpenIG::Base::Animations::Animation::Sequence> seq = new OpenIG::Base::Animations::Animation::Sequence;
            readAnimationSequence(itr->second.node, seq.get(), entityId);
            animation->_sequences[seq->_name] = seq;

            if (++itr == mmtags.end() || itr->first != "Sequence") break;
        }

        // Setup the animation
        OpenIG::Base::ImageGenerator::Entity& entity = context.getImageGenerator()->getEntityMap()[entityId];\
        if (entity.valid())
        {
            OpenIG::Base::Animations::AnimationContainer* ac = dynamic_cast<OpenIG::Base::Animations::AnimationContainer*>(entity->getUserData());
            if (!ac)
            {
                ac  = new OpenIG::Base::Animations::AnimationContainer;
                entity->setUserData(ac);
            }

            (*ac)[animation->_name] = animation;
        }
    }

    void readSubmodel(osgDB::XmlNode* node, OpenIG::PluginBase::PluginContext& context, unsigned int entityId, SubModelMap& smm)
    {
        // The entry and the auto id
        SubModelEntry submodel;
        submodel._id = _subentityId++;

        // we keep this to add readed
        // submodel prior reading its
        // submodels
        bool submodelAdded = false;

        TagValueMap			tags;
        TagValueMultiMap	mmtags;
        readXmlNode(*node, tags, mmtags);

        submodel._name			= tags["Name"].value;
        submodel._fileName		= tags["File"].value;
        submodel._position		= tags["Position"].value;
        submodel._orientation	= tags["Orientation"].value;
        submodel._limits		= tags["Limits"].value;

        TagValueMap::iterator eitr = tags.find("Environmental");
        if (eitr != tags.end())
        {
            submodel._environmentalSet = true;
            submodel._environmental = tags["Environmental"].value == "yes";
        }

        // Read the material
        bool materialSet = false;
        eitr = tags.find("Ambient");
        if (eitr != tags.end())
        {
            submodel._materialSet = true;
            submodel._ambient = tags["Ambient"].value;
        }
        eitr = tags.find("Diffuse");
        if (eitr != tags.end())
        {
            submodel._materialSet = true;
            submodel._diffuse = tags["Diffuse"].value;
        }
        eitr = tags.find("Specular");
        if (eitr != tags.end())
        {
            submodel._materialSet = true;
            submodel._specular = tags["Specular"].value;
        }
        eitr = tags.find("Shininess");
        if (eitr != tags.end())
        {
            submodel._materialSet = true;
            submodel._shininess = tags["Shininess"].value;
        }

        OpenIG::Base::StringUtils::Tokens tokens = OpenIG::Base::StringUtils::instance()->tokenize(submodel._orientation);
        if (tokens.size() == 3)
        {
            float x = atof(tokens.at(0).c_str());
            float y = atof(tokens.at(1).c_str());
            float z = atof(tokens.at(2).c_str());

            submodel._originalOrientation = osg::Vec3(x, y, z);
        }

        tokens = OpenIG::Base::StringUtils::instance()->tokenize(submodel._position);
        if (tokens.size() == 3)
        {
            float x = atof(tokens.at(0).c_str());
            float y = atof(tokens.at(1).c_str());
            float z = atof(tokens.at(2).c_str());

            submodel._originalPosition = osg::Vec3(x, y, z);
        }

        // Create the material
        if (!submodel._ambient.empty() && !submodel._diffuse.empty() && !submodel._specular.empty() && !submodel._shininess.empty())
        {
            submodel._material = new osg::Material;

            tokens = OpenIG::Base::StringUtils::instance()->tokenize(submodel._ambient);
            if (tokens.size() == 4)
            {
                float r = atof(tokens.at(0).c_str());
                float g = atof(tokens.at(1).c_str());
                float b = atof(tokens.at(2).c_str());
                float a = atof(tokens.at(3).c_str());

                submodel._material->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(r,g,b,a));
            }
            tokens = OpenIG::Base::StringUtils::instance()->tokenize(submodel._diffuse);
            if (tokens.size() == 4)
            {
                float r = atof(tokens.at(0).c_str());
                float g = atof(tokens.at(1).c_str());
                float b = atof(tokens.at(2).c_str());
                float a = atof(tokens.at(3).c_str());

                submodel._material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(r, g, b, a));
            }
            tokens = OpenIG::Base::StringUtils::instance()->tokenize(submodel._specular);
            if (tokens.size() == 4)
            {
                float r = atof(tokens.at(0).c_str());
                float g = atof(tokens.at(1).c_str());
                float b = atof(tokens.at(2).c_str());
                float a = atof(tokens.at(3).c_str());

                submodel._material->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(r, g, b, a));
            }
            submodel._material->setShininess(osg::Material::FRONT_AND_BACK, atof(submodel._shininess.c_str()));
        }

        // Read all the lights
        TagValueMultiMap::iterator itr = mmtags.find("Light");
        while (itr != mmtags.end())
        {
            if (!submodelAdded)
            {
                addSubmodel(submodel, context, entityId, smm);
                submodelAdded = true;
            }
            OpenIG::Base::ImageGenerator::Entity& submodelEntity = context.getImageGenerator()->getEntityMap()[submodel._id];
            readLight((itr++)->second.node, *submodelEntity, context, submodel._id);

            if (itr == mmtags.end() || itr->first != "Light") break;
        }

        // Read all the submodels
        itr = mmtags.find("Sub-model");
        while (itr != mmtags.end())
        {
            if (submodel.isValid())
            {
                if (!submodelAdded)
                {
                    addSubmodel(submodel, context, entityId, smm);
                    submodelAdded = true;
                }
                readSubmodel(itr->second.node, context, submodel._id, smm);
            }

            if (++itr == mmtags.end() || itr->first != "Sub-model") break;
        }

        if (!submodelAdded)
        {
            addSubmodel(submodel,context,entityId,smm);
        }

    }

    void addSubmodel(const SubModelEntry& submodel,OpenIG::PluginBase::PluginContext& context,unsigned int entityId, SubModelMap& smm)
    {
        OpenIG::Base::StringUtils::Tokens tokensPosition = OpenIG::Base::StringUtils::instance()->tokenize(submodel._position);
        OpenIG::Base::StringUtils::Tokens tokensOrientation = OpenIG::Base::StringUtils::instance()->tokenize(submodel._orientation);

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

        osg::Matrixd mx = OpenIG::Base::Math::instance()->toMatrix(pos.x(),pos.y(),pos.z(),ori.x(),ori.y(),ori.z());

        std::string subModelFileName = _path + "/" + submodel._fileName;
        osg::notify(osg::NOTICE) << "Model Composition: Loading submodel: (" << submodel._id << ") " << subModelFileName << std::endl;

        context.getImageGenerator()->addEntity(submodel._id,subModelFileName,mx);
        context.getImageGenerator()->bindToEntity(submodel._id,entityId);
        context.getImageGenerator()->setEntityName(submodel._id,submodel._name);

        OpenIG::Base::ImageGenerator::Entity& subentity = context.getImageGenerator()->getEntityMap()[submodel._id];
        subentity->setStateSet(_ss);

        // Special case when submodels can turn off the
        // Environmental mapping and can have its own
        // Material
        if (submodel._environmentalSet)
        {
            osg::StateAttribute::OverrideValue value = 0;
            switch (submodel._environmental)
            {
            case true:
                value = osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE;
                break;
            case false:
                value = osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE;
                break;
            }
            osg::ref_ptr<osg::StateSet> ss = new osg::StateSet;
            ss->setDefine("ENVIRONMENTAL", value);
            ss->merge(*_ss);

            subentity->setStateSet(ss);
        }
        if (submodel._materialSet && submodel._material.valid())
        {
            osg::ref_ptr<osg::StateSet> ss = new osg::StateSet;
            ss->setAttributeAndModes(submodel._material, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
            ss->merge(*subentity->getStateSet());

            subentity->setStateSet(ss);
        }

        smm[submodel._name] = submodel;

    }

};
} // namespace
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

extern "C" EXPORT OpenIG::PluginBase::Plugin* CreatePlugin()
{
    return new OpenIG::Plugins::ModelCompositionPlugin;
}

extern "C" EXPORT void DeletePlugin(OpenIG::PluginBase::Plugin* plugin)
{
    osg::ref_ptr<OpenIG::PluginBase::Plugin> p(plugin);
}
