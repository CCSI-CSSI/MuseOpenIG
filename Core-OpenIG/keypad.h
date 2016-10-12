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
#ifndef KEYPAD_H
#define KEYPAD_H

#if defined(OPENIG_SDK)
	#include <OpenIG-Base/imagegenerator.h>
#else
	#include <Core-Base/imagegenerator.h>
#endif

#include <osgGA/GUIEventHandler>
#include <osgGA/CameraManipulator>

namespace OpenIG
{

class DymmuGUIActionAdapter : public osgGA::GUIActionAdapter, public osg::Referenced
{
public:
    virtual void requestRedraw() {}
    virtual void requestContinuousUpdate(bool) {}
    virtual void requestWarpPointer(float,float) {}

};

class KeyPadCameraManipulator : public osgGA::CameraManipulator
{
public:
    KeyPadCameraManipulator(OpenIG::Base::ImageGenerator* ig);

    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&);
    virtual void setByMatrix(const osg::Matrixd& matrix);
    virtual void setByInverseMatrix(const osg::Matrixd& matrix);
    virtual osg::Matrixd getMatrix() const;
    virtual osg::Matrixd getInverseMatrix() const;

protected:
    osg::Vec3d                          _pos;
    osg::Vec3d                          _ori;
    osg::Vec3d                          _posStep;
    osg::Vec3d                          _oriStep;
    double                              _delta;
    OpenIG::Base::ImageGenerator*             _ig;

    void		update(double dt);
    osg::Vec3d	getHPRfromQuat(osg::Quat quat);
};

class KeyPadEventHandler : public osgGA::GUIEventHandler
{
public:
    KeyPadEventHandler(OpenIG::Base::ImageGenerator* ig);

    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor*);    

    void bindToEntity(unsigned int id);
    void unbind();

    virtual void setByMatrix(const osg::Matrixd& matrix);
    virtual osg::Matrixd getMatrix() const;

protected:
    osg::Vec3d                          _pos;
    osg::Vec3d                          _ori;
    osg::Vec3d                          _posStep;
    osg::Vec3d                          _oriStep;
    double                              _delta;
    OpenIG::Base::ImageGenerator*             _ig;
    osg::ref_ptr<osg::MatrixTransform>  _entity;

    void		update(double dt);
    osg::Vec3d	getHPRfromQuat(osg::Quat quat);
};

} // namespace

#endif // KEYPAD_H
