#ifndef PLANARREFLECTION_H
#define PLANARREFLECTION_H

#include <osg/Camera>
#include <osg/Texture2D>
#include <osg/ClipNode>

namespace OpenIG {
	namespace Plugins {

		class PlanarReflection : public osg::Camera
		{
		public:
			PlanarReflection(osg::Camera * mainCamera);

			osg::Texture2D * getTexture();
			osg::RefMatrix * getTextureProjectionMatrix();

			// Override Node::accept to intercept cull visitor traversal
			virtual void accept(osg::NodeVisitor& nv);

		protected:
			osg::ref_ptr< osg::Texture2D > _texture;
			osg::ref_ptr< osg::RefMatrix > _textureProjectionMatrix;
			osg::ref_ptr< osg::Camera >    _sceneCamera;
		};

	}
}
#endif