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

#include <osg/Notify>

#include <iostream>

#include <animation.h>
#include <mathematics.h>

using namespace igcore;

Animations* Animations::instance()
{
    static Animations s_Animations;
    return &s_Animations;
}

Animations::Animations()
{

}

Animations::~Animations()
{

}

void Animations::resetAnimation( igcore::ImageGenerator* ig, unsigned int entityId, const std::string& name)
{
    if (!ig) return;

    RuntimeAnimationsIterator itr = _animations.begin();
    for ( ; itr != _animations.end(); ++itr )
    {
        osg::ref_ptr<RuntimeAnimation> rtanimation = *itr;
        if (!rtanimation.valid()) continue;

        osg::ref_ptr<Animation> animation = rtanimation->_animation.get();
        if (!animation.valid()) continue;

        //osg::notify(osg::NOTICE) << "checking animation " << rtanimation->_animation->_name << std::endl;

        if (rtanimation->_entityId == entityId && animation->_name == name)
        {
            osg::notify(osg::NOTICE) << "runtime animation " << rtanimation->_animation->_name << " reset" << std::endl;

            igcore::Animations::Animation::SequencesMapIterator smitr = rtanimation->_animation->_sequences.begin();
            for ( ; smitr != rtanimation->_animation->_sequences.end(); ++smitr)
            {
                igcore::Animations::Animation::Sequence* sequence = smitr->second;
                sequence->_enabled = true;
            }

            rtanimation->_startTime = osg::Timer::instance()->tick();
        }
    }

    ImageGenerator::Entity& entity = ig->getEntityMap()[entityId];
    if (!entity.valid()) return;

    AnimationContainer* ac = dynamic_cast<AnimationContainer*>(entity->getUserData());
    if (!ac) return;

    osg::ref_ptr<Animation> animation = (*ac)[name];
    if (!animation.valid()) return;

    Animation::SequencesMapIterator sitr = animation->_sequences.begin();
    for ( ; sitr != animation->_sequences.end(); ++sitr )
    {
        Animation::Sequence* sequence = sitr->second;
        if (!sequence) continue;

        ImageGenerator::Entity& submodel = ig->getEntityMap()[sequence->_playerId];
        if (submodel.valid())
        {
            osg::Matrixd mx = submodel->getMatrix();

            double x = sequence->_playerOriginalPosition.x();
            double y = sequence->_playerOriginalPosition.y();
            double z = sequence->_playerOriginalPosition.z();
            double h = sequence->_playerOriginalOrientation.x();
            double p = sequence->_playerOriginalOrientation.y();
            double r = sequence->_playerOriginalOrientation.z();

            osg::Vec3d hpr(h,p,r);

            if (sequence->_swapPitchRoll)
            {
                osg::Matrixd mxR;
                mxR.makeRotate(osg::DegreesToRadians(hpr.y()), osg::Vec3(1, 0, 0));

                osg::Matrixd mxH;
                mxH.makeRotate(osg::DegreesToRadians(hpr.x()), osg::Vec3(0, 0, 1));

                osg::Matrixd mxP;
                mxP.makeRotate(osg::DegreesToRadians(hpr.z()), osg::Vec3(0, 1, 0));

                mx = mxR * mxP * mxH * osg::Matrixd::translate(osg::Vec3d(x,y,z));

                submodel->setMatrix( mx );
            }
            else
            {
                submodel->setMatrix( Math::instance()->toMatrix(x,y,z,hpr.x(),hpr.y(),hpr.z()) );
            }

            osg::notify(osg::NOTICE) << "animation " << animation->_name << " reset" << std::endl;
        }
    }
}

void Animations::stopAnimation(igcore::ImageGenerator* ig, unsigned int entityId, const std::string& name)
{
    if (!ig) return;

    RuntimeAnimationsIterator itr = _animations.begin();
    while ( itr != _animations.end() )
    {
        osg::ref_ptr<RuntimeAnimation> rtanimation = *itr;
        if (!rtanimation.valid())
        {
            ++itr;
            continue;
        }

        osg::ref_ptr<Animation> animation = rtanimation->_animation.get();
        if (!animation.valid())
        {
            ++itr;
            continue;
        }

        //osg::notify(osg::NOTICE) << "checking animation " << rtanimation->_animation->_name << std::endl;

        if (rtanimation->_entityId == entityId && animation->_name == name)
        {
            osg::notify(osg::NOTICE) << "animation " << rtanimation->_animation->_name << " stopped" << std::endl;
            itr = _animations.erase(itr);
            continue;
        }
        else
        {
            ++itr;
        }
    }
}


void Animations::playAnimation(igcore::ImageGenerator* ig, unsigned int entityId, const std::string& name, RefAnimationSequenceCallbacks* cbs)
{
    if (!ig) return;

    ImageGenerator::Entity& entity = ig->getEntityMap()[entityId];
    if (!entity.valid()) return;

    AnimationContainer* ac = dynamic_cast<AnimationContainer*>(entity->getUserData());
    if (!ac) return;

    osg::ref_ptr<RuntimeAnimation> rtanimation = new RuntimeAnimation;

    rtanimation->_animation = (*ac)[name];
    rtanimation->_sequenceCallbacks = cbs;

    if (!rtanimation->_animation.valid()) return;

    rtanimation->_entityId = entityId;

    _animations.push_back(rtanimation);

    igcore::Animations::Animation::SequencesMapIterator smitr = rtanimation->_animation->_sequences.begin();
    for ( ; smitr != rtanimation->_animation->_sequences.end(); ++smitr)
    {
        igcore::Animations::Animation::Sequence* sequence = smitr->second;
        sequence->_enabled = true;
    }

    rtanimation->_startTime = osg::Timer::instance()->tick();
}

void Animations::updateAnimations(igcore::ImageGenerator* ig)
{
    if (!ig) return;    

    osg::Timer_t now = osg::Timer::instance()->tick();

    RuntimeAnimationsIterator itr = _animations.begin();
    for ( ; itr != _animations.end(); ++itr )
    {
        RuntimeAnimation* rtanimation = *itr;

        if (!rtanimation->_animation.valid()) continue;

        double dt = osg::Timer::instance()->delta_s(rtanimation->_startTime,now);

        Animation* animation = rtanimation->_animation.get();
        double duration = animation->_duration;

        if (dt > duration) continue;

        Animation::SequencesMapIterator sitr = animation->_sequences.begin();
        for ( ; sitr != animation->_sequences.end(); ++sitr )
        {
            Animation::Sequence* sequence = sitr->second;
            if (!sequence) continue;

            double start = sequence->_timeFrame.first;
            double end = sequence->_timeFrame.second;

            if (start > dt) continue;

            double currentSequenceTime = dt - start;
            double sequenceDuration = end - start;

            double t = currentSequenceTime / sequenceDuration;

            if (!sequence->_enabled) continue;

            if (t > 1.0)
            {
                t = 1.0;
                sequence->_enabled = false;
            }

            double rd = sequence->_rotationUpdate.second-sequence->_rotationUpdate.first;
            double actualRotation = sequence->_rotationUpdate.first + rd * t;

            if (rtanimation->_sequenceCallbacks.valid() && rtanimation->_sequenceCallbacks->size())
            {
                //osg::notify(osg::NOTICE) << "sequence for callback check: " << sequence->_name << std::endl;

                AnimationSequenceCallbacksIterator aitr = rtanimation->_sequenceCallbacks->find(sequence->_name);
                if ( aitr != rtanimation->_sequenceCallbacks->end())
                {
                    osg::ref_ptr<AnimationSequencePlaybackCallback>& cb = aitr->second;
                    if (cb.valid())
                    {
                        bool moveOn = cb->operator()(actualRotation);
                        if (!moveOn) continue;
                    }
                }
            }

            osg::Vec3d operationVector = sequence->_operationVector;
            operationVector.normalize();

            osg::Vec3d hpr = operationVector * actualRotation;

            ImageGenerator::Entity& submodel = ig->getEntityMap()[sequence->_playerId];
            if (submodel.valid())
            {
                osg::Matrixd mx = submodel->getMatrix();

                double x = 0.0;
                double y = 0.0;
                double z = 0.0;
                double h = 0.0;
                double p = 0.0;
                double r = 0.0;

                Math::instance()->fromMatrix(mx,x,y,z,h,p,r);

                hpr.x() += sequence->_playerOriginalOrientation.x();
                hpr.y() += sequence->_playerOriginalOrientation.y();
                hpr.z() += sequence->_playerOriginalOrientation.z();

                if (sequence->_swapPitchRoll)
                {
                    osg::Matrixd mxR;
                    mxR.makeRotate(osg::DegreesToRadians(hpr.y()), osg::Vec3(1, 0, 0));

                    osg::Matrixd mxH;
                    mxH.makeRotate(osg::DegreesToRadians(hpr.x()), osg::Vec3(0, 0, 1));

                    osg::Matrixd mxP;
                    mxP.makeRotate(osg::DegreesToRadians(hpr.z()), osg::Vec3(0, 1, 0));

                    mx = mxR * mxP * mxH * osg::Matrixd::translate(osg::Vec3d(x,y,z));

                    submodel->setMatrix( mx );
                }
                else
                {
                    submodel->setMatrix( Math::instance()->toMatrix(x,y,z,hpr.x(),hpr.y(),hpr.z()) );
                }
            }
        }

    }

    itr = _animations.begin();
    while ( itr != _animations.end() )
    {
        RuntimeAnimation* rtanimation = *itr;

        if (!rtanimation->_animation.valid())
        {
            itr = _animations.erase(itr);
            osg::notify(osg::NOTICE) << "erasing null animation" << std::endl;
            continue;
        }

        double dt = osg::Timer::instance()->delta_s(rtanimation->_startTime,now);

        Animation* animation = rtanimation->_animation.get();
        double duration = animation->_duration;

        // This is a temp hack to give the animations
        // time to complete. We sort it out later :-)
        // Nick
        if (dt > duration+5)
        {            
            osg::notify(osg::NOTICE) << "animation " << rtanimation->_animation->_name << " completed" << std::endl;
            itr = _animations.erase(itr);
            continue;
        }
        else
        {
            ++itr;
        }
    }
}
