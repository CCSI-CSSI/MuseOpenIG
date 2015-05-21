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

#ifndef LIGHTPOINTNODE_H
#define LIGHTPOINTNODE_H

#include <osgSim/LightPointNode>
#include <osgSim/LightPointSystem>

#include "LightPointDrawable.h"
#include "LightPointSpriteDrawable.h"

#include <osg/Timer>
#include <osg/BoundingBox>
#include <osg/BlendFunc>
#include <osg/Material>
#include <osg/PointSprite>

#include <osgUtil/CullVisitor>

#include <typeinfo>

namespace igplugins
{

class LightPointNode : public osgSim::LightPointNode
{
public:
    LightPointNode(const osgSim::LightPointNode& lpn, osg::CopyOp op = osg::CopyOp::SHALLOW_COPY)
        : osgSim::LightPointNode(lpn,op)
    {

    }

    virtual void traverse(osg::NodeVisitor& nv)
    {
        if (_lightPointList.empty())
        {
            // no light points so no op.
            return;
        }

        osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(&nv);

        // should we disable small feature culling here?
        if (cv /*&& !cv->isCulled(_bbox)*/)
        {

            osg::Matrix matrix = *(cv->getModelViewMatrix());
            osg::RefMatrix& projection = *(cv->getProjectionMatrix());
            osgUtil::StateGraph* rg = cv->getCurrentStateGraph();

            if (rg->leaves_empty())
            {
                // this is first leaf to be added to StateGraph
                // and therefore should not already know current render bin,
                // so need to add it.
                cv->getCurrentRenderBin()->addStateGraph(rg);
            }

            LightPointDrawable* drawable = NULL;
            osg::Referenced* object = rg->getUserData();
            if (object)
            {
                if (typeid(*object)==typeid(LightPointDrawable))
                {
                    // resuse the user data attached to the render graph.
                    drawable = static_cast<LightPointDrawable*>(object);

                }
                else if (typeid(*object)==typeid(LightPointSpriteDrawable))
                {
                    drawable = static_cast<LightPointSpriteDrawable*>(object);
                }
                else
                {
                    // will need to replace UserData.
                    OSG_WARN << "Warning: Replacing osgUtil::StateGraph::_userData to support osgSim::LightPointNode, may have undefined results."<<std::endl;
                }
            }

            if (!drawable)
            {
                drawable = _pointSprites ? new LightPointSpriteDrawable : new LightPointDrawable;
                rg->setUserData(drawable);

                if (cv->getFrameStamp())
                {
                    drawable->setSimulationTime(cv->getFrameStamp()->getSimulationTime());
                }
            }

            // search for a drawable in the RenderLeaf list equal to the attached the one attached to StateGraph user data
            // as this will be our special light point drawable.
            osgUtil::StateGraph::LeafList::iterator litr;
            for(litr = rg->_leaves.begin();
                litr != rg->_leaves.end() && (*litr)->_drawable.get()!=drawable;
                ++litr)
            {}

            if (litr == rg->_leaves.end())
            {
                // haven't found the drawable added in the RenderLeaf list, therefore this may be the
                // first time through LightPointNode in this frame, so need to add drawable into the StateGraph RenderLeaf list
                // and update its time signatures.

                drawable->reset();
                rg->addLeaf(new osgUtil::RenderLeaf(drawable,&projection,NULL,FLT_MAX));

                // need to update the drawable's frame count.
                if (cv->getFrameStamp())
                {
                    drawable->updateSimulationTime(cv->getFrameStamp()->getSimulationTime());
                }

            }

            if (cv->getComputeNearFarMode() != osgUtil::CullVisitor::DO_NOT_COMPUTE_NEAR_FAR)
                cv->updateCalculatedNearFar(matrix,_bbox);


            const float minimumIntensity = 1.0f/256.0f;
            const osg::Vec3 eyePoint = cv->getEyeLocal();

            double time=drawable->getSimulationTime();
            double timeInterval=drawable->getSimulationTimeInterval();

            const osg::Polytope clipvol(cv->getCurrentCullingSet().getFrustum());
            const bool computeClipping = false;//(clipvol.getCurrentMask()!=0);

            //LightPointDrawable::ColorPosition cp;
            for(LightPointList::iterator itr=_lightPointList.begin();
                itr!=_lightPointList.end();
                ++itr)
            {
                const osgSim::LightPoint& lp = *itr;

                if (!lp._on) continue;

                const osg::Vec3& position = lp._position;

                // skip light point if it is not contianed in the view frustum.
                if (computeClipping && !clipvol.contains(position)) continue;

                // delta vector between eyepoint and light point.
                osg::Vec3 dv(eyePoint-position);

                float intensity = (_lightSystem.valid()) ? _lightSystem->getIntensity() : lp._intensity;

                // slip light point if its intensity is 0.0 or negative.
                if (intensity<=minimumIntensity) continue;

                // (SIB) Clip on distance, if close to limit, add transparancy
                float distanceFactor = 1.0f;
                if (_maxVisibleDistance2!=FLT_MAX)
                {
                    if (dv.length2()>_maxVisibleDistance2) continue;
                    else if (_maxVisibleDistance2 > 0)
                        distanceFactor = 1.0f - osg::square(dv.length2() / _maxVisibleDistance2);
                }

                osg::Vec4 color = lp._color;

                // check the sector.
                if (lp._sector.valid())
                {
                    intensity *= (*lp._sector)(dv);

                    // skip light point if it is intensity is 0.0 or negative.
                    if (intensity<=minimumIntensity) continue;

                }

                // temporary accounting of intensity.
                //color *= intensity;

                // check the blink sequence.
                bool doBlink = lp._blinkSequence.valid();
                if (doBlink && _lightSystem.valid())
                    doBlink = (_lightSystem->getAnimationState() == osgSim::LightPointSystem::ANIMATION_ON);

                if (doBlink)
                {
                    osg::Vec4 bs = lp._blinkSequence->color(time,timeInterval);
                    color[0] *= bs[0];
                    color[1] *= bs[1];
                    color[2] *= bs[2];
                    color[3] *= bs[3];
                }

                // if alpha value is less than the min intentsity then skip
                if (color[3]<=minimumIntensity) continue;

                float pixelSize = cv->pixelSize(position,lp._radius);

                //            cout << "pixelsize = "<<pixelSize<<endl;

                // adjust pixel size to account for intensity.
                if (intensity!=1.0) pixelSize *= sqrt(intensity);

                // adjust alpha to account for max range (Fade on distance)
                color[3] *= distanceFactor;

                // round up to the minimum pixel size if required.
                float orgPixelSize = pixelSize;
                if (pixelSize<_minPixelSize) pixelSize = _minPixelSize;

                osg::Vec3 xpos(position*matrix);

                if (lp._blendingMode==osgSim::LightPoint::BLENDED)
                {
                    if (pixelSize<1.0f)
                    {
                        // need to use alpha blending...
                        color[3] *= pixelSize;
                        // color[3] *= osg::square(pixelSize);

                        if (color[3]<=minimumIntensity) continue;

                        drawable->addBlendedLightPoint(0, xpos,color);
                    }
                    else if (pixelSize<_maxPixelSize)
                    {

                        unsigned int lowerBoundPixelSize = (unsigned int)pixelSize;
                        float remainder = osg::square(pixelSize-(float)lowerBoundPixelSize);

                        // (SIB) Add transparency if pixel is clamped to minpixelsize
                        if (orgPixelSize<_minPixelSize)
                            color[3] *= (2.0/3.0) + (1.0/3.0) * sqrt(orgPixelSize / pixelSize);

                        drawable->addBlendedLightPoint(lowerBoundPixelSize-1, xpos,color);
                        color[3] *= remainder;
                        drawable->addBlendedLightPoint(lowerBoundPixelSize, xpos,color);
                    }
                    else // use a billboard geometry.
                    {
                        drawable->addBlendedLightPoint((unsigned int)(_maxPixelSize-1.0), xpos,color);
                    }
                }
                else // ADDITIVE blending.
                {
                    if (pixelSize<1.0f)
                    {
                        // need to use alpha blending...
                        color[3] *= pixelSize;
                        // color[3] *= osg::square(pixelSize);

                        if (color[3]<=minimumIntensity) continue;

                        drawable->addAdditiveLightPoint(0, xpos,color);
                    }
                    else if (pixelSize<_maxPixelSize)
                    {

                        unsigned int lowerBoundPixelSize = (unsigned int)pixelSize;
                        float remainder = osg::square(pixelSize-(float)lowerBoundPixelSize);

                        // (SIB) Add transparency if pixel is clamped to minpixelsize
                        if (orgPixelSize<_minPixelSize)
                            color[3] *= (2.0/3.0) + (1.0/3.0) * sqrt(orgPixelSize / pixelSize);

                        float alpha = color[3];
                        color[3] = alpha*(1.0f-remainder);
                        drawable->addAdditiveLightPoint(lowerBoundPixelSize-1, xpos,color);
                        color[3] = alpha*remainder;
                        drawable->addAdditiveLightPoint(lowerBoundPixelSize, xpos,color);
                    }
                    else // use a billboard geometry.
                    {
                        drawable->addAdditiveLightPoint((unsigned int)(_maxPixelSize-1.0), xpos,color);
                    }
                }
            }

        }
    }
};

} // namespace

#endif // LIGHTPOINTNODE_H

