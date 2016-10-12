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
#include <Core-PluginBase/plugin.h>
#include <Core-PluginBase/plugincontext.h>

#include <Core-Base/imagegenerator.h>
#include <Core-Base/stringutils.h>
#include <Core-Base/mathematics.h>

#include <Library-Networking/udpnetwork.h>

#include <string>
#include <map>

#include <osg/Timer>
#include <osg/CoordinateSystemNode>

#include <osgDB/FileNameUtils>
#include <osgDB/XmlParser>

#include <boost/thread.hpp>

#include <cigicl/CigiIGSession.h>
#include <cigicl/CigiSOFV3.h>
#include <cigicl/CigiEntityCtrlV3_3.h>

namespace OpenIG {
    namespace Plugins {

        struct EntityState
        {
            unsigned int	id;
            double			lat;
            double			lon;
            double			alt;
            double			h;
            double			p;
            double			r;
        };
        typedef std::map<unsigned int, EntityState>			EntityStateMap;
        EntityStateMap										entities;

        boost::mutex										entitiesMutex;

        void processEntityControlPacket(CigiBasePacket* packet)
        {
            CigiEntityCtrlV3_3* entityControlPacket = dynamic_cast<CigiEntityCtrlV3_3*>(packet);
            if (entityControlPacket)
            {
                unsigned int id = entityControlPacket->GetEntityID();

                entitiesMutex.lock();
                EntityState& es = entities[id];

                es.id = id;
                es.lat = entityControlPacket->GetLat();
                es.lon = entityControlPacket->GetLon();
                es.alt = entityControlPacket->GetAlt();
                es.h = entityControlPacket->GetYaw();
                es.p = entityControlPacket->GetPitch();
                es.r = entityControlPacket->GetRoll();

                entitiesMutex.unlock();
            }
        }

        class CIGIPlugin : public OpenIG::PluginBase::Plugin
        {
        public:

            CIGIPlugin()
                : _host("127.0.0.1")
                , _port(8888)
                , _rhost("127.0.0.1")
                , _rport(8889)
                , _igSession(0)
                , _incommingIgSession(0)
                , _CIGIVersionMajor(3)
                , _CIGIVersionMinor(3)
            {
                _network.reset();
                _rnetwork.reset();
            }

            virtual std::string getName() { return "CIGI"; }

            virtual std::string getDescription() { return "Implements the CIGI communication"; }

            virtual std::string getVersion() { return "1.0.0"; }

            virtual std::string getAuthor() { return "ComPro, Nick"; }

            virtual void config(const std::string& fileName)
            {
                osgDB::XmlNode* root = osgDB::readXmlFile(fileName);
                if (root == 0) return;

                if (root->children.size() == 0) return;

                osgDB::XmlNode* config = root->children.at(0);
                if (config->name != "OpenIG-Plugin-Config") return;

                osgDB::XmlNode::Children::iterator itr = config->children.begin();
                for (; itr != config->children.end(); ++itr)
                {
                    osgDB::XmlNode* child = *itr;

                    if (child->name == "Interface")
                    {
                        _host = child->contents;
                    }
                    if (child->name == "Port")
                    {
                        _port = atoi(child->contents.c_str());
                    }
                    if (child->name == "Host")
                    {
                        _rhost = child->contents;
                    }
                    if (child->name == "HostPort")
                    {
                        _rport = atoi(child->contents.c_str());
                    }
                }
            }

            void sendStartOfFrame(OpenIG::PluginBase::PluginContext& context)
            {

            }

            virtual void beginningOfFrame(OpenIG::PluginBase::PluginContext& context)
            {
                if (_network == 0) return;
                if (_igSession == 0) return;

                static Cigi_uint32 frameCounter = 0;

                const osg::FrameStamp* frameStamp = context.getImageGenerator()->getViewer()->getFrameStamp();

                double dt = frameStamp->getSimulationTime();
                Cigi_uint32 timeStamp = floor((dt * 1000000.0)/10.0);

                CigiSOFV3 sof;
                sof.SetDatabaseID(0);
                sof.SetEarthRefModel(CigiBaseSOF::WGS84);
                sof.SetFrameCntr(frameStamp->getFrameNumber());
                sof.SetIGMode(CigiBaseSOF::Operate);
                sof.SetIGStatus(0);
                sof.SetTimeStamp(timeStamp);
                sof.SetTimeStampValid(true);

                CigiOutgoingMsg &outgoingMessage = _igSession->GetOutgoingMsgMgr();
                outgoingMessage.BeginMsg();
                outgoingMessage << sof;

                Cigi_uint8* message = 0;
                int			length = 0;

                outgoingMessage.PackageMsg(&message, length);

                OpenIG::Library::Networking::Buffer buffer(BUFFER_SIZE);
                buffer.write(message, length);

                outgoingMessage.FreeMsg();

                _network->send(buffer);
            }

            void processIncomingMessage()
            {
                if (_rnetwork == 0) return;
                if (_incommingIgSession == 0) return;

                OpenIG::Library::Networking::Buffer buffer(BUFFER_SIZE);
                _rnetwork->receive(buffer, false);

                CigiIncomingMsg &incomingMessage = _incommingIgSession->GetIncomingMsgMgr();

                incomingMessage.SetReaderCigiVersion(3, 3);
                incomingMessage.UsingIteration(false);
                incomingMessage.RegisterCallBack(
                    CIGI_ENTITY_CTRL_PACKET_ID_V3_3,
                    &OpenIG::Plugins::processEntityControlPacket
                );
                incomingMessage.ProcessIncomingMsg((Cigi_uint8*)buffer.getData(), buffer.getWritten());
            }

            void threadFunc()
            {
                while (1)
                {
                    processIncomingMessage();
                }
            }

            virtual void init(OpenIG::PluginBase::PluginContext&)
            {
                _network = boost::shared_ptr<OpenIG::Library::Networking::UDPNetwork>(
                    new OpenIG::Library::Networking::UDPNetwork(_host)
                );
                _network->setPort(_port);

                _rnetwork = boost::shared_ptr<OpenIG::Library::Networking::UDPNetwork>(
                    new OpenIG::Library::Networking::UDPNetwork(_rhost)
                    );
                _rnetwork->setPort(_rport);

                _thread = boost::thread(boost::bind(&OpenIG::Plugins::CIGIPlugin::threadFunc, this));

                _igSession = new CigiIGSession;
                _igSession->SetCigiVersion(CigiVersionID(_CIGIVersionMajor, _CIGIVersionMinor));
                _igSession->SetSynchronous(true);

                _incommingIgSession = new CigiIGSession;
                _incommingIgSession->SetCigiVersion(CigiVersionID(_CIGIVersionMajor, _CIGIVersionMinor));
                _incommingIgSession->SetSynchronous(true);
            }

            virtual void update(OpenIG::PluginBase::PluginContext& context)
            {
                entitiesMutex.lock();

                EntityStateMap::iterator itr = entities.begin();
                for (;  itr != entities.end(); ++itr)
                {
                    EntityState& es = itr->second;

                    osg::Matrixd l2w;

                    osg::EllipsoidModel em;
                    em.computeLocalToWorldTransformFromLatLongHeight(
                        osg::DegreesToRadians(es.lat),
                        osg::DegreesToRadians(es.lon),
                        es.alt,
                        l2w
                    );

                    if (es.id == 0)
                    {
                        osg::Matrixd mx =
                            osg::Matrixd::rotate(OpenIG::Base::Math::instance()->toQuat(0,90,0)) *
                            osg::Matrixd::rotate(OpenIG::Base::Math::instance()->toQuat(es.h, es.p, es.r)) * l2w;

                        context.getImageGenerator()->setCameraPosition(mx);
                    }
                    else
                    {
                        osg::Matrixd mx = OpenIG::Base::Math::instance()->toMatrix(0, 0, 0, es.h, es.p, es.r) * l2w;

                        context.getImageGenerator()->updateEntity(es.id, mx);
                    }
                }

                entitiesMutex.unlock();
            }

            virtual void clean(OpenIG::PluginBase::PluginContext& context)
            {
                if (_network) _network.reset();
                if (_rnetwork) _rnetwork.reset();

                if (_igSession)
                {
                    delete _igSession;
                    _igSession = NULL;
                }
                if (_incommingIgSession)
                {
                    delete _incommingIgSession;
                    _incommingIgSession = NULL;
                }
            }

        protected:
            boost::shared_ptr<OpenIG::Library::Networking::UDPNetwork>	_network;
            std::string													_host;
            unsigned int												_port;
            boost::shared_ptr<OpenIG::Library::Networking::UDPNetwork>	_rnetwork;
            std::string													_rhost;
            unsigned int												_rport;
            CigiIGSession*												_igSession;
            CigiIGSession*												_incommingIgSession;
            unsigned int												_CIGIVersionMajor;
            unsigned int												_CIGIVersionMinor;
            boost::thread												_thread;

        };
    } // namespace
} // namespace

#if defined(_MSC_VER) || defined(__MINGW32__)
//  Microsoft
#define EXPORT __declspec(dllexport)
#define IMPORT __declspec(dllimport)
#elif defined(__GNUG__)
//  GCC
#define EXPORT __attribute__((visibility("default")))
#define IMPORT
#else
//  do nothing and hope for the best?
#define EXPORT
#define IMPORT
#pragma warning Unknown dynamic link import/export semantics.
#endif

extern "C" EXPORT OpenIG::PluginBase::Plugin* CreatePlugin()
{
    return new OpenIG::Plugins::CIGIPlugin;
}

extern "C" EXPORT void DeletePlugin(OpenIG::PluginBase::Plugin* plugin)
{
    osg::ref_ptr<OpenIG::PluginBase::Plugin> p(plugin);
}
