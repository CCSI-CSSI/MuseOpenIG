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
#include "openig.h"
#include "keypad.h"

#include <Core-Base/commands.h>
#include <Core-Base/mathematics.h>
#include <Core-Base/animation.h>
#include <Core-Base/filesystem.h>

#include <iostream>
#include <sstream>
#include <cctype>
#include <functional>

#include <osg/ValueObject>

#include <osgGA/CameraManipulator>
#include <osgGA/TrackballManipulator>

using namespace OpenIG;
using namespace OpenIG::Base;

namespace OpenIG
{


class AddEntityCommand : public Commands::Command
{
public:
	AddEntityCommand(ImageGenerator* ig)
		: _ig(ig) {}

	virtual const std::string getUsage() const
	{
		return "id filename x y z [optional:heading pitch roll] [optional:x,y,z]";
	}

	virtual const std::string getArgumentsFormat() const
	{
		return "I:F:P:O:S";
	}

	virtual const std::string getDescription() const
	{
		return  "adds entity in the scene\n"
			"     id - the id of the new entity across the scene\n"
			"     filename - the file name of the new entity, no space in\n"
			"     x - the x position of the new entity\n"
			"     y - the y position of the new entity\n"
			"     z - the z position of the new entity\n"
			"     heading - optional, the heading in degrees of the new entity\n"
			"     pitch - optional, the pitch in degrees of the new entity\n"
			"     roll - optional, the roll in degrees of the new entity\n"
			"     x,y,z - optional offset, no space. Used by plugins to perform offets of the vertices,lods,matrices\n"
			"          to avoid precission problems on large databases";
	}

	virtual int exec(const StringUtils::Tokens& tokens)
	{
		if (tokens.size() >= 5)
		{
			unsigned int    id = atoi(tokens.at(0).c_str());
			std::string     fileName = tokens.at(1);
			double          x = atof(tokens.at(2).c_str());
			double          y = atof(tokens.at(3).c_str());
			double          z = atof(tokens.at(4).c_str());
			double          h = 0.0; 
			double          p = 0.0;
			double          r = 0.0;

			if (tokens.size() >= 8)
			{
				h = atof(tokens.at(5).c_str());
				p = atof(tokens.at(6).c_str());
				r = atof(tokens.at(7).c_str());

			}
			osg::Matrixd mx = Math::instance()->toMatrix(x,y,z,h,p,r);

			osg::ref_ptr<osgDB::Options> options;

			if (tokens.size()==9)
			{
				options = new osgDB::Options(tokens.at(8));
			}

			std::string fullPath = fileName;
			if (fullPath.find_first_of('{') != std::string::npos && fullPath.find_first_of('}') != std::string::npos)
			{
				fullPath = FileSystem::path(FileSystem::PathList, fullPath);
			}

			_ig->addEntity(id, fullPath, mx, options.get());

			return 0;
		}
		return -1;
	}

protected:
	OpenIG::Base::ImageGenerator* _ig;
};

class RemoveEntityCommand : public Commands::Command
{
public:
	RemoveEntityCommand(ImageGenerator* ig)
		: _ig(ig) {}

	virtual const std::string getUsage() const
	{
		return "id";
	}

	virtual const std::string getArgumentsFormat() const
	{
		return "I";
	}

	virtual const std::string getDescription() const
	{
		return  "removes a given entity from the scene by a given id\n"
			"     id - the id of the entity";
	}

	virtual int exec(const StringUtils::Tokens& tokens)
	{
		if (tokens.size() == 1)
		{
			unsigned int id = atoi(tokens.at(0).c_str());

			_ig->removeEntity(id);

			return 0;
		}
		return -1;
	}

protected:
	OpenIG::Base::ImageGenerator* _ig;
};

class SetCameraPositionCommand : public Commands::Command
{
public:
	SetCameraPositionCommand (ImageGenerator* ig)
		: _ig(ig) {}

	virtual const std::string getUsage() const
	{
		return "x y z heading pitch roll";
	}

	virtual const std::string getArgumentsFormat() const
	{
		return "D:D:D:D:D:D";
	}

	virtual const std::string getDescription() const
	{
		return "sets the camera position\n"
			"     x - the x position of the camera\n"
			"     y - the y position of the camera\n"
			"     z - the z position of the camera\n"
			"     heading - the heading in degrees of the camera\n"
			"     pitch - the pitch in degrees of the camera\n"
			"     roll - the roll in degrees of the camera";
	}

	virtual int exec(const StringUtils::Tokens& tokens)
	{
		if (tokens.size() == 6)
		{
			double          x = atof(tokens.at(0).c_str());
			double          y = atof(tokens.at(1).c_str());
			double          z = atof(tokens.at(2).c_str());
			double          h = atof(tokens.at(3).c_str());
			double          p = atof(tokens.at(4).c_str());
			double          r = atof(tokens.at(5).c_str());

			osg::Matrixd mx = Math::instance()->toMatrix(x,y,z,h,p+90,r);

			_ig->setCameraPosition(mx);

			return 0;
		}
		return -1;
	}

protected:
	OpenIG::Base::ImageGenerator* _ig;
};

class BindCameraToEntityCommand : public Commands::Command
{
public:
	BindCameraToEntityCommand (ImageGenerator* ig)
		: _ig(ig) {}

	virtual const std::string getUsage() const
	{
		return "id x y z heading pitch roll";
	}

	virtual const std::string getArgumentsFormat() const
	{
		return "I:D:D:D:D:D:D";
	}

	virtual const std::string getDescription() const
	{
		return  "binds the camera to an entity, with an offset in entity space\n"
			"     id - the id of the entity\n"
			"     x - the x position of the camera\n"
			"     y - the y position of the camera\n"
			"     z - the z position of the camera\n"
			"     heading - the heading of the camera in degrees\n"
			"     pitch - the pitch of the camera in degrees\n"
			"     roll - the roll of the camera in degrees";
	}

	virtual int exec(const StringUtils::Tokens& tokens)
	{
		if (tokens.size() == 7)
		{
			unsigned int    id = atoi(tokens.at(0).c_str());
			double          x = atof(tokens.at(1).c_str());
			double          y = atof(tokens.at(2).c_str());
			double          z = atof(tokens.at(3).c_str());
			double          h = atof(tokens.at(4).c_str());
			double          p = atof(tokens.at(5).c_str());
			double          r = atof(tokens.at(6).c_str());

			osg::Matrixd offset = Math::instance()->toMatrix(x,y,z,h,p+90,r);

			_ig->bindCameraToEntity(id,offset);

			return 0;
		}
		return -1;
	}

protected:
	OpenIG::Base::ImageGenerator* _ig;
};

class UnbindCameraFromEntityCommand : public Commands::Command
{
public:
	UnbindCameraFromEntityCommand(ImageGenerator* ig)
		: _ig(ig) {}

	virtual const std::string getUsage() const
	{
		return "(no arguments)";
	}

	virtual const std::string getArgumentsFormat() const
	{
		return "";
	}

	virtual const std::string getDescription() const
	{
		return "unbinds the camera from the entity and gets the current position tanslated in world space";
	}

	virtual int exec(const StringUtils::Tokens&)
	{
		_ig->unbindCameraFromEntity();

		return 0;
	}

protected:
	OpenIG::Base::ImageGenerator* _ig;
};

class SetTimeOfDayCommand : public Commands::Command
{
public:
	SetTimeOfDayCommand (ImageGenerator* ig)
		: _ig(ig) {}

	virtual const std::string getUsage() const
	{
		return "hour minutes";
	}

	virtual const std::string getArgumentsFormat() const
	{
		return "I:I";
	}


	virtual const std::string getDescription() const
	{
		return  "sets the time of day\n"
			"     hour - the hour from 0 - 24\n"
			"     minutes - minutes from 0 to 59";
	}

	virtual int exec(const StringUtils::Tokens& tokens)
	{
		if (tokens.size() == 2)
		{
			unsigned int    hour = atoi(tokens.at(0).c_str());
			unsigned int    minutes = atoi(tokens.at(1).c_str());

			_ig->setTimeOfDay(hour,minutes);

			return 0;
		}
		return -1;
	}

protected:
	OpenIG::Base::ImageGenerator* _ig;
};

class SetWindCommand : public Commands::Command
{
public:
	SetWindCommand (ImageGenerator* ig)
		: _ig(ig) {}

	virtual const std::string getUsage() const
	{
		return "speed direction";
	}

	virtual const std::string getArgumentsFormat() const
	{
		return "D:D";
	}

	virtual const std::string getDescription() const
	{
		return  "sets wind\n"
			"     speed - in meteres per second\n"
			"     direction - in degrees East from North";
	}

	virtual int exec(const StringUtils::Tokens& tokens)
	{
		if (tokens.size() == 2)
		{
			float speed = atof(tokens.at(0).c_str());
			float direction = atof(tokens.at(1).c_str());

			_ig->setWind(speed,direction);

			return 0;
		}
		return -1;
	}

protected:
	OpenIG::Base::ImageGenerator* _ig;
};

class AddLightCommand : public Commands::Command
{
public:
	AddLightCommand (ImageGenerator* ig)
		: _ig(ig) {}

	virtual const std::string getUsage() const
	{
		return "id x y z heading pitch roll";
	}

	virtual const std::string getArgumentsFormat() const
	{
		return "I:D:D:D:D:D:D";
	}

	virtual const std::string getDescription() const
	{
		return  "adds light source with an id to be used across the scene\n"
			"     id - the id of the light source\n"
			"     x - the x position of the new light source\n"
			"     y - the y position of the new light source\n"
			"     z - the z position of the new light source\n"
			"     heading - the heading in degrees of the new light source\n"
			"     pitch - the pitch in degrees of the new light source\n"
			"     roll - the roll in degrees of the new light source";
	}

	virtual int exec(const StringUtils::Tokens& tokens)
	{
		if (tokens.size() == 8)
		{
			unsigned int id     = atoi(tokens.at(0).c_str());
			double x            = atof(tokens.at(1).c_str());
			double y            = atof(tokens.at(2).c_str());
			double z            = atof(tokens.at(3).c_str());
			double h            = atof(tokens.at(4).c_str());
			double p            = atof(tokens.at(5).c_str());
			double r            = atof(tokens.at(6).c_str());

            std::string strLightType = tokens.at(7);
            LightAttributes la;
			if (strLightType.compare(0, 11, "directional") == 0)
			{
				la.lightType = OpenIG::Base::LT_DIRECTIONAL;
			}
			else if (strLightType.compare(0, 5, "point") == 0)
			{
				la.lightType = OpenIG::Base::LT_POINT;
			}
			else if (strLightType.compare(0, 4, "spot") == 0)
			{
				la.lightType = OpenIG::Base::LT_SPOT;
			}

			osg::Matrixd offset = Math::instance()->toMatrix(x,y,z,h,p,r);

			_ig->addLight(id,la, offset);

			return 0;
		}

		return -1;
	}
protected:
	OpenIG::Base::ImageGenerator*     _ig;
};

class BindLightTocameraCommand : public Commands::Command
{
public:
	BindLightTocameraCommand(ImageGenerator* ig)
		: _ig(ig) {}

	virtual const std::string getUsage() const
	{
		return "id [optional: x y z heading pitch roll]";
	}

	virtual const std::string getArgumentsFormat() const
	{
		return "I:D:D:D:D:D:D";
	}

	virtual const std::string getDescription() const
	{
		return  "binds the light source to the camera\n"
			"     id - the id of the light source\n"
			"     x - the x offset of the light source wrt. camera\n"
			"     y - the y offset of the light source wrt. camera\n"
			"     z - the z offset of the light source wrt. camera\n"
			"     heading - the heading offset of the light source wrt. camera in degrees\n"
			"     pitch - the pitch offset of the light source wrt. camera in degrees\n"
			"     roll - the roll offset of the light source wrt. camera in degrees";
	}

	virtual int exec(const StringUtils::Tokens& tokens)
	{
		if (tokens.size() == 1)
		{
			unsigned int id     = atoi(tokens.at(0).c_str());

			_ig->bindLightToCamera(id);

			return 0;
		}
		else
			if (tokens.size() == 7)
			{
				unsigned int id     = atoi(tokens.at(0).c_str());
				double x            = atof(tokens.at(1).c_str());
				double y            = atof(tokens.at(2).c_str());
				double z            = atof(tokens.at(3).c_str());
				double h            = atof(tokens.at(4).c_str());
				double p            = atof(tokens.at(5).c_str());
				double r            = atof(tokens.at(6).c_str());

				osg::Matrixd mx = OpenIG::Base::Math::instance()->toMatrix(x,y,z,h,p,r);

				_ig->bindLightToCamera(id, mx);

				return 0;
			}

			return -1;
	}
protected:
	OpenIG::Base::ImageGenerator*     _ig;
};

class LightAttribsCommand : public Commands::Command
{
public:
	LightAttribsCommand(ImageGenerator* ig)
		: _ig(ig) {}

	virtual const std::string getUsage() const
	{
		return "id attrib attribarguments";
	}

	virtual const std::string getArgumentsFormat() const
	{
		return "I:{ambient;diffuse;specular;attenuation;spotcutoff;ranges;spotangles}:C";
	}


	virtual const std::string getDescription() const
	{
		return  "sets the attribues of the light sources like diffuse, speculat, ambient ....\n"
			"     id - the id of the light source\n"
			"     attrib - light attribute, can be one of these:\n"
			"          ambient red green blue - the ambient comonent of the light source, followed by the color components\n"
			"          diffuse red green blue brightness [optional:cloudbrightness] [optional:waterbrightness]- the diffuse component, followed by a brightness\n"
			"          specular red green blue - the specular component\n"
			"          attenuation distance - the constant attenuation, in meters\n"
			"          spotcutoff value - the spot cutsoff of the light source\n"
			"		   ranges start end - the range of the light\n"
			"		   spotangles inner outer - the angles of the light";
	}

	virtual int exec(const StringUtils::Tokens& tokens)
	{
		if (tokens.size()<=2)
		{
			return -1;
		}

		osg::Vec4f      ambient(0.f,0.f,0.f,1.f);
		osg::Vec4f      diffuse(0.f,0.f,0.f,1.f);
		osg::Vec4f      specular(0.f,0.f,0.f,1.f);
		float           brightness = 0.f;
		float           cloudbrightness = 0.f;
		float			waterbrightness = 0.f;
		float           attenuation = 0.f;
		float           spotcutoff = 0.f;
		unsigned int    mask = 0x0;
		float fStartRage = 0;
		float fEndRange = 0;
		float fSpotInnerAngle = 0;
		float fSpotOuterAngle = 0;

		unsigned int id     = atoi(tokens.at(0).c_str());
		std::string attrib  = tokens.at(1);

		if (attrib == "ambient" && tokens.size() == 5)
		{
			ambient.x() = atof(tokens.at(2).c_str());
			ambient.y() = atof(tokens.at(3).c_str());
			ambient.z() = atof(tokens.at(4).c_str());

			mask |= OpenIG::Base::LightAttributes::AMBIENT;			
		}
		else if (attrib == "diffuse" && tokens.size() == 6)
		{
			diffuse.x() = atof(tokens.at(2).c_str());
			diffuse.y() = atof(tokens.at(3).c_str());
			diffuse.z() = atof(tokens.at(4).c_str());
			brightness = atof(tokens.at(5).c_str());

			mask |= OpenIG::Base::LightAttributes::DIFFUSE;
			mask |= OpenIG::Base::LightAttributes::BRIGHTNESS;
		}
		else if (attrib == "diffuse" && tokens.size() == 8)
		{
			diffuse.x() = atof(tokens.at(2).c_str());
			diffuse.y() = atof(tokens.at(3).c_str());
			diffuse.z() = atof(tokens.at(4).c_str());
			brightness = atof(tokens.at(5).c_str());
			cloudbrightness = atof(tokens.at(6).c_str());
			waterbrightness = atof(tokens.at(7).c_str());

			mask |= OpenIG::Base::LightAttributes::DIFFUSE;
			mask |= OpenIG::Base::LightAttributes::BRIGHTNESS;
			mask |= OpenIG::Base::LightAttributes::CLOUDBRIGHTNESS;
			mask |= OpenIG::Base::LightAttributes::WATERBRIGHTNESS;
		}
		else if (attrib == "specular" && tokens.size() == 5)
		{
			specular.x() = atof(tokens.at(2).c_str());
			specular.y() = atof(tokens.at(3).c_str());
			specular.z() = atof(tokens.at(4).c_str());

			mask |= OpenIG::Base::LightAttributes::SPECULAR;
		}
		else if (attrib == "attenuation" && tokens.size() == 3)
		{
			attenuation = atof(tokens.at(2).c_str());

			mask |= OpenIG::Base::LightAttributes::CONSTANTATTENUATION;
		}
		else if (attrib == "spotcutoff" && tokens.size() == 3)
		{
			spotcutoff = atof(tokens.at(2).c_str());

			mask |= OpenIG::Base::LightAttributes::SPOTCUTOFF;
		}

		// PPP: I added these
		else if (attrib == "ranges" && tokens.size() == 4)
		{
			fStartRage = atof(tokens.at(2).c_str());
			fEndRange  = atof(tokens.at(3).c_str());
			mask |= OpenIG::Base::LightAttributes::RANGES;
		}

		else if (attrib == "spotangles" && tokens.size() == 4)
		{
			fSpotInnerAngle = atof(tokens.at(2).c_str());
			fSpotOuterAngle = atof(tokens.at(3).c_str());
			mask |= OpenIG::Base::LightAttributes::ANGLES;
		}

		OpenIG::Base::LightAttributes definition;
		definition.ambient = ambient;
		definition.diffuse = diffuse;
		definition.specular = specular;
		definition.brightness = brightness;
		definition.constantAttenuation = attenuation;
		definition.spotCutoff = spotcutoff;
		definition.cloudBrightness = cloudbrightness;
		definition.waterBrightness = waterbrightness;
		definition.fStartRange = fStartRage;
		definition.fEndRange = fEndRange;
		definition.fSpotInnerAngle = fSpotInnerAngle;
		definition.fSpotOuterAngle= fSpotOuterAngle;
		definition.dirtyMask = mask;

		_ig->updateLightAttributes(id,definition);

		return 0;
	}
protected:
	OpenIG::Base::ImageGenerator*     _ig;
};




class KeypadCommand : public Commands::Command
{
public:
	KeypadCommand(
		ImageGenerator* ig,
		osg::ref_ptr<KeyPadEventHandler>& keypad,
		osg::ref_ptr<KeyPadCameraManipulator>& keypadCameraManipulator)
		: _ig(ig)
		, _keypad(keypad)
		, _keypadCameraManipulator(keypadCameraManipulator)
	{

	}

	virtual const std::string getUsage() const
	{
		return "[optional:id] [optional:off]";
	}

	virtual const std::string getArgumentsFormat() const
	{
		return "I:{on;off}";
	}


	virtual const std::string getDescription() const
	{
		return  "if id provided, binds the keyboard to an entity to move it in the scene, otherwise controls the camera.\n"
			"if 'off' provided, unbinds from entity or camera\n"
			"     id - the id of the entity\n"
			"          - the keyboard is bound to the entity and is moved by the combination of these numeric pad keys + LEFT ALT\n"
			"          - ENTER: forward\n"
			"          - ./Del: backward\n"
			"          - +: up\n"
			"          - -: down\n"
			"          - 1/End: left\n"
			"          - 3/PgDn: right\n"
			"          - 4/Left: heading to left\n"
			"          - 6/Right: heading to right\n"
			"          - 8/Up: pitch to up\n"
			"          - 2/Down: pitch to down\n"
			"          - 7/Home: roll to left\n"
			"          - 9/PgUp: roll to right";
	}

	virtual int exec(const StringUtils::Tokens& tokens)
	{
		if (!_keypad.valid())
		{
			_keypad = new KeyPadEventHandler(_ig);
			_ig->getViewer()->getView(0)->addEventHandler(_keypad);
		}
		if (!_keypadCameraManipulator.valid())
		{
			_keypadCameraManipulator = new KeyPadCameraManipulator(_ig);
			_keypadCameraManipulator->setUserValue("name",std::string("keypad"));
		}

		if (tokens.size() == 1)
		{            
			if (tokens.at(0).compare(0,3,"off") == 0)
			{
				_keypad->unbind();

				osg::ref_ptr<osgGA::CameraManipulator> manip = _ig->getViewer()->getView(0)->getCameraManipulator();
				if (manip.valid() && _manipulator.valid())
				{
					std::string name;
					if (manip->getUserValue("name",name) && name == "keypad")
					{
						_ig->getViewer()->getView(0)->setCameraManipulator(_manipulator,false);
						_manipulator->setByInverseMatrix(manip->getInverseMatrix());

						osg::ref_ptr<osgGA::GUIEventAdapter> ea = new osgGA::GUIEventAdapter;
						osg::ref_ptr<DymmuGUIActionAdapter> aa = new DymmuGUIActionAdapter;
						_manipulator->setNode(0);
						_manipulator->init(*ea,*aa);
					}
				}
			}
			else
			{
				unsigned int id = atoi(tokens.at(0).c_str());
				_keypad->bindToEntity(id);                
			}

			return 0;
		}
		else
			if (tokens.size() == 0)
			{
				osg::ref_ptr<osgGA::CameraManipulator> manip = _ig->getViewer()->getView(0)->getCameraManipulator();
				if (manip.valid())
				{
					std::string name;
					manip->getUserValue("name",name);
					if (name != "keypad")
					{
						_manipulator = manip;
					}
				}
				_ig->getViewer()->getView(0)->setCameraManipulator(_keypadCameraManipulator);
				if (manip.valid())
				{
					_keypadCameraManipulator->setByMatrix(manip->getMatrix());
					_keypadCameraManipulator->setNode(manip->getNode());
				}
			}

			return -1;
	}
protected:
	OpenIG::Base::ImageGenerator*           _ig;
	osg::ref_ptr<KeyPadEventHandler>&       _keypad;
	osg::ref_ptr<KeyPadCameraManipulator>&  _keypadCameraManipulator;
	osg::ref_ptr<osgGA::CameraManipulator>	_manipulator;
};

class BindLightCommand : public Commands::Command
{
public:
	BindLightCommand(ImageGenerator* ig)
		: _ig(ig) {}

	virtual const std::string getUsage() const
	{
		return "lightId entityId";
	}

	virtual const std::string getArgumentsFormat() const
	{
		return "I:I";
	}

	virtual const std::string getDescription() const
	{
		return  "binds light to an entity. The offset of the light in entity space is done by the light position\n"
			"     lightId - the id of the light source\n"
			"     entityId - the entity id";
	}

	virtual int exec(const StringUtils::Tokens& tokens)
	{
		if (tokens.size() == 2)
		{
			unsigned int id         = atoi(tokens.at(0).c_str());
			unsigned int entityId   = atoi(tokens.at(1).c_str());

			_ig->bindLightToEntity(id,entityId);

			return 0;
		}

		return -1;
	}
protected:
	OpenIG::Base::ImageGenerator* _ig;
};

class UnbindLightCommand : public Commands::Command
{
public:
	UnbindLightCommand(ImageGenerator* ig)
		: _ig(ig) {}

	virtual const std::string getUsage() const
	{
		return "id";
	}

	virtual const std::string getArgumentsFormat() const
	{
		return "I";
	}

	virtual const std::string getDescription() const
	{
		return  "unbinds a light from an entity\n"
			"     id - the id of the light source";
	}

	virtual int exec(const StringUtils::Tokens& tokens)
	{
		if (tokens.size() == 1)
		{
			unsigned int id = atoi(tokens.at(0).c_str());

			_ig->unbindLightFromEntity(id);

			return 0;
		}

		return -1;
	}
protected:
	OpenIG::Base::ImageGenerator* _ig;
};

class UnbindLightFromCameraCommand : public Commands::Command
{
public:
	UnbindLightFromCameraCommand(ImageGenerator* ig)
		: _ig(ig) {}

	virtual const std::string getUsage() const
	{
		return "id";
	}

	virtual const std::string getArgumentsFormat() const
	{
		return "I";
	}

	virtual const std::string getDescription() const
	{
		return  "unbinds a light from the camera\n"
			"     id - the light source id";
	}

	virtual int exec(const StringUtils::Tokens& tokens)
	{
		if (tokens.size() == 1)
		{
			unsigned int id = atoi(tokens.at(0).c_str());

			_ig->unbindLightFromcamera(id);

			return 0;
		}

		return -1;
	}
protected:
	OpenIG::Base::ImageGenerator* _ig;
};

class UpdateLightCommand : public Commands::Command
{
public:
	UpdateLightCommand(ImageGenerator* ig)
		: _ig(ig) {}

	virtual const std::string getUsage() const
	{
		return "id x y z heading pitch roll";
	}

	virtual const std::string getArgumentsFormat() const
	{
		return "I:D:D:D:D:D:D";
	}

	virtual const std::string getDescription() const
	{
		return  "updates the light position and orientation\n"
			"     id - the id of the light source\n"
			"     x - the x position of the light\n"
			"     y - the y position of the light\n"
			"     z - the z position of the light\n"
			"     heading - the heading in degrees of the light\n"
			"     pitch - the pitch in degrees of the light\n"
			"     roll - the roll in degrees of the light";
	}

	virtual int exec(const StringUtils::Tokens& tokens)
	{
		if (tokens.size() == 7)
		{
			unsigned int id = atoi(tokens.at(0).c_str());
			double x        = atof(tokens.at(1).c_str());
			double y        = atof(tokens.at(2).c_str());
			double z        = atof(tokens.at(3).c_str());
			double h        = atof(tokens.at(4).c_str());
			double p        = atof(tokens.at(5).c_str());
			double r        = atof(tokens.at(6).c_str());

			osg::Matrixd mx = Math::instance()->toMatrix(x,y,z,h,p,r);

			_ig->updateLight(id,mx);

			return 0;
		}

		return -1;
	}
protected:
	OpenIG::Base::ImageGenerator* _ig;
};

class RemoveLightCommand : public Commands::Command
{
public:
	RemoveLightCommand(ImageGenerator* ig)
		: _ig(ig) {}

	virtual const std::string getUsage() const
	{
		return "id";
	}

	virtual const std::string getArgumentsFormat() const
	{
		return "I";
	}

	virtual const std::string getDescription() const
	{
		return  "removes a light source from the scene\n"
			"     id - the id of the light source";
	}

	virtual int exec(const StringUtils::Tokens& tokens)
	{
		if (tokens.size() == 1)
		{
			unsigned int id = atoi(tokens.at(0).c_str());

			_ig->removeLight(id);

			return 0;
		}

		return -1;
	}
protected:
	OpenIG::Base::ImageGenerator* _ig;
};

class BindEntityCommand : public Commands::Command
{
public:
	BindEntityCommand(ImageGenerator* ig)
		: _ig(ig) {}

	virtual const std::string getUsage() const
	{
		return "id parentId";
	}

	virtual const std::string getArgumentsFormat() const
	{
		return "I:I";
	}

	virtual const std::string getDescription() const
	{
		return  "binds entity to another entity\n"
			"     id - the entiy to bind\n"
			"     parentId - the id of the parent entity";
	}

	virtual int exec(const StringUtils::Tokens& tokens)
	{
		if (tokens.size() == 2)
		{
			unsigned int id         = atoi(tokens.at(0).c_str());
			unsigned int entityId   = atoi(tokens.at(1).c_str());

			_ig->bindToEntity(id,entityId);

			return 0;
		}

		return -1;
	}
protected:
	OpenIG::Base::ImageGenerator* _ig;
};

class UnbindEntityCommand : public Commands::Command
{
public:
	UnbindEntityCommand(ImageGenerator* ig)
		: _ig(ig) {}

	virtual const std::string getUsage() const
	{
		return "id";
	}

	virtual const std::string getArgumentsFormat() const
	{
		return "I";
	}

	virtual const std::string getDescription() const
	{
		return  "unbinds entity from the parent entity, the position will be transformed in world space from the parent space\n"
			"     id - the entity to unbind";
	}

	virtual int exec(const StringUtils::Tokens& tokens)
	{
		if (tokens.size() == 1)
		{
			unsigned int id = atoi(tokens.at(0).c_str());

			_ig->unbindFromEntity(id);

			return 0;
		}

		return -1;
	}
protected:
	OpenIG::Base::ImageGenerator* _ig;
};

class OffsetCommand: public Commands::Command
{
public:
	OffsetCommand(Engine* ig)
		: _ig(ig) {}

	virtual const std::string getUsage() const
	{
		return "id x y z";
	}

	virtual const std::string getArgumentsFormat() const
	{
		return "I:D:D:D";
	}

	virtual const std::string getDescription() const
	{
		return  "offsets an entity/model by substracting the offset from vertices,lods,matrices .. \n"
			"to avoid precission issues on large databases\n"
			"     id - the id of the entity/model\n"
			"     x - the x offset\n"
			"     y - the y offset\n"
			"     z - the z offset";
	}

	virtual int exec(const StringUtils::Tokens& tokens)
	{
		if (tokens.size() == 4)
		{
			unsigned int id = atoi(tokens.at(0).c_str());

			osg::Vec3d offset;
			offset.x() = atof(tokens.at(1).c_str());
			offset.y() = atof(tokens.at(2).c_str());
			offset.z() = atof(tokens.at(3).c_str());

			std::ostringstream oss;
			oss << offset.x() << "," << offset.y() << "," << offset.z();

			_ig->getPluginContext().addAttribute("VDBOffset", new osgDB::Options(oss.str()));

			osg::Matrixd mx;
			mx = OpenIG::Base::Math::instance()->toMatrix(-offset.x(),-offset.y(),-offset.z(),0.0,0.0,0.0);

			osg::notify(osg::NOTICE) << "OpenIG: New master offset:" << -offset.x() << "," << -offset.y() << "," << -offset.z() << std::endl;

			_ig->updateEntity(id,mx);

			return 0;
		}

		return -1;
	}
protected:
	Engine* _ig;
};

class LessThenAnimationSequencePlaybackCallback : public AnimationSequencePlaybackCallback
{
public:
	LessThenAnimationSequencePlaybackCallback() : _value(0) {}
	LessThenAnimationSequencePlaybackCallback(const std::string& name, double value) : _name(name), _value(value) {}

	virtual bool operator()(double value)
	{
		if (fabs(value) < fabs(_value)) return false;
		else return true;
	}

protected:
	std::string     _name;
	double          _value;
};

class AnimationCommand: public OpenIG::Base::Commands::Command
{
public:
	AnimationCommand(OpenIG::Base::ImageGenerator* ig)
		: _ig(ig) {}

	virtual const std::string getUsage() const
	{
		return "id animationname [optional: sequencename1 limit1 sequencename2 limit2 ....]";
	}

	virtual const std::string getArgumentsFormat() const
	{
		return "I:S:S";
	}

	virtual const std::string getDescription() const
	{
		return  "plays animation on a model, and controling sequences by a given value\n"
			"     id - the entity id\n"
			"     animationname - the name of the animation, obviously definied in the model XML\n"
			"     sequencenameX - the name of the sequence to be controlled by a given limit\n"
			"     limitX - the value of the limit. Example can be landing gear of a plabe to stops at 60 degrees";
	}

	virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
	{
		if (tokens.size() >= 2)
		{
			unsigned int id     = atoi(tokens.at(0).c_str());
			std::string name	= tokens.at(1);

			unsigned int afterNameIdx = 2;
			if (name.at(name.length() - 1) == ':' && tokens.size() > 2)
			{
				name += tokens.at(2);

				++afterNameIdx;
			}
			if (tokens.size() > afterNameIdx && tokens.at(afterNameIdx).at(0) == ':')
			{
				name += tokens.at(3);

				++afterNameIdx;
			}

			osg::ref_ptr<RefAnimationSequenceCallbacks> cbs;
			if (tokens.size() > afterNameIdx)
			{
				unsigned int i = afterNameIdx;
				while (i+1 < tokens.size())
				{
					std::string sequence    = tokens.at(i);
					double value            = atof(tokens.at(i+1).c_str());

					i += 2;

					if (!cbs.valid())
					{
						cbs = new RefAnimationSequenceCallbacks;
					}

					osg::ref_ptr<LessThenAnimationSequencePlaybackCallback> cb = new LessThenAnimationSequencePlaybackCallback(sequence,value);
					(*cbs)[sequence] = cb;
				}
			}

			if (cbs.valid() && cbs->size())
			{
				_ig->playAnimation(id,name,cbs.get());
			}
			else
			{
				_ig->playAnimation(id,name);
			}


			return 0;
		}

		return -1;
	}
protected:
	OpenIG::Base::ImageGenerator* _ig;
};

class MultipleAnimationCommand: public OpenIG::Base::Commands::Command
{
public:
	MultipleAnimationCommand(OpenIG::Base::ImageGenerator* ig)
		: _ig(ig) {}

	virtual const std::string getUsage() const
	{
		return "id animationname1 animationname2 animationname3 .....";
	}

	virtual const std::string getArgumentsFormat() const
	{
		return "I:S";
	}

	virtual const std::string getDescription() const
	{
		return  "mutiple animations playback on a model\n"
			"     id - the id of the model\n"
			"     animationnameX - the name of the animation";
	}

	virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
	{
		if (tokens.size() >= 2)
		{
			unsigned int id     = atoi(tokens.at(0).c_str());
			unsigned int i = 1;
			StringUtils::StringList animations;

			while ( i < tokens.size())
			{
				animations.push_back(tokens.at(i++));
			}

			_ig->playAnimation(id,animations);

			return 0;
		}

		return -1;
	}
protected:
	OpenIG::Base::ImageGenerator* _ig;
};

class ReloadEntityCommand: public OpenIG::Base::Commands::Command
{
public:
	ReloadEntityCommand(OpenIG::Base::ImageGenerator* ig)
		: _ig(ig) {}

	virtual const std::string getUsage() const
	{
		return "id";
	}

	virtual const std::string getArgumentsFormat() const
	{
		return "I";
	}

	virtual const std::string getDescription() const
	{
		return  "reloads an entity\n"
			"     id - the id of the model";
	}

	virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
	{
		if (tokens.size() == 1)
		{
			unsigned int id     = atoi(tokens.at(0).c_str());

			OpenIG::Base::ImageGenerator::Entity& entity = _ig->getEntityMap()[id];
			if (entity.valid())
			{
				std::string fileName;
				entity->getUserValue("fileName",fileName);

				_ig->reloadEntity(id,fileName);
			}

			return 0;
		}

		return -1;
	}
protected:
	OpenIG::Base::ImageGenerator* _ig;
};

class AddEffectCommand : public OpenIG::Base::Commands::Command
{
public:
	AddEffectCommand(OpenIG::Base::ImageGenerator* ig)
		: _ig(ig) {}

	virtual const std::string getUsage() const
	{
		return "id name x y z heading pitch roll [optional:attributes]";
	}

	virtual const std::string getArgumentsFormat() const
	{
		return "I:S:D:D:D:D:D:D:S";
	}

	virtual const std::string getDescription() const
	{
		return  "adds effect in the scene\n"
			"     id - the id of the effect\n"
			"     name - the name of the effect\n"
			"     x - the x position of the new effect\n"
			"     y - the y position of the new effect\n"
			"     z - the z position of the new effect\n"
			"     heading - the heading in degrees of the new effect\n"
			"     pitch - the pitch in degrees of the new effect\n"
			"     roll - the roll in degrees of the new effect\n"
			"     attributes - in the form of token=value token=value";
	}

	virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
	{
		if (tokens.size() >= 8)
		{
			unsigned int	id = atoi(tokens.at(0).c_str());
			std::string		name = tokens.at(1);
			double          x = atof(tokens.at(2).c_str());
			double          y = atof(tokens.at(3).c_str());
			double          z = atof(tokens.at(4).c_str());
			double          h = atof(tokens.at(5).c_str());
			double          p = atof(tokens.at(6).c_str());
			double          r = atof(tokens.at(7).c_str());

			osg::Matrixd mx = Math::instance()->toMatrix(x, y, z, h, p, r);

			std::ostringstream oss;
			for (size_t i = 8; i < tokens.size(); ++i)
			{
				oss << tokens.at(i) << ";";
			}

			_ig->addEffect(id, name, mx, oss.str());

			return 0;
		}

		return -1;
	}
protected:
	OpenIG::Base::ImageGenerator* _ig;
};

class BindEffectCommand : public OpenIG::Base::Commands::Command
{
public:
	BindEffectCommand(OpenIG::Base::ImageGenerator* ig)
		: _ig(ig) {}

	virtual const std::string getUsage() const
	{
		return "id name x y z heading pitch roll";
	}

	virtual const std::string getArgumentsFormat() const
	{
		return "I:S:D:D:D:D:D:D";
	}

	virtual const std::string getDescription() const
	{
		return  "binds effect to an Entity in the scene\n"
			"     id - the id of the effect\n"
			"     entityID - the the ID of the Entity\n"
			"     x - the x position of the new effect\n"
			"     y - the y position of the new effect\n"
			"     z - the z position of the new effect\n"
			"     heading - the heading in degrees of the new effect\n"
			"     pitch - the pitch in degrees of the new effect\n"
			"     roll - the roll in degrees of the new effect";			
	}

	virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
	{
		if (tokens.size() >= 8)
		{
			unsigned int	id = atoi(tokens.at(0).c_str());
			unsigned int	entityID = atoi(tokens.at(1).c_str());
			double          x = atof(tokens.at(2).c_str());
			double          y = atof(tokens.at(3).c_str());
			double          z = atof(tokens.at(4).c_str());
			double          h = atof(tokens.at(5).c_str());
			double          p = atof(tokens.at(6).c_str());
			double          r = atof(tokens.at(7).c_str());

			osg::Matrixd mx = Math::instance()->toMatrix(x, y, z, h, p, r);

			_ig->bindEffect(id, entityID, mx);

			return 0;
		}

		return -1;
	}
protected:
	OpenIG::Base::ImageGenerator* _ig;
};

class CacheCommand : public OpenIG::Base::Commands::Command
{
public:
	CacheCommand(Engine* ig)
		: _ig(ig) {}

	virtual const std::string getUsage() const
	{
		return "filename1 filename2 filename3 ...";
	}

	virtual const std::string getArgumentsFormat() const
	{
		return "S";
	}

	virtual const std::string getDescription() const
	{
		return  "adds files to use the cache\n"
			"     filename1 ... the simple file name with extension without path";
	}

	virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
	{
		if (tokens.size() > 0)
		{
			OpenIG::Base::StringUtils::StringList files;

			for (unsigned int i = 0; i < tokens.size(); ++i)
			{	
				files.push_back(tokens.at(i));
			}

			_ig->addFilesToBeCached(files);

			return 0;
		}

		return -1;
	}
protected:
	Engine* _ig;
};

class LoadConfigCommand : public OpenIG::Base::Commands::Command
{
public:
	LoadConfigCommand() {}

	virtual const std::string getUsage() const
	{
		return "configfilename";
	}

	virtual const std::string getArgumentsFormat() const
	{
		return "F";
	}

	virtual const std::string getDescription() const
	{
		return  "loads config file\n"
			"     configfilename ... the file name of the config file";
	}

	virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
	{
		if (tokens.size() == 1)
		{
			OpenIG::Base::Commands::instance()->loadScript(tokens.at(0));

			return 0;
		}

		return -1;
	}

};

}

void Engine::initCommands()
{
	Commands::instance()->addCommand("addentity", new AddEntityCommand(this));
	Commands::instance()->addCommand("removeentity", new RemoveEntityCommand(this));
	Commands::instance()->addCommand("setcamerapos", new SetCameraPositionCommand(this));
	Commands::instance()->addCommand("bindcamera", new BindCameraToEntityCommand(this));
	Commands::instance()->addCommand("unbindcamera", new UnbindCameraFromEntityCommand(this));
	Commands::instance()->addCommand("tod", new SetTimeOfDayCommand(this));
	Commands::instance()->addCommand("wind", new SetWindCommand(this));
	Commands::instance()->addCommand("addlight", new AddLightCommand(this));
	Commands::instance()->addCommand("bindlighttocamera", new BindLightTocameraCommand(this));
	Commands::instance()->addCommand("updatelightattr", new LightAttribsCommand(this));
	Commands::instance()->addCommand("keypad", new KeypadCommand(this,_keypad,_keypadCameraManipulator));
	Commands::instance()->addCommand("bindlight", new BindLightCommand(this));
	Commands::instance()->addCommand("unbindlight", new UnbindLightCommand(this));
	Commands::instance()->addCommand("unbindlightfromcamera", new UnbindLightFromCameraCommand(this));
	Commands::instance()->addCommand("updatelight", new UpdateLightCommand(this));
	Commands::instance()->addCommand("removelight", new RemoveLightCommand(this));
	Commands::instance()->addCommand("bind", new BindEntityCommand(this));
	Commands::instance()->addCommand("unbind", new UnbindEntityCommand(this));
	Commands::instance()->addCommand("offset", new OffsetCommand(this));
	Commands::instance()->addCommand("playanim", new AnimationCommand(this));
	Commands::instance()->addCommand("mplayanim", new MultipleAnimationCommand(this));
	Commands::instance()->addCommand("reloadentity", new ReloadEntityCommand(this));
	Commands::instance()->addCommand("bindeffect", new BindEffectCommand(this));
	Commands::instance()->addCommand("addeffect", new AddEffectCommand(this));
	Commands::instance()->addCommand("cache", new CacheCommand(this));
	Commands::instance()->addCommand("loadconfig", new LoadConfigCommand());
}
