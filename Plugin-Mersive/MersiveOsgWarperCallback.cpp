#include "fgmersivelib.h"
#include <iostream>
//#include <cstshare/cstshareobject.h>
#include <osgViewer/CompositeViewer>

using std::cout;
using std::endl;
using std::cerr;

MersiveOsgWarperCallback::MersiveOsgWarperCallback(std::string channelName, int screenWidth, int screenHeight, std::string warperIP, osgViewer::CompositeViewer *viewer, const char *simName)
{
    //std::cout << "MersiveOsgWarperCallback::MersiveOsgWarperCallback" << std::endl;

    m_channelName = channelName;
    if(simName != NULL)
        m_simName = simName;
    else
        m_simName = "";
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;
    old_near = 1.0f;
    new_cp.left = 0.0f;
    new_cp.right = 0.0f;
    new_cp.top = 0.0f;
    new_cp.bottom = 0.0f;
    new_cp.near = 0.0f;
    new_cp.far = 0.0f;

    //scene_pos = ShareObj->createShare<osg::Vec3d>( "otwPos" );
    //scene_att = ShareObj->createShare<osg::Vec3>( "otwAtt" );
    //otw_terrain_height = ShareObj->createShare<float>( "OtwHeight" );

    //_mersive_init     = ShareObj->getShare<bool>("MERSIVE_INIT");
    //_mersive_runtime  = ShareObj->getShare<bool>("MERSIVE_RUNTIME");
    //_mersive_postDraw = ShareObj->getShare<bool>("MERSIVE_POSTDRAW");
                //DEBUG = ShareObj->getShare<bool>("MERSIVE_DEBUG");

    _mersive_postDraw = true;
    DEBUG = false;

    warper = new MersiveRuntimeWarper();

    warper->setSimulatorName(m_simName.c_str());

    // connect to warper server
    int err = warper->initialize(warperIP.c_str(), m_channelName.c_str(), NULL, WARP_SERVER_PORT );

    if (err)
    {
      cerr << endl << "Error in warper server initialization: " << endl;
      cerr << warper->getError() << " " << warper->getErrorName() << endl;
      cerr << warper->getErrorString() << endl;
    }

    m_inited = false;
    low = false;
    high = false;

}

void MersiveOsgWarperCallback::operator() (osg::RenderInfo& renderInfo) const
{
    MersiveRuntimeWarper *warperServer = (MersiveRuntimeWarper*)warper;

    osg::Camera * camera = renderInfo.getCurrentCamera();
    osg::Matrix projMatrix;
    osg::Matrix frustumMatrix;
    float _far;

    if (!m_inited)
    {
        //_far = ShareObj->getShare<float>("PERSPECTIVE_FAR");
        _far = 50000.0;
        //std::cout << "MersiveOsgWarperCallback in (m_inited), setting farclip to: " << _far << std::endl;
        // setup the opengl specific details (textures, shaders, etc)
        // that are needed to apply the fast warp/blend
        if (warperServer->initRendering(((MersiveOsgWarperCallback*)this)->m_screenWidth, ((MersiveOsgWarperCallback*)this)->m_screenHeight) == -1)
        {
          cerr << endl << "Error in warper server init rendering: ";
          cerr << warperServer->getError() << " " << warperServer->getErrorName();
          cerr << endl << warperServer->getErrorString() << endl;
        }

        ((MersiveOsgWarperCallback*)this)->mersiveViewOffset = warperServer->getChannelViewMatrix();
        ((MersiveOsgWarperCallback*)this)->cp = warperServer->getChannelClipPlanes();

        ((MersiveOsgWarperCallback*)this)->translateMatrix.set( mersiveViewOffset );

        ((MersiveOsgWarperCallback*)this)->cp.near = 1.0f;
        ((MersiveOsgWarperCallback*)this)->cp.far = _far;
        ((MersiveOsgWarperCallback*)this)->new_cp.far = _far;
        ((MersiveOsgWarperCallback*)this)->new_cp.near  = 1.0f;

//        if(_mersive_init)
//        {
//            if(DEBUG)
//                std::cout << "MersiveOsgWarperCallback _mersive_init" << std::endl;
//            //Now computed in the QtOtw library...
//            if( !(*mersiveSetNear) )
//            {
//                ((MersiveOsgWarperCallback*)this)->new_cp.near  = 50.0f;
//                ((MersiveOsgWarperCallback*)this)->high = true;
//                ((MersiveOsgWarperCallback*)this)->low = false;
//                //camera->setComputeNearFarMode(osg::CullSettings::COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES);
//                std::cout << "MersiveOsgWarperCallback in (m_inited), setting near to: 50.0 ####### " << *mersiveSetNear << std::endl;
//            }
//            else
//            {
//                //Just to be clear we set all 3 even though the near is 1.0 to start...
//                ((MersiveOsgWarperCallback*)this)->new_cp.near  = 1.0f;
//                ((MersiveOsgWarperCallback*)this)->low = true;
//                ((MersiveOsgWarperCallback*)this)->high = false;
//                //camera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
//                std::cout << "MersiveOsgWarperCallback in (m_inited), setting near to: 1.0" << std::endl;
//            }
//        }


        ((MersiveOsgWarperCallback*)this)->new_cp.left   = cp.left   * new_cp.near;
        ((MersiveOsgWarperCallback*)this)->new_cp.right  = cp.right  * new_cp.near;
        ((MersiveOsgWarperCallback*)this)->new_cp.top    = cp.top    * new_cp.near;
        ((MersiveOsgWarperCallback*)this)->new_cp.bottom = cp.bottom * new_cp.near;

        frustumMatrix.makeFrustum( new_cp.left,   new_cp.right,
                                   new_cp.bottom, new_cp.top,
                                   new_cp.near,   new_cp.far );

        // The shadows doesnt work with this warped
        // projection matrix. We store the original one
        // and we hack the Shadowing code to read it
        // from here instead of directly getting from
        // the camera
        osg::Matrixd pm = camera->getProjectionMatrix();
        camera->setUserValue("ProjectionMatrix", pm);

        projMatrix = translateMatrix * frustumMatrix;

        camera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
        camera->setProjectionMatrix(projMatrix);

        ((MersiveOsgWarperCallback*)this)->m_inited = true;

    }//if (!m_inited)

//    if(_mersive_runtime)
//    {
//        if(DEBUG)
//            std::cout << "MersiveOsgWarperCallback _mersive_runtime" << std::endl;
//        //the variable "mersiveSetNear" is computed in the QtOtw library, where
//        //it has access to all of the scene entities....
//        if( ( (*mersiveSetNear && !low) || (!(*mersiveSetNear) && !high) ) && m_inited)
//        {
//            if( !low )
//            {
//                ((MersiveOsgWarperCallback*)this)->new_cp.near  = 1.0f;
//                ((MersiveOsgWarperCallback*)this)->low = true;
//                ((MersiveOsgWarperCallback*)this)->high = false;
//                //camera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
//                std::cout << "MersiveOsgWarperCallback in runtime, setting near to: 1.0" << std::endl;
//            }
//            else if( !high )
//            {
//                 ((MersiveOsgWarperCallback*)this)->new_cp.near  = 50.0f;
//                ((MersiveOsgWarperCallback*)this)->high = true;
//                ((MersiveOsgWarperCallback*)this)->low = false;
//                //camera->setComputeNearFarMode(osg::CullSettings::COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES);
//                std::cout << "MersiveOsgWarperCallback in runtime, setting near to: 50.0 ########2 " << high << std::endl;
//            }

//            ((MersiveOsgWarperCallback*)this)->new_cp.left   = cp.left   * new_cp.near;
//            ((MersiveOsgWarperCallback*)this)->new_cp.right  = cp.right  * new_cp.near;
//            ((MersiveOsgWarperCallback*)this)->new_cp.top    = cp.top    * new_cp.near;
//            ((MersiveOsgWarperCallback*)this)->new_cp.bottom = cp.bottom * new_cp.near;

//            frustumMatrix.makeFrustum( new_cp.left,   new_cp.right,
//                                       new_cp.bottom, new_cp.top,
//                                       new_cp.near,   new_cp.far );

//            projMatrix = translateMatrix * frustumMatrix;

//            camera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
//            camera->setProjectionMatrix(projMatrix);

//        }
//        //End of near/far fix for flashing terrain  CGR
//    }

    if (_mersive_postDraw)
    {
        if(DEBUG)
        {
            //std::cout << "MersiveOsgWarperCallback _mersive_postDraw" << std::endl;
            osg::Matrixd pMatrix = camera->getProjectionMatrix();
            osg::Matrixd vMatrix = camera->getViewMatrix();

            if(pMatrix != pMatrix1)
            {
                for(int x=0;x<4;x++)
                {
                    std::cout << "postDraw getProjectionMatrix = " << pMatrix(x,0) << ' ' << pMatrix(x,1) << ' ' << pMatrix(x,2) << ' ' << pMatrix(x,3) << std::endl;
                }
                ((MersiveOsgWarperCallback*)this)->pMatrix1 = pMatrix;
            }
            if(vMatrix != vMatrix1)
            {
                for(int x=0;x<4;x++)
                {
                    std::cout << "postDraw getViewMatrix = " << vMatrix(x,0) << ' ' << vMatrix(x,1) << ' ' << vMatrix(x,2) << ' ' << vMatrix(x,3) << std::endl;
                }
                ((MersiveOsgWarperCallback*)this)->vMatrix1 = vMatrix;
            }
        }

        int drawErr = warperServer->postDraw();
        if(drawErr)
        {
          cerr << endl << "MersiveOsgWarperCallback: Error in post draw: " << warperServer->getError() << " " << warperServer->getErrorName();
          cerr << endl << warperServer->getErrorString() << endl;
        }
    }
}

