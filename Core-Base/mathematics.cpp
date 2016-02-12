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
#include "mathematics.h"


using namespace OpenIG::Base;

Math* Math::instance()
{
    static Math s_Math;
    return &s_Math;
}

// Code snippet borrowed from osgGA::StandardManipulator
void Math::fixVerticalAxis( osg::Quat& rotation, const osg::Vec3d& localUp, bool disallowFlipOver)
{
    // camera direction vectors
    osg::Vec3d cameraUp = rotation * osg::Vec3d( 0.,1.,0. );
    osg::Vec3d cameraRight = rotation * osg::Vec3d( 1.,0.,0. );
    osg::Vec3d cameraForward = rotation * osg::Vec3d( 0.,0.,-1. );

    // computed directions
    osg::Vec3d newCameraRight1 = cameraForward ^ localUp;
    osg::Vec3d newCameraRight2 = cameraUp ^ localUp;
    osg::Vec3d newCameraRight = (newCameraRight1.length2() > newCameraRight2.length2()) ?
                            newCameraRight1 : newCameraRight2;
    if( newCameraRight * cameraRight < 0. )
        newCameraRight = -newCameraRight;

    // vertical axis correction
    osg::Quat rotationVerticalAxisCorrection;
    rotationVerticalAxisCorrection.makeRotate( cameraRight, newCameraRight );

    double x1 = 0.0;
    double y1 = 0.0;
    double z1 = 0.0;
    double h1 = 0.0;
    double p1 = 0.0;
    double r1 = 0.0;
    fromMatrix(osg::Matrixd::rotate(rotation),x1,y1,z1,h1,p1,r1);

    // rotate camera
    rotation *= rotationVerticalAxisCorrection;

    double x2 = 0.0;
    double y2 = 0.0;
    double z2 = 0.0;
    double h2 = 0.0;
    double p2 = 0.0;
    double r2 = 0.0;
    fromMatrix(osg::Matrixd::rotate(rotation),x2,y2,z2,h2,p2,r2);

    rotation = toMatrix(x1,y1,z1,h1,0,r2).getRotate();

    if( disallowFlipOver )
    {

        // make viewer's up vector to be always less than 90 degrees from "up" axis
        osg::Vec3d newCameraUp = newCameraRight ^ cameraForward;
        if( newCameraUp * localUp < 0. )
            rotation = osg::Quat( osg::PI, osg::Vec3d( 0.,0.,1. ) ) * rotation;

    }
}

// Code snippet borrowed from osgGA::StandardManipulator
osg::Vec3d Math::getUpVector(const osg::CoordinateFrame& cf)
{
    return osg::Vec3d(cf(2,0),cf(2,1),cf(2,2));
}

// Code snippet borrowed from osgGA::StandardManipulator
void Math::fixVerticalAxis( osg::Vec3d& , osg::Quat& rotation, bool disallowFlipOver)
{
   osg::CoordinateFrame coordinateFrame = osg::CoordinateFrame();
   osg::Vec3d localUp = osg::Vec3d(0,0,1);//getUpVector( coordinateFrame );

   fixVerticalAxis( rotation, localUp, disallowFlipOver );
}

osg::Matrixd Math::toMatrix(double x, double y, double z, double h, double p, double r)
{
    osg::Matrixd mxR;
    mxR.makeRotate(osg::DegreesToRadians(r), osg::Vec3(0, 1, 0));
    osg::Matrixd mxP;
    mxP.makeRotate(osg::DegreesToRadians(p), osg::Vec3(1, 0, 0));
    osg::Matrixd mxH;
    mxH.makeRotate(osg::DegreesToRadians(h), osg::Vec3(0, 0, 1));
    osg::Matrixd mxT;
    mxT.makeTranslate(osg::Vec3(x, y, z));

    return (mxR*mxP*mxH*mxT);
}

void Math::fromMatrix(const osg::Matrixd& mx, double& x, double& y, double& z, double& h, double& p, double& r)
{
    x = mx.getTrans().x();
    y = mx.getTrans().y();
    z = mx.getTrans().z();

    osg::Vec3 hpr = fromQuat(mx.getRotate());
    h = osg::RadiansToDegrees(hpr.x());
    p = osg::RadiansToDegrees(hpr.y())-90.0;
    r = osg::RadiansToDegrees(hpr.z());
}

osg::Vec3d Math::fromQuat(const osg::Quat& quat)
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
