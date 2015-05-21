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
#ifndef MATHEMATICS_H
#define MATHEMATICS_H

#include <IgCore/export.h>

#include <osg/Vec3d>
#include <osg/Vec3>
#include <osg/Matrixd>
#include <osg/Quat>
#include <osg/CoordinateSystemNode>

namespace igcore
{

/*! The Math singleton class. Contais convinient handy methods
 * \brief The Math singleton class
 * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
 * \copyright (c)Compro Computer Services, Inc.
 * \date      Sun Jan 11 2015
 */
class IGCORE_EXPORT Math
{
protected:
    Math() {}
    ~Math() {}
public:

    /*!
     * \brief The singleton
     * \return The singleton
     * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
     * \copyright (c)Compro Computer Services, Inc.
     * \date      Sun Jan 11 2015
     */
    static Math* instance();

    /*!
     * \brief Creates osg::Matrixd from coordinates and euler in degrees
     * \param x x coordinate
     * \param y y coordinate
     * \param z z coordinate
     * \param h heading in degrees
     * \param p pitch in degrees
     * \param r roll in degrees
     * \return matrix
     * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
     * \copyright (c)Compro Computer Services, Inc.
     * \date      Sun Jan 11 2015
     */
    osg::Matrixd toMatrix(double x, double y, double z, double h, double p, double r);

    /*!
     * \brief Decompose a matrix to coordinates and euler
     * \param mx the matrix
     * \param x resulting x coordinate
     * \param y resulting y coordinate
     * \param z resulting z coordinate
     * \param h resulting heading in degrees
     * \param p resulting pitch in degrees
     * \param r resulting roll in degrees
     * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
     * \copyright (c)Compro Computer Services, Inc.
     * \date      Sun Jan 11 2015
     */
    void         fromMatrix(const osg::Matrixd& mx, double& x, double& y, double& z, double& h, double& p, double& r);

    /*!
     * \brief Euler from osg::Quat ernion
     * \param the Quaternion
     * \return eulers in degrees, x() heading, y() pitch, z() roll
     * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
     * \copyright (c)Compro Computer Services, Inc.
     * \date      Sun Jan 11 2015
     */
    osg::Vec3d   fromQuat(const osg::Quat& q);

    /*!
     * \brief Code snippet borrowed from osgGA::StandardManipulator. Please refer to the OSG documentation
     * \param eye
     * \param rotation
     * \param disallowFlipOver
     * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
     * \copyright (c)Compro Computer Services, Inc.
     * \date      Sun Jan 11 2015
     */
    void         fixVerticalAxis( osg::Vec3d& eye, osg::Quat& rotation, bool disallowFlipOver = true);

protected:
    /*!
     * \brief Code snippet borrowed from osgGA::StandardManipulator. Please refer to the OSG documentation
     * \param cf
     * \return
     * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
     * \copyright (c)Compro Computer Services, Inc.
     * \date      Sun Jan 11 2015
     */
    osg::Vec3d   getUpVector(const osg::CoordinateFrame& cf);

    /*!
     * \brief Code snippet borrowed from osgGA::StandardManipulator. Please refer to the OSG documentation
     * \param rotation
     * \param localUp
     * \param disallowFlipOver
     * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
     * \copyright (c)Compro Computer Services, Inc.
     * \date      Sun Jan 11 2015
     */
    void         fixVerticalAxis( osg::Quat& rotation, const osg::Vec3d& localUp, bool disallowFlipOver = true);
};

} // namespace

#endif // MATHEMATICS_H
