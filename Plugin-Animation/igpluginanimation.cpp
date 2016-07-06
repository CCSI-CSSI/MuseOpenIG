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
#include <Core-PluginBase/plugin.h>
#include <Core-PluginBase/plugincontext.h>

#include <Core-Base/imagegenerator.h>
#include <Core-Base/animation.h>
#include <Core-Base/stringutils.h>
#include <Core-Base/commands.h>

namespace OpenIG {
	namespace Plugins {

		class AnimationPlugin : public OpenIG::PluginBase::Plugin
		{
		public:

			AnimationPlugin() {}

			virtual std::string getName() { return "Animation"; }

			virtual std::string getDescription() { return "Controls animations"; }

			virtual std::string getVersion() { return "1.0.0"; }

			virtual std::string getAuthor() { return "ComPro, Nick"; }

			virtual void update(OpenIG::PluginBase::PluginContext& context)
			{
				OpenIG::PluginBase::PluginContext::AttributeMapIterator itr = context.getAttributes().begin();
				for (; itr != context.getAttributes().end(); ++itr)
				{
					osg::ref_ptr<osg::Referenced> ref = itr->second;
					OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::AnimationAttributes> *attr = dynamic_cast<OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::AnimationAttributes> *>(ref.get());

					// This is cleaner way of dealing with
					// PluginContext attributes but the Mac
					// compiler doesn't like it. It works ok
					// on Linux though
					// osg::ref_ptr<OpenIG::PluginBase::PluginContext::Attribute<OpenIG::Base::AnimationAttributes> attr = itr->second;

					if (itr->first == "Animation")
					{
						OpenIG::Base::RefAnimationSequenceCallbacks* cbs = dynamic_cast<OpenIG::Base::RefAnimationSequenceCallbacks*>(attr->getValue().sequenceCallbacks.get());

						if (attr->getValue().pause)
						{
							OpenIG::Base::Animations::instance()->pauseResumeAnimation(
								context.getImageGenerator(),
								attr->getValue().entityId,
								attr->getValue().animationName
								);
						}
						else
						if (attr->getValue().restore)
						{
							OpenIG::Base::Animations::instance()->pauseResumeAnimation(
								context.getImageGenerator(),
								attr->getValue().entityId,
								attr->getValue().animationName,
								false
								);
						}
						else
						if (attr->getValue().playback)
						{
							OpenIG::Base::Animations::instance()->playAnimation(
								context.getImageGenerator(),
								attr->getValue().entityId,
								attr->getValue().animationName,
								cbs
								);
						}
						else
						if (attr->getValue().reset)
						{
							OpenIG::Base::Animations::instance()->resetAnimation(
								context.getImageGenerator(),
								attr->getValue().entityId,
								attr->getValue().animationName
								);
						}
						else
						{
							OpenIG::Base::Animations::instance()->stopAnimation(
								context.getImageGenerator(),
								attr->getValue().entityId,
								attr->getValue().animationName
								);
						}
					}
				}

				OpenIG::Base::Animations::instance()->updateAnimations(context.getImageGenerator());
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
    return new OpenIG::Plugins::AnimationPlugin;
}

extern "C" EXPORT void DeletePlugin(OpenIG::PluginBase::Plugin* plugin)
{
    osg::ref_ptr<OpenIG::PluginBase::Plugin> p(plugin);
}
