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

#include <IgCore/commands.h>

#include <osgText/Text>
#include <OpenThreads/Mutex>
#include <OpenThreads/ScopedLock>
#include <osgText/Text>
#include <osgGA/GUIEventHandler>
#include <osgViewer/CompositeViewer>
#include <osg/MatrixTransform>
#include <osg/Version>

#include "openig.h"


namespace openig
{

class Terminal : public osgGA::GUIEventHandler
{
public:
	Terminal(
			osgText::Text* text,
			osgViewer::CompositeViewer* v,
			osg::Group* scene,
			osg::Node* terminal)
        : _viewer(v)
        , _text(text)
        , _commandindex(-1)
		, _scene(scene)
		, _terminal(terminal)
		, _position(0)
		, _insert(false)
        , _terminalOn(false)
	{
		createCursor();
		setText("");
		setCursor();
	}
	void addCommand(const std::string& cmd)
	{
		if (!cmd.empty())
		{
			_commands.push_back(cmd);
			_commandindex = _commands.size();
		}
	}

	void setCursor()
	{
		if (_position < _offsets.size())
		{
			double x = _offsets.at(_position);
			double s = osg::maximum(1.0,_charsizes.at(_position));
			_mxcursor->setMatrix(osg::Matrix::scale(osg::Vec3(s,1,1))*osg::Matrix::translate(x,0,-0.01));
		}
		else
		{
			_mxcursor->setMatrix(osg::Matrix::scale(osg::Vec3(1,1,1))*osg::Matrix::translate(_text->getPosition().x(),0,-0.01));
		}
	}

	void createCursor()
	{
		osg::Camera* camera = dynamic_cast<osg::Camera*>(_terminal->asGroup()->getChild(0));
		if (!camera) return;

		_mxcursor = new osg::MatrixTransform;
		camera->addChild(_mxcursor);

		osg::Geode* geode = new osg::Geode;
		_mxcursor->addChild(geode);

		geode->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF|osg::StateAttribute::OVERRIDE|osg::StateAttribute::PROTECTED);

		_cursorgeometry = new osg::Geometry;
		geode->addDrawable(_cursorgeometry);

		osg::Vec3Array* vertices = new osg::Vec3Array;
		vertices->push_back(osg::Vec3(0,0,-0.01));
		vertices->push_back(osg::Vec3(1,0,-0.01));
		vertices->push_back(osg::Vec3(1,22,-0.01));
		vertices->push_back(osg::Vec3(0,22,-0.01));
		_cursorgeometry->setVertexArray(vertices);

		osg::Vec3Array* normals = new osg::Vec3Array;
		normals->push_back(osg::Vec3(0.0f,0.0f,1.0f));
		_cursorgeometry->setNormalArray(normals);
		_cursorgeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);

		osg::Vec4Array* colors = new osg::Vec4Array;
		colors->push_back(osg::Vec4(1.f,.2,0.2f,0.5f));
		_cursorgeometry->setColorArray(colors);
		_cursorgeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

		_cursorgeometry->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,4));
	}

	void calcOffsets()
	{
		_offsets.clear();
		_charsizes.clear();
		_wordoffsets.clear();
		if (_text->getText().size()==0) 
		{
			_offsets.push_back( 0 );
			_charsizes.push_back( 0);
			return;
		}

		osg::Vec2 ll;
		osg::Vec2 lr; 

		osgText::Text::TextureGlyphQuadMap& tgqm = const_cast<osgText::Text::TextureGlyphQuadMap&>(_text->getTextureGlyphQuadMap());
		osgText::Text::TextureGlyphQuadMap::iterator tgqmi = tgqm.begin();

		std::vector<osg::Vec2>				coords;
		std::vector<osgText::Glyph*>		glyphs;
		for ( ; tgqmi != tgqm.end(); tgqmi++ )
		{
			const osgText::Text::GlyphQuads& gq = tgqmi->second;

#if OSG_VERSION_GREATER_OR_EQUAL(3,3,4)
            coords.insert(coords.end(),gq.getCoords()->begin(),gq.getCoords()->end());
#else
            coords.insert(coords.end(),gq.getCoords().begin(),gq.getCoords().end());
#endif
			for (unsigned int i=0; i<gq.getGlyphs().size(); ++i)
			{
				glyphs.push_back(gq.getGlyphs().at(i));
			}
		}
		
		std::list<unsigned int> keys;
		for (unsigned int i=0; i<_text->getText().size(); ++i)
		{
			keys.push_back(_text->getText().at(i));
		}
		while (!keys.empty())
		{
			unsigned int key = keys.front();

			bool found = false;
			for (unsigned int i=0; i<glyphs.size(); ++i)
			{
				osgText::Glyph* g = glyphs.at(i);
				if (g->getGlyphCode()==key)
				{
					ll = coords[1+(i*4)];
					lr = coords[2+(i*4)];
					_offsets.push_back( _text->getPosition().x()+ll.x() );
					_charsizes.push_back( lr.x()-ll.x() );
					found = true;

					glyphs.erase(glyphs.begin()+i);
					coords.erase(coords.begin()+i*4);
					coords.erase(coords.begin()+i*4);
					coords.erase(coords.begin()+i*4);
					coords.erase(coords.begin()+i*4);
					break;
				}
			}
			if (!found)
			{
				_offsets.push_back( _text->getPosition().x()+ll.x() );
				_charsizes.push_back( lr.x()-ll.x() );
			}
			keys.pop_front();
		}

		_offsets.push_back( _text->getPosition().x()+lr.x() );
		_charsizes.push_back( 0);

		std::string text = getFormatedInput();
		for ( unsigned int i=0; i<_text->getText().size(); ++i )
		{
			while (i<text.size() && text.at(i)==' ') ++i;
			_wordoffsets.push_back(i);
			while (i<text.size() && text.at(i)!=' ') ++i;
		}
	}

	void setText(const std::string& text)
	{
		_text->setText(text);
		_text->update();

		calcOffsets();

		_position = text.size();
		setCursor();
	}

	std::string getFormatedInput()
	{
		return _text->getText().createUTF8EncodedString();
	}


	virtual void keyDown(int key)
	{
        if (!_terminalOn) return;

		OpenThreads::ScopedLock<OpenThreads::Mutex>		l(_mutex);

		if (key == osgGA::GUIEventAdapter::KEY_Return)
		{
			addCommand(getFormatedInput());
			setText("");
		}
		bool textFromCommandBuffer = false;
		if (key==osgGA::GUIEventAdapter::KEY_Up)
		{
			if (_commands.size())
			{
				_commandindex = osg::minimum((int)--_commandindex,(int)(_commands.size()-1));
				_commandindex = osg::maximum((int)_commandindex,(int)0);
				textFromCommandBuffer = true;
			}
		}
		if (key==osgGA::GUIEventAdapter::KEY_Down)
		{
			if (_commands.size())
			{
				_commandindex = osg::minimum((int)++_commandindex,(int)(_commands.size()-1));
				textFromCommandBuffer = true;
			}
		}
		if (textFromCommandBuffer)
		{
			std::string cmd = _commands.at(_commandindex);
			setText(cmd);
		}

		if (key==osgGA::GUIEventAdapter::KEY_BackSpace)
		{
			if (_position > 0)
			{
				--_position;
				osgText::String& s = _text->getText();
				if (_position < _text->getText().size()-1)
				{
					for (unsigned int i=_position; i < _text->getText().size()-1; ++i)
					{
						s[i] = s[i+1];
					}
				}
				s.resize(_text->getText().size()-1);
				_text->update();
				calcOffsets();
                setCursor();
			}
		}
		else
		if (key==osgGA::GUIEventAdapter::KEY_Delete)
		{
			{
				if (_position < _text->getText().size())
				{
					osgText::String& s = _text->getText();
					if (_position < _text->getText().size()-1)
					{
						for (unsigned int i=_position; i < _text->getText().size()-1; ++i)
						{
							s[i] = s[i+1];
						}
					}
					s.resize(_text->getText().size()-1);
					_text->update();
					calcOffsets();
					setCursor();
				}
			}
		}
		else
		if (key <= 255 && key >=0)
		{
			osgText::String& s = _text->getText();
			if (_insert)
			{
				if (_position >= s.size())
					s.push_back(0);
			}
			else
			{
				s.push_back(0);
				for (unsigned int i=_text->getText().size()-1; i>_position; --i)
				{
					s[i] = s[i-1];
				}
			}
			s[_position] = key;

			_text->update();

			calcOffsets();

			++_position;
			setCursor();

		}
	}

	
    bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter&)
	{        
		switch(ea.getEventType())
		{
			case(osgGA::GUIEventAdapter::KEYDOWN):
                //if ((ea.getModKeyMask() & osgGA::GUIEventAdapter::KEY_Alt_L) == 0)
				{
					switch (ea.getKey())
					{
					case osgGA::GUIEventAdapter::KEY_Up:
					case osgGA::GUIEventAdapter::KEY_Down:
						keyDown(ea.getKey());
						break;
					case osgGA::GUIEventAdapter::KEY_Left:
                        if (ea.getModKeyMask()!=0 && _terminalOn)
						{
							bool found = false;
							for (unsigned int i=0; i<_wordoffsets.size()-1; ++i)
							{
								if (_wordoffsets.at(i) < _position && _position <= _wordoffsets.at(i+1))
								{
									found = true;
									_position = _wordoffsets.at(i);
									break;
								}
							}
							if (!found)
							{
								_position = _wordoffsets.at(_wordoffsets.size()-1);
							}
							setCursor();
						}
						else
						if (_position > 0) 
						{
							--_position;
							setCursor();
						}
						break;
					case osgGA::GUIEventAdapter::KEY_Right:
                        if (ea.getModKeyMask()!=0 && _terminalOn)
						{
							bool found = false;
							for (unsigned int i=0; i<_wordoffsets.size()-1; ++i)
							{
								if (_wordoffsets.at(i) <= _position && _position < _wordoffsets.at(i+1))
								{
									found = true;
									_position = _wordoffsets.at(i+1);
									break;
								}
							}
							if (!found)
							{
								_position = _wordoffsets.at(_wordoffsets.size()-1);
							}
							setCursor();
						}
						else
						if (_position+1 < getFormatedInput().size() ) 
						{
							++_position;
							setCursor();
						}
						break;
					case osgGA::GUIEventAdapter::KEY_Home:
						_position = 0;
						setCursor();
						break;
					case osgGA::GUIEventAdapter::KEY_End:
						_position = getFormatedInput().size();
						setCursor();
						break;
					case osgGA::GUIEventAdapter::KEY_Escape:
						setText("");
						break;
					case osgGA::GUIEventAdapter::KEY_Return:
						{
                            igcore::Commands::instance()->exec(getFormatedInput());
							keyDown(ea.getKey());
						}
						break;
					case osgGA::GUIEventAdapter::KEY_F8:
						{							
                            _terminalOn = !_terminalOn;
                            switch (_terminalOn)
							{
							case true:
								_scene->addChild(_terminal);
								break;
							case false:
								_scene->removeChild(_terminal);
								break;
							}
						}
						break;
					case osgGA::GUIEventAdapter::KEY_Insert:
						_insert = !_insert;
						switch (_insert)
						{
						case true:
							{
								osg::Vec4Array* colors = new osg::Vec4Array;
								colors->push_back(osg::Vec4(0.2f,.2,1.0f,0.5f));
								_cursorgeometry->setColorArray(colors);
								_cursorgeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
								_cursorgeometry->dirtyDisplayList();
								_cursorgeometry->getOrCreateVertexBufferObject()->dirty();
							}
							break;
						case false:
							{
								osg::Vec4Array* colors = new osg::Vec4Array;
								colors->push_back(osg::Vec4(1.f,.2,0.2f,0.5f));
								_cursorgeometry->setColorArray(colors);
								_cursorgeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
								_cursorgeometry->dirtyDisplayList();
								_cursorgeometry->getOrCreateVertexBufferObject()->dirty();
							}
							break;
						}
						break;
					default:
                        //if (!std::isupper(ea.getKey()))
						{
							keyDown(ea.getKey());
						}
					}
				}
				break;
			case(osgGA::GUIEventAdapter::FRAME):
				{
					static bool loadDefaultScript = true;
					if (loadDefaultScript)
					{
						std::vector< std::string > tokens;
						tokens.push_back("loadscript");
						loadDefaultScript = false;
					}

					static bool cursorOn = true;
					static osg::Timer_t startTick = osg::Timer::instance()->tick();
					osg::Timer_t nowTick = osg::Timer::instance()->tick();	
					if (osg::Timer::instance()->delta_s(startTick,nowTick) > (_insert?0.125:0.25))
					{
						startTick = nowTick;
						cursorOn = !cursorOn;
					}
					switch (cursorOn)
					{
					case true:
						_mxcursor->setNodeMask(0xFFFFFFFF);
						break;
					case false:
						_mxcursor->setNodeMask(0x0);
						break;
					}
				}
				break;
			default:
				break;
		}
		return false;
	}
protected:
	osgViewer::CompositeViewer*	_viewer;
	osgText::Text*				_text;
	std::vector< std::string >	_commands;
	int							_commandindex;
	osg::Group*					_scene;
    osg::ref_ptr<osg::Node>     _terminal;
	unsigned int				_position;
	std::vector<double>			_offsets;
	std::vector<unsigned int>	_wordoffsets;
	std::vector<double>			_charsizes;
	osg::MatrixTransform*		_mxcursor;
	OpenThreads::Mutex			_mutex;
	bool						_insert;
	osg::Geometry*				_cursorgeometry;
    bool                        _terminalOn;
};

void OpenIG::initTerminal()
{
	if (!_viewer.valid())
        return;

    osg::ref_ptr<osg::Group> terminalGroup = new osg::Group;
#if 0
    _viewer->getView(0)->getSceneData()->asGroup()->addChild(terminalGroup);
#endif

	osg::Camera* camera = new osg::Camera;
    terminalGroup->addChild(camera);

	osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
	if (!wsi)
	{
		osg::notify(osg::NOTICE)<<"Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
        return;
	}

	unsigned int width, height;
	wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(0), width, height);

	width = 1280;
	height = 1024;
	camera->setProjectionMatrix(osg::Matrix::ortho2D(0,width,0,height));
    camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    camera->setViewMatrix(osg::Matrix::identity());
    camera->setClearMask(GL_DEPTH_BUFFER_BIT);
    camera->setRenderOrder(osg::Camera::POST_RENDER);
    camera->setAllowEventFocus(true);
    camera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
    //camera->setCullingMode(osg::CullSettings::VIEW_FRUSTUM_CULLING);


	osg::Geode* geode = new osg::Geode;
	camera->addChild(geode);

	geode->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF|osg::StateAttribute::OVERRIDE|osg::StateAttribute::PROTECTED);

	osg::Geometry* geometry = new osg::Geometry;
    geode->addDrawable(geometry);

    float depth = 0.0f;

    osg::Vec3Array* vertices = new osg::Vec3Array;
    vertices->push_back(osg::Vec3(0,0,depth));
    vertices->push_back(osg::Vec3(1280,0,depth));
    vertices->push_back(osg::Vec3(1280,22,depth));
    vertices->push_back(osg::Vec3(0,22,depth));
    geometry->setVertexArray(vertices);

    osg::Vec3Array* normals = new osg::Vec3Array;
    normals->push_back(osg::Vec3(0.0f,0.0f,1.0f));
    geometry->setNormalArray(normals);
    geometry->setNormalBinding(osg::Geometry::BIND_OVERALL);

    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(osg::Vec4(.2f,.2,0.2f,0.7f));
    geometry->setColorArray(colors);
    geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    geometry->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,4));
    geometry->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::ON);
    geometry->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

	osgText::Text* text = new osgText::Text;
	geode->addDrawable(text);

    geode->getOrCreateStateSet()->setRenderBinDetails(8, "RenderBin");

    text->setPosition(osg::Vec3(2,3,1.f));
    text->setFont("fonts/arial.ttf");
	text->setColor(osg::Vec4(1,1,1,1));
	text->setCharacterSize(20);
	text->setCharacterSizeMode(osgText::TextBase::OBJECT_COORDS);
	text->setAxisAlignment(osgText::TextBase::XY_PLANE);
	text->setFontResolution(256,256);
	text->setText("");
	text->setDataVariance(osg::Object::DYNAMIC);

	Terminal* terminal = 0;
	_viewer->getView(0)->addEventHandler( 
		terminal = new Terminal(
                text,
				_viewer.get(),
				_viewer->getView(0)->getSceneData()->asGroup(),
                terminalGroup.get())
    );

}

}
