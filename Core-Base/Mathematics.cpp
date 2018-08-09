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
#include "Mathematics.h"


using namespace OpenIG::Base;

const float Math::M_PER_FT = 0.3048f;       /* No. of meters in a linear foot     */
const float Math::M_PER_NMI = 1852.000001f; /* No. of meters in a nautical mile   */
const float Math::FT_PER_M = 3.28084f;      /* No. of feet in a meter             */
const float Math::FT_PER_NMI = 6076.11549f; /* No. of feet in a nautical mile     */

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

osg::Quat Math::toQuat(double h, double p, double r)
{
    osg::Quat q;
    q.makeRotate(
        osg::DegreesToRadians(h), osg::Vec3d(0, 0, 1),
        osg::DegreesToRadians(p), osg::Vec3d(1, 0, 0),
        osg::DegreesToRadians(r), osg::Vec3d(0, 1, 0)
        );

    return q;
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

osg::Matrixd Math::toMuseMatrix(double xIn, double yIn, double zIn, double hIn, double pIn, double rIn)
{
    double RAD_PER_DEG=0.0174532952;
    osg::Vec3f x(1.0f,0.0f,0.0f), y(0.0f,1.0f,0.0f), z(0.0f,0.0f,1.0f);
    osg::Vec3f pos(xIn, yIn, zIn);

    osg::Matrix rot1;
    osg::Matrix rot2;
    osg::Matrix rot3;

    osg::Matrixd _modelPos;
    osg::Matrixd _modelAtt;
    osg::Matrixd offset_matrix;

    rot1.makeRotate( RAD_PER_DEG * hIn, z );
    x = rot1.preMult( x );
    y = rot1.preMult( y );
    rot2.makeRotate( RAD_PER_DEG * pIn, x );
    y = rot2.preMult( y );
    rot3.makeRotate( RAD_PER_DEG * rIn, y );
    _modelAtt.set( rot1 * rot2 * rot3 );

    _modelPos.makeTranslate( pos );

    return( offset_matrix * _modelAtt * _modelPos );
}


//Code from Roni Zanolli openig@compro.net, originally used in a non-opensource Compro legacy Muse product
//Used with permission and modified slightly for use in OpenIG by Curtis Rubel openig@compro.net
osg::Matrixd Math::toViewMatrix(double x, double y, double z, double h, double p, double r)
{
    osg::Matrix m;
    osg::Matrix offset_matrix;
    osg::Matrix matrix;
    osg::Matrix rot1;
    osg::Matrix rot2;
    osg::Matrix rot3;

    osg::Vec3f xaxis(1.0f, 0.0f, 0.0f), yaxis(0.0f, 1.0f, 0.0f), zaxis(0.0f, 0.0f, 1.0f);
    offset_matrix.makeLookAt(osg::Vec3f(0.0, 0.0, 0.0), yaxis, zaxis);

    // Prepare the attitude matrix relative to the OpenGL coordinate system
    osg::Matrix translation = osg::Matrix::translate(0.0, 0.0, 0.0);
    rot1.makeRotate(osg::DegreesToRadians(0.0), yaxis);  // Set Yaw rotation
    offset_matrix.postMult(rot1);
    rot2.makeRotate(-osg::DegreesToRadians(0.0), xaxis); // Set Pitch Rotation
    offset_matrix.postMult(rot2);
    rot3.makeRotate(osg::DegreesToRadians(0.0), zaxis);  // Set Roll Rotation
    offset_matrix.postMult(rot3);
    offset_matrix.postMult(translation);

    // Prepare the position matrix relative to the OpenGL coordinate system
    osg::Matrix translation2 = osg::Matrix::translate(-x, -y, -z);
    rot1.makeRotate(-osg::DegreesToRadians(h), zaxis); // Set Yaw rotation
    matrix.postMult(rot1);
    rot2.makeRotate(-osg::DegreesToRadians(p), xaxis); // Set Pitch Rotation
    matrix.postMult(rot2);
    rot3.makeRotate(-osg::DegreesToRadians(r), yaxis); // Set Roll Rotation
    matrix.postMult(rot3);
    matrix.preMult(translation2);

    //Combine and return the new OSG viewmatrix
    return m = matrix * offset_matrix;
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

osg::Vec3d Math::fromQuat(const osg::Quat& quat, bool degrees)
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

    //Return values in degrees if requested, else its radians
    if(degrees)
    {
        heading = osg::RadiansToDegrees(heading);
        pitch   = osg::RadiansToDegrees(pitch);
        roll    = osg::RadiansToDegrees(roll);
    }

    return osg::Vec3d(heading, pitch, roll);
}

// Code from Daniel Baggio
// https://www.compro.net/openig_forum/viewtopic.php?f=10&t=93&start=10
osg::Matrixd Math::toGeocentricCameraMatrix(double lat, double lon, double alt, double h, double p, double r)
{
    osg::Matrixd myCameraMatrix;

    osg::Matrixd cameraRotation;
    osg::Matrixd cameraTrans;

    osg::Vec3 yawAxis(0.f, 0.f, 1.f);
    osg::Vec3 pitchAxis(1.f, 0.f, 0.f);
    osg::Vec3 rollAxis(0.f, 1.f, 0.f);

    cameraRotation.makeIdentity();
    osg::Matrixd pointAhead;
    pointAhead.makeRotate(osg::DegreesToRadians(90.0), pitchAxis);

    osg::Matrixd pointRight;
    pointRight.makeRotate(osg::DegreesToRadians(-90.0), yawAxis);

    osg::Matrixd yawMatrix;
    yawMatrix.makeRotate(osg::DegreesToRadians(-h + 90), yawAxis);
    osg::Matrixd pitchMatrix;
    pitchMatrix.makeRotate(osg::DegreesToRadians(r), pitchAxis);
    osg::Matrixd rollMatrix;
    rollMatrix.makeRotate(osg::DegreesToRadians(-p), rollAxis);

    cameraRotation = pointAhead * pointRight * pitchMatrix * rollMatrix  * yawMatrix;

    osg::EllipsoidModel em;
    osg::Vec3d ecef;
    em.convertLatLongHeightToXYZ(osg::DegreesToRadians(lat), osg::DegreesToRadians(lon), alt, ecef.x(), ecef.y(), ecef.z());

    cameraTrans.makeTranslate(ecef);

    myCameraMatrix = cameraRotation * cameraTrans;
    return myCameraMatrix;
}
