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
#include <Core-Base/stringutils.h>
#include <Core-Base/commands.h>
#include <Core-Base/filesystem.h>

#include <Core-OpenIG/openig.h>

#include <osg/Texture2D>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osg/Referenced>

#include <osgDB/ReadFile>
#include <osgDB/XmlParser>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>

#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>

#include <map>
#include <vector>
#include <string>

#include <boost/thread.hpp>
#include <boost/filesystem.hpp>

#include "MYGUIManager.h"
#include "FileSelectionDialog.h"
#include "ColourPanel.h"

namespace OpenIG { namespace Plugins { 

class UIPlugin : public PluginBase::Plugin
{
public:

	static unsigned ScreenWidth;

	struct LightPointDefinition : osg::Referenced
	{
		std::string name;
		bool		always_on;
		float		minPixelSize;
		float		maxPixelSize;
		float		intensity;
		float		radius;
		float		brightness;
		float		range;
		float		mult;
		bool		sprites;
		std::string	texture;
		bool		fplus;
	};
	typedef osg::ref_ptr<LightPointDefinition>					LightPointDefinitionPtr;
	typedef std::map< std::string, LightPointDefinitionPtr >	LightPointDefinitions;

	struct TOD
	{
		float	on;
		float	off;
		TOD() : on(17), off(6) {}
	};

	struct Brightness
	{
		float	day;
		float	night;
		bool	enabled;
		Brightness() : day(0.01), night(1.0), enabled(true) {}
	};

	class CustomMYGUIManager : public MYGUIManager
	{
	public:
		CustomMYGUIManager(Base::ImageGenerator* ig, const std::string& rootMedia, UIPlugin* plugin)
			: MYGUIManager(rootMedia)
			, _ig(ig)
			, _dialog(0)
			, _textureFileDialog(0)
			, _colorDialog(0)
			, _plugin(plugin)
			, _lightingTab(0)
		{

		}

		virtual void setupResources()
		{
			MYGUIManager::setupResources();
			_platform->getDataManagerPtr()->addResourceLocation(_rootMedia + "/Demos/Demo_Themes", false);
			_platform->getDataManagerPtr()->addResourceLocation(_rootMedia + "/Common/Demos", false);
			_platform->getDataManagerPtr()->addResourceLocation(_rootMedia + "/Common/Themes", false);
		}

		virtual void initializeControls()
		{
			static bool initialized = false;
			if (!initialized)
			{
				MyGUI::PointerManager::getInstance().setVisible(false);
				MyGUI::ResourceManager::getInstance().load("MyGUI_BlackBlueTheme.xml");

				createControls();

				initialized = true;
			}
		}

		void notifyListChangeScroll(MyGUI::ListBox* _sender, size_t _position)
		{
			if (_sender == _commandsDescriptionsListBox)
			{
				_commandsListBox->setScrollPosition(_position);
			}
			else
			if (_sender == _commandsListBox)
			{
				_commandsDescriptionsListBox->setScrollPosition(_position);
			}
		}
		void notifyListChangePosition(MyGUI::ListBox* _sender, size_t _index)
		{
			if (_sender == _commandsDescriptionsListBox)
			{
				_commandsListBox->setIndexSelected(_index);
			}
			else
			if (_sender == _commandsListBox)
			{
				_commandsDescriptionsListBox->setIndexSelected(_index);
			}

			_commandTextBox->setCaption(_commandsListBox->getItem(_index));

			Base::Commands::CommandPtr usage = *_commandsListBox->getItemDataAt<Base::Commands::CommandPtr>(_index, false);
			if (usage.valid())
			{
				_commandUsageTextBox->setCaption(MyGUI::UString("Arguments: ") + MyGUI::UString(usage->getUsage().c_str()));

				std::string format = usage->getArgumentsFormat();

				_fileButton->setEnabled(format.find("F") != std::string::npos);
				_colorButton->setEnabled(format.find("C") != std::string::npos);
			}

			_commandArgumentsEditBox->setOnlyText("");
		}

		

		void createCommandsTab()
		{
			MyGUI::TabItem* tab = _tabRoot->addItem("Commands");

			MyGUI::IntCoord coord;
			MyGUI::Align	align = MyGUI::Align::Stretch;

			coord.set(10, 10, 200, _size.y()-80);
			align = MyGUI::Align::Stretch;

			_commandsListBox = MyGUI::Gui::getInstance().createWidget<MyGUI::ListBox>("ListBox", coord, align, "Overlapped", "CommandListBox");
			_commandsListBox->attachToWidget(tab);
			_commandsListBox->eventListChangeScroll += MyGUI::newDelegate(this, &CustomMYGUIManager::notifyListChangeScroll);
			_commandsListBox->eventListChangePosition += MyGUI::newDelegate(this, &CustomMYGUIManager::notifyListChangePosition);

			size_t index = 0;
			const Base::Commands::CommandsMap& commands = Base::Commands::instance()->getCommands();
			Base::Commands::CommandsMapConstIterator itr = commands.begin();
			for (; itr != commands.end(); ++itr)
			{

				std::string command = itr->first;
				_commandsListBox->addItem(command);
				_commandsListBox->setItemDataAt(index++, itr->second);
			}

			coord.set(205, 10, _size.x()-80-150, _size.y() - 80);
			_commandsDescriptionsListBox = MyGUI::Gui::getInstance().createWidget<MyGUI::ListBox>("ListBox", coord, align, "Overlapped", "DescListBox");
			_commandsDescriptionsListBox->attachToWidget(tab);
			_commandsDescriptionsListBox->eventListChangeScroll += MyGUI::newDelegate(this, &CustomMYGUIManager::notifyListChangeScroll);
			_commandsDescriptionsListBox->eventListChangePosition += MyGUI::newDelegate(this, &CustomMYGUIManager::notifyListChangePosition);

			itr = commands.begin();
			for (; itr != commands.end(); ++itr)
			{
				std::string desc = itr->second->getDescription();
				std::string::size_type pos = desc.find_first_of('\n');
				if (pos != std::string::npos)
				{
					desc = desc.substr(0, pos);
				}

				_commandsDescriptionsListBox->addItem(desc);
			}

			coord.set(10, _size.y() - 65, 200, 20);
			align = MyGUI::Align::Stretch;

			MyGUI::TextBox* commandText = MyGUI::Gui::getInstance().createWidget<MyGUI::TextBox>("TextBox", coord, align, "Overlapped", "CommandText");
			commandText->setCaption("Command:");
			commandText->attachToWidget(tab);

			coord.set(205, _size.y() - 65, _size.x() - 80 - 150, 20);
			align = MyGUI::Align::Stretch;

			_commandUsageTextBox = MyGUI::Gui::getInstance().createWidget<MyGUI::TextBox>("TextBox", coord, align, "Overlapped", "CommandArgumentsText");
			_commandUsageTextBox->setCaption("Arguments: ");
			_commandUsageTextBox->attachToWidget(tab);

			coord.set(10, _size.y() - 50, 200, 20);
			align = MyGUI::Align::Stretch;

			_commandTextBox = MyGUI::Gui::getInstance().createWidget<MyGUI::TextBox>("TextBox", coord, align, "Overlapped", "Command");
			_commandTextBox->setCaption("none");
			_commandTextBox->attachToWidget(tab);

			coord.set(205, _size.y() - 50, _size.x() - 80 - 150 - 235, 20);
			align = MyGUI::Align::Stretch;

			_commandArgumentsEditBox = MyGUI::Gui::getInstance().createWidget<MyGUI::EditBox>("EditBox", coord, align, "Overlapped", "Arguments");
			_commandArgumentsEditBox->attachToWidget(tab);
			_commandArgumentsEditBox->setMaxTextLength(1000);
			_commandArgumentsEditBox->setColour(MyGUI::Colour::Black);

			coord.set(_size.x() - 230, _size.y() - 50, 65, 20);
			align = MyGUI::Align::Stretch;

			_fileButton = MyGUI::Gui::getInstance().createWidget<MyGUI::Button>("Button", coord, align, "Overlapped", "File");
			_fileButton->attachToWidget(tab);
			_fileButton->setCaption("File");
			_fileButton->eventMouseButtonPressed += MyGUI::newDelegate(this, &CustomMYGUIManager::notifyFileButtonClick);

			coord.set(_size.x() - 160, _size.y() - 50, 65, 20);
			align = MyGUI::Align::Stretch;

			_colorButton = MyGUI::Gui::getInstance().createWidget<MyGUI::Button>("Button", coord, align, "Overlapped", "Color");
			_colorButton->attachToWidget(tab);
			_colorButton->setCaption("Color");
			_colorButton->eventMouseButtonPressed += MyGUI::newDelegate(this, &CustomMYGUIManager::notifyColorButtonClick);

			coord.set(_size.x() - 90, _size.y() - 50, 65, 20);
			align = MyGUI::Align::Stretch;

			MyGUI::Button* button = MyGUI::Gui::getInstance().createWidget<MyGUI::Button>("Button", coord, align, "Overlapped", "Execute");
			button->attachToWidget(tab);
			button->setCaption("Execute");
			button->eventMouseButtonPressed += MyGUI::newDelegate(this, &CustomMYGUIManager::notifyExecuteButtonClick);

		}

		void notifyColorButtonClick(MyGUI::Widget* _sender, int _left, int _top, MyGUI::MouseButton _id)
		{
			if (_colorDialog)
			{
				bool visible = _colorDialog->isVisible();
				_colorDialog->setVisible(!visible);
			}
		}

		void notifyExecuteButtonClick(MyGUI::Widget* _sender, int _left, int _top, MyGUI::MouseButton _id)
		{
			std::string command = _commandTextBox->getCaption();
			std::string arguments = _commandArgumentsEditBox->getOnlyText();

			if (!command.empty() && !arguments.empty())
			{
				std::ostringstream oss;
				oss << command << " " << arguments;

				Base::Commands::instance()->exec(oss.str());
			}
		}

		void notifyColourAccept(demo::ColourPanel* _sender)
		{
			_colorDialog->setVisible(false);

			if (_commandsListBox->getIndexSelected() == MyGUI::ITEM_NONE) return;

			Base::Commands::CommandPtr command = *_commandsListBox->getItemDataAt<Base::Commands::CommandPtr>(
				_commandsListBox->getItemIndexSelected(), false
				);
			if (command.valid())
			{
				const std::string format = command->getArgumentsFormat();
				const std::string cmd = _commandArgumentsEditBox->getOnlyText();

				Base::StringUtils::Tokens cmdTokens = Base::StringUtils::instance()->tokenize(cmd);
				Base::StringUtils::Tokens formatTokens = Base::StringUtils::instance()->tokenize(format, ":");

				MyGUI::Colour color = _colorDialog->getColour();

				std::ostringstream oss;
				for (size_t i = 0; i < cmdTokens.size(); ++i)
				{
					if (i < formatTokens.size())
					{
						std::string token = formatTokens.at(i);
						if (token.at(0) == 'C')
						{
							oss << " " << std::setprecision(5) << color.red << " " << color.green  << " " << color.blue ;
							i += 2;
							continue;
						}
					}
					oss << " " << cmdTokens.at(i);
				}
				if (cmdTokens.size() < formatTokens.size() && formatTokens.at(cmdTokens.size()).at(0) == 'C')
				{
					oss << " " << std::setprecision(5) << color.red << " " << color.green  << " " << color.blue;
				}
				_commandArgumentsEditBox->setOnlyText(oss.str().c_str());
			}
		}

		void notifyFileButtonClick(MyGUI::Widget* _sender, int _left, int _top, MyGUI::MouseButton _id)
		{

			if (_dialog == 0)
			{
				osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
				if (!wsi)
				{
					osg::notify(osg::NOTICE) << "Error, no WindowSystemInterface available, cannot create windows." << std::endl;
					return;
				}

				unsigned int screen_width, screen_height;
				wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(0), screen_width, screen_height);

				screen_width = osg::minimum(screen_width, UIPlugin::ScreenWidth);

				_dialog = new FileSelectionDialog("Select File");
				_dialog->centerOnScreen(screen_width, screen_height);
				_dialog->setVisible(true);
				_dialog->setDialogTitle("Select file");
				_dialog->setButtonCaption("Select");
				_dialog->addFileExtension(".osg");
				_dialog->addFileExtension(".osgb");
				_dialog->addFileExtension(".flt");
				_dialog->addFileExtension(".obj");

				
				// callback
				boost::function<void(FileSelectionDialog*, const std::string&)> my_function = boost::bind(&CustomMYGUIManager::selectDialogCallback, this, _1, _2);
				_dialog->eventEndDialog(my_function);
				_dialog->update();
			}
			else
			{
				bool visible = _dialog->isVisible();
				_dialog->setVisible(!visible);
			}
		}

		void selectDialogCallback(FileSelectionDialog* dialog, const std::string& fileName)
		{
			if (_commandsListBox->getIndexSelected() == MyGUI::ITEM_NONE) return;

			Base::Commands::CommandPtr command = *_commandsListBox->getItemDataAt<Base::Commands::CommandPtr>(
				_commandsListBox->getItemIndexSelected(), false
			);
			if (command.valid())
			{
				const std::string format = command->getArgumentsFormat();
				const std::string cmd = _commandArgumentsEditBox->getOnlyText();

				Base::StringUtils::Tokens cmdTokens = Base::StringUtils::instance()->tokenize(cmd);
				Base::StringUtils::Tokens formatTokens = Base::StringUtils::instance()->tokenize(format, ":");

				std::ostringstream oss;
				for (size_t i = 0; i < cmdTokens.size(); ++i)
				{
					if (i < formatTokens.size())
					{
						std::string token = formatTokens.at(i);
						if (token.at(0) == 'F')
						{
							oss << " " << "\"" << fileName << "\"";
							continue;
						}
					}
					oss << " " << cmdTokens.at(i);
				}
				if (cmdTokens.size() < formatTokens.size() && formatTokens.at(cmdTokens.size()).at(0) == 'F')
				{
					oss << " " << "\"" << fileName << "\"";
				}
				_commandArgumentsEditBox->setOnlyText(oss.str().c_str());
			}
		}

		void createLightingTab()
		{
			MyGUI::TabItem* tab = _lightingTab = _tabRoot->addItem("Lighting");

			MyGUI::IntCoord coord;
			MyGUI::Align	align = MyGUI::Align::Stretch;

			float width = (_size.x() - 60 + 50) / 24.f;
			float top = 30.f;
			float height = _size.y() - 120;

			coord.set(10, top - 20, width * 5, 25);
			MyGUI::TextBox* text = MyGUI::Gui::getInstance().createWidget<MyGUI::TextBox>("TextBox", coord, align, "Overlapped", "text");
			text->attachToWidget(tab);
			text->setColour(MyGUI::Colour::Blue);
			text->setCaption("Name pattern");

			coord.set(10, top, width*5, height);
			_lists["pattern"] = MyGUI::Gui::getInstance().createWidget<MyGUI::ListBox>("ListBox", coord, align, "Overlapped", "pattern");
			_lists["pattern"]->attachToWidget(tab);

			coord.set(10+width*5, top - 20, width * 2, 25);
			text = MyGUI::Gui::getInstance().createWidget<MyGUI::TextBox>("TextBox", coord, align, "Overlapped", "text");
			text->attachToWidget(tab);
			text->setColour(MyGUI::Colour::Blue);
			text->setCaption("always_on");

			coord.set(10 + width * 5, top, width * 2, height);
			_lists["always_on"] = MyGUI::Gui::getInstance().createWidget<MyGUI::ListBox>("ListBox", coord, align, "Overlapped", "always_on");
			_lists["always_on"]->attachToWidget(tab);

			coord.set(10 + width * 7, top - 20, width, 25);
			text = MyGUI::Gui::getInstance().createWidget<MyGUI::TextBox>("TextBox", coord, align, "Overlapped", "text");
			text->attachToWidget(tab);
			text->setColour(MyGUI::Colour::Blue);
			text->setCaption("min Px.");

			coord.set(10 + width * 7, top, width, height);
			_lists["minPixelSize"] = MyGUI::Gui::getInstance().createWidget<MyGUI::ListBox>("ListBox", coord, align, "Overlapped", "minPixelSize");
			_lists["minPixelSize"]->attachToWidget(tab);

			coord.set(10 + width * 8, top - 20, width, 25);
			text = MyGUI::Gui::getInstance().createWidget<MyGUI::TextBox>("TextBox", coord, align, "Overlapped", "text");
			text->attachToWidget(tab);
			text->setColour(MyGUI::Colour::Blue);
			text->setCaption("max Px.");

			coord.set(10 + width * 8, top, width, height);
			_lists["maxPixelSize"] = MyGUI::Gui::getInstance().createWidget<MyGUI::ListBox>("ListBox", coord, align, "Overlapped", "maxPixelSize");
			_lists["maxPixelSize"]->attachToWidget(tab);

			coord.set(10 + width * 9, top - 20, width, 25);
			text = MyGUI::Gui::getInstance().createWidget<MyGUI::TextBox>("TextBox", coord, align, "Overlapped", "text");
			text->attachToWidget(tab);
			text->setColour(MyGUI::Colour::Blue);
			text->setCaption("intens.");

			coord.set(10 + width * 9, top, width, height);
			_lists["intensity"] = MyGUI::Gui::getInstance().createWidget<MyGUI::ListBox>("ListBox", coord, align, "Overlapped", "intensity");
			_lists["intensity"]->attachToWidget(tab);

			coord.set(10 + width * 10, top - 20, width, 25);
			text = MyGUI::Gui::getInstance().createWidget<MyGUI::TextBox>("TextBox", coord, align, "Overlapped", "text");
			text->attachToWidget(tab);
			text->setColour(MyGUI::Colour::Blue);
			text->setCaption("radius");

			coord.set(10 + width * 10, top, width, height);
			_lists["radius"] = MyGUI::Gui::getInstance().createWidget<MyGUI::ListBox>("ListBox", coord, align, "Overlapped", "radius");
			_lists["radius"]->attachToWidget(tab);

			coord.set(10 + width * 11, top - 20, width, 25);
			text = MyGUI::Gui::getInstance().createWidget<MyGUI::TextBox>("TextBox", coord, align, "Overlapped", "text");
			text->attachToWidget(tab);
			text->setColour(MyGUI::Colour::Blue);
			text->setCaption("bright.");

			coord.set(10 + width * 11, top, width, height);
			_lists["brightness"] = MyGUI::Gui::getInstance().createWidget<MyGUI::ListBox>("ListBox", coord, align, "Overlapped", "brightness");
			_lists["brightness"]->attachToWidget(tab);

			coord.set(10 + width * 12, top - 20, width, 25);
			text = MyGUI::Gui::getInstance().createWidget<MyGUI::TextBox>("TextBox", coord, align, "Overlapped", "text");
			text->attachToWidget(tab);
			text->setColour(MyGUI::Colour::Blue);
			text->setCaption("range");

			coord.set(10 + width * 12, top, width, height);
			_lists["range"] = MyGUI::Gui::getInstance().createWidget<MyGUI::ListBox>("ListBox", coord, align, "Overlapped", "range");
			_lists["range"]->attachToWidget(tab);

			coord.set(10 + width * 13, top - 20, width, 25);
			text = MyGUI::Gui::getInstance().createWidget<MyGUI::TextBox>("TextBox", coord, align, "Overlapped", "text");
			text->attachToWidget(tab);
			text->setColour(MyGUI::Colour::Blue);
			text->setCaption("mult.");

			coord.set(10 + width * 13, top, width, height);
			_lists["mult"] = MyGUI::Gui::getInstance().createWidget<MyGUI::ListBox>("ListBox", coord, align, "Overlapped", "mult");
			_lists["mult"]->attachToWidget(tab);

			coord.set(10 + width * 14, top - 20, width, 25);
			text = MyGUI::Gui::getInstance().createWidget<MyGUI::TextBox>("TextBox", coord, align, "Overlapped", "text");
			text->attachToWidget(tab);
			text->setColour(MyGUI::Colour::Blue);
			text->setCaption("sprites");

			coord.set(10 + width * 14, top, width * 2, height);
			_lists["sprites"] = MyGUI::Gui::getInstance().createWidget<MyGUI::ListBox>("ListBox", coord, align, "Overlapped", "sprites");
			_lists["sprites"]->attachToWidget(tab);

			coord.set(10 + width * 16, top - 20, width*6, 25);
			text = MyGUI::Gui::getInstance().createWidget<MyGUI::TextBox>("TextBox", coord, align, "Overlapped", "text");
			text->attachToWidget(tab);
			text->setColour(MyGUI::Colour::Blue);
			text->setCaption("texture");

			coord.set(10 + width * 16, top, width * 6, height);
			_lists["texture"] = MyGUI::Gui::getInstance().createWidget<MyGUI::ListBox>("ListBox", coord, align, "Overlapped", "texture");
			_lists["texture"]->attachToWidget(tab);

			coord.set(10 + width * 22, top, width, height);
			_lists["f+"] = MyGUI::Gui::getInstance().createWidget<MyGUI::ListBox>("ListBox", coord, align, "Overlapped", "f+");
			_lists["f+"]->attachToWidget(tab);

			coord.set(10 + width * 22, top - 20, width, 25);
			text = MyGUI::Gui::getInstance().createWidget<MyGUI::TextBox>("TextBox", coord, align, "Overlapped", "text");
			text->attachToWidget(tab);
			text->setColour(MyGUI::Colour::Blue);
			text->setCaption("f+");

			
			coord.set(_size.x() - 160, _size.y() - 50, 65, 20);
			align = MyGUI::Align::Stretch;

			MyGUI::Button* button = MyGUI::Gui::getInstance().createWidget<MyGUI::Button>("Button", coord, align, "Overlapped", "TextureFile");
			button->attachToWidget(tab);
			button->setCaption("File");
			button->eventMouseButtonPressed += MyGUI::newDelegate(this, &CustomMYGUIManager::notifyTextureFileButtonClick);

			coord.set(_size.x() - 90, _size.y() - 50, 65, 20);
			align = MyGUI::Align::Stretch;

			button = MyGUI::Gui::getInstance().createWidget<MyGUI::Button>("Button", coord, align, "Overlapped", "Update");
			button->attachToWidget(tab);
			button->setCaption("Update");
			button->eventMouseButtonPressed += MyGUI::newDelegate(this, &CustomMYGUIManager::notifyUpdateButtonClick);

			coord.set(_size.x() - 230, _size.y() - 50, 65, 20);
			align = MyGUI::Align::Stretch;

			button = MyGUI::Gui::getInstance().createWidget<MyGUI::Button>("Button", coord, align, "Overlapped", "Revert");
			button->attachToWidget(tab);
			button->setCaption("Revert");
			button->eventMouseButtonPressed += MyGUI::newDelegate(this, &CustomMYGUIManager::notifyRevertButtonClick);

			coord.set(10, _size.y() - 85, 800, 55);
			MyGUI::TabControl* tabControl = MyGUI::Gui::getInstance().createWidget<MyGUI::TabControl>("TabControl", coord, align, "Overlapped", "LightingTabControl");
			tabControl->attachToWidget(tab);

			{
				MyGUI::TabItem* tab = tabControl->addItem("Time of day on/off");

				coord.set(10, 5, 30, 20);
				MyGUI::TextBox* text = MyGUI::Gui::getInstance().createWidget<MyGUI::TextBox>("TextBox", coord, align, "Overlapped", "text");
				text->attachToWidget(tab);
				text->setColour(MyGUI::Colour::Blue);
				text->setCaption("on:");

				coord.set(10+30, 5, 65, 20);
				MyGUI::EditBox* edit = MyGUI::Gui::getInstance().createWidget<MyGUI::EditBox>("EditBox", coord, align, "Overlapped", "tod_on");
				edit->attachToWidget(tab);

				coord.set(10+30+65+5, 5, 30, 20);
				text = MyGUI::Gui::getInstance().createWidget<MyGUI::TextBox>("TextBox", coord, align, "Overlapped", "text");
				text->attachToWidget(tab);
				text->setColour(MyGUI::Colour::Blue);
				text->setCaption("off:");

				coord.set(10 + 30 + 65 + 5 + 30, 5, 65, 20);
				edit = MyGUI::Gui::getInstance().createWidget<MyGUI::EditBox>("EditBox", coord, align, "Overlapped", "tod_off");
				edit->attachToWidget(tab);

				tab = tabControl->addItem("Overall brightness");

				coord.set(10, 5, 60, 20);
				text = MyGUI::Gui::getInstance().createWidget<MyGUI::TextBox>("TextBox", coord, align, "Overlapped", "text");
				text->attachToWidget(tab);
				text->setColour(MyGUI::Colour::Blue);
				text->setCaption("enabled:");

				coord.set(10 + 60, 5, 65, 20);
				MyGUI::ComboBox* combo = MyGUI::Gui::getInstance().createWidget<MyGUI::ComboBox>("ComboBox", coord, align, "Overlapped", "lighting_brightness_enabled");
				combo->addItem("true");
				combo->addItem("false");
				combo->attachToWidget(tab);

				coord.set(10+60+65+5, 5, 30, 20);
				text = MyGUI::Gui::getInstance().createWidget<MyGUI::TextBox>("TextBox", coord, align, "Overlapped", "text");
				text->attachToWidget(tab);
				text->setColour(MyGUI::Colour::Blue);
				text->setCaption("day:");

				coord.set(10 + 60 + 65 + 30 + 5 + 5, 5, 65, 20);
				edit = MyGUI::Gui::getInstance().createWidget<MyGUI::EditBox>("EditBox", coord, align, "Overlapped", "lighting_brightness_day");
				edit->attachToWidget(tab);

				coord.set(10 + 60 + 65 + 30 + 5 + 5 + 65 + 5, 5, 35, 20);
				text = MyGUI::Gui::getInstance().createWidget<MyGUI::TextBox>("TextBox", coord, align, "Overlapped", "text");
				text->attachToWidget(tab);
				text->setColour(MyGUI::Colour::Blue);
				text->setCaption("night:");

				coord.set(10 + 60 + 65 + 30 + 5 + 5 + 65 + 35 + 5 + 5, 5, 65, 20);
				edit = MyGUI::Gui::getInstance().createWidget<MyGUI::EditBox>("EditBox", coord, align, "Overlapped", "lighting_brightness_night");
				edit->attachToWidget(tab);

				tab = tabControl->addItem("On clouds brightness");

				coord.set(10, 5, 30, 20);
				text = MyGUI::Gui::getInstance().createWidget<MyGUI::TextBox>("TextBox", coord, align, "Overlapped", "text");
				text->attachToWidget(tab);
				text->setColour(MyGUI::Colour::Blue);
				text->setCaption("day:");

				coord.set(10 + 30, 5, 65, 20);
				edit = MyGUI::Gui::getInstance().createWidget<MyGUI::EditBox>("EditBox", coord, align, "Overlapped", "lighting_on_clouds_day");
				edit->attachToWidget(tab);

				coord.set(10 + 30 + 65 + 5, 5, 35, 20);
				text = MyGUI::Gui::getInstance().createWidget<MyGUI::TextBox>("TextBox", coord, align, "Overlapped", "text");
				text->attachToWidget(tab);
				text->setColour(MyGUI::Colour::Blue);
				text->setCaption("night:");

				coord.set(10 + 30 + 65 + 5 + 35 + 5, 5, 65, 20);
				edit = MyGUI::Gui::getInstance().createWidget<MyGUI::EditBox>("EditBox", coord, align, "Overlapped", "lighting_on_clouds_night");
				edit->attachToWidget(tab);

				tab = tabControl->addItem("On water brightness");

				coord.set(10, 5, 30, 20);
				text = MyGUI::Gui::getInstance().createWidget<MyGUI::TextBox>("TextBox", coord, align, "Overlapped", "text");
				text->attachToWidget(tab);
				text->setColour(MyGUI::Colour::Blue);
				text->setCaption("day:");

				coord.set(10 + 30, 5, 65, 20);
				edit = MyGUI::Gui::getInstance().createWidget<MyGUI::EditBox>("EditBox", coord, align, "Overlapped", "lighting_on_water_day");
				edit->attachToWidget(tab);

				coord.set(10 + 30 + 65 + 5, 5, 35, 20);
				text = MyGUI::Gui::getInstance().createWidget<MyGUI::TextBox>("TextBox", coord, align, "Overlapped", "text");
				text->attachToWidget(tab);
				text->setColour(MyGUI::Colour::Blue);
				text->setCaption("night:");

				coord.set(10 + 30 + 65 + 5 + 35 + 5, 5, 65, 20);
				edit = MyGUI::Gui::getInstance().createWidget<MyGUI::EditBox>("EditBox", coord, align, "Overlapped", "lighting_on_water_night");
				edit->attachToWidget(tab);

			}
			updateLightsTab(_xmlFileName,_defs, _tod, _overallBrightness, _onCloudsBrightness, _onWaterBrightness);
		}

		void notifyRevertButtonClick(MyGUI::Widget* _sender, int _left, int _top, MyGUI::MouseButton _id)
		{
			if (_plugin && Base::FileSystem::fileExists(_xmlFileName + ".backup"))
			{
				_plugin->readFromXML(_xmlFileName + ".backup",true);
			}
		}

		void notifyUpdateButtonClick(MyGUI::Widget* _sender, int _left, int _top, MyGUI::MouseButton _id)
		{
			if (_lists.find("pattern") == _lists.end()) return;

			std::string TimeofDay_on;
			std::string TimeofDay_off;
			std::string LandingLightBrightness_enable;
			std::string LandingLightBrightness_day;
			std::string LandingLightBrightness_night;
			std::string LightBrightnessOnClouds_day;
			std::string LightBrightnessOnClouds_night;
			std::string LightBrightnessOnWater_day;
			std::string LightBrightnessOnWater_night;

			MyGUI::EditBox* edit = MyGUI::Gui::getInstance().findWidget<MyGUI::EditBox>("tod_on");
			if (edit) TimeofDay_on = edit->getOnlyText();

			edit = MyGUI::Gui::getInstance().findWidget<MyGUI::EditBox>("tod_off");
			if (edit) TimeofDay_off = edit->getOnlyText();
			
			MyGUI::ComboBox* combo = MyGUI::Gui::getInstance().findWidget<MyGUI::ComboBox>("lighting_brightness_enabled");
			if (combo) LandingLightBrightness_enable = combo->getIndexSelected() == 0 ? "true" : "false";
			
			edit = MyGUI::Gui::getInstance().findWidget<MyGUI::EditBox>("lighting_brightness_day");
			if (edit) LandingLightBrightness_day = edit->getOnlyText();
			
			edit = MyGUI::Gui::getInstance().findWidget<MyGUI::EditBox>("lighting_brightness_night");
			if (edit) LandingLightBrightness_night = edit->getOnlyText();
			
			edit = MyGUI::Gui::getInstance().findWidget<MyGUI::EditBox>("lighting_on_clouds_day");
			if (edit) LightBrightnessOnClouds_day = edit->getOnlyText();
			
			edit = MyGUI::Gui::getInstance().findWidget<MyGUI::EditBox>("lighting_on_clouds_night");
			if (edit) LightBrightnessOnClouds_night = edit->getOnlyText();
			
			edit = MyGUI::Gui::getInstance().findWidget<MyGUI::EditBox>("lighting_on_water_day");
			if (edit) LightBrightnessOnWater_day = edit->getOnlyText();
			
			edit = MyGUI::Gui::getInstance().findWidget<MyGUI::EditBox>("lighting_on_water_night");
			if (edit) LightBrightnessOnWater_night = edit->getOnlyText();
			
			osg::ref_ptr<osgDB::XmlNode> root = new osgDB::XmlNode;
			root->name = "OsgNodeSettings";
			root->type = osgDB::XmlNode::GROUP;

			{
				osg::ref_ptr<osgDB::XmlNode> child = new osgDB::XmlNode;
				
				child->type = osgDB::XmlNode::NODE;
				child->name = "TimeofDay";
				child->properties["on"] = TimeofDay_on;
				child->properties["off"] = TimeofDay_off;

				root->children.push_back(child);
			}
			{
				osg::ref_ptr<osgDB::XmlNode> child = new osgDB::XmlNode;

				child->type = osgDB::XmlNode::NODE;
				child->name = "LandingLightBrightness";
				child->properties["enable"] = LandingLightBrightness_enable;
				child->properties["day"] = LandingLightBrightness_day;
				child->properties["night"] = LandingLightBrightness_night;

				root->children.push_back(child);
			}
			{
				osg::ref_ptr<osgDB::XmlNode> child = new osgDB::XmlNode;

				child->type = osgDB::XmlNode::NODE;
				child->name = "LightBrightnessOnClouds";
				child->properties["day"] = LightBrightnessOnClouds_day;
				child->properties["night"] = LightBrightnessOnClouds_night;

				root->children.push_back(child);
			}
			{
				osg::ref_ptr<osgDB::XmlNode> child = new osgDB::XmlNode;

				child->type = osgDB::XmlNode::NODE;
				child->name = "LightBrightnessOnWater";
				child->properties["day"] = LightBrightnessOnWater_day;
				child->properties["night"] = LightBrightnessOnWater_night;

				root->children.push_back(child);
			}

			size_t numItems = _lists["pattern"]->getItemCount();
			for (size_t i = 0; i < numItems; ++i)
			{
				std::string pattern = _lists["pattern"]->getItemNameAt(i);
				std::string always_on;
				std::string minPixelSize;
				std::string maxPixelSize;
				std::string intensity;
				std::string radius;
				std::string brightness;
				std::string range;
				std::string mult;
				std::string sprites;
				std::string texture;
				std::string fplus;

				MyGUI::ComboBox* combo = dynamic_cast<MyGUI::ComboBox*>(_lists["always_on"]->getChildAt(i));
				if (combo) always_on = combo->getItemNameAt(combo->getIndexSelected());

				MyGUI::EditBox* edit = dynamic_cast<MyGUI::EditBox*>(_lists["minPixelSize"]->getChildAt(i));
				if (edit) minPixelSize = edit->getOnlyText();

				edit = dynamic_cast<MyGUI::EditBox*>(_lists["maxPixelSize"]->getChildAt(i));
				if (edit) maxPixelSize = edit->getOnlyText();

				edit = dynamic_cast<MyGUI::EditBox*>(_lists["intensity"]->getChildAt(i));
				if (edit) intensity = edit->getOnlyText();

				edit = dynamic_cast<MyGUI::EditBox*>(_lists["radius"]->getChildAt(i));
				if (edit) radius = edit->getOnlyText();

				edit = dynamic_cast<MyGUI::EditBox*>(_lists["brightness"]->getChildAt(i));
				if (edit) brightness = edit->getOnlyText();

				edit = dynamic_cast<MyGUI::EditBox*>(_lists["range"]->getChildAt(i));
				if (edit) range = edit->getOnlyText();

				edit = dynamic_cast<MyGUI::EditBox*>(_lists["mult"]->getChildAt(i));
				if (edit) mult = edit->getOnlyText();

				combo = dynamic_cast<MyGUI::ComboBox*>(_lists["sprites"]->getChildAt(i));
				if (combo) sprites = combo->getItemNameAt(combo->getIndexSelected());

				edit = dynamic_cast<MyGUI::EditBox*>(_lists["texture"]->getChildAt(i));
				if (edit) texture = edit->getOnlyText();

				combo = dynamic_cast<MyGUI::ComboBox*>(_lists["f+"]->getChildAt(i));
				if (combo) fplus = combo->getItemNameAt(combo->getIndexSelected());

				osg::ref_ptr<osgDB::XmlNode> child = new osgDB::XmlNode;

				child->type = osgDB::XmlNode::NODE;
				child->name = "LightPointNode";
				child->properties["name"]				= pattern;
				child->properties["always_on"]			= always_on;
				child->properties["minPixelSize"]		= minPixelSize;
				child->properties["maxPixelSize"]		= maxPixelSize;
				child->properties["intensity"]			= intensity;
				child->properties["radius"]				= radius;
				child->properties["brightness"]			= brightness;
				child->properties["range"]				= range;
				child->properties["minPixelSizeMultiplierForSprites"] = mult;
				child->properties["sprites"]			= sprites;
				child->properties["texture"]			= texture;
				child->properties["fplus"]				= fplus;

				root->children.push_back(child);

			}

			std::ofstream file;
			file.open(_xmlFileName.c_str(), std::ios::out);
			if (file.is_open())
			{
				osg::ref_ptr<osgDB::XmlNode> xml = new osgDB::XmlNode;
				xml->type = osgDB::XmlNode::ROOT;
				xml->children.push_back(root);
				xml->write(file);
				file.close();
			}
		}

		void notifyTextureFileButtonClick(MyGUI::Widget* _sender, int _left, int _top, MyGUI::MouseButton _id)
		{

			if (_textureFileDialog == 0)
			{
				osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
				if (!wsi)
				{
					osg::notify(osg::NOTICE) << "Error, no WindowSystemInterface available, cannot create windows." << std::endl;
					return;
				}

				unsigned int screen_width, screen_height;
				wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(0), screen_width, screen_height);

				screen_width = osg::minimum(screen_width, UIPlugin::ScreenWidth);

				_textureFileDialog = new FileSelectionDialog("Select File");
				_textureFileDialog->centerOnScreen(screen_width, screen_height);
				_textureFileDialog->setVisible(true);
				_textureFileDialog->setDialogTitle("Select file");
				_textureFileDialog->setButtonCaption("Select");
				_textureFileDialog->addFileExtension(".png");
				_textureFileDialog->addFileExtension(".tiff");
				_textureFileDialog->addFileExtension(".jpeg");
				_textureFileDialog->addFileExtension(".jpg");
				_textureFileDialog->addFileExtension(".bmp"); 
				_textureFileDialog->addFileExtension(".dds");


				// callback
				boost::function<void(FileSelectionDialog*, const std::string&)> my_function = boost::bind(&CustomMYGUIManager::selectTextureDialogCallback, this, _1, _2);
				_textureFileDialog->eventEndDialog(my_function);
				_textureFileDialog->update();
			}
			else
			{
				bool visible = _textureFileDialog->isVisible();
				_textureFileDialog->setVisible(!visible);
			}
		}

		void selectTextureDialogCallback(FileSelectionDialog* dialog, const std::string& fileName)
		{
			if (_lists.find("pattern") == _lists.end()) return;
			if (_lists["pattern"]->getIndexSelected() == MyGUI::ITEM_NONE) return;

			size_t index = _lists["pattern"]->getIndexSelected();
			std::string name = _lists["pattern"]->getItemNameAt(index);

			std::ostringstream oss;
			oss << "edit_" << name;

			MyGUI::EditBox* edit = MyGUI::Gui::getInstance().findWidget<MyGUI::EditBox>(oss.str().c_str());
			if (edit)
			{
				edit->setOnlyText(fileName);
			}
		}


		void createConfigTab()
		{
			MyGUI::TabItem* tab = _tabRoot->addItem("Config");
		}


		void createControls()
		{
			osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
			if (!wsi)
			{
				osg::notify(osg::NOTICE) << "Error, no WindowSystemInterface available, cannot create windows." << std::endl;
				return;
			}

			unsigned int screen_width, screen_height;
			wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(0), screen_width, screen_height);

			screen_width = osg::minimum(screen_width, UIPlugin::ScreenWidth);

			_size.x() = screen_width - 120;
			_size.y() = screen_height - 160;

			MyGUI::IntCoord coord;
			coord.set(60, 80, screen_width - 120, screen_height-160);

			MyGUI::Align align = MyGUI::Align::Left;

			_tabRoot = MyGUI::Gui::getInstance().createWidget<MyGUI::TabControl>("TabControl", coord, align, "Overlapped", "Main");

			_root = _tabRoot;
			setOverallAlpha(0.8);

			createCommandsTab();
			createLightingTab();
			//createConfigTab();

			_tabRoot->selectSheetIndex(1);

			if (_colorDialog == 0)
			{
				_colorDialog = new demo::ColourPanel();
				_colorDialog->eventColourAccept = MyGUI::newDelegate(this, &CustomMYGUIManager::notifyColourAccept);
				_colorDialog->setVisible(false);
				_colorDialog->centerOnScreen(screen_width, screen_height);
			}
		}

		void notifyMouseButtonPressed(MyGUI::Widget* _sender, int _left, int _top, MyGUI::MouseButton _id)
		{
			notifyMouseSetFocus(_sender, 0);
		}

		void notifyMouseSetFocus(MyGUI::Widget* _sender, MyGUI::Widget* _old)
		{
			try
			{
				LightPointDefinitionPtr def = *_sender->getUserData<LightPointDefinitionPtr>();
				if (!def.valid()) return;

				for (size_t i = 0; i < _lists["pattern"]->getItemCount(); ++i)
				{
					std::string name = _lists["pattern"]->getItemNameAt(i);
					if (name == def->name)
					{
						_lists["pattern"]->setIndexSelected(i);
						break;
					}
				}
				
			}
			catch (const std::exception&)
			{

			}
		}

		TOD						_tod;
		Brightness				_overallBrightness;
		Brightness				_onCloudsBrightness;
		Brightness				_onWaterBrightness;
		std::string				_xmlFileName;

		

		void updateLightsTab(const std::string& xmlFileName, const LightPointDefinitions& defs, 
							TOD& tod, Brightness& overall, Brightness& onClouds, Brightness& onWater, bool fromBackup = false)
		{
			MyGUI::ListBox* list = _lists["pattern"];
			if (!list)
			{
				_defs = defs;

				if (!fromBackup) _xmlFileName = xmlFileName;
				
				_tod = tod;
				_overallBrightness = overall;
				_onCloudsBrightness = onClouds;
				_onWaterBrightness = onWater;
				return;
			}

			_defs = defs;
			_tod = tod;
			_overallBrightness = overall;
			_onCloudsBrightness = onClouds;
			_onWaterBrightness = onWater;

			ListBoxes::iterator itr = _lists.begin();
			for (; itr != _lists.end(); ++itr)
			{
				MyGUI::ListBox* list = itr->second;
				if (!list) continue;

				if (itr->first != "pattern")
				{
					MyGUI::EnumeratorWidgetPtr enumerator = list->getEnumerator();
					MyGUI::Gui::getInstance().destroyWidgets(enumerator);
				}
				else
					list->removeAllItems();
			}
			
			
			unsigned index = 0;
			for (LightPointDefinitions::const_iterator itr = _defs.begin();
				itr != _defs.end(); 
				++itr)
			{
				const LightPointDefinitionPtr& def = itr->second;

				_lists["pattern"]->addItem(def->name);

				float height = 21.f;

				MyGUI::Align	align = MyGUI::Align::Stretch;
				MyGUI::IntCoord coord;
				float width = _lists["always_on"]->getAbsoluteCoord().width;
				coord.set(0, 0 + index, width, height);
				MyGUI::ComboBox* combo = MyGUI::Gui::getInstance().createWidget<MyGUI::ComboBox>("ComboBox", coord, align, "Overlapped", "combo");

				combo->attachToWidget(_lists["always_on"]);
				combo->setComboModeDrop(true);
				combo->addItem("true");
				combo->addItem("false");
				combo->setIndexSelected(def->always_on ? 0 : 1);
				combo->setNeedMouseFocus(true);
				combo->setUserData(itr->second);
				combo->eventMouseButtonPressed += MyGUI::newDelegate(this, &CustomMYGUIManager::notifyMouseButtonPressed);

				{
					std::ostringstream oss;
					oss << def->minPixelSize;

					float width = _lists["minPixelSize"]->getAbsoluteCoord().width;
					coord.set(0, 0 + index, width, height);
					MyGUI::EditBox* edit = MyGUI::Gui::getInstance().createWidget<MyGUI::EditBox>("EditBox", coord, align, "Overlapped", "edit");
					edit->attachToWidget(_lists["minPixelSize"]);
					edit->setOnlyText(oss.str().c_str());
					edit->setUserData(itr->second);
					edit->setNeedMouseFocus(true);
					edit->eventMouseButtonPressed += MyGUI::newDelegate(this, &CustomMYGUIManager::notifyMouseButtonPressed);
				}

				{
					std::ostringstream oss;
					oss << def->maxPixelSize;

					float width = _lists["maxPixelSize"]->getAbsoluteCoord().width;
					coord.set(0, 0 + index, width, height);
					MyGUI::EditBox* edit = MyGUI::Gui::getInstance().createWidget<MyGUI::EditBox>("EditBox", coord, align, "Overlapped", "edit");
					edit->attachToWidget(_lists["maxPixelSize"]);
					edit->setOnlyText(oss.str().c_str());
					edit->setUserData(itr->second);
					edit->setNeedMouseFocus(true);
					edit->eventMouseButtonPressed += MyGUI::newDelegate(this, &CustomMYGUIManager::notifyMouseButtonPressed);
				}

				{
					std::ostringstream oss;
					oss << def->intensity;

					float width = _lists["intensity"]->getAbsoluteCoord().width;
					coord.set(0, 0 + index, width, height);
					MyGUI::EditBox* edit = MyGUI::Gui::getInstance().createWidget<MyGUI::EditBox>("EditBox", coord, align, "Overlapped", "edit");
					edit->attachToWidget(_lists["intensity"]);
					edit->setOnlyText(oss.str().c_str());
					edit->setUserData(itr->second);
					edit->setNeedMouseFocus(true);
					edit->eventMouseButtonPressed += MyGUI::newDelegate(this, &CustomMYGUIManager::notifyMouseButtonPressed);
				}

				{
					std::ostringstream oss;
					oss << def->radius;

					float width = _lists["radius"]->getAbsoluteCoord().width;
					coord.set(0, 0 + index, width, height);
					MyGUI::EditBox* edit = MyGUI::Gui::getInstance().createWidget<MyGUI::EditBox>("EditBox", coord, align, "Overlapped", "edit");
					edit->attachToWidget(_lists["radius"]);
					edit->setOnlyText(oss.str().c_str());
					edit->setUserData(itr->second);
					edit->setNeedMouseFocus(true);
					edit->eventMouseButtonPressed += MyGUI::newDelegate(this, &CustomMYGUIManager::notifyMouseButtonPressed);
				}

				{
					std::ostringstream oss;
					oss << def->brightness;

					float width = _lists["brightness"]->getAbsoluteCoord().width;
					coord.set(0, 0 + index, width, height);
					MyGUI::EditBox* edit = MyGUI::Gui::getInstance().createWidget<MyGUI::EditBox>("EditBox", coord, align, "Overlapped", "edit");
					edit->attachToWidget(_lists["brightness"]);
					edit->setOnlyText(oss.str().c_str());
					edit->setUserData(itr->second);
					edit->setNeedMouseFocus(true);
					edit->eventMouseButtonPressed += MyGUI::newDelegate(this, &CustomMYGUIManager::notifyMouseButtonPressed);
				}

				{
					std::ostringstream oss;
					oss << def->range;

					float width = _lists["range"]->getAbsoluteCoord().width;
					coord.set(0, 0 + index, width, height);
					MyGUI::EditBox* edit = MyGUI::Gui::getInstance().createWidget<MyGUI::EditBox>("EditBox", coord, align, "Overlapped", "edit");
					edit->attachToWidget(_lists["range"]);
					edit->setOnlyText(oss.str().c_str());
					edit->setUserData(itr->second);
					edit->setNeedMouseFocus(true);
					edit->eventMouseButtonPressed += MyGUI::newDelegate(this, &CustomMYGUIManager::notifyMouseButtonPressed);
				}

				{
					std::ostringstream oss;
					oss << def->mult;

					float width = _lists["mult"]->getAbsoluteCoord().width;
					coord.set(0, 0 + index, width, height);
					MyGUI::EditBox* edit = MyGUI::Gui::getInstance().createWidget<MyGUI::EditBox>("EditBox", coord, align, "Overlapped", "mult");
					edit->attachToWidget(_lists["mult"]);
					edit->setOnlyText(oss.str().c_str());
					edit->setUserData(itr->second);
					edit->setNeedMouseFocus(true);
					edit->eventMouseButtonPressed += MyGUI::newDelegate(this, &CustomMYGUIManager::notifyMouseButtonPressed);
				}

				width = _lists["sprites"]->getAbsoluteCoord().width;
				coord.set(0, 0 + index, width, height);
				combo = MyGUI::Gui::getInstance().createWidget<MyGUI::ComboBox>("ComboBox", coord, align, "Overlapped", "combo");
				combo->attachToWidget(_lists["sprites"]);
				combo->setComboModeDrop(true);
				combo->addItem("true");
				combo->addItem("false");
				combo->setIndexSelected(def->sprites ? 0 : 1);
				combo->setUserData(itr->second);
				combo->setNeedMouseFocus(true);
				combo->eventMouseButtonPressed += MyGUI::newDelegate(this, &CustomMYGUIManager::notifyMouseButtonPressed);

				std::ostringstream oss;
				oss << "edit_" << itr->first;
				width = _lists["texture"]->getAbsoluteCoord().width;
				coord.set(0, 0 + index, width, height);
				MyGUI::EditBox* edit = MyGUI::Gui::getInstance().createWidget<MyGUI::EditBox>("EditBox", coord, align, "Overlapped", oss.str().c_str());
				edit->attachToWidget(_lists["texture"]);
				edit->setOnlyText(def->texture.c_str());
				edit->setUserData(itr->second);
				edit->setNeedMouseFocus(true);
				edit->eventMouseButtonPressed += MyGUI::newDelegate(this, &CustomMYGUIManager::notifyMouseButtonPressed);

				width = _lists["f+"]->getAbsoluteCoord().width;
				coord.set(0, 0 + index, width, height);
				combo = MyGUI::Gui::getInstance().createWidget<MyGUI::ComboBox>("ComboBox", coord, align, "Overlapped", "combo");
				combo->attachToWidget(_lists["f+"]);
				combo->setComboModeDrop(true);
				combo->addItem("true");
				combo->addItem("false");
				combo->setIndexSelected(def->fplus ? 0 : 1);
				combo->setUserData(itr->second);
				combo->setNeedMouseFocus(true);
				combo->eventMouseButtonPressed += MyGUI::newDelegate(this, &CustomMYGUIManager::notifyMouseButtonPressed);

				index += height;
			}

			{
				std::ostringstream oss;
				oss << std::setprecision(5) << _tod.on;
				MyGUI::EditBox* edit = MyGUI::Gui::getInstance().findWidget<MyGUI::EditBox>("tod_on");
				if (edit) edit->setOnlyText(oss.str().c_str());
			}
			{
				std::ostringstream oss;
				oss << std::setprecision(5) << _tod.off;
				MyGUI::EditBox* edit = MyGUI::Gui::getInstance().findWidget<MyGUI::EditBox>("tod_off");
				if (edit) edit->setOnlyText(oss.str().c_str());
			}
			{
				MyGUI::ComboBox* combo = MyGUI::Gui::getInstance().findWidget<MyGUI::ComboBox>("lighting_brightness_enabled");
				if (combo) combo->setIndexSelected(_overallBrightness.enabled ? 0 : 1);
			}
			{
				std::ostringstream oss;
				oss << std::setprecision(5) << _overallBrightness.day;
				MyGUI::EditBox* edit = MyGUI::Gui::getInstance().findWidget<MyGUI::EditBox>("lighting_brightness_day");
				if (edit) edit->setOnlyText(oss.str().c_str());
			}
			{
				std::ostringstream oss;
				oss << std::setprecision(5) << _overallBrightness.night;
				MyGUI::EditBox* edit = MyGUI::Gui::getInstance().findWidget<MyGUI::EditBox>("lighting_brightness_night");
				if (edit) edit->setOnlyText(oss.str().c_str());
			}
			{
				std::ostringstream oss;
				oss << std::setprecision(5) << _onCloudsBrightness.day;
				MyGUI::EditBox* edit = MyGUI::Gui::getInstance().findWidget<MyGUI::EditBox>("lighting_on_clouds_day");
				if (edit) edit->setOnlyText(oss.str().c_str());
			}
			{
				std::ostringstream oss;
				oss << std::setprecision(5) << _onCloudsBrightness.night;
				MyGUI::EditBox* edit = MyGUI::Gui::getInstance().findWidget<MyGUI::EditBox>("lighting_on_clouds_night");
				if (edit) edit->setOnlyText(oss.str().c_str());
			}
			{
				std::ostringstream oss;
				oss << std::setprecision(5) << _onWaterBrightness.day;
				MyGUI::EditBox* edit = MyGUI::Gui::getInstance().findWidget<MyGUI::EditBox>("lighting_on_water_day");
				if (edit) edit->setOnlyText(oss.str().c_str());
			}
			{
				std::ostringstream oss;
				oss << std::setprecision(5) << _onWaterBrightness.night;
				MyGUI::EditBox* edit = MyGUI::Gui::getInstance().findWidget<MyGUI::EditBox>("lighting_on_water_night");
				if (edit) edit->setOnlyText(oss.str().c_str());
			}
		}

		void cleanup()
		{
			try
			{
				if (_dialog) delete _dialog;
				_dialog = 0; 

				if (_textureFileDialog) delete _textureFileDialog;
				_textureFileDialog = 0;

				if (_colorDialog) delete _colorDialog;
				_colorDialog = 0;
			}
			catch (const std::exception& e)
			{
				osg::notify(osg::NOTICE) << "UI: cleanup exeception: " << e.what() << std::endl;
			}
		}

	protected:
		MyGUI::TabControl*		_tabRoot;
		osg::Vec2i				_size;
		Base::ImageGenerator*	_ig;
		LightPointDefinitions	_defs;
		OpenThreads::Mutex		_mutex;

		MyGUI::ListBox*			_commandsListBox;
		MyGUI::ListBox*			_commandsDescriptionsListBox;
		MyGUI::TextBox*			_commandTextBox;
		MyGUI::EditBox*			_commandArgumentsEditBox;
		MyGUI::TextBox*			_commandUsageTextBox;
		FileSelectionDialog*	_dialog;
		FileSelectionDialog*	_textureFileDialog;
		demo::ColourPanel*		_colorDialog;
		MyGUI::Button*			_fileButton;
		MyGUI::Button*			_colorButton;
		UIPlugin*				_plugin;
		MyGUI::TabItem*			_lightingTab;
		

		typedef std::map< std::string, MyGUI::ListBox*>	ListBoxes;
		ListBoxes				_lists;

	};

	UIPlugin()
	{
	}

    virtual std::string getName() { return "UI"; }

    virtual std::string getDescription( ) { return "Implements simple user interface"; }

    virtual std::string getVersion() { return "2.0.0"; }

    virtual std::string getAuthor() { return "ComPro, Nick"; }

	virtual void config(const std::string& fileName)
	{					
		osgDB::XmlNode* root = osgDB::readXmlFile(fileName);
		if (root == 0 || root->children.size() == 0) return;

		osgDB::XmlNode* config = root->children.at(0);
		if (config->name != "OpenIG-Plugin-Config") return;

		osg::notify(osg::NOTICE) << "UI: Parsing " << fileName << " for MYGUI Media root" << std::endl;

		osgDB::XmlNode::Children::iterator itr = config->children.begin();
		for (; itr != config->children.end(); ++itr)
		{
			osgDB::XmlNode* child = *itr;

			if (child->name == "MediaRoot")
			{
				_rootMedia = child->contents;
				osg::notify(osg::NOTICE) << "UI: MYGUI Media root set to: " << _rootMedia << std::endl;
			}
			if (child->name == "ScreenWidth")
			{
				UIPlugin::ScreenWidth = atoi(child->contents.c_str());
				if (UIPlugin::ScreenWidth == 0)
				{
					osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
					if (!wsi)
					{
						osg::notify(osg::NOTICE) << "Error, no WindowSystemInterface available, cannot create windows." << std::endl;
						return;
					}

					unsigned int screen_width, screen_height;
					wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(0), screen_width, screen_height);

					UIPlugin::ScreenWidth = screen_width;
				}
			}
		}

		const char* env = getenv("OPENIG_MYGUI_ROOT");
		if (env)
		{
			std::string path(env);
			if (Base::FileSystem::fileExists(path+"/Common/Themes/MyGUI_BlackBlueTheme.xml"))
			{
				_rootMedia = path;
				osg::notify(osg::NOTICE) << "UI: MYGUI Media root set to: " << _rootMedia << std::endl;
				return;
			}
		}
	}			

	virtual void init(PluginBase::PluginContext& context)
	{
		_mygui = new CustomMYGUIManager(context.getImageGenerator(),_rootMedia,this);

		_geode = new osg::Geode;
		_geode->setCullingActive(false);
		_geode->addDrawable(_mygui.get());
		_geode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
		_geode->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		_geode->getOrCreateStateSet()->setRenderBinDetails(1000, "RenderBin");

		_camera = new osg::Camera;
		_camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
		_camera->setRenderOrder(osg::Camera::POST_RENDER);
		_camera->setAllowEventFocus(false);
		_camera->setProjectionMatrix(osg::Matrix::ortho2D(0.0, 1.0, 0.0, 1.0));
		_camera->addChild(_geode.get());
		_camera->setClearMask(0);

		context.getImageGenerator()->getViewer()->getView(0)->getSceneData()->asGroup()->addChild(_camera);
		context.getImageGenerator()->getViewer()->getView(0)->addEventHandler(new MYGUIHandler(_camera.get(), _mygui.get(), context.getImageGenerator(), UIPlugin::ScreenWidth));

	}
	
	LightPointDefinitions	_defs;
	TOD						_tod;
	Brightness				_overallBrightness;
	Brightness				_onCloudsBrightness;
	Brightness				_onWaterBrightness;
	std::string				_xmlFileName;

	void readFromXML(const std::string& xmlFileName, bool fromBackup = false)
	{
		osgDB::XmlNode* root = osgDB::readXmlFile(xmlFileName);
		if (!root || !root->children.size()) return;

		_defs.clear();

		osg::notify(osg::NOTICE) << "UI: processing: " << xmlFileName << std::endl;

		typedef std::multimap< std::string, osgDB::XmlNode::Properties >		TagProperties;
		TagProperties	tags;

		for (osgDB::XmlNode::Children::iterator itr = root->children.at(0)->children.begin();
			itr != root->children.at(0)->children.end();
			++itr)
		{
			osgDB::XmlNode* child = *itr;
			tags.insert(std::pair<std::string, osgDB::XmlNode::Properties>(child->name, child->properties));
		}

		for (TagProperties::iterator itr = tags.begin();
			itr != tags.end();
			++itr)
		{
			typedef std::map< std::string, std::string>	Properties;
			Properties properties;

			for (osgDB::XmlNode::Properties::iterator pitr = itr->second.begin();
				pitr != itr->second.end();
				++pitr)
			{
				properties[pitr->first] = pitr->second;
			}

			if (itr->first == "TimeofDay")
			{
				_tod.on = atof(properties["on"].c_str());
				_tod.off = atof(properties["off"].c_str());
				continue;
			}

			if (itr->first == "LandingLightBrightness")
			{
				_overallBrightness.day = atof(properties["day"].c_str());
				_overallBrightness.night = atof(properties["night"].c_str());
				_overallBrightness.enabled = properties["enable"] == "true";
				continue;
			}

			if (itr->first == "LightBrightnessOnClouds")
			{
				_onCloudsBrightness.day = atof(properties["day"].c_str());
				_onCloudsBrightness.night = atof(properties["night"].c_str());
				continue;
			}
			if (itr->first == "LightBrightnessOnWater")
			{
				_onWaterBrightness.day = atof(properties["day"].c_str());
				_onWaterBrightness.night = atof(properties["night"].c_str());
				continue;
			}

			if (itr->first == "LightPointNode")
			{
				LightPointDefinitionPtr def = new LightPointDefinition;
				def->name = properties["name"];
				def->always_on = properties["always_on"] == "true";
				def->minPixelSize = atof(properties["minPixelSize"].c_str());
				def->maxPixelSize = atof(properties["maxPixelSize"].c_str());
				def->intensity = atof(properties["intensity"].c_str());
				def->radius = atof(properties["radius"].c_str());
				def->brightness = atof(properties["brightness"].c_str());
				def->range = atof(properties["range"].c_str());
				def->mult = atof(properties["minPixelSizeMultiplierForSprites"].c_str());
				def->sprites = properties["sprites"] == "true";
				def->texture = properties["texture"];
				def->fplus = properties["fplus"] == "true";

				_defs[def->name] = def;
			}
		}

		if (_mygui.valid())
		{
			_mygui->updateLightsTab(xmlFileName, _defs, _tod, _overallBrightness, _onCloudsBrightness, _onWaterBrightness, fromBackup);
		}
	}

	virtual void databaseRead(const std::string& fileName, osg::Node* node, const osgDB::Options*)
	{
		_xmlFileName = fileName + ".lighting.xml";
		if (osgDB::fileExists(_xmlFileName))
		{
			// Copy the original file into
			// a backup one
			std::string backup = _xmlFileName + ".backup";

			boost::filesystem::path xml_path(_xmlFileName.c_str());
			boost::filesystem::path backup_path(backup.c_str());
			try
			{
				boost::filesystem::remove(backup_path);
			}
			catch (...)
			{
			}
			try
			{
				boost::filesystem::copy_file(xml_path, backup_path);
			}
			catch (const std::exception& e)
			{
				osg::notify(osg::NOTICE) << "UI: exception thrown file creating a backup file: " << backup << ", " << e.what() << std::endl;
			}

			// Read the XML
			readFromXML(_xmlFileName);
		}
	}

    virtual void update(PluginBase::PluginContext& context)
    {		
		static bool once = false;
		if (!once)
		{
			once = true;

			osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(context.getImageGenerator()->getViewer()->getView(0)->getCamera()->getGraphicsContext());
			if (gw)
			{
				// Send window size for MyGUI to initialize
				int x, y, w, h;
				gw->getWindowRectangle(x, y, w, h);
				w = osg::minimum(w, (int)UIPlugin::ScreenWidth);
				context.getImageGenerator()->getViewer()->getView(0)->getEventQueue()->windowResize(x, y, w, h);

			}
		}
    }

	virtual void clean(PluginBase::PluginContext& context)
	{		
		_mygui->cleanup();
	}

protected:
	osg::ref_ptr<CustomMYGUIManager>	_mygui;
	osg::ref_ptr<osg::Geode>			_geode;
	osg::ref_ptr<osg::Camera>			_camera;
	std::string							_rootMedia;
};

} // namespace
} // namespace

unsigned OpenIG::Plugins::UIPlugin::ScreenWidth = 1600;

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
	return new OpenIG::Plugins::UIPlugin;
}

extern "C" EXPORT void DeletePlugin(OpenIG::PluginBase::Plugin* plugin)
{
	osg::ref_ptr<OpenIG::PluginBase::Plugin> p(plugin);
}
