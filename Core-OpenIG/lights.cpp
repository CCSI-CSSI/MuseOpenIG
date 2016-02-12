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

#include <Core-Base/mathematics.h>

#include <iostream>

#include <osg/ValueObject>

#include <OpenThreads/ScopedLock>

using namespace OpenIG;
using namespace OpenIG::Base;

void Engine::createSunMoonLight()
{
	if (_lightImplementationCallback.valid())
	{
		LightAttributes lightAttributes;

		_lightImplementationCallback->createLight(0, lightAttributes, _lightsGroup);
	}
}

void Engine::addLight(unsigned int id, const LightAttributes& lightAttributes, const osg::Matrixd& mx)
{
    removeLight(id);
	
    osg::ref_ptr<osg::MatrixTransform> mxt = new osg::MatrixTransform;
    mxt->setMatrix(mx);

    _lights[id] = mxt;
    _lightsGroup->insertChild(0,mxt);

    if (_lightImplementationCallback.valid())
    {
        osg::ref_ptr<osg::Node> light = dynamic_cast<osg::Node*>(_lightImplementationCallback->createLight(id,lightAttributes,_lightsGroup));
        if (light.valid())
        {
            mxt->addChild(light);
        }
    }
}

void Engine::updateLightAttributes(unsigned int id, const LightAttributes& definition)
{
    LightsMapIterator itr = _lights.find(id);
    if ( itr != _lights.end() && _lightImplementationCallback.valid())
    {
        _lightImplementationCallback->updateLight(id,definition);

		LightAttributes& attr = _lightAttributes[id];

		if ((definition.dirtyMask & LightAttributes::AMBIENT) == LightAttributes::AMBIENT) attr.ambient = definition.ambient;
		if ((definition.dirtyMask & LightAttributes::DIFFUSE) == LightAttributes::DIFFUSE) attr.diffuse = definition.diffuse;
		if ((definition.dirtyMask & LightAttributes::BRIGHTNESS) == LightAttributes::BRIGHTNESS) attr.brightness = definition.brightness;
		if ((definition.dirtyMask & LightAttributes::CLOUDBRIGHTNESS) == LightAttributes::CLOUDBRIGHTNESS) attr.cloudBrightness = definition.cloudBrightness;
		if ((definition.dirtyMask & LightAttributes::WATERBRIGHTNESS) == LightAttributes::WATERBRIGHTNESS) attr.waterBrightness = definition.waterBrightness;
		if ((definition.dirtyMask & LightAttributes::CONSTANTATTENUATION) == LightAttributes::CONSTANTATTENUATION) attr.constantAttenuation = definition.constantAttenuation;
		if ((definition.dirtyMask & LightAttributes::ENABLED) == LightAttributes::ENABLED) attr.enabled = definition.enabled;
		if ((definition.dirtyMask & LightAttributes::SPECULAR) == LightAttributes::SPECULAR) attr.specular = definition.specular;
		if ((definition.dirtyMask & LightAttributes::SPOTCUTOFF) == LightAttributes::SPOTCUTOFF) attr.spotCutoff = definition.spotCutoff;
        if ((definition.dirtyMask & LightAttributes::RANGES) == LightAttributes::RANGES)
        {
            attr.fStartRange = definition.fStartRange;
            attr.fEndRange = definition.fEndRange;
        }
        if ((definition.dirtyMask & LightAttributes::ANGLES) == LightAttributes::ANGLES)
        {
            attr.fSpotInnerAngle = definition.fSpotInnerAngle;
            attr.fSpotOuterAngle = definition.fSpotOuterAngle;
        }
    }
}

OpenIG::Base::ImageGenerator::LightAttributesMap& Engine::getLightAttributesMap()
{
	return _lightAttributes;
}

OpenIG::Base::LightAttributes Engine::getLightAttributes(unsigned int id)
{	
	LightAttributesMap::iterator itr = _lightAttributes.find(id);
	if (itr != _lightAttributes.end())
	{		
		return itr->second;
	}

	LightAttributes attr;
	return attr;
}

void Engine::removeLight(unsigned int id)
{
    LightsMapIterator itr = _lights.find(id);
    if ( itr != _lights.end())
    {
        osg::ref_ptr<osg::MatrixTransform> light = itr->second;

        const osg::Node::ParentList& pl = light->getParents();

        for ( size_t i=0; i<pl.size(); ++i )
        {
            pl[i]->removeChild(light);
        }

        _lightsGroup->removeChild(light);
        _lights.erase(itr);
    }
}

void Engine::updateLight(unsigned int id, const osg::Matrixd& mx)
{
    LightsMapIterator itr = _lights.find(id);
    if ( itr != _lights.end())
    {
        itr->second->setMatrix(mx);
    }
}

void Engine::bindLightToEntity(unsigned int id, unsigned int entityId)
{
    LightsMapIterator itr = _lights.find(id);
    if ( itr != _lights.end())
    {
        EntityMapIterator eitr = _entities.find(entityId);
        if ( eitr != _entities.end() )
        {
            osg::ref_ptr<osg::MatrixTransform> light = itr->second;

            const osg::Node::ParentList& pl = light->getParents();

            for ( size_t i=0; i<pl.size(); ++i )
            {
                pl[i]->removeChild(light);
            }

            _lightsGroup->removeChild(light);

            eitr->second->addChild(light);
        }
    }
}

void Engine::unbindLightFromEntity(unsigned int id)
{
    LightsMapIterator itr = _lights.find(id);
    if ( itr != _lights.end())
    {
        osg::ref_ptr<osg::MatrixTransform> light = itr->second;

        osg::NodePath np;
        np.push_back(light);

        osg::ref_ptr<osg::Group> parent = light->getNumParents() ? light->getParent(0) : 0;
        while (parent.valid())
        {
            np.insert(np.begin(),parent);
            parent = parent->getNumParents() ? parent->getParent(0) : 0;
        }

        osg::Matrixd mx = osg::computeLocalToWorld(np);
        light->setMatrix(mx);

        const osg::Node::ParentList& pl = light->getParents();

        for ( size_t i=0; i<pl.size(); ++i )
        {
            pl[i]->removeChild(light);
        }

        _lightsGroup->insertChild(0,light);
    }
}

struct FindAndEnableLightNodeVisitor : public osg::NodeVisitor
{
    FindAndEnableLightNodeVisitor(unsigned int id, bool enabled, Engine* ig)
        : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
        , _id(id)
        , _enabled(enabled)
        , _ig(ig)
    {

    }

    virtual void apply(osg::Node& node)
    {
        osg::ref_ptr<osg::LightSource> ls = dynamic_cast<osg::LightSource*>(&node);
        if (ls.valid())
        {
            osg::ref_ptr<osg::Light> light = ls->getLight();
            if (light.valid())
            {
                unsigned int id = 0;
                light->getUserValue("id",id);

                if (id == _id)
                {
                    light->setUserValue("enabled",(bool)_enabled);

                    if (_ig->getLightImplementationCallback())
                    {
                        OpenIG::Base::LightAttributes la;
                        la.enabled = _enabled;
                        la.dirtyMask = OpenIG::Base::LightAttributes::ENABLED;

                        _ig->getLightImplementationCallback()->updateLight(id,la);
                    }

                }
            }
        }
        traverse(node);
    }

protected:
    unsigned int            _id;
    bool                    _enabled;
    Engine*          _ig;
};

void Engine::enableLight(unsigned int id, bool enable)
{
    LightsMapIterator itr = _lights.find(id);
    if ( itr != _lights.end())
    {
        FindAndEnableLightNodeVisitor nv(id,enable,this);

        switch (enable)
        {
        case true:
            itr->second->setNodeMask(0xFFFFFFFC);
            itr->second->accept(nv);
            break;
        case false:
            itr->second->accept(nv);
            itr->second->setNodeMask(0x0);
            break;
        }        
    }
}

bool Engine::isLightEnabled(unsigned int id)
{
    LightsMapIterator itr = _lights.find(id);
    if ( itr != _lights.end())
    {
        return itr->second->getNodeMask() != 0x0;
    }
    return false;
}

class FollowCameraUpdateCallback : public osg::NodeCallback
{
public:
    FollowCameraUpdateCallback(OpenIG::Base::ImageGenerator* ig)
        : _ig(ig)
    {

    }

    virtual void operator()(osg::Node* node, osg::NodeVisitor*)
    {
        osg::MatrixTransform* mxt = dynamic_cast<osg::MatrixTransform*>(node);
        if (!mxt) return;

        osg::ref_ptr<osg::Camera> camera = _ig->getViewer()->getView(0)->getCamera();

        osg::Matrixd mx = camera->getInverseViewMatrix();
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
        double h = 0.0;
        double p = 0.0;
        double r = 0.0;

        OpenIG::Base::Math::instance()->fromMatrix(mx,x,y,z,h,p,r);

        osg::Matrixd hmx;
        hmx.makeRotate(osg::DegreesToRadians(h),osg::Vec3(0,0,1));
        osg::Matrixd pmx;
        pmx.makeRotate(osg::DegreesToRadians(p),osg::Vec3(1,0,0));
        osg::Matrixd rmx;
        rmx.makeRotate(osg::DegreesToRadians(r),osg::Vec3(0,1,0));

        osg::Matrixd worldMx = rmx * pmx * hmx * osg::Matrix::translate(osg::Vec3(x,y,z));
        osg::RefMatrix* refMx = dynamic_cast<osg::RefMatrix*>(mxt->getUserData());
        osg::Matrixd offsetMx = refMx ? *refMx : osg::Matrixd::identity();

        mxt->setMatrix( offsetMx * worldMx );
    }

protected:
    OpenIG::Base::ImageGenerator*     _ig;
};

void Engine::bindLightToCamera(unsigned int id, const osg::Matrixd& offset)
{
    unbindLightFromEntity(id);

    LightsMapIterator itr = _lights.find(id);
    if ( itr != _lights.end())
    {
        itr->second->setUpdateCallback(new FollowCameraUpdateCallback(this));
        itr->second->setUserData( new osg::RefMatrixd(offset));
    }
}

void Engine::unbindLightFromcamera(unsigned int id)
{
    LightsMapIterator itr = _lights.find(id);
    if ( itr != _lights.end())
    {
        itr->second->setUpdateCallback(0);
        itr->second->setUserData(0);
    }
}
