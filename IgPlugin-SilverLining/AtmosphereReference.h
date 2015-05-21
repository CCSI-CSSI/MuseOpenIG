// Copyright (c) 2008-2012 Sundog Software, LLC. All rights reserved worldwide.

#pragma once

#include <SilverLining.h>
#include <osg/Referenced>

namespace igplugins
{

class AtmosphereReference : public osg::Referenced
{
public:
	SilverLining::Atmosphere *atmosphere;
	bool atmosphereInitialized;

	AtmosphereReference() : atmosphereInitialized(false), atmosphere(0) {}
};

} // namespace
