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

//#*****************************************************************************
//#*	This code was taken from 
//#*	https://github.com/xarray/osgRecipes/tree/master/integrations/osgmygui
//#*	and the author is Wang Rui <wangray84@gmail.com>
//#*****************************************************************************

#ifndef H_MYGUIDRAWABLE
#define H_MYGUIDRAWABLE

#include <MYGUI/MyGUI.h>
#include <MYGUI/MyGUI_OpenGLPlatform.h>

#include <osg/Camera>
#include <osg/Drawable>

#include <osgGA/GUIEventHandler>
#include <osgGA/CameraManipulator>

#include <queue>

#include <Core-Base/imagegenerator.h>

namespace OpenIG {
	namespace Plugins {

		class MYGUIManager;
		class MyOpenGLPlatform;

		class MYGUIHandler : public osgGA::GUIEventHandler
		{
		public:
			MYGUIHandler(osg::Camera* c, MYGUIManager* m, Base::ImageGenerator* ig, unsigned width) 
				: _camera(c)
				, _manager(m)
				, _ig(ig)
				, _screenWidth(width)
			{
				if (_camera.valid()) _camera->setNodeMask(0x0);
			}
			virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

		protected:
			osg::observer_ptr<osg::Camera>			_camera;
			MYGUIManager*							_manager;
			Base::ImageGenerator*					_ig;
			osg::ref_ptr<osgGA::CameraManipulator>	_cameraManipulator;
			unsigned								_screenWidth;
		};

		class MyOpenGLRenderManager : public MyGUI::OpenGLRenderManager
		{
		public:
			virtual void begin()
			{
				//glClearColor(0, 0, 0, 1);
				glClear(GL_DEPTH_BUFFER_BIT);
				glLoadIdentity();

				//save current attributes
				glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
				glPushAttrib(GL_ALL_ATTRIB_BITS);

				glPolygonMode(GL_FRONT, GL_FILL);
				glMatrixMode(GL_PROJECTION);
				glPushMatrix();
				glLoadIdentity();
				glOrtho(-1, 1, -1, 1, -1, 1);

				glMatrixMode(GL_MODELVIEW);
				glPushMatrix();
				glLoadIdentity();

				glDisable(GL_LIGHTING);
				glDisable(GL_DEPTH_TEST);
				glDisable(GL_FOG);
				glDisable(GL_TEXTURE_GEN_S);
				glDisable(GL_TEXTURE_GEN_T);
				glDisable(GL_TEXTURE_GEN_R);

				//glFrontFace(GL_CW);
				//glCullFace(GL_BACK);
				//glEnable(GL_CULL_FACE);

				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				glEnable(GL_TEXTURE_2D);
			}
		};

		class MyOpenGLPlatform
		{
		public:
			MyOpenGLPlatform() :
				mIsInitialise(false)
			{
				mMyRenderManager = new MyOpenGLRenderManager();
				mDataManager = new MyGUI::OpenGLDataManager();
				mLogManager = new MyGUI::LogManager();
			}

			~MyOpenGLPlatform()
			{
				assert(!mIsInitialise);
				delete mMyRenderManager;
				delete mDataManager;
				delete mLogManager;
			}

			void initialise(MyGUI::OpenGLImageLoader* _loader, const std::string& _logName = MYGUI_PLATFORM_LOG_FILENAME)
			{
				assert(!mIsInitialise);
				mIsInitialise = true;

				if (!_logName.empty())
					MyGUI::LogManager::getInstance().createDefaultSource(_logName);

				mMyRenderManager->initialise(_loader);
				mDataManager->initialise();
			}

			void shutdown()
			{
				assert(mIsInitialise);
				mIsInitialise = false;

				mMyRenderManager->shutdown();
				mDataManager->shutdown();
			}

			virtual MyOpenGLRenderManager* getMyRenderManagerPtr()
			{
				assert(mIsInitialise);
				return mMyRenderManager;
			}

			MyGUI::OpenGLDataManager* getDataManagerPtr()
			{
				assert(mIsInitialise);
				return mDataManager;
			}

		protected:
			MyOpenGLRenderManager* mMyRenderManager;

		private:
			bool							mIsInitialise;
			MyGUI::OpenGLDataManager*		mDataManager;
			MyGUI::LogManager*				mLogManager;
		};

		class MYGUIManager : public osg::Drawable, public MyGUI::OpenGLImageLoader
		{
		public:
			MYGUIManager();
			MYGUIManager(const std::string& rootMedia);
			MYGUIManager(const MYGUIManager& copy, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);
			META_Object(osg, MYGUIManager)

				void setResourcePathFile(const std::string& file) { _resourcePathFile = file; }
			const std::string& getResourcePathFile() const { return _resourcePathFile; }

			void setResourceCoreFile(const std::string& file) { _resourceCoreFile = file; }
			const std::string& getResourceCoreFile() const { return _resourceCoreFile; }

			void pushEvent(const osgGA::GUIEventAdapter* ea)
			{
				_eventsToHandle.push(ea);
			}

			// image loader methods
			virtual void* loadImage(int& width, int& height, MyGUI::PixelFormat& format, const std::string& filename);
			virtual void saveImage(int width, int height, MyGUI::PixelFormat format, void* texture, const std::string& filename);

			// drawable methods
			virtual void drawImplementation(osg::RenderInfo& renderInfo) const;
			virtual void releaseGLObjects(osg::State* state = 0) const;

			void setOverallAlpha(float alpha);
			void setIntersectionPoint(const osg::Vec3d& point, bool update = false);

			const osg::Vec3d& getIntersectionPoint() const { return _intersectionPoint; }

			void lock()
			{
				_mutex.lock();
			}
			void unlock()
			{
				_mutex.unlock();
			}

		protected:
			virtual ~MYGUIManager() {}

			virtual void updateEvents() const;
			virtual void setupResources();
			virtual void initializeControls() {}

			MyGUI::MouseButton convertMouseButton(int button) const;
			MyGUI::KeyCode convertKeyCode(int key) const;

			std::queue< osg::ref_ptr<const osgGA::GUIEventAdapter> > _eventsToHandle;
			MyGUI::Gui*												_gui;
			MyOpenGLPlatform*										_platform;
			std::string												_resourcePathFile;
			std::string												_resourceCoreFile;
			std::string												_rootMedia;
			unsigned int											_activeContextID;
			bool													_initialized;
			MyGUI::Widget*											_root;
			OpenThreads::Mutex										_mutex;
			osg::Vec3d												_intersectionPoint;
			bool													_intersectionPointUpdated;

		};
	}
}
#endif
