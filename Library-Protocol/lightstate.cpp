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
//#*	author    Trajce Nikolov Nick openig@compro.net
//#*	copyright(c)Compro Computer Services, Inc.
//#*
//#*    Please direct any questions or comments to the OpenIG Forums
//#*    Email address: openig@compro.net
//#*


#include <Library-Protocol/lightstate.h>

using namespace OpenIG::Library::Protocol;

LightState::LightState()
	: id(0)	
	, brightness(1.f)
	, constantAttenuation(50.f)
	, spotCutoff(20.f)
	, enabled(true)
	, cloudBrightness(1.f)
	, waterBrightness(1.f)
	, lod(1000.f)
	, realLightLOD(1000.f)
	, dirtyMask(0xffffffff)
	, startRange(0.f)
	, endRange(100.f)
	, spotInnerAngle(10.f)
	, spotOuterAngle(120.f)
	, lightType(0)
{
}

int LightState::write(OpenIG::Library::Networking::Buffer &buf) const
{
	buf << (unsigned char)opcode();

	buf << id;

	buf << mx(0, 0) << mx(0, 1) << mx(0, 2) << mx(0, 3);
	buf << mx(1, 0) << mx(1, 1) << mx(1, 2) << mx(1, 3);
	buf << mx(2, 0) << mx(2, 1) << mx(2, 2) << mx(2, 3);
	buf << mx(3, 0) << mx(3, 1) << mx(3, 2) << mx(3, 3);

	buf << ambient.r() << ambient.g() << ambient.b() << ambient.a();
	buf << diffuse.r() << diffuse.g() << diffuse.b() << diffuse.a();
	buf << specular.r() << specular.g() << specular.b() << specular.a();

	buf << brightness;
	buf << constantAttenuation;
	buf << spotCutoff;
	buf << (unsigned int)(enabled ? 1 : 0);
	buf << cloudBrightness;
	buf << waterBrightness;
	buf << lod;
	buf << realLightLOD;
	buf << dirtyMask;
	buf << startRange;
	buf << endRange;
	buf << spotInnerAngle;
	buf << spotOuterAngle;
	buf << lightType;

	return 
		sizeof(unsigned char) +
		sizeof(unsigned int) +
		sizeof(osg::Matrixd::value_type) * 16 +
		sizeof(osg::Vec4::value_type) * 3 * 4 +
		sizeof(float) * 9 +
		sizeof(double) * 2 +
		sizeof(unsigned int) * 3;
}

int LightState::read(OpenIG::Library::Networking::Buffer &buf)
{
	unsigned char op;

	buf >> op;

	buf >> id;

	buf >> mx(0, 0) >> mx(0, 1) >> mx(0, 2) >> mx(0, 3);
	buf >> mx(1, 0) >> mx(1, 1) >> mx(1, 2) >> mx(1, 3);
	buf >> mx(2, 0) >> mx(2, 1) >> mx(2, 2) >> mx(2, 3);
	buf >> mx(3, 0) >> mx(3, 1) >> mx(3, 2) >> mx(3, 3);

	buf >> ambient.r() >> ambient.g() >> ambient.b() >> ambient.a();
	buf >> diffuse.r() >> diffuse.g() >> diffuse.b() >> diffuse.a();
	buf >> specular.r() >> specular.g() >> specular.b() >> specular.a();

	buf >> brightness;
	buf >> constantAttenuation;
	buf >> spotCutoff;

	unsigned int i;
	buf >> i;
	if (i == 0)
		enabled = false;
	else
		enabled = true;

	buf >> cloudBrightness;
	buf >> waterBrightness;
	buf >> lod;
	buf >> realLightLOD;
	buf >> dirtyMask;
	buf >> startRange;
	buf >> endRange;
	buf >> spotInnerAngle;
	buf >> spotOuterAngle;
	buf >> lightType;

	return
		sizeof(unsigned char) +
		sizeof(unsigned int) +
		sizeof(osg::Matrixd::value_type) * 16 +
		sizeof(osg::Vec4::value_type) * 3 * 4 +
		sizeof(float) * 9 +
		sizeof(double) * 2 +
		sizeof(unsigned int) * 3;
}