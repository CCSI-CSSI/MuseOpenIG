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
#include <IgCore/attributes.h>
#include <IgCore/stringutils.h>

#include <IgPluginCore/plugin.h>
#include <IgPluginCore/plugincontext.h>

#include <osgDB/ReadFile>
#include <osgDB/XmlParser>
#include <osgDB/FileUtils>

#include <osg/Node>
#include <osg/Material>

namespace igplugins
{

class TimeOfDayBasedMaterialPlugin : public igplugincore::Plugin
{
public:

    TimeOfDayBasedMaterialPlugin(){}

    virtual std::string getName() { return "TimeOfDayBasedMaterial"; }

    virtual std::string getDescription( ) { return "Tunes the material based on time of day. Applied on a model based on xml"; }

    virtual std::string getVersion() { return "1.0.0"; }

    virtual std::string getAuthor() { return "ComPro, Nick"; }

    virtual void update(igplugincore::PluginContext& context)
    {
        osg::ref_ptr<osg::Referenced> ref = context.getAttribute("TOD");
        igplugincore::PluginContext::Attribute<igcore::TimeOfDayAttributes> *attr = dynamic_cast<igplugincore::PluginContext::Attribute<igcore::TimeOfDayAttributes> *>(ref.get());
        if (attr && _runtimeMaterial.valid())
        {
            float hour = attr->getValue().getHour();
            if ( hour >= 0.0 && hour <= 12.0)
            {
                float t = hour / 12;

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
            if ( hour >= 12.0 && hour <= 24.0)
            {
                float t = (hour-12.f) / 12;

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

    virtual void entityAdded(igplugincore::PluginContext&, unsigned int, osg::Node& node, const std::string& fileName)
    {
        if (!osgDB::fileExists(fileName+".xml")) return;

        osgDB::XmlNode* root = osgDB::readXmlFile(fileName+".xml");
        if (!root) return;
        if (!root->children.size()) return;
        if (root->children.at(0)->name != "OpenIg-Model-Definition")  return;

        osgDB::XmlNode::Children::iterator itr = root->children.at(0)->children.begin();
        for ( ; itr != root->children.at(0)->children.end(); ++itr )
        {
            if ((**itr).name == "Material")
            {
                osgDB::XmlNode* materialNode = *itr;
                osgDB::XmlNode::Children::iterator citr = materialNode->children.begin();

                osg::ref_ptr<osg::Material> material;

                for ( ; citr != materialNode->children.end(); ++citr )
                {
                    if ((**citr).name == "Name")
                    {
                        if ((**citr).contents == "Day")
                            _dayMaterial = material = new osg::Material;
                        else
                        if ((**citr).contents == "Night")
                            _nightMaterial = material = new osg::Material;
                    }
                    if ((**citr).name == "Ambient")
                    {
                        if (material.valid())
                        {
                            igcore::StringUtils::Tokens tokens = igcore::StringUtils::instance()->tokenize((**citr).contents);
                            if (tokens.size() == 4)
                            {
                                float r = atof(tokens.at(0).c_str());
                                float g = atof(tokens.at(1).c_str());
                                float b = atof(tokens.at(2).c_str());
                                float a = atof(tokens.at(3).c_str());

                                material->setAmbient(osg::Material::FRONT_AND_BACK,osg::Vec4(r,g,b,a));
                            }
                        }
                    }
                    if ((**citr).name == "Diffuse")
                    {
                        if (material.valid())
                        {
                            igcore::StringUtils::Tokens tokens = igcore::StringUtils::instance()->tokenize((**citr).contents);
                            if (tokens.size() == 4)
                            {
                                float r = atof(tokens.at(0).c_str());
                                float g = atof(tokens.at(1).c_str());
                                float b = atof(tokens.at(2).c_str());
                                float a = atof(tokens.at(3).c_str());

                                material->setDiffuse(osg::Material::FRONT_AND_BACK,osg::Vec4(r,g,b,a));
                            }
                        }
                    }
                    if ((**citr).name == "Specular")
                    {
                        if (material.valid())
                        {
                            igcore::StringUtils::Tokens tokens = igcore::StringUtils::instance()->tokenize((**citr).contents);
                            if (tokens.size() == 4)
                            {
                                float r = atof(tokens.at(0).c_str());
                                float g = atof(tokens.at(1).c_str());
                                float b = atof(tokens.at(2).c_str());
                                float a = atof(tokens.at(3).c_str());

                                material->setSpecular(osg::Material::FRONT_AND_BACK,osg::Vec4(r,g,b,a));
                            }
                        }
                    }
                    if ((**citr).name == "Shininess")
                    {
                        if (material.valid())
                        {
                            float shininess = atof((**citr).contents.c_str());

                            material->setShininess(osg::Material::FRONT_AND_BACK,shininess);
                        }
                    }
                }
            }
        }

        if (_dayMaterial.valid() && _nightMaterial.valid())
        {
            _runtimeMaterial = new osg::Material(*_dayMaterial);

            node.getOrCreateStateSet()->setAttributeAndModes(_runtimeMaterial,osg::StateAttribute::ON|osg::StateAttribute::PROTECTED|osg::StateAttribute::OVERRIDE);
        }

    }
protected:
    osg::ref_ptr<osg::Material>         _dayMaterial;
    osg::ref_ptr<osg::Material>         _nightMaterial;
    osg::ref_ptr<osg::Material>         _runtimeMaterial;

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
    return new igplugins::TimeOfDayBasedMaterialPlugin;
}

extern "C" EXPORT void DeletePlugin(igplugincore::Plugin* plugin)
{
    osg::ref_ptr<igplugincore::Plugin> p(plugin);
}
