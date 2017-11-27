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
//#*    Please direct any questions or comments to the OpenIG Forums
//#*    Email address: openig@compro.net
//#*
//#*
//#*****************************************************************************
#include <Core-PluginBase/Plugin.h>
#include <Core-PluginBase/PluginContext.h>

#include <Core-Base/ImageGenerator.h>
#include <Core-Base/StringUtils.h>
#include <Core-Base/Commands.h>
#include <Core-Base/Types.h>
#include <Core-Base/FileSystem.h>

#include <string>
#include <iostream>

#include <osgDB/FileNameUtils>

#include <osgParticle/ExplosionEffect>
#include <osgParticle/ExplosionDebrisEffect>
#include <osgParticle/SmokeEffect>
#include <osgParticle/SmokeTrailEffect>
#include <osgParticle/FireEffect>

namespace OpenIG {
    namespace Plugins {

        class OSGParticleEffectsPlugin : public OpenIG::PluginBase::Plugin
        {
        public:

            OSGParticleEffectsPlugin() {}

            virtual std::string getName() { return "OSGParticleEffects"; }

            virtual std::string getDescription() { return "Implements effects using the osgParticle foundation"; }

            virtual std::string getVersion() { return "1.0.0"; }

            virtual std::string getAuthor() { return "ComPro, Nick"; }

            class OSGParticleEffectImplementationCallback : public OpenIG::Base::GenericImplementationCallback
            {
            public:
                OSGParticleEffectImplementationCallback(OpenIG::Base::ImageGenerator *ig)
                    : _ig(ig)
                {

                }

                virtual osg::Node* create(unsigned int id, const std::string& name, OpenIG::Base::GenericAttribute* attributes = 0)
                {
                    destroy(id);

                    float scale = 10.f;
                    float intensity = 1.f;
                    float emiterDuration = 65.f;
                    float particleDuration = 10.f;
                    float particleSpeed = 0.0f;
                    float mass = 0.0f;

                    if (attributes != 0)
                    {
                        attributes->getUserValue("scale", scale);
                        attributes->getUserValue("intensity", intensity);
                        attributes->getUserValue("emiterduration", emiterDuration);
                        attributes->getUserValue("particleduration", particleDuration);
                        attributes->getUserValue("speed", particleSpeed);
                        attributes->getUserValue("mass", mass);
                    }

#ifdef _WIN32
                    std::string resourcePath = OpenIG::Base::FileSystem::path(OpenIG::Base::FileSystem::Resources, "../resources");
#else
                    std::string resourcePath = OpenIG::Base::FileSystem::path(OpenIG::Base::FileSystem::Resources, "../../resources");
#endif
                    osg::ref_ptr<osgParticle::ParticleEffect> effect;
                    if (name == "ExplosionEffect")
                    {
                        effect = new osgParticle::ExplosionEffect(osg::Vec3(0, 0, 0), scale, intensity);
                    }
                    if (name == "ExplosionDebrisEffect")
                    {
                        effect = new osgParticle::ExplosionDebrisEffect(osg::Vec3(0, 0, 0), scale, intensity);
                    }
                    if (name == "SmokeEffect")
                    {
                        effect = new osgParticle::SmokeEffect(osg::Vec3(0, 0, 0), scale, intensity);
                    }
                    if (name == "FireEffect")
                    {
                        effect = new osgParticle::FireEffect(osg::Vec3(0, 0, 0), scale, intensity);
                    }
                    if (name == "SmokeTrailEffect")
                    {
                        effect = new osgParticle::SmokeTrailEffect(osg::Vec3(0, 0, 0), scale, intensity);

                        if(effect.valid())
                        {
                            effect->setWind(osg::Vec3(1.0f,0.0f,0.0f));
                            effect->setTextureFileName(resourcePath + "/textures/continuous_smoke.rgb");
                        }
                    }

                    if (!effect.valid()) return 0;

                    effect->setParticleDuration(particleDuration);
                    effect->setEmitterDuration(emiterDuration);

                    if (name != "SmokeTrailEffect")
                        effect->setTextureFileName(resourcePath + "/textures/smoke.rgb");

                    _effects[id] = effect;

                    effect->setUseLocalParticleSystem(false);
                    effect->getEmitter()->setUseDefaultTemplate(true);

                    if (particleSpeed != 0.0f)
                    {
                        osg::ref_ptr<osgParticle::ModularEmitter> emitter = dynamic_cast<osgParticle::ModularEmitter*>(effect->getEmitter());
                        if (emitter.valid())
                        {
                            osgParticle::RadialShooter* shooter = dynamic_cast<osgParticle::RadialShooter*>(emitter->getShooter());
                            if (shooter)
                            {
                                shooter->setInitialSpeedRange(particleSpeed, particleSpeed);
                            }
                        }
                    }
                    if (mass != 0.0f)
                    {
                        osgParticle::Particle& p = const_cast<osgParticle::Particle&>(effect->getParticleSystem()->getDefaultParticleTemplate());
                        p.setMass(mass);
                    }

                    osg::ref_ptr<osg::Group> group = new osg::Group;
                    group->addChild(effect);

                    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
                    geode->addDrawable(effect->getParticleSystem());

#define USE_LOG_DEPTH_BUFFER 1
                    const char *VertSource = {
#ifdef USE_LOG_DEPTH_BUFFER
                        "uniform float Fcoef;																		\n"
#endif
                        "varying vec3 eyeVec;																		\n"
                        "void main()																				\n"
                        "{																							\n"
                        "   eyeVec = -vec3(gl_ModelViewMatrix * gl_Vertex);											\n"
                        "   gl_FrontColor = gl_Color;																\n"
                        "   gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;									\n"
                        "   gl_TexCoord[0] = gl_TextureMatrix[0] *gl_MultiTexCoord0;								\n"
#ifdef USE_LOG_DEPTH_BUFFER
                        "   gl_Position.z = (log2(max(1e-6, 1.0 + gl_Position.w)) * Fcoef - 1.0) * gl_Position.w;	\n"
#endif
                        "}																							\n"
                    };
                    const char *FragSource = {
                        "varying vec3 eyeVec;                                                   \n"
                        "uniform sampler2D baseTexture;                                         \n"
                        "void computeFogColor(inout vec4 color)                                 \n"
                        "{                                                                      \n"
                        "   float fogExp = gl_Fog.density * length(eyeVec);                     \n"
                        "   float fogFactor = exp(-(fogExp * fogExp));                          \n"
                        "   fogFactor = clamp(fogFactor, 0.0, 1.0);                             \n"
                        "   vec4 clr = color;                                                   \n"
                        "   color = mix(gl_Fog.color, color, fogFactor);                        \n"
                        "   color.a = clr.a;                                                    \n"
                        "}                                                                      \n"
                        "void main()															\n"
                        "{																		\n"
                        "   vec4 color = texture2D( baseTexture, gl_TexCoord[0].xy )*gl_Color;	\n"
                        "	computeFogColor(color);												\n"
                        "	gl_FragColor = color;												\n"
                        "}																		\n"
                    };

                    osg::ref_ptr<osg::Program> pointProgram = new osg::Program;
                    pointProgram->addShader(new osg::Shader(osg::Shader::VERTEX, VertSource));
                    pointProgram->addShader(new osg::Shader(osg::Shader::FRAGMENT, FragSource));

                    osg::StateSet *stateSet = geode->getOrCreateStateSet();
                    //Commented out to allow smoke trails to be white until we can fix whatever the shader issue is at the moment CGR for NICK...
                    //stateSet->setAttributeAndModes(pointProgram.get(), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);

                    _ig->getViewer()->getView(0)->getSceneData()->asGroup()->addChild(geode);

                    _effectsGeodes[id] = geode;

                    return group.release();
                }

                virtual void destroy(unsigned int id)
                {
                    EffectsMap::iterator itr = _effects.find(id);
                    if (itr == _effects.end()) return;

                    _effects.erase(itr);

                    EffectsGeodesMap::iterator gitr = _effectsGeodes.find(id);
                    if (gitr != _effectsGeodes.end())
                    {
                        osg::ref_ptr<osg::Geode> geode = gitr->second;

                        osg::Node::ParentList pl = geode->getParents();
                        osg::Node::ParentList::iterator pitr = pl.begin();
                        for (; pitr != pl.end(); ++pitr)
                        {
                            osg::Group* parent = *pitr;
                            parent->removeChild(geode);
                        }

                        _effectsGeodes.erase(gitr);
                    }
                }

                virtual void update(unsigned int, OpenIG::Base::GenericAttribute*)
                {
                }

            protected:
                typedef std::map< unsigned int, osg::ref_ptr<osgParticle::ParticleEffect> >	EffectsMap;
                EffectsMap					_effects;

                typedef std::map< unsigned int, osg::ref_ptr<osg::Geode> >					EffectsGeodesMap;
                EffectsGeodesMap			_effectsGeodes;

                OpenIG::Base::ImageGenerator		*_ig;
            };

            virtual void init(OpenIG::PluginBase::PluginContext& context)
            {
                context.getImageGenerator()->setEffectImplementationCallback(new OSGParticleEffectImplementationCallback(context.getImageGenerator()));
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
    return new OpenIG::Plugins::OSGParticleEffectsPlugin;
}

extern "C" EXPORT void DeletePlugin(OpenIG::PluginBase::Plugin* plugin)
{
    osg::ref_ptr<OpenIG::PluginBase::Plugin> p(plugin);
}
