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
#include <IgCore/attributes.h>
#include <IgCore/mathematics.h>
#include <IgCore/commands.h>

#include <iostream>
#include <sstream>

#include <osg/ref_ptr>
#include <osg/FrontFace>
#include <osg/Material>

#include <osgDB/XmlParser>
#include <osgDB/ReadFile>

#include <osgViewer/CompositeViewer>

#include <osgParticle/PrecipitationEffect>

namespace igplugins
{

class SkyDomePlugin : public igplugincore::Plugin
{
public:

    SkyDomePlugin() {}

    virtual std::string getName() { return "SkyDome"; }

    virtual std::string getDescription( ) { return "Simple skydome implementation"; }

    virtual std::string getVersion() { return "1.0.0"; }

    virtual std::string getAuthor() { return "ComPro, Nick"; }

    virtual void config(const std::string& fileName)
    {
        osgDB::XmlNode* root = osgDB::readXmlFile(fileName);
        if (root == 0)
        {
            osg::notify(osg::NOTICE) << "SkyDome: failed to read XML file: " << fileName << std::endl;
            return;
        }

        if (root->children.size() == 0)
        {
            osg::notify(osg::NOTICE) << "SkyDome: empty XML file: " << fileName << std::endl;
            return;
        }

        osgDB::XmlNode* config = root->children.at(0);
        if (config->name != "OpenIG-Plugin-Config")
        {
            osg::notify(osg::NOTICE) << "SkyDome: <OpenIG-Plugin-Config> tag missing in " << fileName << std::endl;
            return;
        }

        osgDB::XmlNode::Children::iterator itr = config->children.begin();
        for ( ; itr != config->children.end(); ++itr)
        {
            osgDB::XmlNode* child = *itr;
            if (child->name == "SkyDome-Model")
            {
                _modelFileName = child->contents;
            }
        }
    }

protected:
    std::string     _modelFileName;


    virtual void init(igplugincore::PluginContext& context)
    {

        igcore::Commands::instance()->addCommand("fog",  new SetFogCommand(context.getImageGenerator()));
        igcore::Commands::instance()->addCommand("rain", new RainCommand(context.getImageGenerator()));
        igcore::Commands::instance()->addCommand("snow", new SnowCommand(context.getImageGenerator()));
#if 1
        osg::ref_ptr<osgViewer::CompositeViewer> viewer = context.getImageGenerator()->getViewer();
        if (!viewer.valid()) return;
        if (!viewer->getNumViews()) return;
        if (!viewer->getView(0)->getSceneData()) return;


        viewer->getView(0)->getSceneData()->asGroup()->addChild(createSkyDome(context.getImageGenerator()));
#else
        context.getImageGenerator()->getScene()->asGroup()->addChild(createSkyDome(context.getImageGenerator()));
#endif
    }

    virtual void update(igplugincore::PluginContext& context)
    {
        osgViewer::CompositeViewer* viewer = context.getImageGenerator()->getViewer();
        if (!viewer) return;
        if (viewer->getNumViews()==0) return;

        osg::ref_ptr<osg::Camera> camera = viewer->getView(0)->getCamera();

        osg::Vec3d eye = camera->getInverseViewMatrix().getTrans();
        double radius = osg::minimum((double)context.getImageGenerator()->getScene()->getBound().radius(),92600.0);
        radius = osg::maximum(radius,1.0);
        osg::Vec3d scale(radius,radius,radius);

        if (_sky.valid())
        {
            _sky->setEye(eye);
            _sky->setScale(scale);

            {
                osg::ref_ptr<osg::Referenced> ref = context.getAttribute("TOD");
                igplugincore::PluginContext::Attribute<igcore::TimeOfDayAttributes> *attr = dynamic_cast<igplugincore::PluginContext::Attribute<igcore::TimeOfDayAttributes> *>(ref.get());
                if (attr)
                {
                    _sky->setSunPosition((float)attr->getValue().getHour());

                    if (!_precipitation.valid())
                    {
                        _precipitation = new osgParticle::PrecipitationEffect;
                        context.getImageGenerator()->getViewer()->getView(0)->getSceneData()->asGroup()->addChild(_precipitation);

                        _precipitation->snow(0);
                        _precipitation->rain(0);

                        _precipitation->setFog(context.getImageGenerator()->getFog());
                    }

                    _precipitation->getFog()->setColor(_sky->getSun()->getDiffuse());
                }
            }
            {
                osg::ref_ptr<osg::Referenced> ref = context.getAttribute("Rain");
                igplugincore::PluginContext::Attribute<igcore::RainSnowAttributes> *attr = dynamic_cast<igplugincore::PluginContext::Attribute<igcore::RainSnowAttributes> *>(ref.get());
                if (attr)
                {
                    if (!_precipitation.valid())
                    {
                        _precipitation = new osgParticle::PrecipitationEffect;
                        context.getImageGenerator()->getViewer()->getView(0)->getSceneData()->asGroup()->addChild(_precipitation);

                        _precipitation->snow(0);
                        _precipitation->rain(0);
                    }

                    _precipitation->rain(attr->getValue().getFactor());
                }

                {
                    osg::ref_ptr<osg::Referenced> ref = context.getAttribute("Fog");
                    igplugincore::PluginContext::Attribute<igcore::FogAttributes> *attr = dynamic_cast<igplugincore::PluginContext::Attribute<igcore::FogAttributes> *>(ref.get());
                    if (attr)
                    {
                        if (!_precipitation.valid())
                        {
                            _precipitation = new osgParticle::PrecipitationEffect;
                            context.getImageGenerator()->getViewer()->getView(0)->getSceneData()->asGroup()->addChild(_precipitation);

                            _precipitation->snow(0);
                            _precipitation->rain(0);

                            _precipitation->setFog(context.getImageGenerator()->getFog());
                        }

                        _precipitation->getFog()->setColor(_sky->getSun()->getDiffuse());                    }
                }

                {
                    osg::ref_ptr<osg::Referenced> ref = context.getAttribute("Wind");
                    igplugincore::PluginContext::Attribute<igcore::WindAttributes> *attr = dynamic_cast<igplugincore::PluginContext::Attribute<igcore::WindAttributes> *>(ref.get());
                    if (attr)
                    {
                        if (!_precipitation.valid())
                        {
                            _precipitation = new osgParticle::PrecipitationEffect;
                            context.getImageGenerator()->getViewer()->getView(0)->getSceneData()->asGroup()->addChild(_precipitation);

                            _precipitation->snow(0);
                            _precipitation->rain(0);

                            _precipitation->setFog(context.getImageGenerator()->getFog());
                        }

                        float direction = attr->getValue()._direction+90.0;
                        float speed = attr->getValue()._speed;

                        osg::Matrixd mx = igcore::Math::instance()->toMatrix(0,0,0,direction,0,0);
                        osg::Quat q = mx.getRotate();

                        osg::Vec3 v(0,1,0);
                        v = q * v;
                        v.normalize();

                        v *= speed;

                        _precipitation->setWind(v);
                    }
                }
            }
            {
                osg::ref_ptr<osg::Referenced> ref = context.getAttribute("Snow");
                igplugincore::PluginContext::Attribute<igcore::RainSnowAttributes> *attr = dynamic_cast<igplugincore::PluginContext::Attribute<igcore::RainSnowAttributes> *>(ref.get());
                if (attr)
                {
                    if (!_precipitation.valid())
                    {
                        _precipitation = new osgParticle::PrecipitationEffect;
                        context.getImageGenerator()->getViewer()->getView(0)->getSceneData()->asGroup()->addChild(_precipitation);

                        _precipitation->snow(0);
                        _precipitation->rain(0);

                        _precipitation->setFog(context.getImageGenerator()->getFog());
                    }

                    _precipitation->snow(attr->getValue().getFactor());
                }
            }
        }
    }

    // Code taken from muse/libs/OtwOsg
    // slightly modified to fit into OpenIg framework
    class Sky : public osg::Group
    {
    public:
        Sky( igcore::ImageGenerator* ig, const std::string& fileName, float ceiling=18520.0f, float visibility=92600 )
            : _ig(ig)
        {
            sun_ = ig->getSunOrMoonLight()->getLight();

            osg::ref_ptr<osg::StateSet> skystate = new osg::StateSet;
            skystate->setGlobalDefaults();
            setStateSet(skystate);
            skystate->setMode(GL_NORMALIZE, osg::StateAttribute::ON );
            //skystate->setAttributeAndModes(new osg::FrontFace(osg::FrontFace::COUNTER_CLOCKWISE), osg::StateAttribute::ON);

            _dome = osgDB::readNodeFile( fileName );

            if (!_dome.valid())
            {
                osg::notify(osg::NOTICE) << "SkyDome: failed to load sky model : " << fileName << std::endl;
            }
            _domeScale = new osg::MatrixTransform;
            _domeScale->setMatrix( osg::Matrix::scale( visibility, visibility, ceiling ) );
            _domeScale->addChild( _dome );
            _domeTranslate = new osg::MatrixTransform;
            _domeTranslate->addChild( _domeScale );
            addChild( _domeTranslate );

#if 1
            //osg::Material* material = dynamic_cast<osg::Material*>(skystate->getAttribute(osg::StateAttribute::MATERIAL));

            //if (!material)
            //{
               osg::Material* material = new osg::Material;
               skystate->setAttribute(material,osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE|osg::StateAttribute::PROTECTED);
            //}
                //    _domeScale->setMatrix( osg::Matrix::scale( VISIBILITY_MAX, VISIBILITY_MAX, VISIBILITY_MAX/5.0 ) );
            //material->setColorMode( osg::Material::SPECULAR);
            material->setDiffuse(osg::Material::FRONT_AND_BACK,osg::Vec4( 1.0,1.0,1.0,1.0 ));
            material->setAmbient(osg::Material::FRONT_AND_BACK,osg::Vec4( 1.0,1.0,1.0,1.0 ));
            material->setSpecular(osg::Material::FRONT_AND_BACK,osg::Vec4( 0.0,0.0,0.0,1.0 ));
            material->setShininess( osg::Material::FRONT_AND_BACK, 10.0 );

            skystate->setAttributeAndModes(new osg::Program,osg::StateAttribute::ON|osg::StateAttribute::PROTECTED|osg::StateAttribute::OVERRIDE);
#endif
            sky_light = _ig->getSunOrMoonLight()->getLight();
            //sky_light->setLightNum(3);
            sky_light->setAmbient( osg::Vec4( 0.0,0.0,0.0,1.0 ) );
            sky_light->setSpecular( osg::Vec4( 1.0,1.0,1.0,1.0 ) );
            sky_light->setDiffuse( osg::Vec4( 1.0,1.0,1.0,1.0 ) );
            //sky_light->setPosition( osg::Vec4( 1.0,1.0,1.0,1.0 ) );
#if 0
            osg::LightSource *lightS1 = _ig->getSunOrMoonLight();//new osg::LightSource;
            //lightS1->setLight(sky_light);
            lightS1->setLocalStateSetModes(osg::StateAttribute::ON);
            lightS1->setStateSetModes(*skystate, osg::StateAttribute::ON);
            addChild(lightS1);
#endif
        }


        void setEye( osg::Vec3 eye )
        {
            _domeTranslate->setMatrix( osg::Matrix::translate( eye.x(), eye.y(), 0.0 ) );
        }

        void setScale( osg::Vec3 scale )
        {
            _domeScale->setMatrix( osg::Matrix::scale( scale ) );
        }

        void setSun()
        {
            setSunPosition( 12.0f );
        }

        void setSunPosition( float hours )
        {
            float intFactor;
            float angle;
            osg::Vec4                     sunAmbient_;
            osg::Vec4                     sunDiffuse_;
            osg::Vec4                     sunPosition_;

            float timeOfDay = hours;

            angle = timeOfDay * 2.0 * osg::PI / 24.0;

            if( hours <= 6.0 || hours >= 18.0 )
            {
                intFactor = 0.0;
            }
            else
            {
                if( hours < 12.0 )
                {
                    intFactor = sqrt(hours / 6.0 - 1.0)*1.7;
                }
                else
                {
                    intFactor = sqrt(-hours / 6.0 + 3.0)*1.7;
                }
            }

            if( intFactor > 1.0 ) intFactor = 1.0;

            float colorFactor;

            if( intFactor > 0.32 )
                colorFactor = 1.0;
            else
                colorFactor = 1.0 + (0.32 - intFactor) * 50.0f;//_visibility;


            //Cosmetic changes
            if( hours >= 14.0f && 22.0f >= hours )
            {
                if( hours == 14.0f )
                {
                    sunAmbient_.set( 0.6, 0.6, 0.6, 1.0 );
                    sunDiffuse_.set( 0.6, 0.6, 0.6, 1.0 );

                }
                else if( hours == 15.0f )
                {
                    sunAmbient_.set( 0.4, 0.4, 0.2, 1.0 );
                    sunDiffuse_.set( 0.4, 0.4, 0.3, 1.0 );

                }
                else if( hours == 16.0f )
                {
                    sunAmbient_.set( 0.2, 0.2, 0.105, 1.0 );
                    sunDiffuse_.set( 0.2, 0.2, 0.105, 1.0 );

                }
                else if( hours == 17.0f )
                {
                    sunAmbient_.set( 0.1, 0.1, 0.05, 1.0 );
                    sunDiffuse_.set( 0.1, 0.1, 0.05, 1.0 );


                    sky_light->setDiffuse( osg::Vec4(0.0,0.0,0.45,1.0) );
                    sky_light->setAmbient( osg::Vec4(1.0,0.54,0.0,1.0) );
                }
                else if( hours >= 18.0f && hours <= 20.0f )
                {
                    sunAmbient_.set( 0.1, 0.1, 0.05, 1.0 );
                    sunDiffuse_.set( 0.1, 0.1, 0.05, 1.0 );


                    sky_light->setDiffuse( osg::Vec4(1.0,0.54,0.0,1.0) );
                    sky_light->setAmbient( osg::Vec4(1.0,0.54,0.0,1.0) );
                }
                else if( hours >= 21.0f && hours <= 22.0f )
                {
                    sunAmbient_.set( 0.03, 0.03, 0.03, 1.0 );
                    sunDiffuse_.set( 0.03, 0.03, 0.03, 1.0 );


                    sky_light->setDiffuse( osg::Vec4(0.0,0.0,0.45,1.0) );
                    sky_light->setAmbient( osg::Vec4(0.0,0.0,0.45,1.0) );
                }

                if( hours >= 14.0 && hours <= 16.0f )
                {
                    sky_light->setDiffuse( osg::Vec4(0.6*intFactor, 0.6*intFactor, 0.6*intFactor, 1.0) );
                    sky_light->setAmbient( osg::Vec4(0.9*intFactor*colorFactor, 0.9*intFactor, 0.8*intFactor, 1.0) );

                    sunAmbient_.set( 0.6*intFactor, 0.6*intFactor, 0.6*intFactor, 1.0 );
                    sunDiffuse_.set( 0.9*intFactor*colorFactor, 0.9*intFactor, 0.8*intFactor, 1.0 );
                }

                sun_->setAmbient( sunAmbient_ );
                sun_->setDiffuse( sunDiffuse_ );

            }
            else if( hours == 06.0f )
            {
                sky_light->setDiffuse( osg::Vec4(1.0,0.54,0.0,1.0) );
                sky_light->setAmbient( osg::Vec4(1.0,0.54,0.0,1.0) );

                sunAmbient_.set( 0.1, 0.1, 0.05, 1.0 );
                sunDiffuse_.set( 0.1, 0.1, 0.05, 1.0 );
            }
            else if( hours >= 23.0f && hours <= 05.0f )
            {
                sky_light->setDiffuse( osg::Vec4(0.0,0.0,0.0,1.0) );
                sky_light->setAmbient( osg::Vec4(0.0,0.0,0.0,1.0) );

                sunAmbient_.set( 0.0, 0.0, 0.0, 1.0 );
                sunDiffuse_.set( 0.0, 0.0, 0.0, 1.0 );

            }
            else
            {
                sunAmbient_.set( 0.6*intFactor, 0.6*intFactor, 0.6*intFactor, 1.0 );
                sunDiffuse_.set( 0.9*intFactor*colorFactor, 0.9*intFactor, 0.8*intFactor, 1.0 );


                sky_light->setDiffuse( osg::Vec4(0.6*intFactor, 0.6*intFactor, 0.6*intFactor, 1.0) );
                sky_light->setAmbient( osg::Vec4(0.9*intFactor*colorFactor, 0.9*intFactor, 0.8*intFactor, 1.0) );
            }

            sun_->setAmbient( sunAmbient_ );
            sun_->setDiffuse( sunDiffuse_ );


            if( hours >= 19.0f )
                sunPosition_.set( sin(angle), 0.0, -cos(angle), 0.0 );
            else //Black Horizon Fix
                sunPosition_.set( sin(angle)*0.1, 0.0, -cos(angle), 0.0 );
            sun_->setPosition( sunPosition_ );

            sunPosition_.z() *= -1.0;
            setTOD( sunPosition_ );
        }

        void setTOD( osg::Vec4 sunpos )
        {
            sunpos.normalize();
            sky_light->setPosition( osg::Vec4(-sunpos.x(), -sunpos.y(), -sunpos.z(), 0.0) );
            //sky_light->setPosition( osg::Vec4(0.0,0.0,-1.0, 0.0) );
        }

        osg::ref_ptr<osg::Light> getSkyLight( void )    { return sky_light; }
        osg::ref_ptr<osg::Light> getSun( void )         { return sun_; }

    private:
        osg::ref_ptr<osg::Light>            sky_light;
        osg::ref_ptr<osg::Light>            sun_;
        osg::ref_ptr<osg::MatrixTransform> _domeScale;
        osg::ref_ptr<osg::MatrixTransform> _domeTranslate;
        osg::ref_ptr<osg::Node>             _dome;
        igcore::ImageGenerator*             _ig;

    };

    osg::Node* createSkyDome(igcore::ImageGenerator* ig)
    {
        _sky = new Sky(ig,_modelFileName);
        //_sky->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
        return _sky.get();
    }

    osg::ref_ptr<Sky>                               _sky;
    osg::ref_ptr<osgParticle::PrecipitationEffect>  _precipitation;

    class SetFogCommand : public igcore::Commands::Command
    {
    public:
        SetFogCommand (igcore::ImageGenerator* ig)
            : _ig(ig) {}

        virtual const std::string getUsage() const
        {
            return "visibility";
        }

        virtual const std::string getDescription() const
        {
            return  "sets the visibility of the scene by using fog\n"
                    "     visibility - in meteres, the distance of the fog";
        }

        virtual int exec(const igcore::StringUtils::Tokens& tokens)
        {
            if (tokens.size() == 1)
            {
                double visibility = atof(tokens.at(0).c_str());

                _ig->setFog(visibility);

                return 0;
            }
            return -1;
        }

    protected:
        igcore::ImageGenerator* _ig;
    };

    class RainCommand : public igcore::Commands::Command
    {
    public:
        RainCommand(igcore::ImageGenerator* ig)
            : _ig(ig) {}

        virtual const std::string getUsage() const
        {
            return "rainfactor";
        }

        virtual const std::string getDescription() const
        {
            return  "adds rain to the scene\n"
                    "     rainfactor - from 0.0-1.0, 0 no rain, 1 heavy rain";
        }

        virtual int exec(const igcore::StringUtils::Tokens& tokens)
        {
            if (tokens.size() == 1)
            {
                double factor = atof(tokens.at(0).c_str());

                _ig->setRain(factor);

                return 0;
            }

            return -1;
        }
    protected:
        igcore::ImageGenerator* _ig;
    };

    class SnowCommand : public igcore::Commands::Command
    {
    public:
        SnowCommand(igcore::ImageGenerator* ig)
            : _ig(ig) {}

        virtual const std::string getUsage() const
        {
            return "snowfactor";
        }

        virtual const std::string getDescription() const
        {
            return  "adds snow to the scene\n"
                    "     snowfactor - from 0.0-1.0. 0 no snow, 1 heavy snow";
        }

        virtual int exec(const igcore::StringUtils::Tokens& tokens)
        {
            if (tokens.size() == 1)
            {
                double factor = atof(tokens.at(0).c_str());

                _ig->setSnow(factor);

                return 0;
            }

            return -1;
        }
    protected:
        igcore::ImageGenerator* _ig;
    };
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
    return new igplugins::SkyDomePlugin;
}

extern "C" EXPORT void DeletePlugin(igplugincore::Plugin* plugin)
{
    osg::ref_ptr<igplugincore::Plugin> p(plugin);
}
