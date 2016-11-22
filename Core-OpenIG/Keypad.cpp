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
#include "Keypad.h"

using namespace OpenIG;

KeyPadEventHandler::KeyPadEventHandler(OpenIG::Base::ImageGenerator* ig)
    : _delta(0.25)
    , _ig(ig)
{

}

bool KeyPadEventHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&, osg::Object*, osg::NodeVisitor*)
{
    switch (ea.getEventType())
    {
    case(osgGA::GUIEventAdapter::FRAME) :
        update(0.0);
        break;
    case(osgGA::GUIEventAdapter::KEYDOWN) :
        if (ea.getModKeyMask() && osgGA::GUIEventAdapter::KEY_Alt_L)
        switch (ea.getKey())
        {
        case osgGA::GUIEventAdapter::KEY_KP_Add:
        case osgGA::GUIEventAdapter::KEY_Plus:
            _posStep.z() -= _delta;
            break;
        case osgGA::GUIEventAdapter::KEY_KP_Subtract:
        case osgGA::GUIEventAdapter::KEY_Minus:
            _posStep.z() += _delta;
            break;
        case osgGA::GUIEventAdapter::KEY_KP_Enter:
        case osgGA::GUIEventAdapter::KEY_Return:
            _posStep.y() += _delta;
            break;
        case osgGA::GUIEventAdapter::KEY_KP_Delete:
        case osgGA::GUIEventAdapter::KEY_Delete:
        case '.':
            _posStep.y() -= _delta;
            break;
        case osgGA::GUIEventAdapter::KEY_KP_1:
        case osgGA::GUIEventAdapter::KEY_KP_End:
        case osgGA::GUIEventAdapter::KEY_1:
        case osgGA::GUIEventAdapter::KEY_End:
            _posStep.x() -= _delta;
            break;
        case osgGA::GUIEventAdapter::KEY_KP_3:
        case osgGA::GUIEventAdapter::KEY_KP_Page_Down:
        case osgGA::GUIEventAdapter::KEY_3:
        case osgGA::GUIEventAdapter::KEY_Page_Down:
            _posStep.x() += _delta;
            break;
        case osgGA::GUIEventAdapter::KEY_KP_0:
        case osgGA::GUIEventAdapter::KEY_0:
        case osgGA::GUIEventAdapter::KEY_KP_Insert:
        case osgGA::GUIEventAdapter::KEY_Insert:
            _posStep = osg::Vec3d(0.0, 0.0, 0.0);
            _oriStep = osg::Vec3d(0.0, 0.0, 0.0);
            break;
            break;
        case osgGA::GUIEventAdapter::KEY_KP_4:
        case osgGA::GUIEventAdapter::KEY_KP_Left:
        case osgGA::GUIEventAdapter::KEY_4:
        case osgGA::GUIEventAdapter::KEY_Left:
            _oriStep.x() += osg::DegreesToRadians(_delta);
            break;
        case osgGA::GUIEventAdapter::KEY_KP_6:
        case osgGA::GUIEventAdapter::KEY_KP_Right:
        case osgGA::GUIEventAdapter::KEY_6:
        case osgGA::GUIEventAdapter::KEY_Right:
            _oriStep.x() -= osg::DegreesToRadians(_delta);
            break;
        case osgGA::GUIEventAdapter::KEY_KP_5:
        case osgGA::GUIEventAdapter::KEY_5:
            _oriStep = osg::Vec3d(0.0, 0.0, 0.0);
            break;
        case osgGA::GUIEventAdapter::KEY_KP_7:
        case osgGA::GUIEventAdapter::KEY_KP_Home:
        case osgGA::GUIEventAdapter::KEY_7:
        case osgGA::GUIEventAdapter::KEY_Home:
            _oriStep.z() -= osg::DegreesToRadians(_delta);
            break;
        case osgGA::GUIEventAdapter::KEY_KP_9:
        case osgGA::GUIEventAdapter::KEY_KP_Page_Up:
        case osgGA::GUIEventAdapter::KEY_9:
        case osgGA::GUIEventAdapter::KEY_Page_Up:
            _oriStep.z() += osg::DegreesToRadians(_delta);
            break;
        case osgGA::GUIEventAdapter::KEY_KP_8:
        case osgGA::GUIEventAdapter::KEY_KP_Up:
        case osgGA::GUIEventAdapter::KEY_8:
        case osgGA::GUIEventAdapter::KEY_Up:
            _oriStep.y() += osg::DegreesToRadians(_delta);
            break;
        case osgGA::GUIEventAdapter::KEY_KP_2:
        case osgGA::GUIEventAdapter::KEY_KP_Down:
        case osgGA::GUIEventAdapter::KEY_2:
        case osgGA::GUIEventAdapter::KEY_Down:
            _oriStep.y() -= osg::DegreesToRadians(_delta);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return false;
}
void KeyPadEventHandler::setByMatrix(const osg::Matrixd& matrix)
{
    _pos = matrix.getTrans();
    _ori = getHPRfromQuat(matrix.getRotate());

}

osg::Matrixd KeyPadEventHandler::getMatrix() const
{
    osg::Matrixd mxR;
    mxR.makeRotate(_ori.y(), osg::Vec3(1, 0, 0));

    osg::Matrixd mxH;
    mxH.makeRotate(_ori.x(), osg::Vec3(0, 0, 1));

    osg::Matrixd mxP;
    mxP.makeRotate(_ori.z(), osg::Vec3(0, 1, 0));

    return mxR * mxP * mxH * osg::Matrixd::translate(_pos);
}

void KeyPadEventHandler::update(double)
{
    _ori += _oriStep;

    osg::Matrixd mxR;
    mxR.makeRotate(_ori.y(), osg::Vec3(1, 0, 0));

    osg::Matrixd mxH;
    mxH.makeRotate(_ori.x(), osg::Vec3(0, 0, 1));

    osg::Matrixd mxP;
    mxP.makeRotate(_ori.z(), osg::Vec3(0, 1, 0));

    osg::Matrixd mx;
    mx = mxP * mxR * mxH;

    _pos += _posStep * mx;

    if (_entity.valid())
    {
        _entity->setMatrix(getMatrix());
    }

}
osg::Vec3d KeyPadEventHandler::getHPRfromQuat(osg::Quat quat)
{
    // From: http://guardian.curtin.edu.au/cga/faq/angles.html
    // Except OSG exchanges pitch & roll from what is listed on that page
    double qx = quat.x();
    double qy = quat.y();
    double qz = quat.z();
    double qw = quat.w();

    double sqx = qx * qx;
    double sqy = qy * qy;
    double sqz = qz * qz;
    double sqw = qw * qw;

    double term1 = 2 * (qx*qy + qw*qz);
    double term2 = sqw + sqx - sqy - sqz;
    double term3 = -2 * (qx*qz - qw*qy);
    double term4 = 2 * (qw*qx + qy*qz);
    double term5 = sqw - sqx - sqy + sqz;

    double heading = atan2(term1, term2);
    double pitch = atan2(term4, term5);
    double roll = asin(term3);

    return osg::Vec3d(heading, pitch, roll);
}

void KeyPadEventHandler::bindToEntity(unsigned int id)
{
    _entity = _ig->getEntityMap()[id];
    if (_entity.valid())
    {
        setByMatrix(_entity->getMatrix());
    }
}

void KeyPadEventHandler::unbind()
{
    _entity = 0;
}


KeyPadCameraManipulator::KeyPadCameraManipulator(OpenIG::Base::ImageGenerator* ig)
    : _delta(0.25)
    , _ig(ig)
{

}

bool KeyPadCameraManipulator::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&)
{
    switch (ea.getEventType())
    {
    case(osgGA::GUIEventAdapter::FRAME) :
        update(0.0);
        break;
    case(osgGA::GUIEventAdapter::KEYDOWN) :
        if (ea.getModKeyMask() && osgGA::GUIEventAdapter::KEY_Alt_L)
        switch (ea.getKey())
        {
        case osgGA::GUIEventAdapter::KEY_KP_Add:
        case osgGA::GUIEventAdapter::KEY_Plus:
            _posStep.z() -= _delta;
            break;
        case osgGA::GUIEventAdapter::KEY_KP_Subtract:
        case osgGA::GUIEventAdapter::KEY_Minus:
            _posStep.z() += _delta;
            break;
        case osgGA::GUIEventAdapter::KEY_KP_Enter:
        case osgGA::GUIEventAdapter::KEY_Return:
            _posStep.y() += _delta;
            break;
        case osgGA::GUIEventAdapter::KEY_KP_Delete:
        case osgGA::GUIEventAdapter::KEY_Delete:
        case '.':
            _posStep.y() -= _delta;
            break;
        case osgGA::GUIEventAdapter::KEY_KP_1:
        case osgGA::GUIEventAdapter::KEY_KP_End:
        case osgGA::GUIEventAdapter::KEY_1:
        case osgGA::GUIEventAdapter::KEY_End:
            _posStep.x() -= _delta;
            break;
        case osgGA::GUIEventAdapter::KEY_KP_3:
        case osgGA::GUIEventAdapter::KEY_KP_Page_Down:
        case osgGA::GUIEventAdapter::KEY_3:
        case osgGA::GUIEventAdapter::KEY_Page_Down:
            _posStep.x() += _delta;
            break;
        case osgGA::GUIEventAdapter::KEY_KP_0:
        case osgGA::GUIEventAdapter::KEY_0:
        case osgGA::GUIEventAdapter::KEY_KP_Insert:
        case osgGA::GUIEventAdapter::KEY_Insert:
            _posStep = osg::Vec3d(0.0, 0.0, 0.0);
            _oriStep = osg::Vec3d(0.0, 0.0, 0.0);
            break;
            break;
        case osgGA::GUIEventAdapter::KEY_KP_4:
        case osgGA::GUIEventAdapter::KEY_KP_Left:
        case osgGA::GUIEventAdapter::KEY_4:
        case osgGA::GUIEventAdapter::KEY_Left:
            _oriStep.x() += osg::DegreesToRadians(_delta);
            break;
        case osgGA::GUIEventAdapter::KEY_KP_6:
        case osgGA::GUIEventAdapter::KEY_KP_Right:
        case osgGA::GUIEventAdapter::KEY_6:
        case osgGA::GUIEventAdapter::KEY_Right:
            _oriStep.x() -= osg::DegreesToRadians(_delta);
            break;
        case osgGA::GUIEventAdapter::KEY_KP_5:
        case osgGA::GUIEventAdapter::KEY_5:
            _oriStep = osg::Vec3d(0.0, 0.0, 0.0);
            break;
        case osgGA::GUIEventAdapter::KEY_KP_7:
        case osgGA::GUIEventAdapter::KEY_KP_Home:
        case osgGA::GUIEventAdapter::KEY_7:
        case osgGA::GUIEventAdapter::KEY_Home:
            _oriStep.z() -= osg::DegreesToRadians(_delta);
            break;
        case osgGA::GUIEventAdapter::KEY_KP_9:
        case osgGA::GUIEventAdapter::KEY_KP_Page_Up:
        case osgGA::GUIEventAdapter::KEY_9:
        case osgGA::GUIEventAdapter::KEY_Page_Up:
            _oriStep.z() += osg::DegreesToRadians(_delta);
            break;
        case osgGA::GUIEventAdapter::KEY_KP_8:
        case osgGA::GUIEventAdapter::KEY_KP_Up:
        case osgGA::GUIEventAdapter::KEY_8:
        case osgGA::GUIEventAdapter::KEY_Up:
            _oriStep.y() += osg::DegreesToRadians(_delta);
            break;
        case osgGA::GUIEventAdapter::KEY_KP_2:
        case osgGA::GUIEventAdapter::KEY_KP_Down:
        case osgGA::GUIEventAdapter::KEY_2:
        case osgGA::GUIEventAdapter::KEY_Down:
            _oriStep.y() -= osg::DegreesToRadians(_delta);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return false;
}
void KeyPadCameraManipulator::setByMatrix(const osg::Matrixd& matrix)
{
    _pos = matrix.getTrans();
    _ori = getHPRfromQuat(matrix.getRotate());

}

osg::Matrixd KeyPadCameraManipulator::getMatrix() const
{
    osg::Matrixd mxR;
    mxR.makeRotate(_ori.z(), osg::Vec3(0, 1, 0));

    osg::Matrixd mxH;
    mxH.makeRotate(_ori.x(), osg::Vec3(0, 0, 1));

    osg::Matrixd mxP;
    mxP.makeRotate(_ori.y(), osg::Vec3(1, 0, 0));

    return mxP * mxR * mxH * osg::Matrixd::translate(osg::Vec3d(_pos.x(),_pos.y(),_pos.z()));
}

void KeyPadCameraManipulator::setByInverseMatrix(const osg::Matrixd& matrix)
{
    _pos = osg::Matrix::inverse(matrix).getTrans();
    _ori = getHPRfromQuat(osg::Matrix::inverse(matrix).getRotate());
}

osg::Matrixd KeyPadCameraManipulator::getInverseMatrix() const
{
    return osg::Matrix::inverse(getMatrix());
}

void KeyPadCameraManipulator::update(double)
{
    _ori += _oriStep;
    _pos += osg::Vec3d(_posStep.x(),_posStep.z(),-_posStep.y())*osg::Matrix::rotate(getMatrix().getRotate());

}
osg::Vec3d KeyPadCameraManipulator::getHPRfromQuat(osg::Quat quat)
{
    // From: http://guardian.curtin.edu.au/cga/faq/angles.html
    // Except OSG exchanges pitch & roll from what is listed on that page
    double qx = quat.x();
    double qy = quat.y();
    double qz = quat.z();
    double qw = quat.w();

    double sqx = qx * qx;
    double sqy = qy * qy;
    double sqz = qz * qz;
    double sqw = qw * qw;

    double term1 = 2 * (qx*qy + qw*qz);
    double term2 = sqw + sqx - sqy - sqz;
    double term3 = -2 * (qx*qz - qw*qy);
    double term4 = 2 * (qw*qx + qy*qz);
    double term5 = sqw - sqx - sqy + sqz;

    double heading = atan2(term1, term2);
    double pitch = atan2(term4, term5);
    double roll = asin(term3);

    return osg::Vec3d(heading, pitch, roll);
}



