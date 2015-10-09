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
#include <IgCore/animation.h>
#include <IgCore/stringutils.h>
#include <IgCore/commands.h>

#include <string>

#include <osgAnimation/AnimationManagerBase>
#include <osgAnimation/BasicAnimationManager>

#include <osgDB/FileNameUtils>

namespace igplugins
{

	class FBXAnimationPlugin : public igplugincore::Plugin
	{
	public:

		FBXAnimationPlugin() {}

		virtual std::string getName() { return "FBXAnimation"; }

		virtual std::string getDescription() { return "Controls FBX animations"; }

		virtual std::string getVersion() { return "1.0.0"; }

		virtual std::string getAuthor() { return "ComPro, Nick"; }

		struct AnimationManagerFinder : public osg::NodeVisitor
		{
			osg::ref_ptr<osgAnimation::BasicAnimationManager> _am;

			AnimationManagerFinder() : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}
			void apply(osg::Node& node) {
				if (_am.valid())
					return;
				if (node.getUpdateCallback()) {
					osgAnimation::AnimationManagerBase* b = dynamic_cast<osgAnimation::AnimationManagerBase*>(node.getUpdateCallback());
					if (b) {
						_am = new osgAnimation::BasicAnimationManager(*b);
						return;
					}
				}
				traverse(node);
			}
		};


        virtual void entityAdded(igplugincore::PluginContext&, unsigned int id, osg::Node& entity, const std::string& fileName)
		{
			std::string extension = osgDB::getLowerCaseFileExtension(fileName);

			//osg::notify(osg::NOTICE) << "FBXAnimation: entity added, " << extension << std::endl;

			if (extension != "fbx" 
				&& fileName.find(".fbx.") == std::string::npos
				&& fileName.find(".FBX.") == std::string::npos) return;

			AnimationManagerFinder afnv;
			entity.accept(afnv);

			if (afnv._am.valid())
			{			
				for (osgAnimation::AnimationList::const_iterator animIter = afnv._am->getAnimationList().begin();
					animIter != afnv._am->getAnimationList().end(); ++animIter)
				{
					(*animIter)->setPlayMode(osgAnimation::Animation::ONCE);
					osg::notify(osg::NOTICE) << "FBXAnimation: entity: " << id << ", animation: " << (**animIter).getName() << std::endl;
				}	

				entity.setUserData(afnv._am);
				entity.setUpdateCallback(afnv._am.get());
			}
			else
			{
				osg::notify(osg::NOTICE) << "FBXAnimation: no animations found in " << fileName << std::endl;
			}
		}

		virtual void update(igplugincore::PluginContext& context)
		{
			igplugincore::PluginContext::AttributeMapIterator itr = context.getAttributes().begin();
			for (; itr != context.getAttributes().end(); ++itr)
			{
				osg::ref_ptr<osg::Referenced> ref = itr->second;
				igplugincore::PluginContext::Attribute<igcore::AnimationAttributes> *attr = dynamic_cast<igplugincore::PluginContext::Attribute<igcore::AnimationAttributes> *>(ref.get());

				// This is cleaner way of dealing with
				// PluginContext attributes but the Mac
				// compiler doesn't like it. It works ok
				// on Linux though
				// osg::ref_ptr<igplugincore::PluginContext::Attribute<igcore::AnimationAttributes> attr = itr->second;

				if (itr->first == "Animation")
				{
					igcore::StringUtils::Tokens tokens = igcore::StringUtils::instance()->tokenize(attr->getValue()._animationName, ":");

					if (tokens.size() < 2) continue;
                    if (tokens.at(0).compare(0,3,"fbx") == 0)
					{						
						std::string fbxAnimtionName = tokens.at(1);

						osg::notify(osg::NOTICE) << "FBXAnimation: starting FBX animation: " << fbxAnimtionName << std::endl;

						unsigned int entityID = attr->getValue()._entityId;
						if (context.getImageGenerator()->getEntityMap().count(entityID) == 0) continue;

						igcore::ImageGenerator::Entity entity = context.getImageGenerator()->getEntityMap()[entityID];
						if (!entity.valid()) continue;

						osg::ref_ptr<osgAnimation::BasicAnimationManager> am = dynamic_cast<osgAnimation::BasicAnimationManager*>(entity->getUserData());
						if (!am.valid())
						{
							osg::notify(osg::NOTICE) << "FBXAnimation: NULL animation manager: " << fbxAnimtionName << std::endl;
							continue;
						}

						osgAnimation::Animation* animation = 0;

						for (osgAnimation::AnimationList::const_iterator animIter = am->getAnimationList().begin();
							animIter != am->getAnimationList().end(); ++animIter)
						{
							if ((**animIter).getName() == fbxAnimtionName)
							{
								animation = *animIter;
								osg::notify(osg::NOTICE) << "FBXAnimation: found animation : " << fbxAnimtionName << std::endl;
								break;
							}
						}

						if (animation == 0)
						{
							osg::notify(osg::NOTICE) << "FBXAnimation: not found animation : " << fbxAnimtionName << std::endl;
							continue;
						}

						if (attr->getValue()._playback)
						{
							osg::notify(osg::NOTICE) << "FBXAnimation: animation playback start: " << fbxAnimtionName << std::endl;

							osgAnimation::Animation::PlayMode playMode = osgAnimation::Animation::LOOP;
							if (tokens.size() > 2)
							{
                                if (tokens.at(2).compare(0,4,"ONCE") == 0) playMode = osgAnimation::Animation::ONCE;
                                else if (tokens.at(2).compare(0,4,"STAY") == 0) playMode = osgAnimation::Animation::STAY;
                                else if (tokens.at(2).compare(0,4,"LOOP") == 0) playMode = osgAnimation::Animation::LOOP;
                                else if (tokens.at(2).compare(0,5,"PPONG") == 0) playMode = osgAnimation::Animation::PPONG;

								osg::notify(osg::NOTICE) << "FBXAnimation: animation playback mode: " << tokens.at(2) << std::endl;
							}

							animation->setPlayMode(playMode);
							am->playAnimation(animation);
						}
						else
						if (!attr->getValue()._playback)
						{
							am->stopAnimation(animation);
						}
					}
				}
			}
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
	return new igplugins::FBXAnimationPlugin;
}

extern "C" EXPORT void DeletePlugin(igplugincore::Plugin* plugin)
{
	osg::ref_ptr<igplugincore::Plugin> p(plugin);
}
