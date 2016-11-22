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

//#*****************************************************************************
//#*	This code was taken from
//#*	https://github.com/xarray/osgRecipes/tree/master/integrations/osgmygui
//#*	and the author is Wang Rui <wangray84@gmail.com>
//#*****************************************************************************

#include "MYGUIManager.h"

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osg/Version>

#include <iomanip>

#include <Core-Base/Commands.h>

using namespace OpenIG::Plugins;

bool MYGUIHandler::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
    static bool terminalOn = false;
    static bool CTRLpressed = false;
    static bool cameraMaskCaptured = false;
    static unsigned cameraMask = 0x0;
    static bool on = false;

    int width = ea.getWindowWidth(), height = ea.getWindowHeight();
    width = osg::minimum(width, (int)_screenWidth);
    switch ( ea.getEventType() )
    {
    case osgGA::GUIEventAdapter::RESIZE:
        if ( _camera.valid() )
        {
            _camera->setProjectionMatrix( osg::Matrixd::ortho2D(0.0, width, 0.0, height) );
            _camera->setViewport( 0.0, 0.0, width, height );
        }
        break;
    case osgGA::GUIEventAdapter::PUSH:
        if (ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON && CTRLpressed)
        {
            osgUtil::LineSegmentIntersector::Intersections intersections;
            if (_ig->getViewer()->getView(0)->computeIntersections(ea, intersections))
            {
                osgUtil::LineSegmentIntersector::Intersections::iterator hitr = intersections.begin();
                if (hitr != intersections.end())
                {
                    osg::Vec3d intersection = hitr->getWorldIntersectPoint();

                    _manager->setIntersectionPoint(intersection);
                }
            }
        }
        break;
    case osgGA::GUIEventAdapter::KEYUP:
        if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Control_L)
        {
            CTRLpressed = false;
            _manager->setOverallAlpha(0.8);
        }
        break;
    case osgGA::GUIEventAdapter::KEYDOWN:
        if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Control_L)
        {
            CTRLpressed = true;
            _manager->setOverallAlpha(0.05);
        }

        if (ea.getKey() == osgGA::GUIEventAdapter::KEY_F8)
        {
            terminalOn = !terminalOn;
        }

        if (ea.getKey() == osgGA::GUIEventAdapter::KEY_F9 || ea.getKey() == osgGA::GUIEventAdapter::KEY_F12)
        {
            on = !on;
            switch (on)
            {
            case true:
                _camera->setNodeMask(0xFFFFFFFF);
                _cameraManipulator = _ig->getViewer()->getView(0)->getCameraManipulator();
                _ig->getViewer()->getView(0)->setCameraManipulator(0);
                break;
            case false:
                _camera->setNodeMask(0x0);
                _ig->getViewer()->getView(0)->setCameraManipulator(_cameraManipulator,false);
                break;
            }
        }
        break;
    default:
        break;
    }

    if (CTRLpressed) return false;
    if (terminalOn) return false;

    // As MyGUI handle all events within the OpenGL context, we have to record the event here
    // and process it later in the draw implementation
    if ( ea.getEventType()!=osgGA::GUIEventAdapter::FRAME )
        _manager->pushEvent( &ea );
    return false;
}

MYGUIManager::MYGUIManager(const std::string& rootMedia)
    : _gui(0), _platform(0), _rootMedia(rootMedia),
    _resourcePathFile("resources.xml"), _resourceCoreFile("MyGUI_Core.xml"),
    _activeContextID(0), _initialized(false), _root(0), _intersectionPointUpdated(false)
{
    setSupportsDisplayList( false );
    getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    getOrCreateStateSet()->setMode( GL_DEPTH_TEST, osg::StateAttribute::OFF );
}

MYGUIManager::MYGUIManager()
    : _gui(0), _platform(0), _rootMedia(""),
    _resourcePathFile("resources.xml"), _resourceCoreFile("MyGUI_Core.xml"),
    _activeContextID(0), _initialized(false), _root(0), _intersectionPointUpdated(false)
{
    setSupportsDisplayList(false);
    getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
}

MYGUIManager::MYGUIManager( const MYGUIManager& copy,const osg::CopyOp& copyop )
:   osg::Drawable(copy, copyop), _eventsToHandle(copy._eventsToHandle),
    _gui(copy._gui), _platform(copy._platform),
    _resourcePathFile(copy._resourcePathFile),
    _resourceCoreFile(copy._resourceCoreFile),
    _rootMedia(copy._rootMedia),
    _activeContextID(copy._activeContextID),
    _initialized(copy._initialized),
    _root(copy._root)
    , _intersectionPointUpdated(false)
{}
void MYGUIManager::setIntersectionPoint(const osg::Vec3d& point, bool update)
{
    _intersectionPoint = point;

    if (!update)
    {
        _intersectionPointUpdated = true;
        return;
    }

    if (_root && _intersectionPointUpdated)
    {
        try
        {
            MyGUI::ListBox* list = MyGUI::Gui::getInstance().findWidget<MyGUI::ListBox>("CommandListBox");
            MyGUI::EditBox* edit = MyGUI::Gui::getInstance().findWidget<MyGUI::EditBox>("Arguments");

            if (edit && list)
            {
                if (list->getIndexSelected() == MyGUI::ITEM_NONE) return;

                Base::Commands::CommandPtr command = *list->getItemDataAt<Base::Commands::CommandPtr>(
                    list->getItemIndexSelected(), false
                    );
                if (command.valid())
                {
                    const std::string format = command->getArgumentsFormat();
                    const std::string cmd = edit->getOnlyText();

                    Base::StringUtils::Tokens cmdTokens = Base::StringUtils::instance()->tokenize(cmd);
                    Base::StringUtils::Tokens formatTokens = Base::StringUtils::instance()->tokenize(format, ":");

                    std::ostringstream oss;
                    for (size_t i = 0; i < cmdTokens.size(); ++i)
                    {
                        if (i < formatTokens.size())
                        {
                            std::string token = formatTokens.at(i);
                            if (token.at(0) == 'P')
                            {
                                oss << std::setprecision(10) << " " << _intersectionPoint.x() << " " << _intersectionPoint.y() << " " << _intersectionPoint.z();
                                i += 2;
                                continue;
                            }
                            if (token.at(0) == 'F')
                            {
                                oss << " \"" << cmdTokens.at(i) << "\"";
                                continue;
                            }
                        }
                        oss << " " << cmdTokens.at(i);
                    }
                    if (cmdTokens.size() < formatTokens.size() && formatTokens.at(cmdTokens.size()).at(0) == 'P')
                    {
                        oss << std::setprecision(10) << " " << _intersectionPoint.x() << " " << _intersectionPoint.y() << " " << _intersectionPoint.z();
                    }
                    edit->setOnlyText(oss.str().c_str());

                }
            }
        }
        catch (const std::exception& e)
        {
            osg::notify(osg::NOTICE) << "UI: exception: " << e.what() << std::endl;
        }
    }

    _intersectionPointUpdated = false;
}


void* MYGUIManager::loadImage( int& width, int& height, MyGUI::PixelFormat& format, const std::string& filename )
{
    std::string fullname = MyGUI::OpenGLDataManager::getInstance().getDataPath( filename );
    osg::ref_ptr<osg::Image> image = osgDB::readImageFile( fullname );
    void* result = NULL;
    if ( image.valid() )
    {
        width = image->s();
        height = image->t();
        if ( image->getDataType()!=GL_UNSIGNED_BYTE || image->getPacking()!=1 )
        {
            format = MyGUI::PixelFormat::Unknow;
            return result;
        }

        unsigned int num = 0;
        switch ( image->getPixelFormat() )
        {
        case GL_LUMINANCE: case GL_ALPHA: format = MyGUI::PixelFormat::L8; num = 1; break;
        case GL_LUMINANCE_ALPHA: format = MyGUI::PixelFormat::L8A8; num = 2; break;
        case GL_RGB: format = MyGUI::PixelFormat::R8G8B8; num = 3; break;
        case GL_RGBA: format = MyGUI::PixelFormat::R8G8B8A8; num = 4; break;
        default: format = MyGUI::PixelFormat::Unknow; return result;
        }

        unsigned int size = width * height * num;
        unsigned char* dest = new unsigned char[size];
        image->flipVertical();
        if ( image->getPixelFormat()==GL_RGB || image->getPixelFormat()==GL_RGBA )
        {
            // FIXME: I don't an additional conversion here but...
            // MyGUI will automatically consider it as BGR so I should do such stupid thing
            unsigned int step = (image->getPixelFormat()==GL_RGB ? 3 : 4);
            unsigned char* src = image->data();
            for ( unsigned int i=0; i<size; i+=step )
            {
                dest[i+0] = src[i+2];
                dest[i+1] = src[i+1];
                dest[i+2] = src[i+0];
                if ( step==4 ) dest[i+3] = src[i+3];
            }
        }
        else
            memcpy( dest, image->data(), size );
        result = dest;
    }
    return result;
}

void MYGUIManager::saveImage( int width, int height, MyGUI::PixelFormat format, void* texture, const std::string& filename )
{
    GLenum pixelFormat = 0;
    unsigned int internalFormat = 0;
    switch ( format.getValue() )
    {
    case MyGUI::PixelFormat::L8: pixelFormat = GL_ALPHA; internalFormat = 1; break;
    case MyGUI::PixelFormat::L8A8: pixelFormat = GL_LUMINANCE_ALPHA; internalFormat = 2; break;
    case MyGUI::PixelFormat::R8G8B8: pixelFormat = GL_BGR; internalFormat = 3; break;
    case MyGUI::PixelFormat::R8G8B8A8: pixelFormat = GL_BGRA; internalFormat = 4; break;
    default: return;
    }

    unsigned int size = width * height * internalFormat;
    unsigned char* imageData = new unsigned char[size];
    memcpy( imageData, texture, size );

    osg::ref_ptr<osg::Image> image = new osg::Image;
    image->setImage( width, height, 1, internalFormat, pixelFormat, GL_UNSIGNED_BYTE,
        static_cast<unsigned char*>(imageData), osg::Image::USE_NEW_DELETE );
    image->flipVertical();
    osgDB::writeImageFile( *image, filename );
}



void MYGUIManager::drawImplementation( osg::RenderInfo& renderInfo ) const
{
    MYGUIManager* constMe = const_cast<MYGUIManager*>(this);

    unsigned int contextID = renderInfo.getContextID();
    if ( !_initialized )
    {
        constMe->_platform = new MyOpenGLPlatform;
        constMe->_platform->initialise( constMe );
        constMe->setupResources();

        constMe->_gui = new MyGUI::Gui;
        constMe->_gui->initialise( _resourceCoreFile );
        constMe->initializeControls();

        constMe->_activeContextID = contextID;
        constMe->_initialized = true;
    }
    else if ( contextID==_activeContextID )
    {
        osg::State* state = renderInfo.getState();
        state->disableAllVertexArrays();
        state->disableTexCoordPointer( 1 );

        glPushMatrix();
        glPushAttrib( GL_ALL_ATTRIB_BITS );
        if ( _platform )
        {
            constMe->setIntersectionPoint(getIntersectionPoint(),true);
            constMe->lock();
            updateEvents();
            _platform->getMyRenderManagerPtr()->drawOneFrame();
            constMe->unlock();
        }
        glPopAttrib();
        glPopMatrix();

        state->dirtyAllVertexArrays();
#if OSG_VERSION_LESS_OR_EQUAL(3,5,4)
        state->dirtyTexCoordPointer(1);
#endif
        state->dirtyAllAttributes();
    }
}

void MYGUIManager::releaseGLObjects( osg::State* state ) const
{
    if ( state && state->getGraphicsContext() )
    {
        osg::GraphicsContext* gc = state->getGraphicsContext();
        if ( gc->makeCurrent() )
        {
            MYGUIManager* constMe = const_cast<MYGUIManager*>(this);
            if ( constMe->_gui )
            {
                constMe->_gui->shutdown();
                delete constMe->_gui;
                constMe->_gui = nullptr;
            }
            if ( constMe->_platform )
            {
                constMe->_platform->shutdown();
                delete constMe->_platform;
                constMe->_platform = nullptr;
            }
            gc->releaseContext();
        }
    }
}
void MYGUIManager::setOverallAlpha(float alpha)
{
    if (_root)
    {
        _root->setAlpha(alpha);
    }
}


void MYGUIManager::updateEvents() const
{
    unsigned int size = _eventsToHandle.size();
    for ( unsigned int i=0; i<size; ++i )
    {
        const osgGA::GUIEventAdapter& ea = *(_eventsToHandle.front());
        int x = ea.getX(), y = ea.getY(), key = ea.getKey();
        if ( ea.getMouseYOrientation()==osgGA::GUIEventAdapter::Y_INCREASING_UPWARDS )
            y = ea.getWindowHeight() - y;

        switch ( ea.getEventType() )
        {
        case osgGA::GUIEventAdapter::PUSH:
            MyGUI::InputManager::getInstance().injectMousePress( x, y, convertMouseButton(ea.getButton()) );
            break;
        case osgGA::GUIEventAdapter::RELEASE:
            MyGUI::InputManager::getInstance().injectMouseRelease( x, y, convertMouseButton(ea.getButton()) );
            break;
        case osgGA::GUIEventAdapter::DRAG:
        case osgGA::GUIEventAdapter::MOVE:
            MyGUI::InputManager::getInstance().injectMouseMove( x, y, 0 );
            break;
        case osgGA::GUIEventAdapter::KEYDOWN:
            if ( key<127 )
                MyGUI::InputManager::getInstance().injectKeyPress( convertKeyCode(key), (char)key );
            else
                MyGUI::InputManager::getInstance().injectKeyPress( convertKeyCode(key) );
            break;
        case osgGA::GUIEventAdapter::KEYUP:
            MyGUI::InputManager::getInstance().injectKeyRelease( convertKeyCode(key) );
            break;
        case osgGA::GUIEventAdapter::RESIZE:
            _platform->getMyRenderManagerPtr()->setViewSize( ea.getWindowWidth(), ea.getWindowHeight() );
            break;
        default:
            break;
        }
        const_cast<MYGUIManager*>(this)->_eventsToHandle.pop();
    }
}

void MYGUIManager::setupResources()
{
    if ( !_platform ) return;

    _platform->getDataManagerPtr()->addResourceLocation(_rootMedia, false);
    _platform->getDataManagerPtr()->addResourceLocation(_rootMedia + "/Common/Base", false );
    _platform->getDataManagerPtr()->addResourceLocation(_rootMedia + "/MyGUI_Media", false);
    _platform->getDataManagerPtr()->addResourceLocation(_rootMedia + "/Demos/Demo_Themes", false);
    _platform->getDataManagerPtr()->addResourceLocation(_rootMedia + "/Common/Demos", false);
    _platform->getDataManagerPtr()->addResourceLocation(_rootMedia + "/Common/Themes", false);
    _platform->getDataManagerPtr()->addResourceLocation(_rootMedia + "/Demos/Demo_Colour", false);
}

MyGUI::MouseButton MYGUIManager::convertMouseButton( int button ) const
{
    switch ( button )
    {
    case osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON:
        return MyGUI::MouseButton::Left;
    case osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON:
        return MyGUI::MouseButton::Middle;
    case osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON:
        return MyGUI::MouseButton::Right;
    default: break;
    }
    return MyGUI::MouseButton::None;
}

MyGUI::KeyCode MYGUIManager::convertKeyCode( int key ) const
{
    static std::map<int, MyGUI::KeyCode> s_keyCodeMap;
    if ( !s_keyCodeMap.size() )
    {
        s_keyCodeMap['1'] = MyGUI::KeyCode::One;
        s_keyCodeMap['2'] = MyGUI::KeyCode::Two;
        s_keyCodeMap['3'] = MyGUI::KeyCode::Three;
        s_keyCodeMap['4'] = MyGUI::KeyCode::Four;
        s_keyCodeMap['5'] = MyGUI::KeyCode::Five;
        s_keyCodeMap['6'] = MyGUI::KeyCode::Six;
        s_keyCodeMap['7'] = MyGUI::KeyCode::Seven;
        s_keyCodeMap['8'] = MyGUI::KeyCode::Eight;
        s_keyCodeMap['9'] = MyGUI::KeyCode::Nine;
        s_keyCodeMap['0'] = MyGUI::KeyCode::Zero;

        s_keyCodeMap['a'] = MyGUI::KeyCode::A;
        s_keyCodeMap['b'] = MyGUI::KeyCode::B;
        s_keyCodeMap['c'] = MyGUI::KeyCode::C;
        s_keyCodeMap['d'] = MyGUI::KeyCode::D;
        s_keyCodeMap['e'] = MyGUI::KeyCode::E;
        s_keyCodeMap['f'] = MyGUI::KeyCode::F;
        s_keyCodeMap['g'] = MyGUI::KeyCode::G;
        s_keyCodeMap['h'] = MyGUI::KeyCode::H;
        s_keyCodeMap['i'] = MyGUI::KeyCode::I;
        s_keyCodeMap['j'] = MyGUI::KeyCode::J;
        s_keyCodeMap['k'] = MyGUI::KeyCode::K;
        s_keyCodeMap['l'] = MyGUI::KeyCode::L;
        s_keyCodeMap['m'] = MyGUI::KeyCode::M;
        s_keyCodeMap['n'] = MyGUI::KeyCode::N;
        s_keyCodeMap['o'] = MyGUI::KeyCode::O;
        s_keyCodeMap['p'] = MyGUI::KeyCode::P;
        s_keyCodeMap['q'] = MyGUI::KeyCode::Q;
        s_keyCodeMap['r'] = MyGUI::KeyCode::R;
        s_keyCodeMap['s'] = MyGUI::KeyCode::S;
        s_keyCodeMap['t'] = MyGUI::KeyCode::T;
        s_keyCodeMap['u'] = MyGUI::KeyCode::U;
        s_keyCodeMap['v'] = MyGUI::KeyCode::V;
        s_keyCodeMap['w'] = MyGUI::KeyCode::W;
        s_keyCodeMap['x'] = MyGUI::KeyCode::X;
        s_keyCodeMap['y'] = MyGUI::KeyCode::Y;
        s_keyCodeMap['z'] = MyGUI::KeyCode::Z;

        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_F1] = MyGUI::KeyCode::F1;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_F2] = MyGUI::KeyCode::F2;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_F3] = MyGUI::KeyCode::F3;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_F4] = MyGUI::KeyCode::F4;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_F5] = MyGUI::KeyCode::F5;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_F6] = MyGUI::KeyCode::F6;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_F7] = MyGUI::KeyCode::F7;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_F8] = MyGUI::KeyCode::F8;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_F9] = MyGUI::KeyCode::F9;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_F10] = MyGUI::KeyCode::F10;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_Escape] = MyGUI::KeyCode::Escape;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_Tab] = MyGUI::KeyCode::Tab;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_Return] = MyGUI::KeyCode::Return;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_Space] = MyGUI::KeyCode::Space;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_Minus] = MyGUI::KeyCode::Minus;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_Equals] = MyGUI::KeyCode::Equals;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_Backslash] = MyGUI::KeyCode::Backslash;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_Slash] = MyGUI::KeyCode::Slash;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_Semicolon] = MyGUI::KeyCode::Semicolon;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_Comma] = MyGUI::KeyCode::Comma;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_Period] = MyGUI::KeyCode::Period;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_Insert] = MyGUI::KeyCode::Insert;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_Delete] = MyGUI::KeyCode::Delete;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_Home] = MyGUI::KeyCode::Home;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_End] = MyGUI::KeyCode::End;

        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_Num_Lock] = MyGUI::KeyCode::NumLock;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_Scroll_Lock] = MyGUI::KeyCode::ScrollLock;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_Caps_Lock] = MyGUI::KeyCode::Capital;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_BackSpace] = MyGUI::KeyCode::Backspace;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_Page_Down] = MyGUI::KeyCode::PageDown;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_Page_Up] = MyGUI::KeyCode::PageUp;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_Leftbracket] = MyGUI::KeyCode::LeftBracket;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_Rightbracket] = MyGUI::KeyCode::RightBracket;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_Quotedbl] = MyGUI::KeyCode::Apostrophe;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_Left] = MyGUI::KeyCode::ArrowLeft;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_Right] = MyGUI::KeyCode::ArrowRight;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_Up] = MyGUI::KeyCode::ArrowUp;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_Down] = MyGUI::KeyCode::ArrowDown;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_KP_1] = MyGUI::KeyCode::Numpad1;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_KP_2] = MyGUI::KeyCode::Numpad2;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_KP_3] = MyGUI::KeyCode::Numpad3;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_KP_4] = MyGUI::KeyCode::Numpad4;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_KP_5] = MyGUI::KeyCode::Numpad5;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_KP_6] = MyGUI::KeyCode::Numpad6;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_KP_7] = MyGUI::KeyCode::Numpad7;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_KP_8] = MyGUI::KeyCode::Numpad8;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_KP_9] = MyGUI::KeyCode::Numpad9;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_KP_0] = MyGUI::KeyCode::Numpad0;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_KP_Enter] = MyGUI::KeyCode::NumpadEnter;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_Control_L] = MyGUI::KeyCode::LeftControl;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_Control_R] = MyGUI::KeyCode::RightControl;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_Alt_L] = MyGUI::KeyCode::LeftAlt;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_Alt_R] = MyGUI::KeyCode::RightAlt;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_Shift_L] = MyGUI::KeyCode::LeftShift;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_Shift_R] = MyGUI::KeyCode::RightShift;
        s_keyCodeMap[osgGA::GUIEventAdapter::KEY_Num_Lock] = MyGUI::KeyCode::NumLock;
    }

    std::map<int, MyGUI::KeyCode>::iterator itr = s_keyCodeMap.find(key);
    if ( itr!=s_keyCodeMap.end() ) return itr->second;
    return MyGUI::KeyCode::None;
}
