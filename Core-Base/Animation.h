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

#ifndef ANIMATION_H
#define ANIMATION_H

#if defined(OPENIG_SDK)
	#include <OpenIG-Base/Export.h>
	#include <OpenIG-Base/ImageGenerator.h>
#else
	#include <Core-Base/Export.h>
	#include <Core-Base/ImageGenerator.h>
#endif

#include <osg/Referenced>
#include <osg/ref_ptr>
#include <osg/Vec3>
#include <osg/Timer>

#include <utility>
#include <vector>
#include <string>
#include <map>

namespace OpenIG {
	namespace Base {

		/*! This callback is here to give the user ability to control the animation
		 *  playback via runtime value. In our simple animation playback implementation
		 *  defined in this core, this is the callback that sniffs the change of the
		 *  animation player orientation or position value and can stop it at certain\
		 *  values on criteria used in the inherits. See \ref OpenIG::Base::ImageGenerator::playAnimation
		 * \class AnimationSequencePlaybackCallback
		 * \brief The AnimationSequencePlaybackCallback class
		 * \author    Trajce Nikolov Nick openig@compro.net
		 * \copyright (c)Compro Computer Services, Inc.
		 * \date      Sun Jan 11 2015
		 */
		class IGCORE_EXPORT AnimationSequencePlaybackCallback : public osg::Referenced
		{
		public:
			/*! The current positional or orientational value is passed here
			 *  so the inherits can decide to stop the animation or let it play
			 * \brief operator ()
			 * \param value The current run-time value that is result of the linear interpolation
			 *              of orientation the orientational operation vector
			 * \return  When inherits returns false, the playback is stopped, otherwise nothing happens
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Sun Jan 11 2015
			 */
			virtual bool operator()(double value) = 0;
		};

		typedef std::map< std::string, osg::ref_ptr< AnimationSequencePlaybackCallback > >                    AnimationSequenceCallbacks;
		typedef std::map< std::string, osg::ref_ptr< AnimationSequencePlaybackCallback > >::iterator          AnimationSequenceCallbacksIterator;
		typedef std::map< std::string, osg::ref_ptr< AnimationSequencePlaybackCallback > >::const_iterator    AnimationSequenceCallbacksConstIterator;

		class IGCORE_EXPORT RefAnimationSequenceCallbacks : public AnimationSequenceCallbacks, public osg::Referenced {};

		/*! This is the singleton class that is managing the core simple animations
		 * \brief The Animations class
		 * \class Animations
		 * \author    Trajce Nikolov Nick openig@compro.net
		 * \copyright (c)Compro Computer Services, Inc.
		 * \date      Sun Jan 11 2015
		 */
		class IGCORE_EXPORT Animations
		{
		protected:
			/*! Animations constructor
			 * \brief Animations constructor
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Sun Jan 11 2015
			 */
			Animations();

			/*! Animations destructor
			 * \brief Animations destructor
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Sun Jan 11 2015
			 */
			~Animations();

		public:
			/*! The singleton
			 * \brief The singleton
			 * \return The singleton
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Sun Jan 11 2015
			 */
			static Animations* instance();

			/*! This is the struct used to define simple animation. The core is doing
			 *  linear update of positions and/or orientations of the players. See
			 *  \ref ModelCompositionPlugin how the XML definitions are converted to
			 *  this kind of simple Animations
			 * \brief The Animation struct
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Sun Jan 11 2015
			 */
			struct Animation : public osg::Referenced
			{
				Animation() : _duration(0.0) {}
				Animation(const std::string& name, double duration)
					: _name(name)
					, _duration(duration)
				{

				}

				/*! Defintion of a simple \ref Animation sequence
				 * See \ref ModelCompositionPlugin implemenetation
				 * as a reference
				 * \brief The Sequence struct
				 * \author    Trajce Nikolov Nick openig@compro.net
				 * \copyright (c)Compro Computer Services, Inc.
				 * \date      Sun Jan 11 2015
				 */
				struct Sequence : public osg::Referenced
				{
					Sequence() : _swapPitchRoll(false), _enabled(true) {}

					/*! \brief The name of the \ref Sequence */
					std::string                     _name;
					/*! \brief The name of the player */
					std::string                     _player;
					/*! \brief The time in seconds <start,end> of this sequence */
					std::pair<double, double>        _timeFrame;
					/*! \brief The vector of the linear interpolation of the orientation */
					osg::Vec3                       _operationVector;
					/*! \brief The update of the orientation along the \ref _operationVector <start,end> */
					std::pair<double, double>        _rotationUpdate;
					/*! \brief The vector of the linear interpolation of the positional update */
					osg::Vec3                       _positionalOperationVector;
					/*! \brief The update of the position <start,end>  */
					std::pair<osg::Vec3, osg::Vec3>  _positionalUpdate;
					/*! \brief The ID of the player. Since our players are \ref
					 *  OpenIG::Base::ImageGenerator::Entity this is the ID */
					unsigned int                    _playerId;
					/*! \brief The original player orientation  */
					osg::Vec3                       _playerOriginalOrientation;
					/*! \brief The original player position */
					osg::Vec3                       _playerOriginalPosition;
					/*! \brief flags to swap pitch and roll in the animation playback computation
					 *  It is introduced to cover the variety of the Modelers used to build the model */
					bool                            _swapPitchRoll;
					/*! \brief Enabled or disabled, if false the sequence is not processed */
					bool                            _enabled;
				};


				/*! \brief Name std::string based map of \ref Sequence */
				typedef std::map< std::string, osg::ref_ptr<Sequence> >                 SequencesMap;
				typedef std::map< std::string, osg::ref_ptr<Sequence> >::iterator       SequencesMapIterator;
				typedef std::map< std::string, osg::ref_ptr<Sequence> >::const_iterator SequencesMapConstIterator;

				/*! \brief Name std::string based map of \ref Sequence */
				SequencesMap                                _sequences;
				/*! \brief Name of the \ref Animation */
				std::string                                 _name;
				/*! \brief Duration of the animation in seconds */
				double                                      _duration;

			};

			/*! \brief Referenced pointer of std::vector of \ref Animation */
			struct AnimationContainer : public std::map< std::string, osg::ref_ptr< Animation > >, public osg::Referenced {};

			/*! Plays simple animations. See \ref OpenIG::Base::ImageGenerator::playAnimation .It calls this method
			 *  on the \ref OpenIG::Base::ImageGenerator .
			 * \brief Plays simple animations. See \ref OpenIG::Base::ImageGenerator::playAnimation
			 * \param ig            The \ref OpenIG::Base::ImageGenerator instance to call it for playback
			 * \param entityId      The \ref OpenIG::Base::ImageGenerator::Entity to play the animation on.
			 *                      See \ref OpenIG::Base::ImageGenerator::playAnimation
			 * \param name          The name of the animation
			 * \param cbs           Optional animation callbacks. See \ref OpenIG::Base::AnimationSequencePlaybackCallback
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Sun Jan 11 2015
			 */
			void playAnimation(OpenIG::Base::ImageGenerator* ig, unsigned int entityId, const std::string& name, RefAnimationSequenceCallbacks* cbs = 0);

			/*! Stops the playback of simple animations. See \ref OpenIG::Base::ImageGenerator::stopAnimation .It calls this method
			 *  on the \ref OpenIG::Base::ImageGenerator .
			 * \brief Stops simple animations. See \ref OpenIG::Base::ImageGenerator::stopAnimation
			 * \param ig            The \ref OpenIG::Base::ImageGenerator instance to call
			 * \param entityId      The \ref OpenIG::Base::ImageGenerator::Entity to stop the animation on.
			 *                      See \ref OpenIG::Base::ImageGenerator::stopAnimation
			 * \param name          The name of the animation
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Mon May 18 2015
			 */
			void stopAnimation(OpenIG::Base::ImageGenerator* ig, unsigned int entityId, const std::string& name);

			/*! Reset a simple animations. See \ref OpenIG::Base::ImageGenerator::resetAnimation .It calls this method
			 *  on the \ref OpenIG::Base::ImageGenerator .
			 * \brief Plays simple animations. See \ref OpenIG::Base::ImageGenerator::resetAnimation
			 * \param ig            The \ref OpenIG::Base::ImageGenerator instance to call
			 * \param entityId      The \ref OpenIG::Base::ImageGenerator::Entity to reset the animation on.
			 *                      See \ref OpenIG::Base::ImageGenerator::resetAnimation
			 * \param name          The name of the animation
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Mon May 18 2015
			 */
			void resetAnimation(OpenIG::Base::ImageGenerator* ig, unsigned int entityId, const std::string& name);

			/*! Pause/Resume animation
			* \brief Pause/Resume anmation
			* \param ig				The \ref OpenIG::Base::ImageGenerator instance to call
			* \param entityId		The \ref OpenIG::Base::ImageGenerator::Entity to reset the animation on.
			*						See \ref OpenIG::Base::ImageGenerator::resetAnimation
			* \param name			The name of the animation
			* \param pauseResume	true to pause, false to resume
			* \author    Trajce Nikolov Nick openig@compro.net
			* \copyright (c)Compro Computer Services, Inc.
			* \date      Tue May 16 2016
			*/
			void pauseResumeAnimation(OpenIG::Base::ImageGenerator* ig, unsigned int entityId, const std::string& name, bool pauseResume = true);


			/*! Updates the animation, should be called in a loop. There is a plugin available that calls this method
			 *  in the \ref igplugincore::Plugin::update call.
			 * \brief Updates the animation, should be called in a loop.
			 * \param ig    The image generator instance
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Sun Jan 11 2015
			 */
			void updateAnimations(OpenIG::Base::ImageGenerator* ig);

		protected:
			/*! This struct is used internaly to control the playback
			 * \brief The RuntimeAnimation struct
			 * \author    Trajce Nikolov Nick openig@compro.net
			 * \copyright (c)Compro Computer Services, Inc.
			 * \date      Sun Jan 11 2015
			 */
			struct RuntimeAnimation : osg::Referenced
			{
				RuntimeAnimation() : _startTime(0), _pauseTime(0), _pausedTime(0.0), _entityId(0), _resumed(false) {}

				osg::Timer_t                                _startTime;
				osg::Timer_t                                _pauseTime;
				double										_pausedTime;
				osg::observer_ptr< Animation >              _animation;
				unsigned int                                _entityId;
				osg::ref_ptr<RefAnimationSequenceCallbacks> _sequenceCallbacks;
				bool										_resumed;
			};

			typedef std::vector< osg::ref_ptr<RuntimeAnimation> >                   RuntimeAnimations;
			typedef std::vector< osg::ref_ptr<RuntimeAnimation> >::iterator         RuntimeAnimationsIterator;
			typedef std::vector< osg::ref_ptr<RuntimeAnimation> >::const_iterator   RuntimeAnimationsConstIterator;

			/*! \brief std::vector of \ref RuntimeAnimation */
			RuntimeAnimations   _animations;
		};
	} // namespace
} // namespace

#endif // ANIMATION_H
