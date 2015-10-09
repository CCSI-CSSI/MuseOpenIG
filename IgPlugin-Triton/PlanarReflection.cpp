#include "PlanarReflection.h"
#include <osg/NodeVisitor>
#include <osgUtil/CullVisitor>
#include <osgDB/WriteFile>
#include <iostream>

PlanarReflection::PlanarReflection( osg::Camera * sceneCamera )
{
    int w = 1024, h = 1024;
    _sceneCamera = sceneCamera;

    _texture = new osg::Texture2D;
    _texture->setInternalFormat( GL_RGBA );
    _texture->setSourceFormat( GL_RGBA );
    _texture->setSourceType( GL_UNSIGNED_BYTE );
    _texture->setTextureWidth( w );
    _texture->setTextureHeight( h );
    _texture->setFilter( osg::Texture::MIN_FILTER, osg::Texture::NEAREST );
    _texture->setFilter( osg::Texture::MAG_FILTER, osg::Texture::NEAREST );
    _textureProjectionMatrix = new osg::RefMatrix();

    // set up the background color and clear mask.
    setClearColor(osg::Vec4(1.0f, 1.0f, 1.0f, 0.0f));
    setClearMask( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );

    setViewport( 0,0,w,h );
    setReferenceFrame( osg::Transform::ABSOLUTE_RF );

    // set the camera to render before the main camera.
    setRenderOrder(osg::Camera::PRE_RENDER);
    attach( osg::Camera::COLOR_BUFFER, _texture );

    setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT );
}

// Intercept cull visitor and adjust traversal mask to skip triton drawable
void PlanarReflection::accept(osg::NodeVisitor& nv)
{
    osgUtil::CullVisitor * cv = dynamic_cast< osgUtil::CullVisitor* >( &nv );
    if( cv ) {
        // Update view and projection matrices from main camera
        osg::Matrix view = _sceneCamera->getViewMatrix();
        osg::Matrix projection = _sceneCamera->getProjectionMatrix();

        setViewMatrix( view );
        setProjectionMatrix( projection );

        // Triton Shader expects matrix that takes world coords with origin translated to camera position
        view.setTrans( 0, 0, 0 );
        _textureProjectionMatrix->set( view *
                                       projection *
                                       osg::Matrix::translate( 1,1,1 ) *
                                       osg::Matrix::scale( 0.5, 0.5, 0.5 ) );
    }
    Camera::accept( nv );
}


osg::Texture2D * PlanarReflection::getTexture()
{
    return _texture.get();
}

osg::RefMatrix * PlanarReflection::getTextureProjectionMatrix()
{
    return _textureProjectionMatrix.get();
}
