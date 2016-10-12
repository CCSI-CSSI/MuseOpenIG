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

#include <iostream>
#include <sstream>

#include <OpenIG-Networking/tcpserver.h>
#include <OpenIG-Networking/buffer.h>
#include <OpenIG-Networking/parser.h>
#include <OpenIG-Networking/factory.h>

#include <OpenIG-Protocol/camera.h>
#include <OpenIG-Protocol/header.h>
#include <OpenIG-Protocol/hot.h>
#include <OpenIG-Protocol/hotresponse.h>
#include <OpenIG-Protocol/los.h>
#include <OpenIG-Protocol/losresponse.h>
#include <OpenIG-Protocol/entitystate.h>

#include <boost/shared_ptr.hpp>

#include <osgViewer/CompositeViewer>
#include <osg/ValueObject>
#include <osgSim/LineOfSight>
#include <osgSim/HeightAboveTerrain>
#include <osgSim/ElevationSlice>
#include <osgDB/ReadFile>

osg::Vec3d getHOT(const osg::Vec3d& position, osgViewer::CompositeViewer& viewer)
{
    osg::Vec3d result;

    osgSim::LineOfSight los;

    osg::Vec3d s = position + osg::Vec3d(0, 0, 1000);
    osg::Vec3d e = position - osg::Vec3d(0, 0, 1000);
    los.addLOS(s, e);

    if (viewer.getView(0)->getSceneData())
    {
        los.computeIntersections(viewer.getView(0)->getSceneData());
        if (los.getNumLOS())
        {
            const osgSim::LineOfSight::Intersections& intersections = los.getIntersections(0);
            osgSim::LineOfSight::Intersections::const_iterator itr = intersections.begin();
            if (itr != intersections.end())
            {
                result = *itr;
            }
        }
    }

    return result;
}

void getLOS(const osg::Vec3d& start, const osg::Vec3d& end, osgViewer::CompositeViewer& viewer, osg::Vec3d& position, osg::Vec3f& normal)
{
    if (!viewer.getView(0)->getSceneData()) return;

    osg::Vec3d s = start;
    osg::Vec3d e = end;

    osg::ref_ptr<osgUtil::IntersectorGroup> intersectorGroup = new osgUtil::IntersectorGroup;
    osg::ref_ptr<osgUtil::LineSegmentIntersector> intersector = new osgUtil::LineSegmentIntersector(s, e);

    intersectorGroup->addIntersector(intersector.get());
    osgUtil::IntersectionVisitor intersectVisitor(intersectorGroup.get());

    viewer.getView(0)->getSceneData()->accept(intersectVisitor);

    if (intersectorGroup->containsIntersections())
    {
        osgUtil::IntersectorGroup::Intersectors& intersectors = intersectorGroup->getIntersectors();
        osgUtil::IntersectorGroup::Intersectors::iterator intersector_itr = intersectors.begin();
        if (intersector_itr == intersectors.end()) return;

        osgUtil::LineSegmentIntersector* lsi = dynamic_cast<osgUtil::LineSegmentIntersector*>(intersector_itr->get());
        if (lsi)
        {
            osgUtil::LineSegmentIntersector::Intersections& intersections = lsi->getIntersections();
            osgUtil::LineSegmentIntersector::Intersections::iterator itr = intersections.begin();
            if (itr == intersections.end()) return;

            const osgUtil::LineSegmentIntersector::Intersection& intersection = *itr;
            position = intersection.localIntersectionPoint;
            normal = intersection.localIntersectionNormal;
        }
    }
}

struct MyTCPServer : public OpenIG::Library::Networking::TCPServer
{
    explicit MyTCPServer(const std::string& host, unsigned port, OpenIG::Library::Networking::Buffer& buf)
        : OpenIG::Library::Networking::TCPServer(host, port)
        , buffer(buf)
    {
    }

    virtual void process()
    {
        OpenIG::Library::Networking::TCPServer::process();
        if (buffer.getWritten())
        {
            send(buffer);

            buffer.rewrite();
        }
    }

    OpenIG::Library::Networking::Buffer& buffer;
};

struct Parser : public OpenIG::Library::Networking::Parser
{
    Parser()
    {
        OpenIG::Library::Networking::Factory::instance()->addTemplate(new OpenIG::Library::Protocol::Header);
        OpenIG::Library::Networking::Factory::instance()->addTemplate(new OpenIG::Library::Protocol::HOT);
        OpenIG::Library::Networking::Factory::instance()->addTemplate(new OpenIG::Library::Protocol::LOS);
        OpenIG::Library::Networking::Factory::instance()->addTemplate(new OpenIG::Library::Protocol::EntityState);
    }

    virtual OpenIG::Library::Networking::Packet* parse(OpenIG::Library::Networking::Buffer& buffer)
    {
        const unsigned char* opcode = buffer.fetch();

        OpenIG::Library::Networking::Packet* packet = OpenIG::Library::Networking::Factory::instance()->packet(*opcode);
        if (packet)
        {
            packet->read(buffer);

            OpenIG::Library::Protocol::Header* header = dynamic_cast<OpenIG::Library::Protocol::Header*>(packet);
            if (header && header->magic != OpenIG::Library::Protocol::SWAP_BYTES_COMPARE)
            {
                buffer.setSwapBytes(true);
            }
        }
        return packet;
    }

};

struct HeaderCallback : public OpenIG::Library::Networking::Packet::Callback
{
    HeaderCallback() {}

    void process(OpenIG::Library::Networking::Packet& packet)
    {
        OpenIG::Library::Protocol::Header* h = dynamic_cast<OpenIG::Library::Protocol::Header*>(&packet);
        if (h)
        {
            std::cout << "TQServer TCP -- HeaderCallBack frame: " << h->frameNumber <<  std::endl;
        }
    }
};

struct HOTCallback : public OpenIG::Library::Networking::Packet::Callback
{
    HOTCallback(osgViewer::CompositeViewer& v, MyTCPServer* net)
        : viewer(v)
        , network(net)
    {
    }

    virtual void process(OpenIG::Library::Networking::Packet& packet)
    {
        OpenIG::Library::Protocol::HOT* hot = dynamic_cast<OpenIG::Library::Protocol::HOT*>(&packet);
        if (hot)
        {
            std::cout << "TQServer TCP --HOT request recieved. Request ID:" << hot->id << std::endl;

            if (network->buffer.getWritten() == 0)
            {
                OpenIG::Library::Protocol::Header header(hot->id);
                header.write(network->buffer);
            }

            OpenIG::Library::Protocol::HOTResponse response;
            response.id = hot->id;
            response.position = getHOT(hot->position, viewer);
            response.write(network->buffer);
        }
    }

    osgViewer::CompositeViewer&		viewer;
    MyTCPServer*					network;
};

struct LOSCallback : public OpenIG::Library::Networking::Packet::Callback
{
    LOSCallback(osgViewer::CompositeViewer& v, MyTCPServer* n)
        : viewer(v)
        , network(n)
    {
    }

    virtual void process(OpenIG::Library::Networking::Packet& packet)
    {
        OpenIG::Library::Protocol::LOS* los = dynamic_cast<OpenIG::Library::Protocol::LOS*>(&packet);
        if (los)
        {
            std::cout << "TQServer TCP--LOS request recieved. Request ID:" << los->id << std::endl;

            if (network->buffer.getWritten() == 0)
            {
                OpenIG::Library::Protocol::Header header(los->id);
                header.write(network->buffer);
            }

            OpenIG::Library::Protocol::LOSResponse response;
            response.id = los->id;
            getLOS(los->start, los->end, viewer, response.position, response.normal);
            response.write(network->buffer);
        }
    }
    osgViewer::CompositeViewer&		viewer;
    MyTCPServer*					network;
};

struct EntityStateCallback : public OpenIG::Library::Networking::Packet::Callback
{
    EntityStateCallback(osgViewer::CompositeViewer& v)
        : viewer(v)
    {
    }

    virtual void process(OpenIG::Library::Networking::Packet& packet)
    {
        OpenIG::Library::Protocol::EntityState* estate = dynamic_cast<OpenIG::Library::Protocol::EntityState*>(&packet);
        if (estate)
        {
            std::cout << "TQServer TCP--Entity update recieved: " << estate->entityID << std::endl;

            osg::Vec3d position = estate->mx.getTrans();
            osg::Matrixd mx;
            mx.makeLookAt(position + osg::Vec3d(0, 0, 100), position, osg::Vec3d(0, 0, 1));

            viewer.getView(0)->getCamera()->setViewMatrix(mx);
        }
    }
    osgViewer::CompositeViewer& viewer;
};

struct CameraCallback : public OpenIG::Library::Networking::Packet::Callback
{
    CameraCallback(osgViewer::CompositeViewer& v)
        : viewer(v)
    {

    }

    virtual void process(OpenIG::Library::Networking::Packet& packet)
    {
        OpenIG::Library::Protocol::Camera* cp = dynamic_cast<OpenIG::Library::Protocol::Camera*>(&packet);
        if (cp)
        {
            std::cout << "TQServer TCP--Camera update recieved!!" << std::endl;
            osg::Vec3d position = cp->mx.getTrans();
            osg::Matrixd mx;
            mx.makeLookAt(position + osg::Vec3d(0, 0, 100), position, osg::Vec3d(0, 0, 1));

            std::cout << "TQServer TCP--Camera update pos.x(): " << position.x() << std::endl;
            viewer.getView(0)->getCamera()->setViewMatrix(mx);
        }
    }

    osgViewer::CompositeViewer& viewer;
};

void createInvisibleViewer(osgViewer::CompositeViewer& viewer, osg::ArgumentParser& arguments)
{
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;

    traits->x = 0;
    traits->y = 0;
    traits->width = 1;
    traits->height = 1;
    traits->windowDecoration = false;
    traits->doubleBuffer = false;
    traits->screenNum = 0;
    traits->sharedContext = 0;
    traits->vsync = false;

    osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
    if (gc.valid())
    {
        gc->setClearColor(osg::Vec4f(0.2f, 0.2f, 0.6f, 1.0f));
        gc->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    else
    {
        osg::notify(osg::NOTICE) << " \tGraphicsWindow has not been created successfully." << std::endl;
        return;
    }

    osgViewer::View* view = new osgViewer::View;
    viewer.addView(view);
    view->getCamera()->setGraphicsContext(gc.get());
    view->getCamera()->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));
    double aspectratio = static_cast<double>(traits->width) / static_cast<double>(traits->height);
    view->getCamera()->setProjectionMatrixAsPerspective(45, aspectratio, 1.0, 100000);
    view->getCamera()->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
    view->getCamera()->setCullingMode(osg::CullSettings::VIEW_FRUSTUM_CULLING);

    viewer.setThreadingModel(osgViewer::ViewerBase::CullDrawThreadPerContext);

    osg::ref_ptr<osg::Node> scene = osgDB::readRefNodeFiles(arguments);
    view->setSceneData(scene);

}

struct NotifyErrorHandler : public OpenIG::Library::Networking::ErrorHandler
{
    virtual std::ostream& operator<<(const std::ostringstream& oss)
    {
        std::cout << oss.str();
        return std::cout;
    }
};

int main(int argc, char** argv)
{
    osg::ArgumentParser arguments(&argc, argv);

    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName() + " is a sample Host for OpenIG");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName() + " databaseFileName [options]");
    arguments.getApplicationUsage()->addCommandLineOption("--host <host ip address>", "The IP of the host");
    arguments.getApplicationUsage()->addCommandLineOption("--port <port>", "The port to be used");

    unsigned int helpType = 0;
    if ((helpType = arguments.readHelpType()))
    {
        arguments.getApplicationUsage()->write(std::cout, helpType);
        return 1;
    }

    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }

    std::string host = "127.0.0.1";
    std::string port = "8889";

    while (arguments.read("--host", host));
    while (arguments.read("--port", port));

    osgViewer::CompositeViewer viewer;
    createInvisibleViewer(viewer, arguments);

    //OpenIG::Library::Networking::Network::log = boost::shared_ptr<OpenIG::Library::Networking::ErrorHandler>(new NotifyErrorHandler);

    OpenIG::Library::Networking::Buffer responseBuffer(BUFFER_SIZE);

    boost::shared_ptr<MyTCPServer> network = boost::shared_ptr<MyTCPServer>(new MyTCPServer(host, atoi(port.c_str()), responseBuffer));

    network->addCallback((OpenIG::Library::Networking::Packet::Opcode)OPCODE_HEADER, new HeaderCallback);
    network->addCallback((OpenIG::Library::Networking::Packet::Opcode)OPCODE_HOT, new HOTCallback(viewer,network.get()));
    network->addCallback((OpenIG::Library::Networking::Packet::Opcode)OPCODE_LOS, new LOSCallback(viewer,network.get()));
    network->addCallback((OpenIG::Library::Networking::Packet::Opcode)OPCODE_ENTITYSTATE, new EntityStateCallback(viewer));

    network->setParser(new Parser);

    unsigned int frameNumber = 1;

    viewer.realize();
    while (!viewer.done())
    {
        // We create header packet with the frame number
        OpenIG::Library::Protocol::Header header(frameNumber++);
        header.write(responseBuffer);

        network->process();
        viewer.frame();
    }
}
