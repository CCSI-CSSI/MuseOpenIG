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

//#*****************************************************************************
//#* Original code borrowed from here 
//#* http://www.ogre3d.org/addonforums/viewtopic.php?f=17&t=14492
//#*****************************************************************************


#ifndef DIALOG_H
#define DIALOG_H

#include <boost/filesystem.hpp>
#include <boost/function.hpp>

#include <MYGUI/MyGUI.h>
#include <MYGUI/MyGUI_OpenGLPlatform.h>

namespace OpenIG {
	namespace Plugins {

		class FileSelectionDialog
		{
		public:
			FileSelectionDialog(const std::string& name);

			const std::string& getName();

			void centerOnScreen(unsigned width, unsigned height);

			void update();
			void setVisible(bool set);
			bool isVisible() const;
			void setDialogTitle(const std::string& title);
			void setButtonCaption(const std::string& caption);
			void addFileExtension(const std::string& ext);
			void setDefaultExtension(int index);
			void setUserString(const std::string& value);
			void buildGUI();
			const std::string& getUserString();
			void eventEndDialog(boost::function<void(FileSelectionDialog*, const std::string&)> method);

		private:
			void windowCloseCallback(MyGUI::Window* _sender, const std::string& _name);
			void itemAcceptedCallback(MyGUI::ListBox* _sender, size_t _index);
			void itemSelectedCallback(MyGUI::ListBox* _sender, size_t _index);
			void recentPathSelectedCallback(MyGUI::ComboBox* _sender, size_t _index);
			void endDialogCallback(MyGUI::Widget* _sender);
			void windowResizeCallback(MyGUI::Widget* _sender, const std::string& _key, const std::string& _value);
			void extensionChangeCallback(MyGUI::ComboBox* _sender, size_t _index);


			void updatePathList(boost::filesystem::path my_path);

			//static int nDialogs;
			//int mDialogIndex;
			//void (GUIManager::*mUserMethod)(const std::string&, const std::string&);

			std::string mName;
			std::string mUserString;

			// user callback
			boost::function<void(FileSelectionDialog*, const std::string&)> mUserMethod;

			// widgets
			MyGUI::Window* mWindow;
			MyGUI::List* mPathList;
			MyGUI::Edit* mFileEdit;
			MyGUI::Button* mButton;
			MyGUI::ComboBox* mRecentCombo;
			MyGUI::ComboBox* mExtensionCombo;

			boost::filesystem::path mCurrentPath;   // current viewed directory
			std::vector<boost::filesystem::path> mPath_set;      // list of paths in the viewed directory
			boost::filesystem::path* mSelectedFile;
			std::vector<boost::filesystem::path> mRecentPaths;
		};
	}
}
#endif   // DIALOG_H