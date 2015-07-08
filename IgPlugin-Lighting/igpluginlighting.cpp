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
// Lighting GLSL inspired from the following article
// http://www.ozone3d.net/tutorials/glsl_lighting_phong_p3.php

#include <IgPluginCore/plugin.h>
#include <IgPluginCore/plugincontext.h>

#include <IgCore/imagegenerator.h>
#include <IgCore/attributes.h>
#include <IgCore/configuration.h>
#include <IgCore/globalidgenerator.h>
#include <IgCore/commands.h>
#include <IgCore/stringutils.h>

#include <osg/Version>
#include <osg/ref_ptr>
#include <osg/LightSource>
#include <osg/Light>
#include <osg/Material>
#include <osg/TextureRectangle>
#include <osg/ValueObject>

#include <osgUtil/PositionalStateContainer>

#include <osgDB/XmlParser>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include <osgShadow/ShadowedScene>
#include <osgShadow/MinimalShadowMap>


#include <map>
#include <iostream>

#define LIGHT_DIFFUSE                                0
#define LIGHT_SPECULAR                               1
#define LIGHT_POSITION                               2
#define LIGHT_DIRECTION                              3
#define LIGHT_CONST_ATTENUATION_SPOT_CUTOFF_ENABLED  4
#define LIGHT_MAX                                    5

namespace igplugins
{

class DummyLight : public osg::Light
{
public:
    DummyLight(unsigned int id, bool enabled)
        : osg::Light(id)
        , _enabled(enabled)
        , _id(id)
    {
        setUserValue("id",(unsigned int)id);
        setUserValue("enabled",(bool)enabled);
    }

    inline bool getEnabled() const
    {
        return _enabled;
    }

	inline void setEnabled(bool enabled)
	{
		_enabled = enabled;
	}

    virtual void apply(osg::State& state) const
    {
        if (_id >=8) return;
        osg::Light::apply(state);
    }


protected:
    bool            _enabled;
    unsigned int    _id;
};

class LightManager
{
public:
    static LightManager*    instance();
    static void             setMaxNumOfLights(unsigned int num);

    void addUpdateLight(unsigned int id, const DummyLight& light)
    {
        if (id >= _maxNumOfLights)
        {
            // warn here
            return;
        }

        if (!_image.valid())
        {
            _image = new osg::Image;
            _image->allocateImage(LIGHT_MAX,_maxNumOfLights+1,1,GL_RGBA,GL_FLOAT);
            _image->setInternalTextureFormat(GL_RGBA16F_ARB);
            _image->setDataVariance(osg::Object::DYNAMIC);

            memset(_image->data(),0,_image->getTotalSizeInBytes());

            _texture = new osg::TextureRectangle;
            _texture->setDataVariance(osg::Object::DYNAMIC);
            _texture->setImage(_image);
            _texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
            _texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);            
            _texture->setUnRefImageDataAfterApply(false);
            _texture->setUseHardwareMipMapGeneration(false);
        }

#if 0
        double angle = osg::inDegrees(light.getSpotCutoff());
        double cosSpotCutoff = cos(angle);
#else
        double cosSpotCutoff = light.getSpotCutoff();
#endif
        osg::Vec4f* data = (osg::Vec4f*)(_image->data(0,id));
        data[LIGHT_DIFFUSE] = light.getDiffuse();
        data[LIGHT_SPECULAR] = light.getSpecular();
        data[LIGHT_POSITION] = light.getPosition();
        data[LIGHT_DIRECTION] = osg::Vec4(
                light.getDirection().x(),
                light.getDirection().y(),
                light.getDirection().z(),
                1.f);
        data[LIGHT_CONST_ATTENUATION_SPOT_CUTOFF_ENABLED] =
                osg::Vec4(light.getConstantAttenuation(),
                (float)cosSpotCutoff,
                light.getEnabled() ? 1.f : 0.f, 0.f);

        _dirty = true;

        //osg::notify(osg::NOTICE) << "updated light: " << id << std::endl;
    }

    void updateTextureObject()
    {
        if (_image.valid() && _dirty)
        {
            _image->dirty();

            _dirty = false;
        }
    }

    inline unsigned int getMaxNumOfLights() const
    {
        return LightManager::_maxNumOfLights;
    }

    inline osg::TextureRectangle* getTextureRectangle() const
    {
        return _texture;
    }

protected:
    LightManager() {}
    ~LightManager() {}

    osg::ref_ptr<osg::Image>            _image;
    osg::ref_ptr<osg::TextureRectangle> _texture;
    bool                                _dirty;
    static unsigned int                 _maxNumOfLights;
};

LightManager* LightManager::instance()
{
    static LightManager s_LightManager;
    return &s_LightManager;
}

void LightManager::setMaxNumOfLights(unsigned int num)
{
    LightManager::_maxNumOfLights = num;
}

unsigned int LightManager::_maxNumOfLights = 0;

class UpdateLightAttribscallback : public osg::NodeCallback
{
public:
    UpdateLightAttribscallback(igcore::ImageGenerator* ig, osg::Group* scene)
		: _ig(ig)
        , _scene(scene)
    {

    }

    virtual void operator()(osg::Node*, osg::NodeVisitor* nv)
    {
        osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
        if (!cv) return;

        osgUtil::RenderStage *rs = cv->getCurrentRenderStage();

        typedef std::pair<const osg::Light*, osg::Matrix>   LightMatrix;
        typedef std::set< LightMatrix >                     LightMatrixSet;

        LightMatrixSet lms;

        unsigned int numLights = 0;

        osgUtil::PositionalStateContainer::AttrMatrixList& aml = rs->getPositionalStateContainer()->getAttrMatrixList();
        osgUtil::PositionalStateContainer::AttrMatrixList::iterator itr = aml.begin();
        for ( ; itr != aml.end(); )
        {
            if (numLights++ >= LightManager::instance()->getMaxNumOfLights()) break;

            const osg::Light* light = dynamic_cast<const osg::Light*>(itr->first.get());
            osg::RefMatrix *refMatrix = itr->second.get();

            if (light)
            {
                ++itr;
            }
            else
            {
                ++itr;
                continue;
            }

            LightMatrix lm(light,osg::Matrix(refMatrix?*refMatrix:osg::Matrix::identity()));
            lms.insert(lm);
        }

        LightMatrixSet::iterator litr = lms.begin();
        for ( ; litr != lms.end(); ++litr )
        {
            const DummyLight* light = dynamic_cast<const DummyLight*>(litr->first);
            osg::Matrix matrix = litr->second;

            if (!light) continue;

			bool enabled = false;
			light->getUserValue("enabled", enabled);

			if (light->getUserData())
			{
				DummyLight* nclight = const_cast<DummyLight*>(light);
				osg::LightSource* ls = dynamic_cast<osg::LightSource*>(nclight->getUserData());
				if (ls)
				{
					osg::NodePath np;
					np.push_back(ls);
					
					osg::ref_ptr<osg::Group> parent = ls->getNumParents() ? ls->getParent(0) : 0;
					while (parent)
					{
						np.insert(np.begin(), parent);
						parent = parent->getNumParents() ? parent->getParent(0) : 0;
					}

					osg::Matrixd wmx = osg::computeLocalToWorld(np);

					osg::Matrixd final = osg::Matrixd::translate(osg::Vec3(light->getPosition().x(), light->getPosition().y(), light->getPosition().z())) * wmx;

					osg::Vec3d lw = final.getTrans();

					double lod = 0.0;
					if (nclight->getUserValue("realLightLOD", lod) && lod > 0.0)
					{
						if ((cv->getEyePoint() - lw).length() > lod)
						{
							enabled = false;
						}						
					}

				}
			}

            osg::Vec4 litPos = light->getPosition() * matrix;			
            osg::Vec3 litDir = osg::Matrix::transform3x3(light->getDirection(), matrix);
            
            osg::ref_ptr<DummyLight> lightDesc = new DummyLight(light->getLightNum(),enabled);
            lightDesc->setPosition(litPos);
            lightDesc->setDirection(litDir);
            lightDesc->setDiffuse(light->getDiffuse());
            lightDesc->setSpecular(light->getSpecular());
            lightDesc->setConstantAttenuation(light->getConstantAttenuation());
            lightDesc->setSpotCutoff(light->getSpotCutoff());
            lightDesc->setAmbient(light->getAmbient());

            LightManager::instance()->addUpdateLight(lightDesc->getLightNum(),*lightDesc);
        }

        if (LightManager::instance()->getTextureRectangle())
        {
            static bool once = true;
            if (once)
            {
                once = false;

                int textureSlot = igcore::Configuration::instance()->getConfig("Lighting-Implementation-Texture-Slot",(int)5);

                _scene->getOrCreateStateSet()->setTextureAttributeAndModes(textureSlot,
                    LightManager::instance()->getTextureRectangle(),
                    osg::StateAttribute::ON);

                _scene->getOrCreateStateSet()->addUniform(
                    new osg::Uniform("maxNumOfLights",
                    (int)LightManager::instance()->getMaxNumOfLights()));

                _scene->getOrCreateStateSet()->addUniform(
                    new osg::Uniform("lightParamsSampler", textureSlot));
            }
        }

    }

protected:
    osg::observer_ptr<osg::Group>   _scene;
	igcore::ImageGenerator*			_ig;
};


class LightingPlugin : public igplugincore::Plugin
{
public:
    LightingPlugin()
        : _maxNumLights(0)
        , _cloudsShadowsTextureSlot(6)
		, _lightBrightness_enable(true)
		, _lightBrightness_day(1.f)
		, _lightBrightness_night(1.f)
		, _todHour(12)
    {

    }

    virtual std::string getName() { return "Lighting"; }

    virtual std::string getDescription() { return "Implements scene lighting, beyond the OpenGL 8 lights limit"; }

    virtual std::string getVersion() { return "1.0.0"; }

    virtual std::string getAuthor() { return "ComPro, Nick"; }

    virtual void config(const std::string& fileName)
    {
        _cloudsShadowsTextureSlot = igcore::Configuration::instance()->getConfig("Clouds-Shadows-Texture-Slot",6);

        osgDB::XmlNode* root = osgDB::readXmlFile(fileName);
        if (root == 0) return;

        if (root->children.size() == 0) return;

        osgDB::XmlNode* config = root->children.at(0);
        if (config->name != "OpenIG-Plugin-Config") return;

        osgDB::XmlNode::Children::iterator itr = config->children.begin();
        for ( ; itr != config->children.end(); ++itr)
        {
            osgDB::XmlNode* child = *itr;

            if (child->name == "Max-Num-Of-Lights")
            {
                _maxNumLights = atoi(child->contents.c_str());

                igcore::GlobalIdGenerator::instance()->initIdGroup("Real-Lights",8, _maxNumLights);

                LightManager::instance()->setMaxNumOfLights(_maxNumLights);
            }

            if (child->name == "Material")
            {
                _sceneMaterial = new osg::Material;

                osgDB::XmlNode::Children::iterator citr = child->children.begin();
                for ( ; citr != child->children.end(); ++citr)
                {
                    if ((**citr).name == "Ambient")
                    {
                        igcore::StringUtils::Tokens tokens = igcore::StringUtils::instance()->tokenize((**citr).contents);
                        if (tokens.size() == 4)
                        {
                            float r = atof(tokens.at(0).c_str());
                            float g = atof(tokens.at(1).c_str());
                            float b = atof(tokens.at(2).c_str());
                            float a = atof(tokens.at(3).c_str());

                            if (_sceneMaterial.valid()) _sceneMaterial->setAmbient(osg::Material::FRONT_AND_BACK,osg::Vec4(r,g,b,a));
                        }
                    }
                    if ((**citr).name == "Diffuse")
                    {
                        igcore::StringUtils::Tokens tokens = igcore::StringUtils::instance()->tokenize((**citr).contents);
                        if (tokens.size() == 4)
                        {
                            float r = atof(tokens.at(0).c_str());
                            float g = atof(tokens.at(1).c_str());
                            float b = atof(tokens.at(2).c_str());
                            float a = atof(tokens.at(3).c_str());

                            if (_sceneMaterial.valid()) _sceneMaterial->setDiffuse(osg::Material::FRONT_AND_BACK,osg::Vec4(r,g,b,a));
                        }
                    }
                    if ((**citr).name == "Specular")
                    {
                        igcore::StringUtils::Tokens tokens = igcore::StringUtils::instance()->tokenize((**citr).contents);
                        if (tokens.size() == 4)
                        {
                            float r = atof(tokens.at(0).c_str());
                            float g = atof(tokens.at(1).c_str());
                            float b = atof(tokens.at(2).c_str());
                            float a = atof(tokens.at(3).c_str());

                            if (_sceneMaterial.valid()) _sceneMaterial->setSpecular(osg::Material::FRONT_AND_BACK,osg::Vec4(r,g,b,a));
                        }
                    }
                    if ((**citr).name == "Shininess")
                    {
                        if (_sceneMaterial.valid()) _sceneMaterial->setShininess(osg::Material::FRONT_AND_BACK,atof((**citr).contents.c_str()));
                    }
                }
            }
        }
    }

    class EffectsCommand : public igcore::Commands::Command
    {
    public:
        EffectsCommand(igcore::ImageGenerator* ig)
            : _ig(ig)
        {

        }

        virtual const std::string getUsage() const
        {
            return "id effect on/off";
        }

        virtual const std::string getDescription() const
        {
            return  "turns on/off shader effects on an entity\n"
                    "     id - the id of the new entity across the scene\n"
                    "     effect - one of these\n"
                    "           envmapping\n"
                    "           aomapping\n"
                    "           lighting\n"
                    "           shadows\n"
                    "     on/off - turns the effect on or off";
        }

        virtual int exec(const igcore::StringUtils::Tokens& tokens)
        {
            if (tokens.size() == 3)
            {
                unsigned int    id = atoi(tokens.at(0).c_str());
                std::string     effect = tokens.at(1);
                bool            on = tokens.at(2) == "on";

                if (_ig->getEntityMap().count(id)==0) return -1;

                igcore::ImageGenerator::Entity& entity = _ig->getEntityMap()[id];
                if (!entity.valid()) return -1;

                std::map< std::string, std::string > e2d;

                e2d["envmapping"] = "ENVIRONMENTAL";
                e2d["aomapping"] = "AO";
                e2d["lighting"] = "SIMPLELIGHTING;LIGHTING";
                e2d["shadows"] = "SHADOWING";

                std::map< std::string, std::string >::iterator itr = e2d.find(effect);
                if ( itr == e2d.end() ) return -1;

                igcore::StringUtils::Tokens defines = igcore::StringUtils::instance()->tokenize(itr->second,";");
                for (size_t i = 0; i < defines.size(); ++i)
                {
#if OSG_VERSION_GREATER_OR_EQUAL(3,3,7)
                    switch (on)
                    {
                    case true:
                        entity->getOrCreateStateSet()->setDefine(defines.at(i),
                                osg::StateAttribute::ON|
                                osg::StateAttribute::PROTECTED|
                                osg::StateAttribute::OVERRIDE);
                        break;
                    case false:
                        entity->getOrCreateStateSet()->setDefine(defines.at(i),
                                osg::StateAttribute::OFF|
                                osg::StateAttribute::PROTECTED|
                                osg::StateAttribute::OVERRIDE);
                        break;
                    }
#endif
                }

                return 0;
            }
            return -1;
        }

    protected:
        igcore::ImageGenerator* _ig;
    };
	class UpdateTODBasedLightingUniformCallback : public osg::Uniform::Callback
	{
	public:
		UpdateTODBasedLightingUniformCallback(bool& enabled, float& onDay, float& onNight, unsigned int& tod)
			: _enabled(enabled)
			, _onDay(onDay)
			, _onNight(onNight)
			, _tod(tod)
		{

		}

		virtual void operator () (osg::Uniform* u, osg::NodeVisitor*)
		{
			if (_enabled)
			{
				float factor = _tod > 4 && _tod < 19 ? _onDay : _onNight;
				u->set(factor);
			}
			else
			{
				u->set(1.f);
			}
		}

	protected:
		bool&			_enabled;
		float&			_onDay;
		float&			_onNight;
		unsigned int&	_tod;
	};

	void updateFromXML(const std::string& fileName)
	{
		std::string xmlFile = fileName;
		if (xmlFile.empty())
		{
			xmlFile = _currentXMLFile;
		}

		if (!osgDB::fileExists(xmlFile))
		{
			osg::notify(osg::NOTICE) << "Lighting: xml file does not exists: " << xmlFile << std::endl;
			return;
		}

		osgDB::XmlNode* root = osgDB::readXmlFile(xmlFile);
		if (!root)
		{
			osg::notify(osg::NOTICE) << "Lighting: NULL root : " << xmlFile << std::endl;
			return;
		}
		if (!root->children.size())
		{
			osg::notify(osg::NOTICE) << "Lighting: root with no children: " << xmlFile << std::endl;
			return;
		}
		if (root->children.at(0)->name != "OsgNodeSettings")
		{
			osg::notify(osg::NOTICE) << "Lighting: OsgNodeSettings tag not found: " << xmlFile << std::endl;
			return;
		}

		osg::notify(osg::NOTICE) << "Lighting: current file: " << xmlFile << std::endl;

		_currentXMLFile = xmlFile;

		osgDB::XmlNode::Children::iterator itr = root->children.at(0)->children.begin();
		for (; itr != root->children.at(0)->children.end(); ++itr)
		{
			osgDB::XmlNode* child = *itr;

			//<LandingLightBrightness  enable="true" day="0.05" night="5"/>
			if (child->name == "LandingLightBrightness")
			{
				osgDB::XmlNode::Properties::iterator pitr = child->properties.begin();
				for (; pitr != child->properties.end(); ++pitr)
				{
					if (pitr->first == "enable")
					{
						_lightBrightness_enable = pitr->second == "true";
					}
					if (pitr->first == "day")
					{
						_lightBrightness_day = atof(pitr->second.c_str());
					}
					if (pitr->first == "night")
					{
						_lightBrightness_night = atof(pitr->second.c_str());
					}
				}
			}
		}
	}

	class UpdateFromXMLCommand : public igcore::Commands::Command
	{
	public:
		UpdateFromXMLCommand(LightingPlugin* plugin)
			: _plugin(plugin)
		{

		}

		virtual int exec(const igcore::StringUtils::Tokens& tokens)
		{
			if (tokens.size() == 1)
			{
				std::string command = tokens.at(0);
				if (command == "update")
				{
					_plugin->updateFromXML("");
				}
				return 0;
			}
			return -1;
		}

		virtual const std::string getUsage() const
		{
			return "command";
		}
		virtual const std::string getDescription() const
		{
			return  "updates the lighting factor based on XML definition\n"
				"        command - one of these: update";
		}

	protected:
		LightingPlugin*  _plugin;
	};


    virtual void init(igplugincore::PluginContext& context)
    {
		igcore::Commands::instance()->addCommand("lighting", new UpdateFromXMLCommand(this));

        _lightImplementationCallback = new ComplexLightImplementationCallback(context.getImageGenerator());
        context.getImageGenerator()->setLightImplementationCallback(_lightImplementationCallback);

        osg::Shader* mainVertexShader = new osg::Shader( osg::Shader::VERTEX,
            "#version 120                                                       \n"
            "#pragma import_defines(SIMPLELIGHTING,SHADOWING,ENVIRONMENTAL,USER)\n"
            "                                                                   \n"
            "#ifdef SIMPLELIGHTING                                              \n"
            "varying vec3 lightDirs[8];                                         \n"
            "#endif                                                             \n"
            "                                                                   \n"
            "varying vec3 normal;                                               \n"
            "varying vec3 eyeVec;                                               \n"
            "                                                                   \n"
            "#ifdef ENVIRONMENTAL                                               \n"
            "uniform mat4 osg_ViewMatrixInverse;                                \n"
            "uniform vec3 cameraPos;        									\n"
            "                                                                   \n"
            "mat3 getLinearPart( mat4 m )										\n"
            "{																	\n"
            "	mat3 result;													\n"
            "																	\n"
            "	result[0][0] = m[0][0];											\n"
            "	result[0][1] = m[0][1];											\n"
            "	result[0][2] = m[0][2];											\n"
            "																	\n"
            "	result[1][0] = m[1][0];											\n"
            "	result[1][1] = m[1][1];											\n"
            "	result[1][2] = m[1][2];											\n"
            "																	\n"
            "	result[2][0] = m[2][0];											\n"
            "	result[2][1] = m[2][1];											\n"
            "	result[2][2] = m[2][2];											\n"
            "																	\n"
            "	return result;													\n"
            "}																	\n"
            "                                                                   \n"
            "void environmentalMapping()										\n"
            "{																	\n"
            "	mat4 modelWorld4x4 = osg_ViewMatrixInverse * gl_ModelViewMatrix;\n"
            "																	\n"
            "	mat3 modelWorld3x3 = getLinearPart( modelWorld4x4 );			\n"
            "																	\n"
            "	vec4 worldPos = modelWorld4x4 *  gl_Vertex;						\n"
            "																	\n"
            "	vec3 n = normalize( modelWorld3x3 * gl_Normal );				\n"
            "																	\n"
            "	vec3 e = normalize( worldPos.xyz - cameraPos.xyz );				\n"
            "																	\n"
            "	gl_TexCoord[4].xyz = reflect( e, n );                           \n"
            "}																	\n"
            "#endif                                                             \n"
            "                                                                   \n"
            "#ifdef SHADOWING                                                   \n"
            "void dynamicShadow( in vec4 ecPosition );                          \n"
            "#endif                                                             \n"
            "                                                                   \n"
            "#ifdef USER                                                        \n"
            "userFunction();                                                    \n"
            "#endif                                                             \n"
            "                                                                   \n"
            "void main()                                                        \n"
            "{                                                                  \n"
            "   gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;         \n"
            "   gl_TexCoord[0] = gl_TextureMatrix[0] *gl_MultiTexCoord0;        \n"
            "                                                                   \n"
            "   eyeVec = -vec3(gl_ModelViewMatrix * gl_Vertex);                 \n"
            "   normal = normalize( gl_NormalMatrix * gl_Normal );              \n"
            "                                                                   \n"
            "#ifdef SIMPLELIGHTING                                              \n"
            "   vec3 vVertex = vec3(gl_ModelViewMatrix * gl_Vertex);            \n"
            "   lightDirs[0] = gl_LightSource[0].position.xyz;                  \n"
            "   lightDirs[1] = vec3(gl_LightSource[1].position.xyz - vVertex);  \n"
            "   lightDirs[2] = vec3(gl_LightSource[2].position.xyz - vVertex);  \n"
            "   lightDirs[3] = vec3(gl_LightSource[3].position.xyz - vVertex);  \n"
            "   lightDirs[4] = vec3(gl_LightSource[4].position.xyz - vVertex);  \n"
            "   lightDirs[5] = vec3(gl_LightSource[5].position.xyz - vVertex);  \n"
            "   lightDirs[6] = vec3(gl_LightSource[6].position.xyz - vVertex);  \n"
            "   lightDirs[7] = vec3(gl_LightSource[7].position.xyz - vVertex);  \n"
            "#endif                                                             \n"
            "                                                                   \n"
            "#ifdef SHADOWING                                                   \n"
            "   vec4  ecPos  = gl_ModelViewMatrix * gl_Vertex;                  \n"
            "   dynamicShadow( ecPos );                                         \n"
            "#endif                                                             \n"
            "                                                                   \n"
            "#ifdef ENVIRONMENTAL                                               \n"
            "   environmentalMapping();                                         \n"
            "#endif                                                             \n"
            "                                                                   \n"
            "#ifdef USER                                                        \n"
            "   userFunction();                                                 \n"
            "#endif                                                             \n"
            "                                                                   \n"
            "}                                                                  \n"
        );

        osg::Shader* mainFragmentShader = new osg::Shader( osg::Shader::FRAGMENT,
            "#version 120                                                           \n"
            "#pragma import_defines ( SIMPLELIGHTING, LIGHTING, SHADOWING, ENVIRONMENTAL, AO, USER, ENVIRONMENTAL_FACTOR )\n"
            "#extension GL_ARB_texture_rectangle : enable                           \n"
            "                                                                       \n"
            "uniform sampler2D baseTexture;                                         \n"
            "const float cos_outer_cone_angle = 0.4; // 36 degrees                  \n"
            "varying vec3 normal;                                                   \n"
            "varying vec3 eyeVec;                                                   \n"
			"																		\n"
			"uniform float	todBasedLightBrightness;								\n"
			"uniform bool	todBasedLightBrightnessEnabled;							\n"
            "                                                                       \n"
            "void computeFogColor(inout vec4 color)                                 \n"
            "{                                                                      \n"
            "    if (gl_FragCoord.w > 0.0)                                          \n"
            "    {                                                                  \n"
            "        const float LOG2 = 1.442695;									\n"
            "        float z = gl_FragCoord.z / gl_FragCoord.w;                     \n"
            "        float fogFactor = exp2( -gl_Fog.density *						\n"
            "            gl_Fog.density *											\n"
            "            z *														\n"
            "            z *														\n"
            "            LOG2 );													\n"
            "        fogFactor = clamp(fogFactor, 0.0, 1.0);                        \n"
            "                                                                       \n"
            "        vec4 clr = color;                                              \n"
            "        color = mix(gl_Fog.color, color, fogFactor );                  \n"
            "        color.a = clr.a;                                               \n"
            "    }                                                                  \n"
            "}                                                                      \n"
            "void computeAmbientColor(inout vec4 color)                             \n"
            "{                                                                      \n"
            "	vec4 final_color =                                                  \n"
            "   (gl_FrontLightModelProduct.sceneColor * gl_FrontMaterial.ambient) + \n"
            "	(gl_LightSource[0].ambient * gl_FrontMaterial.ambient);             \n"
            "                                                                       \n"
            "	vec3 N = normalize(normal);                                         \n"
            "	vec3 L = normalize(gl_LightSource[0].position.xyz);                 \n"
            "                                                                       \n"
            "	float lambertTerm = max(dot(N,L),0.0);                              \n"
            "                                                                       \n"
            "	//if(lambertTerm > 0.0)                                             \n"
            "	{                                                                   \n"
            "		final_color += gl_LightSource[0].diffuse *                      \n"
            "		               gl_FrontMaterial.diffuse *                       \n"
            "					   lambertTerm;                                     \n"
            "                                                                       \n"
            "		vec3 E = normalize(eyeVec);                                     \n"
            "		vec3 R = reflect(-L, N);                                        \n"
            "		float specular = pow( max(dot(R, E), 0.0),                      \n"
            "		                 gl_FrontMaterial.shininess );                  \n"
            "		final_color +=  gl_LightSource[0].specular *                    \n"
            "                               gl_FrontMaterial.specular *				\n"
            "					   specular;                                        \n"
            "	}                                                                   \n"
            "                                                                       \n"
            "	color += final_color;                                               \n"
            "}                                                                      \n"
            "                                                                       \n"
            "#ifdef SIMPLELIGHTING                                                  \n"
            "varying vec3 lightDirs[8];                                                  \n"
            "uniform bool lightsEnabled[8];                                         \n"
            "                                                                       \n"
            "void computeColorForLightSourceSimple(int i, inout vec4 color)			\n"
            "{                                                                      \n"
            "   if (!lightsEnabled[i]) return;                                      \n"
            "                                                                       \n"
            "       vec4 final_color = vec4(0.0,0.0,0.0,1.0);                       \n"
            "                                                                       \n"
            "		float distSqr = dot(lightDirs[i],lightDirs[i]);                 \n"
            "		float invRadius = gl_LightSource[i].constantAttenuation;		\n"
            "		float att = clamp(1.0-invRadius * sqrt(distSqr), 0.0, 1.0);		\n"
            "		vec3 L = lightDirs[i] * inversesqrt(distSqr);                   \n"
            "		vec3 D = normalize(gl_LightSource[i].spotDirection);			\n"
            "                                                                       \n"
            "		float cos_cur_angle = dot(-L, D);                               \n"
            "                                                                       \n"
            "		float cos_inner_cone_angle = gl_LightSource[i].spotCosCutoff;	\n"
            "                                                                       \n"
            "		float cos_inner_minus_outer_angle =                             \n"
            "			cos_inner_cone_angle - cos_outer_cone_angle;                \n"
            "                                                                       \n"
            "		float spot = 0.0;                                               \n"
            "		spot = clamp((cos_cur_angle - cos_outer_cone_angle) /			\n"
            "			cos_inner_minus_outer_angle, 0.0, 1.0);                     \n"
            "                                                                       \n"
            "		vec3 N = normalize(normal);                                     \n"
            "                                                                       \n"
            "		float lambertTerm = max( dot(N,L), 0.0);                        \n"
            "		if(lambertTerm > 0.0)                                           \n"
            "		{                                                               \n"
			"			float todBasedFactor = todBasedLightBrightnessEnabled ?		\n"
			"				todBasedLightBrightness : 1.0;							\n"
            "			final_color += gl_LightSource[i].diffuse * todBasedFactor *	\n"
            "				gl_FrontMaterial.diffuse *                              \n"
            "				lambertTerm * spot * att;                               \n"
            "                                                                       \n"
            "			vec3 E = normalize(eyeVec);                                 \n"
            "			vec3 R = reflect(-L, N);                                    \n"
            "                                                                       \n"
            "			float specular = pow( max(dot(R, E), 0.0),                  \n"
            "				gl_FrontMaterial.shininess );                           \n"
            "                                                                       \n"
            "			final_color += gl_LightSource[i].specular *                 \n"
            "				gl_FrontMaterial.specular *                             \n"
            "				specular * spot * att;                                  \n"
            "		}                                                               \n"
            "                                                                       \n"
            "                                                                       \n"
            "		color += final_color;                                           \n"
            "}                                                                      \n"
            "                                                                       \n"
            "void simpleLighting( inout vec4 color )                                \n"
            "{                                                                      \n"
            "	for (int i = 1; i < 8; i++)                                         \n"
            "	{                                                                   \n"
            "		computeColorForLightSourceSimple(i,color);                      \n"
            "	}                                                                   \n"
            "}                                                                      \n"
            "#endif                                                                 \n"
            "                                                                       \n"
            "#ifdef LIGHTING                                                        \n"
            "                                                                       \n"
            "#define LIGHT_DIFFUSE                               0                  \n"
            "#define LIGHT_SPECULAR                              1                  \n"
            "#define LIGHT_POSITION                              2                  \n"
            "#define LIGHT_DIRECTION                             3                  \n"
            "#define LIGHT_CONST_ATTENUATION_SPOT_CUTOFF_ENABLED 4                  \n"
            "                                                                       \n"
            "vec4 lightAmbient;                                                     \n"
            "vec4 lightDiffuse;                                                     \n"
            "vec4 lightSpecular;                                                    \n"
            "vec4 lightPosition;                                                    \n"
            "vec4 lightDirection;                                                   \n"
            "vec4 lightAttenuationSpotCutoffEnabled;                                \n"
            "                                                                       \n"            
            "uniform sampler2DRect  lightParamsSampler;                             \n"
            "uniform int            maxNumOfLights;                                 \n"
            "                                                                       \n"
            "#endif                                                                 \n"
            "                                                                       \n"
            "#ifdef SHADOWING                                                       \n"
            "uniform float shadowsFactor;                                           \n"
            "float dynamicShadow();                                                 \n"
            "#endif                                                                 \n"
            "                                                                       \n"
            "#ifdef AO                                                              \n"
            "uniform float ambientOcclusionFactor;                                  \n"
            "uniform sampler2D ambientOcclusionTexture;                             \n"
            "#endif                                                                 \n"
            "                                                                       \n"
            "#ifdef ENVIRONMENTAL                                                   \n"
            "uniform float environmentalFactor;                                     \n"
            "uniform samplerCube environmentalMapTexture;                           \n"
            "#endif                                                                 \n"
            "                                                                       \n"
            "#ifdef USER                                                            \n"
            "userFunction( in vec4 colorIn, out vec4 colorOut);                     \n"
            "#endif                                                                 \n"
            "                                                                       \n"
            "#ifdef LIGHTING                                                        \n"
            "bool setupLight( in int lightId )                                      \n"
            "{                                                                      \n"
            "   lightAmbient = vec4(0.2,0.2,0.2,1.0);                               \n"
            "   lightDiffuse = texture2DRect(lightParamsSampler, vec2( LIGHT_DIFFUSE, lightId) );          \n"
            "   lightSpecular = texture2DRect(lightParamsSampler, vec2( LIGHT_SPECULAR, lightId) );        \n"
            "   lightPosition = texture2DRect(lightParamsSampler, vec2( LIGHT_POSITION, lightId) );        \n"
            "   lightDirection = texture2DRect(lightParamsSampler, vec2( LIGHT_DIRECTION, lightId) );      \n"
            "   lightAttenuationSpotCutoffEnabled = texture2DRect(lightParamsSampler, vec2( LIGHT_CONST_ATTENUATION_SPOT_CUTOFF_ENABLED, lightId) );      \n"
            "                                                                       \n"
            "   return true;                                                        \n"
            "}                                                                      \n"
            "                                                                       \n"            
            "void computeColorForLightSource(int i, inout vec4 color)				\n"
            "{                                                                      \n"
            "       lightAttenuationSpotCutoffEnabled = texture2DRect(lightParamsSampler, vec2( LIGHT_CONST_ATTENUATION_SPOT_CUTOFF_ENABLED, i));\n"
            "       if (lightAttenuationSpotCutoffEnabled.z < 1.0) return;          \n"
            "                                                                       \n"
            "       setupLight(i);                                                  \n"
            "                                                                       \n"
            "       vec4 final_color = vec4(0.0);                                   \n"
            "       vec3 lightDir = lightPosition.xyz+eyeVec;                       \n"
            "		float distSqr = dot(lightDir,lightDir);                         \n"
            "		float invRadius = lightAttenuationSpotCutoffEnabled.x;          \n"
            "		float att = clamp(1.0-invRadius * sqrt(distSqr), 0.0, 1.0);		\n"
            "       vec3 L = lightDir.xyz * inversesqrt(distSqr);                   \n"
            "		vec3 D = normalize(lightDirection.xyz);                         \n"
            "                                                                       \n"
            "		float cos_cur_angle = dot(-L, D);                               \n"
            "                                                                       \n"
            "		float cos_inner_cone_angle =lightAttenuationSpotCutoffEnabled.y;\n"
            "                                                                       \n"
            "		float cos_inner_minus_outer_angle =                             \n"
            "			cos_inner_cone_angle - cos_outer_cone_angle;                \n"
            "                                                                       \n"
            "		float spot = 0.0;                                               \n"
            "		spot = clamp((cos_cur_angle - cos_outer_cone_angle) /			\n"
            "			cos_inner_minus_outer_angle, 0.0, 1.0);                     \n"
            "                                                                       \n"
            "		vec3 N = normalize(normal);                                     \n"
            "                                                                       \n"
            "		float lambertTerm = max( dot(N,L), 0.0);                        \n"            
            "                                                                       \n"
            "       if (lambertTerm > 0.0)                                          \n"
            "       {                                                               \n"
			"			float todBasedFactor = todBasedLightBrightnessEnabled ?		\n"
			"				todBasedLightBrightness : 1.0;							\n"
            "           final_color += lightDiffuse * gl_FrontMaterial.diffuse *	\n"
			"				todBasedFactor * lambertTerm * spot * att;				\n"
            "                                                                       \n"
            "           vec3 E = normalize(eyeVec);                                 \n"
            "           vec3 R = reflect(-L,N);                                     \n"
            "           float specular = pow( max(dot(R,E), 0.0), gl_FrontMaterial.shininess);\n"
            "                                                                       \n"
            "           final_color += lightSpecular * gl_FrontMaterial.specular * specular;\n"
            "       }                                                               \n"
            "                                                                       \n"
            "       color += final_color;                                           \n"
            "                                                                       \n"
            "}                                                                      \n"
            "                                                                       \n"
            "void lighting( inout vec4 color )                                      \n"
            "{                                                                      \n"
            "	vec4 clr = vec4(0.0);                                               \n"
            "                                                                       \n"
            "	for (int i = 8; i < maxNumOfLights; i++)                            \n"
            "	{                                                                   \n"
            "		computeColorForLightSource(i,clr);                              \n"
            "	}                                                                   \n"                        
            "   color = clr;                                                        \n"
            "}                                                                      \n"
            "#endif                                                                 \n"
            "#ifdef AO                                                              \n"
            "void computeAmbientOcclusion( inout vec4 color )                       \n"
            "{                                                                      \n"
            "   vec4 aocolor = texture2D(ambientOcclusionTexture,gl_TexCoord[0].xy);\n"
            "   aocolor.rgb *= ambientOcclusionFactor;                              \n"
            "   color.rgb *= aocolor.rgb;                                           \n"
            "}                                                                      \n"
            "#endif                                                                 \n"
            "#ifdef ENVIRONMENTAL                                                   \n"
            "void computeEnvironmentalMap( inout vec4 color )                       \n"
            "{																		\n"
            "   vec3 v = gl_TexCoord[4].xzy;                                        \n"
            "   v.y *= -1.0;                                                        \n"
            "	vec3 cube_color =													\n"
            "		textureCube(environmentalMapTexture, v).rgb;                    \n"
            "																		\n"
            "   vec3 mixed_color =                                                  \n"
            "       mix(cube_color, color.rgb, 1.0-ENVIRONMENTAL_FACTOR).rgb;       \n"
            "	color.rgb *= mixed_color;                                           \n"
            "}																		\n"
            "#endif                                                                 \n"
            "void main()                                                            \n"
            "{                                                                      \n"
            "   vec4 color = texture2D( baseTexture, gl_TexCoord[0].xy );           \n"
            "#ifdef SHADOWING                                                       \n"
            "   float shadow = dynamicShadow();                                     \n"
            "   color.rgb = mix( color.rgb*(1.0-shadowsFactor),color.rgb,shadow );  \n"
            "#endif                                                                 \n"
            "   vec4 clr = vec4(0.0,0.0,0.0,1.0);                                   \n"
            "#ifdef LIGHTING                                                        \n"
            "   lighting(clr);                                                      \n"
            "#endif                                                                 \n"
            "#ifdef SIMPLELIGHTING                                                  \n"
            "   simpleLighting(clr);                                                \n"
            "#endif                                                                 \n"
            "#if defined(LIGHTING) || defined(SIMPLELIGHTING)                       \n"
            "   computeAmbientColor(clr);                                           \n"
            "   color *= clr;                                                       \n"
            "#endif                                                                 \n"
            "#ifdef ENVIRONMENTAL                                                   \n"
            "   computeEnvironmentalMap(color);                                     \n"
            "#endif                                                                 \n"
            "#ifdef AO                                                              \n"
            "   computeAmbientOcclusion(color);                                     \n"
            "#endif                                                                 \n"
            "#ifdef USER                                                            \n"
            "   userFunction(color,color);                                          \n"
            "#endif                                                                 \n"
            "	computeFogColor(color);                                             \n"
            "   gl_FragColor = color;                                               \n"
            "}                                                                      \n"
        );

        std::ostringstream ossvs;

        ossvs << "#define SELF_SHADOW_STAGE 1                                                                \n";
        ossvs << "#define CLOUDS_SHADOW_STAGE " << _cloudsShadowsTextureSlot << "                            \n";
        ossvs << "uniform mat4 cloudShadowCoordMatrix;                                                       \n";
        ossvs << "void dynamicShadow( in vec4 ecPosition )                                                   \n";
        ossvs << "{                                                                                          \n";
        ossvs << "    // generate coords for shadow mapping                                                  \n";
        ossvs << "    gl_TexCoord[SELF_SHADOW_STAGE].s = dot( ecPosition, gl_EyePlaneS[SELF_SHADOW_STAGE] ); \n";
        ossvs << "    gl_TexCoord[SELF_SHADOW_STAGE].t = dot( ecPosition, gl_EyePlaneT[SELF_SHADOW_STAGE] ); \n";
        ossvs << "    gl_TexCoord[SELF_SHADOW_STAGE].p = dot( ecPosition, gl_EyePlaneR[SELF_SHADOW_STAGE] ); \n";
        ossvs << "    gl_TexCoord[SELF_SHADOW_STAGE].q = dot( ecPosition, gl_EyePlaneQ[SELF_SHADOW_STAGE] ); \n";
        ossvs << "    gl_TexCoord[CLOUDS_SHADOW_STAGE] = cloudShadowCoordMatrix * ecPosition;                \n";
        ossvs << "}                                                                                          \n";

        osg::Shader* shadowVertexShader = new osg::Shader( osg::Shader::VERTEX, ossvs.str());

        std::ostringstream ossfs;

        ossfs << "#define SELF_SHADOW_STAGE  1                                                                   \n";
        ossfs << "#define CLOUDS_SHADOW_STAGE " << _cloudsShadowsTextureSlot << "                                \n";
        ossfs << "uniform sampler2DShadow    shadowTexture;                                                      \n";
        ossfs << "uniform sampler2D          cloudShadowTexture;                                                 \n";
        ossfs << "float dynamicShadow()                                                                          \n";
        ossfs << "{                                                                                              \n";
        ossfs << "   float selfShadow = shadow2DProj( shadowTexture, gl_TexCoord[SELF_SHADOW_STAGE] ).r;         \n";
        ossfs << "   float cloudsShadow = texture2D( cloudShadowTexture, gl_TexCoord[CLOUDS_SHADOW_STAGE].xy ).r;\n";
        ossfs << "   return selfShadow * cloudsShadow;                                                           \n";
        ossfs << "}                                                                                              \n";

        osg::Shader* shadowFragmentShader = new osg::Shader( osg::Shader::FRAGMENT,ossfs.str());

        osgShadow::ShadowedScene* scene = dynamic_cast<osgShadow::ShadowedScene*>(
            context.getImageGenerator()->getScene()
        );
        if (scene != 0)
        {
            osgShadow::MinimalShadowMap* msm = dynamic_cast<osgShadow::MinimalShadowMap*>(
                scene->getShadowTechnique()
            );
            if (msm != 0)
            {
                msm->setMainVertexShader(mainVertexShader);
                msm->setMainFragmentShader(mainFragmentShader);
                msm->setShadowVertexShader(shadowVertexShader);
                msm->setShadowFragmentShader(shadowFragmentShader);

                mainVertexShader->setName("Lighting: main vertex shader");
                mainFragmentShader->setName("Lighting: main fragment shader");
                shadowVertexShader->setName("Lighting: shadow vertex shader");
                shadowFragmentShader->setName("Lighting: shadow fragment shader");

                osg::StateSet* ss = context.getImageGenerator()->getViewer()->getView(0)->getSceneData()->getOrCreateStateSet();

#if OSG_VERSION_GREATER_OR_EQUAL(3,3,7)
                ss->setDefine("SIMPLELIGHTING");
#if 0
                // This is realy processing intensive and it is
                // meant for small area lighting, not a whole
                // terrain, like models, runway etc
                ss->setDefine("LIGHTING");
#endif
                ss->setDefine("SHADOWING");
                ss->setDefine("ENVIRONMENTAL_FACTOR","0");
#else
                osg::notify(osg::NOTICE) << "NOTE: Plugin Lighting built with version prior to 3.3.7." << std::endl;
                osg::notify(osg::NOTICE) << "  The shader composition will not have effect thus the" << std::endl;
                osg::notify(osg::NOTICE) << "  special rendering effects will not take place." << std::endl;
#endif
                if (!_sceneMaterial.valid())
                {
                    osg::Material* sceneMaterial = new osg::Material;
                    sceneMaterial->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(0.7,0.7,0.7,1.0));
                    sceneMaterial->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(0.6,0.6,0.6,1.0));
                    sceneMaterial->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0.6,0.6,0.6,1.0));
                    sceneMaterial->setShininess(osg::Material::FRONT_AND_BACK, 60);

                    ss->setAttributeAndModes(sceneMaterial, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
                }
                else
                {
                    ss->setAttributeAndModes(_sceneMaterial, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
                }

                osg::Uniform* u = new osg::Uniform(osg::Uniform::BOOL,"lightsEnabled", 8);
                osg::Uniform::Callback* cb = new UpdateLightsEnabledUniformCallback(context.getImageGenerator());
                u->setUpdateCallback(cb);
                ss->addUniform(u);

                float shadowsFactor = igcore::Configuration::instance()->getConfig("Shadows-Factor",0.5);
                ss->addUniform(new osg::Uniform("shadowsFactor",shadowsFactor));

                unsigned int defaultDiffuseSlot = igcore::Configuration::instance()->getConfig("Default-diffuse-texture-slot",0);
                ss->addUniform(new osg::Uniform("baseTexture",(int)defaultDiffuseSlot),osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE);

                osg::Uniform* cu = new osg::Uniform("cameraPos",osg::Vec3d());
                cu->setUpdateCallback(new UpdateCameraPosUniformCallback(context.getImageGenerator()));
                ss->addUniform(cu);

				osg::Uniform* todBasedLightingUniform = new osg::Uniform("todBasedLightBrightness", (float)1.f);
				todBasedLightingUniform->setUpdateCallback(new UpdateTODBasedLightingUniformCallback(
					_lightBrightness_enable, _lightBrightness_day, _lightBrightness_night, _todHour)
					);
				ss->addUniform(todBasedLightingUniform);

				osg::Uniform* todBasedLightingEnabledUniform = new osg::Uniform("todBasedLightBrightnessEnabled", (bool)true);
				ss->addUniform(todBasedLightingEnabledUniform);
            }
        }

        igcore::Commands::instance()->addCommand("effects", new EffectsCommand(context.getImageGenerator()));
    }


    virtual void update(igplugincore::PluginContext& context)
    {
        LightManager::instance()->updateTextureObject();

		osg::ref_ptr<osg::Referenced> ref = context.getAttribute("TOD");
		igplugincore::PluginContext::Attribute<igcore::TimeOfDayAttributes> *attr = dynamic_cast<igplugincore::PluginContext::Attribute<igcore::TimeOfDayAttributes> *>(ref.get());
		if (attr)
		{
			_todHour = attr->getValue().getHour();
		}
    }	

	virtual void databaseRead(const std::string& fileName, osg::Node*, const osgDB::Options*)
	{
		std::string xmlFile = fileName + ".lighting.xml";
		updateFromXML(xmlFile);		
	}


    virtual void clean(igplugincore::PluginContext& context)
    {
        context.getImageGenerator()->setLightImplementationCallback(0);
    }

protected:

    class UpdateLightsEnabledUniformCallback : public osg::Uniform::Callback
    {
    public:
        UpdateLightsEnabledUniformCallback(igcore::ImageGenerator* ig)
            : _ig(ig)
        {

        }

        virtual void operator () (osg::Uniform* u, osg::NodeVisitor* )
        {
            for (size_t i = 0; i<8; ++i)
            {
                u->setElement(i,_ig->isLightEnabled(i));
            }
        }

    protected:
        igcore::ImageGenerator*     _ig;
    };

    class UpdateCameraPosUniformCallback : public osg::Uniform::Callback
    {
    public:
        UpdateCameraPosUniformCallback(igcore::ImageGenerator* ig)
            : _ig(ig)
        {

        }

        virtual void operator () (osg::Uniform* u, osg::NodeVisitor* )
        {
            osg::Vec3d eye;
            osg::Vec3d center;
            osg::Vec3d up;

            _ig->getViewer()->getView(0)->getCamera()->getViewMatrixAsLookAt(eye,center,up);

            u->set(eye);
        }

    protected:
        igcore::ImageGenerator*     _ig;
    };

    class ComplexLightImplementationCallback : public igcore::LightImplementationCallback
    {
    public:
        ComplexLightImplementationCallback(igcore::ImageGenerator* ig)
            : _ig(ig)
        {

        }

        virtual osg::Referenced* createLight(
                unsigned int id,
                const igcore::LightAttributes& definition,
                osg::Group* lightsGroup)
        {
            osg::LightSource* light = new osg::LightSource;
            light->setLight(new DummyLight(id,true));
            light->getLight()->setAmbient(definition._ambient);
            light->getLight()->setDiffuse(definition._diffuse*definition._brightness);
            light->getLight()->setSpecular(definition._specular);
            light->getLight()->setConstantAttenuation(1.f/definition._constantAttenuation);
            light->getLight()->setSpotCutoff(definition._spotCutoff);
            light->getLight()->setPosition(osg::Vec4(0,0,0,1));
            light->getLight()->setDirection(osg::Vec3(0,1,0));
			light->getLight()->setUserData(light);

            if (id >=1 && id < 8)
            {
                light->setStateSetModes(*_ig->getViewer()->getView(0)->getSceneData()->getOrCreateStateSet(),osg::StateAttribute::ON);
            }
            else
            {
                osg::ref_ptr<osg::StateAttribute> attr = light->getOrCreateStateSet()->getAttribute(osg::StateAttribute::LIGHT);
                if (attr.valid())
                {
                    light->getOrCreateStateSet()->removeAttribute(attr);
                    osg::notify(osg::NOTICE) << "Light attr removed" << std::endl;
                }
            }

            _lights[id] = light;
            _lightsGroup = lightsGroup;

            if (!_dummyGroup.valid() && _lightsGroup.valid())
            {
                _dummyGroup = new osg::Group;
                _lightsGroup->addChild(_dummyGroup);

                _dummyGroup->setCullCallback(new UpdateLightAttribscallback(_ig,_ig->getScene()->asGroup()));
            }

            return light;
        }

        virtual void updateLight(unsigned int id, const igcore::LightAttributes& definition)
        {
            LightsMapIterator itr = _lights.find(id);
            if ( itr != _lights.end())
            {
                osg::LightSource* light = itr->second;

                if (definition._dirtyMask & igcore::LightAttributes::AMBIENT)
                    light->getLight()->setAmbient(definition._ambient);

                if (definition._dirtyMask & igcore::LightAttributes::DIFFUSE && definition._dirtyMask & igcore::LightAttributes::BRIGHTNESS)
                    light->getLight()->setDiffuse(definition._diffuse*definition._brightness);

                if (definition._dirtyMask & igcore::LightAttributes::SPECULAR)
                    light->getLight()->setSpecular(definition._specular);

                if (definition._dirtyMask & igcore::LightAttributes::CONSTANTATTENUATION)
                    light->getLight()->setConstantAttenuation(1.f/definition._constantAttenuation);

                if (definition._dirtyMask & igcore::LightAttributes::SPOTCUTOFF)
                    light->getLight()->setSpotCutoff(definition._spotCutoff);

				if (definition._dirtyMask & igcore::LightAttributes::REALLIGHTLOD)
				{
					light->getLight()->setUserValue("realLightLOD", (double)definition._realLightLOD);
				}

                if (definition._dirtyMask & igcore::LightAttributes::ENABLED)
                {
                    osg::ref_ptr<DummyLight> light = new DummyLight(id,definition._enabled);
                    LightManager::instance()->addUpdateLight(id,*light);
                }

            }
        }

    protected:
        igcore::ImageGenerator*         _ig;
        osg::observer_ptr<osg::Group>   _lightsGroup;
        osg::ref_ptr<osg::Group>        _dummyGroup;

        typedef std::map<unsigned int, osg::ref_ptr<osg::LightSource> >                 LightsMap;
        typedef std::map<unsigned int, osg::ref_ptr<osg::LightSource> >::iterator       LightsMapIterator;

        LightsMap                       _lights;

    };

    osg::ref_ptr<igcore::LightImplementationCallback>       _lightImplementationCallback;
    unsigned int                                            _maxNumLights;
    unsigned int                                            _cloudsShadowsTextureSlot;
    osg::ref_ptr<osg::Material>                             _sceneMaterial;
	bool													_lightBrightness_enable;
	float													_lightBrightness_day;
	float													_lightBrightness_night;
	unsigned int											_todHour;
	std::string												_currentXMLFile;
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
    return new igplugins::LightingPlugin;
}

extern "C" EXPORT void DeletePlugin(igplugincore::Plugin* plugin)
{
    osg::ref_ptr<igplugincore::Plugin> p(plugin);
}
