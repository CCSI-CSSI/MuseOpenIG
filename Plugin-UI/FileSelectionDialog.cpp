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
//#* Original code borrowed from here 
//#* http://www.ogre3d.org/addonforums/viewtopic.php?f=17&t=14492
//#*****************************************************************************

#include "FileSelectionDialog.h"
using namespace boost::filesystem;

using namespace OpenIG::Plugins;

//int Dialog::nDialogs = 0;

FileSelectionDialog::FileSelectionDialog(const std::string& name) :
mName(name),
mWindow(0),
mPathList(0),
mFileEdit(0),
mButton(0),
mRecentCombo(0),
mExtensionCombo(0),
mSelectedFile(0),
mUserMethod(0)
{
	buildGUI();
}

//==========================================================================
// Useful after after deleted all widgets from scene
//==========================================================================
void FileSelectionDialog::buildGUI(){
	// load dialog layout
	MyGUI::VectorWidgetPtr widgets = MyGUI::LayoutManager::getInstance().loadLayout("FileDialog.layout");

	mWindow = (MyGUI::Window*)*widgets.begin();
	mWindow->eventWindowButtonPressed += MyGUI::newDelegate(this, &FileSelectionDialog::windowCloseCallback);
	mWindow->setVisible(false);

	mPathList = (MyGUI::List*)mWindow->findWidget("List");
	mPathList->eventListSelectAccept += MyGUI::newDelegate(this, &FileSelectionDialog::itemAcceptedCallback);
	mPathList->eventListChangePosition += MyGUI::newDelegate(this, &FileSelectionDialog::itemSelectedCallback);

	mFileEdit = (MyGUI::Edit*)mWindow->findWidget("Edit");

	mButton = (MyGUI::Button*)mWindow->findWidget("Confirm");
	mButton->eventMouseButtonClick += MyGUI::newDelegate(this, &FileSelectionDialog::endDialogCallback);

	mRecentCombo = (MyGUI::ComboBox*)mWindow->findWidget("Recent");
	mRecentCombo->eventComboChangePosition += MyGUI::newDelegate(this, &FileSelectionDialog::recentPathSelectedCallback);

	mExtensionCombo = (MyGUI::ComboBox*)mWindow->findWidget("Extension");
	mExtensionCombo->eventComboChangePosition += MyGUI::newDelegate(this, &FileSelectionDialog::extensionChangeCallback);
	mExtensionCombo->addItem("All files \".\"");
	mExtensionCombo->setIndexSelected(0);
}

void FileSelectionDialog::centerOnScreen(unsigned width, unsigned height)
{
	if (mWindow)
	{
		MyGUI::IntSize size = mWindow->getSize();
		MyGUI::IntPoint pos = mWindow->getPosition();

		pos.left = width / 2 - size.width / 2;
		pos.top = height / 2 - size.height / 2;

		mWindow->setPosition(pos);
	}
}

//==========================================================================
// User closed Dialog window
//==========================================================================
void FileSelectionDialog::windowCloseCallback(MyGUI::Window* _sender, const std::string& _name)
{
	mWindow->setVisible(false);
}

//==========================================================================
// User selected a path (double-clicked)
//==========================================================================
void FileSelectionDialog::itemAcceptedCallback(MyGUI::ListBox* _sender, size_t _index)
{
	if (_index == 0) // user clicked on the "[...]", so back up a directory
	{
		updatePathList(mCurrentPath.parent_path().string());
	}

	// it's a folder, open it
	else if (is_directory(mPath_set[_index - 1]))
	{
		updatePathList(mPath_set[_index - 1]);
	}

	// it's a file, notify user and quit
	else
	{
		endDialogCallback(0);
	}
}

//==========================================================================
// User selected a path (single click)
//==========================================================================
void FileSelectionDialog::itemSelectedCallback(MyGUI::ListBox* _sender, size_t _index)
{
	//TODO fix bug?: if in filelist, file / folder selected and then click to list box to empty line, calls this with _index = int.max(?)
	if (_index > mPath_set.size())
		return;

	// make sure it's a file
	if (_index != 0 && !is_directory(mPath_set[_index - 1]))
	{
		// mark as selected
		mSelectedFile = &mPath_set[_index - 1];
		mFileEdit->setCaption(mPathList->getItemNameAt(_index));
	}
}

//==========================================================================
// User re-opened a recently visited folder
//==========================================================================
void FileSelectionDialog::recentPathSelectedCallback(MyGUI::ComboBox* _sender, size_t _index)
{
	updatePathList(mRecentPaths[_index].parent_path());
}

//==========================================================================
// Dialog's closed
//==========================================================================
void FileSelectionDialog::endDialogCallback(MyGUI::Widget* _sender)
{
	// last minute fix  --forgot to account for the user entering the filename himself
	// Note that since the Dialog has no idea if the user should be creating a new file, it doesn't check if the
	// file exists or not. That's your job.

	std::string filename = mFileEdit->getCaption();

	// make sure it has an extension
	if (mExtensionCombo->getIndexSelected() != 0)
	{
		std::string extension = mExtensionCombo->getCaption();
		if (filename.length() > extension.length())
		{
			if (filename.compare(filename.length() - extension.length(), extension.length(), extension))
				filename.append(extension);
		}
		else
			filename.append(extension);
	}

	mSelectedFile = new path(mPath_set[0].parent_path().append(filename.begin(), filename.end()));
	//--

	if (mSelectedFile)
	{
		// notify user of selected file (empty string if none)
		if (mUserMethod)
		{
			mUserMethod(this, mSelectedFile->string());
		}

		// update recently visited paths
		bool newEntry = true;
		for (size_t i = 0; i < mRecentPaths.size(); i++)
		{
			if (*mSelectedFile == mRecentPaths[i])
			{
				newEntry = false;
				break;
			}
		}

		if (newEntry)
		{
			mRecentPaths.push_back(*mSelectedFile);
			mRecentCombo->addItem(mSelectedFile->parent_path().string());
		}

		// close dialog
		mWindow->setVisible(false);
	}

	// follow up of the fix
	delete mSelectedFile;
	mSelectedFile = 0;
}

//==========================================================================
// called when a new extension is set
//==========================================================================
void FileSelectionDialog::extensionChangeCallback(MyGUI::ComboBox* _sender, size_t _index)
{
	update();
}

//==========================================================================
// Set callback for when a file is selected
//==========================================================================
void FileSelectionDialog::eventEndDialog(boost::function<void(FileSelectionDialog*, const std::string&)> method)
{
	mUserMethod = method;
}

//==========================================================================
// get dialog name
//==========================================================================
const std::string& FileSelectionDialog::getName()
{
	return mName;
}

//==========================================================================
// Must be called once dialog is initialized with the functions below
//==========================================================================
void FileSelectionDialog::update()
{
	updatePathList(current_path());
}

//==========================================================================
// show/hide dialog
//==========================================================================
void FileSelectionDialog::setVisible(bool set)
{
	mWindow->setVisible(set);
}

bool FileSelectionDialog::isVisible() const
{
	return mWindow->getVisible();
}

//==========================================================================
// set dialog window title
//==========================================================================
void FileSelectionDialog::setDialogTitle(const std::string& title)
{
	mWindow->setCaption(title);
}

//==========================================================================
// set button caption (ie: save, load, save as..)
//==========================================================================
void FileSelectionDialog::setButtonCaption(const std::string& caption)
{
	mButton->setCaption(caption);
}

//==========================================================================
// Attaches a string to this dialog that can be retrieved later.
// Handy for using the same Dialog for different things
//==========================================================================
void FileSelectionDialog::setUserString(const std::string& value)
{
	mUserString = value;
}

//==========================================================================
// Return user string
//==========================================================================
const std::string& FileSelectionDialog::getUserString()
{
	return mUserString;
}

//==========================================================================
// Add an extension to the extension combobox
//==========================================================================
void FileSelectionDialog::addFileExtension(const std::string& ext)
{
	mExtensionCombo->addItem(ext);
	mExtensionCombo->setIndexSelected(0);   // MyGUI appears to reset 
	// the selected item to ITEM_NONE
	// whenever a new item is added
}

//==========================================================================
// If this isn't called, the default is show all files.
//==========================================================================
void FileSelectionDialog::setDefaultExtension(int index)
{
	mExtensionCombo->setIndexSelected(index + 1);
}

//==========================================================================
// Sort paths alphabetically (folders first, files last)
//==========================================================================
bool sortFunction(boost::filesystem::path p1, boost::filesystem::path p2)
{
	bool d1 = is_directory(p1);
	bool d2 = is_directory(p2);

	if (d1 && d2)
		return (p1.filename() < p2.filename());
	else if (d1)
		return true;
	else if (d2)
		return false;
	else
		return (p1.filename() < p2.filename());
}

//==========================================================================
// Update the paths in the dialog
// my_path; parent directory
// Note: this function is slow. Mainly because i'm lazy.
//==========================================================================
void FileSelectionDialog::updatePathList(path my_path)
{
	mCurrentPath = my_path;

	mPathList->removeAllItems();
	mPath_set.clear();

	mPathList->addItem("[...]");

	std::vector<path> temp_paths;

	try
	{
		copy(directory_iterator(mCurrentPath), directory_iterator(), back_inserter(temp_paths));
	}

	catch (const filesystem_error&)
	{
		// the user probably backed up too far. Jackass.
		mCurrentPath = current_path().parent_path();
		copy(directory_iterator(mCurrentPath), directory_iterator(), back_inserter(temp_paths));
	}

	// some shitty OSs don't sort paths..
	sort(temp_paths.begin(), temp_paths.end(), sortFunction);

	// extension
	std::string ext = mExtensionCombo->getItemNameAt(mExtensionCombo->getIndexSelected());

	for (size_t i = 0; i < temp_paths.size(); i++)
	{
		// folder
		if (is_directory(temp_paths[i]))
		{
			mPath_set.push_back(temp_paths[i]);
			mPathList->addItem(MyGUI::UString("[") + MyGUI::UString(temp_paths[i].filename().c_str()) + ']');
		}

		// file
		else
		{
			// filter to extension (if any)
			if (mExtensionCombo->getIndexSelected() != 0)
			{
				if (temp_paths[i].extension() == ext)
				{
					MyGUI::UString path = temp_paths[i].filename().c_str();
					mPathList->addItem(path);
					mPath_set.push_back(temp_paths[i]);
				}
			}

			else
			{
				MyGUI::UString path = temp_paths[i].filename().c_str();
				mPathList->addItem(path);
				mPath_set.push_back(temp_paths[i]);
			}
		}
	}
}