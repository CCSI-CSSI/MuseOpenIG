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
#include <IgCore/commands.h>
#include <IgCore/attributes.h>

#include <string>
#include <iostream>

#include <osgDB/FileNameUtils>

#include <osgParticle/ExplosionEffect>
#include <osgParticle/ExplosionDebrisEffect>
#include <osgParticle/SmokeEffect>
#include <osgParticle/SmokeTrailEffect>
#include <osgParticle/FireEffect>

namespace igplugins
{

	class OSGParticleEffectsPlugin: public igplugincore::Plugin
	{
	public:

		OSGParticleEffectsPlugin() {}

		virtual std::string getName() { return "OSGParticleEffects"; }

		virtual std::string getDescription() { return "Implements effects using osgParticle foundation"; }

		virtual std::string getVersion() { return "1.0.0"; }

		virtual std::string getAuthor() { return "ComPro, Nick"; }

		class OSGParticleEffectImplementationCallback : public igcore::GenericImplementationCallback
		{
		public:
			OSGParticleEffectImplementationCallback(igcore::ImageGenerator *ig)
				: _ig(ig)
			{

			}

			virtual osg::Node* create(unsigned int id, const std::string& name, igcore::GenericAttribute* attributes = 0)
			{
				destroy(id);

				float scale = 10.f;
				float intensity = 1.f;
				float emiterDuration = 65.f;
				float particleDuration = 10.f;

#if 1
				if (attributes != 0)
				{
					attributes->getUserValue("scale", scale);
					attributes->getUserValue("intensity", intensity);
					attributes->getUserValue("emiterduration", emiterDuration);
					attributes->getUserValue("particleduration", particleDuration);
				}
#endif

				osg::ref_ptr<osgParticle::ParticleEffect> effect;
				if (name == "ExplosionEffect")
				{
					effect = new osgParticle::ExplosionEffect(osg::Vec3(0,0,0), scale, intensity);					
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
				}

				if (!effect.valid()) return 0;

				effect->setParticleDuration(particleDuration);
				effect->setEmitterDuration(emiterDuration);
				
				_effects[id] = effect;

				std::cout << "Effect created: " << name << std::endl;

				effect->setUseLocalParticleSystem(false);

				osg::ref_ptr<osg::Group> group = new osg::Group;
				group->addChild(effect);

				osg::ref_ptr<osg::Geode> geode = new osg::Geode;
				geode->addDrawable(effect->getParticleSystem());

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

			virtual void update(unsigned int, igcore::GenericAttribute*)
			{
			}

		protected:
			typedef std::map< unsigned int, osg::ref_ptr<osgParticle::ParticleEffect> >	EffectsMap;
			EffectsMap					_effects;

			typedef std::map< unsigned int, osg::ref_ptr<osg::Geode> >					EffectsGeodesMap;
			EffectsGeodesMap			_effectsGeodes;

			igcore::ImageGenerator		*_ig;
		};

		virtual void init(igplugincore::PluginContext& context)
		{
			context.getImageGenerator()->setEffectImplementationCallback(new OSGParticleEffectImplementationCallback(context.getImageGenerator()));
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
	return new igplugins::OSGParticleEffectsPlugin;
}

extern "C" EXPORT void DeletePlugin(igplugincore::Plugin* plugin)
{
	osg::ref_ptr<igplugincore::Plugin> p(plugin);
}
