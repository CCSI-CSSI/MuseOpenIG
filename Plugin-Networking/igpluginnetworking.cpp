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
#include <Core-PluginBase/plugin.h>
#include <Core-PluginBase/plugincontext.h>

#include <Core-Base/imagegenerator.h>
#include <Core-Base/stringutils.h>
#include <Core-Base/commands.h>

#include <Core-OpenIG/openig.h>
#include <Core-OpenIG/renderbins.h>

#include <Library-Networking/packet.h>
#include <Library-Networking/network.h>
#include <Library-Networking/udpnetwork.h>
#include <Library-Networking/tcpserver.h>
#include <Library-Networking/tcpclient.h>
#include <Library-Networking/buffer.h>
#include <Library-Networking/parser.h>
#include <Library-Networking/factory.h>

#include <osgDB/XmlParser>

#include <osg/LineWidth>

#include <osgText/Text>

#include <OpenThreads/Barrier>
#include <OpenThreads/Block>

#include <sstream>

#include <stdlib.h>

#define USE_BLOCK_FOR_SYNC 1

namespace OpenIG { namespace Plugins {

const unsigned int SWAP_BYTES_COMPARE = 0x12345678;
#define OPCODE_HEADER					1
#define OPCODE_ENTITYSTATE				100
#define OPCODE_CAMERA					101
#define OPCODE_HOSTID					102

struct Header : public OpenIG::Library::Networking::Packet
{
	Header(unsigned int frameNum = 0)
		: magic(0)
		, masterIsDead(0)
		, frameNumber(frameNum)
	{		
	}

	META_Packet(OPCODE_HEADER, Header);

	virtual int write(OpenIG::Library::Networking::Buffer &buf) const
	{
		buf << (unsigned char)opcode() << SWAP_BYTES_COMPARE << masterIsDead << frameNumber;

		return sizeof(unsigned char) + sizeof(unsigned int) + sizeof(unsigned short) + sizeof(unsigned int);
	}

	virtual int read(OpenIG::Library::Networking::Buffer &buf)
	{
		unsigned char op;		

		buf >> op >> magic >> masterIsDead >> frameNumber;

		return sizeof(unsigned char) + sizeof(unsigned int) + sizeof(unsigned short) + sizeof(unsigned int);
	}

	unsigned int	magic;
	unsigned short	masterIsDead;
	unsigned int	frameNumber;
};

struct EntityState: public OpenIG::Library::Networking::Packet
{
	EntityState()
		: entityID(0)
	{
	}

	META_Packet(OPCODE_ENTITYSTATE, EntityState);

	virtual int write(OpenIG::Library::Networking::Buffer &buf) const
	{		
		buf << (unsigned char)opcode();
		buf << entityID;
		buf << mx(0, 0) << mx(0, 1) << mx(0, 2) << mx(0, 3);
		buf << mx(1, 0) << mx(1, 1) << mx(1, 2) << mx(1, 3);
		buf << mx(2, 0) << mx(2, 1) << mx(2, 2) << mx(2, 3);
		buf << mx(3, 0) << mx(3, 1) << mx(3, 2) << mx(3, 3);

		return sizeof(unsigned char) + sizeof(entityID) + sizeof(osg::Matrixd::value_type) * 16;
	}

	virtual int read(OpenIG::Library::Networking::Buffer &buf)
	{		
		unsigned char op;

		buf >> op;
		buf >> entityID;
		buf >> mx(0, 0) >> mx(0, 1) >> mx(0, 2) >> mx(0, 3);
		buf >> mx(1, 0) >> mx(1, 1) >> mx(1, 2) >> mx(1, 3);
		buf >> mx(2, 0) >> mx(2, 1) >> mx(2, 2) >> mx(2, 3);
		buf >> mx(3, 0) >> mx(3, 1) >> mx(3, 2) >> mx(3, 3);

		return sizeof(unsigned char) + sizeof(entityID) + sizeof(osg::Matrixd::value_type) * 16;
	}

	unsigned int	entityID;
	osg::Matrixd	mx;
};

struct CameraPacket : public OpenIG::Library::Networking::Packet
{
	CameraPacket()
		: bindToEntity(0)
	{
	}

	META_Packet(OPCODE_CAMERA, CameraPacket);

	virtual int write(OpenIG::Library::Networking::Buffer &buf) const
	{
		buf << (unsigned char)opcode();
		buf << bindToEntity;
		buf << mx(0, 0) << mx(0, 1) << mx(0, 2) << mx(0, 3);
		buf << mx(1, 0) << mx(1, 1) << mx(1, 2) << mx(1, 3);
		buf << mx(2, 0) << mx(2, 1) << mx(2, 2) << mx(2, 3);
		buf << mx(3, 0) << mx(3, 1) << mx(3, 2) << mx(3, 3);

		return sizeof(unsigned char) + sizeof(osg::Matrixd::value_type) * 16 + sizeof(bindToEntity);
	}

	virtual int read(OpenIG::Library::Networking::Buffer &buf)
	{
		unsigned char op;

		buf >> op;
		buf >> bindToEntity;
		buf >> mx(0, 0) >> mx(0, 1) >> mx(0, 2) >> mx(0, 3);
		buf >> mx(1, 0) >> mx(1, 1) >> mx(1, 2) >> mx(1, 3);
		buf >> mx(2, 0) >> mx(2, 1) >> mx(2, 2) >> mx(2, 3);
		buf >> mx(3, 0) >> mx(3, 1) >> mx(3, 2) >> mx(3, 3);

		return sizeof(unsigned char) + sizeof(osg::Matrixd::value_type) * 16 + sizeof(bindToEntity);
	}

	osg::Matrixd mx;
	unsigned int bindToEntity;
};

struct Parser : public OpenIG::Library::Networking::Parser
{
	Parser()
	{
		OpenIG::Library::Networking::Factory::instance()->addTemplate(new Header);
		OpenIG::Library::Networking::Factory::instance()->addTemplate(new EntityState);
		OpenIG::Library::Networking::Factory::instance()->addTemplate(new CameraPacket);
	}

	virtual OpenIG::Library::Networking::Packet* parse(OpenIG::Library::Networking::Buffer& buffer)
	{		
		const unsigned char* opcode = buffer.fetch();

		OpenIG::Library::Networking::Packet* packet = OpenIG::Library::Networking::Factory::instance()->packet(*opcode);
		if (packet)
		{
			packet->read(buffer);

			Header* header = dynamic_cast<Header*>(packet);
			if (header && header->magic != SWAP_BYTES_COMPARE)
			{
				buffer.setSwapBytes(true);
			}
		}
		return packet;
	}
	
};

class NetworkingPlugin;
struct HeaderCallback : public OpenIG::Library::Networking::Packet::Callback
{
	HeaderCallback(OpenIG::Base::ImageGenerator* ig, NetworkingPlugin* nplugin);
		
	virtual void process(OpenIG::Library::Networking::Packet& packet);

	OpenIG::Base::ImageGenerator* imageGenerator;
	NetworkingPlugin*		plugin;
	unsigned int			frameNumber;
	bool					packetDropsDetected;
};


struct EntityStateCallback : public OpenIG::Library::Networking::Packet::Callback
{
	EntityStateCallback(OpenIG::Base::ImageGenerator* ig)
		: imageGenerator(ig)
	{

	}

	virtual void process(OpenIG::Library::Networking::Packet& packet)
	{
		EntityState* es = dynamic_cast<EntityState*>(&packet);
		if (es)
		{
			imageGenerator->updateEntity(es->entityID, es->mx);
		}
	}

	OpenIG::Base::ImageGenerator* imageGenerator;
};

struct CameraPacketCallback : public OpenIG::Library::Networking::Packet::Callback
{
	CameraPacketCallback(OpenIG::Engine* ig)
		: imageGenerator(ig)
	{

	}

	virtual void process(OpenIG::Library::Networking::Packet& packet)
	{
		CameraPacket* cp = dynamic_cast<CameraPacket*>(&packet);
		if (cp)
		{
			if (cp->bindToEntity)
				imageGenerator->bindCameraUpdate(osg::Matrixd::inverse(cp->mx));
			else
				imageGenerator->setCameraPosition(cp->mx, true);
		}
	}

	OpenIG::Engine* imageGenerator;
};

class NetworkingPlugin : public OpenIG::PluginBase::Plugin
{
public:

	NetworkingPlugin()
		: _mode(None)
		, _protocol(Unknown)
		, _multiThreaded(true)
		, _port(0)
		, _timeout(1.0)
		, _timeoutDT(0.01)
		, _statsOn(false)
		, _dt(0.0)
		, _timeGraphSteps(10)
	{
	}

    virtual std::string getName() { return "Networking"; }

    virtual std::string getDescription( ) { return "Implements simple networking over UDP or TCP"; }

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

			if (child->name == "Mode")
			{
				std::transform(child->contents.begin(), child->contents.end(), child->contents.begin(), ::toupper);
				if (child->contents == "MASTER")
				{
					_mode = Master;

					#if defined (__linux) || defined (__APPLE__)			
						setenv("OPENIG_NETWORKING_MODE", "MASTER", true);
					#elif   defined (_WIN32)
						_putenv_s("OPENIG_NETWORKING_MODE", "MASTER");
					#endif
				}
				else
				if (child->contents == "SLAVE")
				{
					_mode = Slave;

					#if defined (__linux) || defined (__APPLE__)			
						setenv("OPENIG_NETWORKING_MODE", "SLAVE", true);
					#elif   defined (_WIN32)
						_putenv_s("OPENIG_NETWORKING_MODE", "SLAVE");
					#endif
				}
				else
				{
					_mode = None;

					#if defined (__linux) || defined (__APPLE__)			
						setenv("OPENIG_NETWORKING_MODE", "NONE", true);
					#elif   defined (_WIN32)
						_putenv_s("OPENIG_NETWORKING_MODE", "NONE");
					#endif
				}
			}
			else
			if (child->name == "Port")
			{
				_port = atoi(child->contents.c_str());
			}
			else
			if (child->name == "Timeout")
			{
				_timeout = atof(child->contents.c_str());
			}
			else
			if (child->name == "TimeoutDT")
			{
				_timeoutDT = atof(child->contents.c_str());
			} 
			else
			if (child->name == "Protocol")
			{
				std::transform(child->contents.begin(), child->contents.end(), child->contents.begin(), ::toupper);
				if (child->contents == "UDP")
					_protocol = UDP;
				else
					if (child->contents == "TCP")
						_protocol = TCP;
					else
						_protocol = Unknown;
			}
			else
			if (child->name == "TCP-Server")
			{
				_server = child->contents;
			}
			else
			if (child->name == "Multi-Threaded")
			{
				std::transform(child->name.begin(), child->name.end(), child->name.begin(), ::tolower);
				_multiThreaded = child->name == "yes";
			}
			else
			if (child->name == "Name")
			{
				_slaveName = child->contents;
			}
			else
			if (child->name == "Host")
			{
				_host = child->contents;
			}
		}
	}			

	void SlaveThreadFunc()
	{	
		while (1)
		{
			// Here we block until the main thread
			// releases it - and this will happen
			// in the plugin.update(...)
			_barrier.block(2);

			if (_network)
			{
				_network->process();
			}
			

			// The main thread after the release
			// is blocking it again to ensure
			// the network packets are processed
			// thus stuff is updated - so we release
			// here again
#if defined(USE_BLOCK_FOR_SYNC)
			_block.release();
#else
			_barrier.release();
#endif

		}
	}	

	class NetworkStatsCommand : public OpenIG::Base::Commands::Command
	{
	public:
		NetworkStatsCommand(NetworkingPlugin* plugin)
			: _plugin(plugin)
		{
		}
		virtual const std::string getUsage() const
		{
			return "on/off";
		}

		virtual const std::string getArgumentsFormat() const
		{
			return "{on;off}";
		}

		virtual const std::string getDescription() const
		{
			return  "sets onscreen network statistic\n"
				"     on/off - if on it shows the network stats, otherwise it hides it\n";
		}

		virtual int exec(const OpenIG::Base::StringUtils::Tokens& tokens)
		{
			if (tokens.size() == 1 && _plugin != 0)
			{
                bool on = (tokens.at(0).compare(0,2,"on") == 0);
                osg::notify(osg::NOTICE) << "Netstats set to: " << on << std::endl;
                _plugin->setNetworkStats(on);

				return 0;
			}

			return -1;
		}

	protected:
		NetworkingPlugin* _plugin;
	};

	void setNetworkStats(bool on)
	{
		_statsOn = on;

		if (!_stats.valid()) return;
		switch (on)
		{
		case true:
			_stats->setNodeMask(0xFFFFFFFF);
			break;
		case false:
			_stats->setNodeMask(0x0);
			break;
		}
	}

	struct OSGNotifyErrorHandler : public OpenIG::Library::Networking::ErrorHandler
	{
		virtual std::ostream& operator<<(const std::ostringstream& oss)
		{
			osg::notify(osg::NOTICE) << oss.str();
			return osg::notify(osg::NOTICE);
		}
	};

	virtual void init(OpenIG::PluginBase::PluginContext& context)
	{
		OpenIG::Base::Commands::instance()->addCommand("netstats", new NetworkStatsCommand(this));

		_ig = context.getImageGenerator();

		OpenIG::Library::Networking::Network::log = boost::shared_ptr<OpenIG::Library::Networking::ErrorHandler>(new OSGNotifyErrorHandler);
		switch (_protocol)
		{
		case UDP:
			_network = boost::shared_ptr<OpenIG::Library::Networking::UDPNetwork>(new OpenIG::Library::Networking::UDPNetwork(_host));
			break;
		case TCP:
			switch (_mode)
			{
			case Master:
				_network = boost::shared_ptr<OpenIG::Library::Networking::TCPServer>(new OpenIG::Library::Networking::TCPServer(_host,_port));
				break;
			case Slave:
				_network = boost::shared_ptr<OpenIG::Library::Networking::TCPClient>(new OpenIG::Library::Networking::TCPClient(_host,_server));
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
		
		_network->setPort(_port);

		_network->addCallback((OpenIG::Library::Networking::Packet::Opcode)OPCODE_HEADER, new HeaderCallback(_ig,this));
		_network->addCallback((OpenIG::Library::Networking::Packet::Opcode)OPCODE_ENTITYSTATE, new EntityStateCallback(_ig));
		_network->addCallback((OpenIG::Library::Networking::Packet::Opcode)OPCODE_CAMERA, new CameraPacketCallback(dynamic_cast<OpenIG::Engine*>(_ig)));

		_network->setParser(new Parser);
		
		if (_mode == Slave)
		{		
			if (_multiThreaded)
			{
				_slaveThread = boost::thread(boost::bind(&OpenIG::Plugins::NetworkingPlugin::SlaveThreadFunc, this));
			}

			createNetworkStatsTimeout();
			createNetworkStatsFrameDrops();
		}

		if (_mode == Master && _protocol == TCP)
		{
			createNetworkStatsTCPClients();
		}

	}


    virtual void update(OpenIG::PluginBase::PluginContext& context)
    {		
		switch (_mode)
		{
		case Master:
			{		
				OpenIG::Library::Networking::Buffer buffer(BUFFER_SIZE);

				Header header(context.getImageGenerator()->getViewer()->getFrameStamp()->getFrameNumber());
				header.write(buffer);

				OpenIG::Base::ImageGenerator::EntityMapIterator itr = _ig->getEntityMap().begin();
				for (; itr != _ig->getEntityMap().end(); ++itr)
				{
					EntityState estate;
					estate.entityID = itr->first;
					estate.mx = itr->second->getMatrix();
					estate.write(buffer);
				}

				CameraPacket camera;
				camera.mx = _ig->getViewer()->getView(0)->getCamera()->getViewMatrix();

				unsigned int id = 0;
				if (_ig->isCameraBoundToEntity() && _ig->getViewer()->getView(0)->getCamera()->getUserValue("bindTo", id))
				{
					camera.bindToEntity = id;
				}
				camera.write(buffer);

				if (_network.get())
				{
					_network->send(buffer);
				}

				OpenIG::Library::Networking::TCPServer* server = dynamic_cast<OpenIG::Library::Networking::TCPServer*>(_network.get());
				if (server)
				{
					std::vector<std::string> clients;
					server->getConnectedClients(clients);

					std::ostringstream oss;
					oss << "Clients connected: " << clients.size() << std::endl;
					
					std::vector<std::string>::iterator itr = clients.begin();
					for (; itr != clients.end(); ++itr)
					{
						oss << *itr << std::endl;
					}

					_tcpClientsText->setText(oss.str());
				}
			}		
			break;	
		case Slave:
			if (_multiThreaded)
			{
				//while (!_barrier.numThreadsCurrentlyBlocked());
				_barrier.release();
				// NOTE: With the syncing of these two threads, the network update
				// is happing right here

#if defined(USE_BLOCK_FOR_SYNC)
				osg::Timer_t start = osg::Timer::instance()->tick();
				
				_block.block(_timeout * 1000);

				osg::Timer_t end = osg::Timer::instance()->tick();

				if ((_dt = osg::Timer::instance()->delta_s(start, end)) >= _timeout - _timeoutDT)
				{
					_dt = osg::minimum(_dt, _timeout);
					osg::notify(osg::NOTICE) << "Networking: TIMEOUT" << std::endl;
				}
				
				_block.reset();
#else
				_barrier.block(2); 
				
#endif

				updateNetworkStatsTimeout();
			}
			else
			{
				if (_network)
				{
					_network->process();
				}
			}
			break;
		}
    }

	virtual void clean(OpenIG::PluginBase::PluginContext& context)
	{		
		switch (_mode)
		{
		case Master:
		{
			for (unsigned int i = 0; i < 50; ++i)
			{
				Header header(context.getImageGenerator()->getViewer()->getFrameStamp()->getFrameNumber());
				header.masterIsDead = 1;

				if (_network.get())
				{
					*_network << header;
				}
			}
		}
		break;
		}
	}

	void updateNetworkStatsFrameDrops(bool flag)
	{
		double y = flag ? _frameDropsGraphPosAndSize.w() : 0.0;;

		if (!_frameDropsGraphVertices.valid()) return;

		osg::ref_ptr<osg::Vec3Array> newVertices = new osg::Vec3Array;
		for (unsigned int i = 0; i < _frameDropsGraphVertices->size(); ++i)
		{
			if (i)
			{
				osg::Vec3& v = _frameDropsGraphVertices->at(i);
				v.x() = _frameDropsGraphPosAndSize.x() + (i - 1);
				newVertices->push_back(v);
			}
		}

		newVertices->push_back(osg::Vec3(_frameDropsGraphPosAndSize.x() + (_frameDropsGraphPosAndSize.z() - 1), _frameDropsGraphPosAndSize.y() + y, 1));

		_frameDropsGraphGeometry->setVertexArray(_frameDropsGraphVertices = newVertices);
		_frameDropsGraphVertices->dirty();
	}

protected:
	enum Mode
	{
		Master,
		Slave,
		None
	};

	enum Protocol
	{
		UDP,
		TCP,
		Unknown
	};

	Mode													_mode;
	Protocol												_protocol;
	bool													_multiThreaded;
	std::string												_server;
	unsigned int											_port;	
	OpenIG::Base::ImageGenerator*									_ig;	
	boost::thread											_slaveThread;
	boost::thread											_masterTCPThread;
	boost::shared_ptr<OpenIG::Library::Networking::Network>			_network;
	OpenThreads::Barrier									_barrier;
	double													_timeout;
	OpenThreads::Block										_block;
	double													_timeoutDT;
	osg::ref_ptr<osg::Group>								_stats;
	bool													_statsOn;
	double													_dt;
	osg::ref_ptr<osg::Vec3Array>							_timeGraphVertices;
	osg::ref_ptr<osg::Geometry>								_timeGraphGeometry;
	osg::Vec4												_timeGraphPosAndSize;
	unsigned int											_timeGraphSteps;
	osg::ref_ptr<osg::Vec3Array>							_frameDropsGraphVertices;
	osg::Vec4												_frameDropsGraphPosAndSize;
	osg::ref_ptr<osg::Geometry>								_frameDropsGraphGeometry;
	std::string												_slaveName;
	osg::ref_ptr<osgText::Text>								_tcpClientsText;
	std::string												_host;


	void updateNetworkStatsTimeout()
	{
		double d = _timeout / _timeGraphSteps;
		double y = _dt * _timeGraphPosAndSize.w() / _timeout;

		if (!_timeGraphVertices.valid()) return;
		if (!_timeGraphGeometry.valid()) return;

		osg::ref_ptr<osg::Vec3Array> newVertices = new osg::Vec3Array;
		for (unsigned int i = 0; i < _timeGraphVertices->size(); ++i)
		{
			if (i)
			{
				osg::Vec3& v = _timeGraphVertices->at(i);
				v.x() = _timeGraphPosAndSize.x() + (i - 1);
				newVertices->push_back(v);
			}
		}

		newVertices->push_back(osg::Vec3(_timeGraphPosAndSize.x() + (_timeGraphPosAndSize.z() - 1), _timeGraphPosAndSize.y() + y, 1));

		_timeGraphGeometry->setVertexArray(_timeGraphVertices = newVertices);
		_timeGraphVertices->dirty();
	}

	void createNetworkStatsTCPClients()
	{
		if (!_stats.valid())
		{
			_stats = new osg::Group;
			_ig->getViewer()->getView(0)->getSceneData()->asGroup()->addChild(_stats);
		}

		setNetworkStats(_statsOn);

		osg::Camera* camera = new osg::Camera;
		_stats->addChild(camera);

		osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
		if (!wsi)
		{
			osg::notify(osg::NOTICE) << "Error, no WindowSystemInterface available, cannot create windows." << std::endl;
			return;
		}

		unsigned int width, height;
		wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(0), width, height);

		width = 1280;
		height = 1024;
		camera->setProjectionMatrix(osg::Matrix::ortho2D(0, width, 0, height));
		camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
		camera->setViewMatrix(osg::Matrix::identity());
		camera->setClearMask(GL_DEPTH_BUFFER_BIT);
		camera->setRenderOrder(osg::Camera::POST_RENDER);
		camera->setAllowEventFocus(true);
		camera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

		osg::Geode* geode = new osg::Geode;
		camera->addChild(geode);

		geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);

		osg::Geometry* geometry = new osg::Geometry;
		geode->addDrawable(geometry);

		float depth = 0.0f;

		const osg::Vec2 size(400, 600);

		osg::Vec3Array* vertices = new osg::Vec3Array;
		vertices->push_back(osg::Vec3(1280 - size.x(), 30, depth));
		vertices->push_back(osg::Vec3(1280 - 10, 30, depth));
		vertices->push_back(osg::Vec3(1280 - 10, 30 + size.y(), depth));
		vertices->push_back(osg::Vec3(1280 - size.x(), 30 + size.y(), depth));
		geometry->setVertexArray(vertices);

		osg::Vec3Array* normals = new osg::Vec3Array;
		normals->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));
		geometry->setNormalArray(normals);
		geometry->setNormalBinding(osg::Geometry::BIND_OVERALL);

		osg::Vec4Array* colors = new osg::Vec4Array;
		colors->push_back(osg::Vec4(.2f, .2, 0.8f, 0.35f));
		geometry->setColorArray(colors);
		geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

		geometry->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, 4));
		geometry->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
		geometry->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

		geode->getOrCreateStateSet()->setRenderBinDetails(HUD_RENDER_BIN, "RenderBin");

		_tcpClientsText = new osgText::Text;
		geode->addDrawable(_tcpClientsText);

		_tcpClientsText->setFont("fonts/arial.ttf");
		_tcpClientsText->setPosition(osg::Vec3(1280 - size.x() + 10, 30 + size.y()-10, 1));
		_tcpClientsText->setAlignment(osgText::TextBase::LEFT_TOP);
		_tcpClientsText->setColor(osg::Vec4(1, 1, 1, 1));
		_tcpClientsText->setCharacterSize(20);
		_tcpClientsText->setCharacterSizeMode(osgText::TextBase::OBJECT_COORDS);
		_tcpClientsText->setAxisAlignment(osgText::TextBase::XY_PLANE);
		_tcpClientsText->setFontResolution(256, 256);
		_tcpClientsText->setText("Slaves connected:");
		_tcpClientsText->setDataVariance(osg::Object::DYNAMIC);

	}

	void createNetworkStatsFrameDrops()
	{
		if (!_stats.valid())
		{
			_stats = new osg::Group;
			_ig->getViewer()->getView(0)->getSceneData()->asGroup()->addChild(_stats);
		}

		setNetworkStats(_statsOn);

		osg::Camera* camera = new osg::Camera;
		_stats->addChild(camera);

		osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
		if (!wsi)
		{
			osg::notify(osg::NOTICE) << "Error, no WindowSystemInterface available, cannot create windows." << std::endl;
			return;
		}

		unsigned int width, height;
		wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(0), width, height);

		width = 1280;
		height = 1024;
		camera->setProjectionMatrix(osg::Matrix::ortho2D(0, width, 0, height));
		camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
		camera->setViewMatrix(osg::Matrix::identity());
		camera->setClearMask(GL_DEPTH_BUFFER_BIT);
		camera->setRenderOrder(osg::Camera::POST_RENDER);
		camera->setAllowEventFocus(true);
		camera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

		osg::Geode* geode = new osg::Geode;
		camera->addChild(geode);

		geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);

		osg::Geometry* geometry = new osg::Geometry;
		geode->addDrawable(geometry);

		float depth = 0.0f;

		const osg::Vec2 size(600, 200);

		osg::Vec3Array* vertices = new osg::Vec3Array;
		vertices->push_back(osg::Vec3(1280 - size.x(), 30+size.y()+20, depth));
		vertices->push_back(osg::Vec3(1280 - 10, 30+size.y()+20, depth));
		vertices->push_back(osg::Vec3(1280 - 10, 30 + size.y()*2+20, depth));
		vertices->push_back(osg::Vec3(1280 - size.x(), 30 + size.y()*2+20, depth));
		geometry->setVertexArray(vertices);

		osg::Vec3Array* normals = new osg::Vec3Array;
		normals->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));
		geometry->setNormalArray(normals);
		geometry->setNormalBinding(osg::Geometry::BIND_OVERALL);

		osg::Vec4Array* colors = new osg::Vec4Array;
		colors->push_back(osg::Vec4(1.f, 1., 1.f, 0.5f));
		geometry->setColorArray(colors);
		geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

		geometry->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, 4));
		geometry->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
		geometry->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

		geode->getOrCreateStateSet()->setRenderBinDetails(HUD_RENDER_BIN, "RenderBin");

		osg::Vec4 posAndSize(1280 - size.x(), 30+size.y()+20, size.x() - 10, size.y());
		osg::Vec2 margin(10, 10);

		_frameDropsGraphGeometry = new osg::Geometry;
		geode->addDrawable(_frameDropsGraphGeometry);

		_frameDropsGraphVertices = new osg::Vec3Array;
		_frameDropsGraphGeometry->setVertexArray(_frameDropsGraphVertices);

		for (unsigned int i = 0; i < (unsigned int)posAndSize.z(); ++i)
		{
			_frameDropsGraphVertices->push_back(osg::Vec3(posAndSize.x() + i, posAndSize.y(), 1));
		}
		_frameDropsGraphGeometry->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP, 0, _timeGraphVertices->size()));
		_frameDropsGraphGeometry->getOrCreateStateSet()->setAttributeAndModes(new osg::LineWidth(1.5f), osg::StateAttribute::ON);

		_frameDropsGraphPosAndSize = posAndSize;

		_frameDropsGraphGeometry->setNormalArray(normals);
		_frameDropsGraphGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);

		osg::Vec4Array* colors2 = new osg::Vec4Array;
		colors2->push_back(osg::Vec4(1.0f, .0f, .0f, 1.0f));
		_frameDropsGraphGeometry->setColorArray(colors2);
		_frameDropsGraphGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

		std::string timesFont("fonts/arial.ttf");

		osg::StateSet* stateset = geode->getOrCreateStateSet();
		stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

		osgText::Text* text = new  osgText::Text;
		geode->addDrawable(text);

		text->setFont(timesFont);
		text->setPosition(osg::Vec3(posAndSize.x(), posAndSize.y() + posAndSize.w(), 1));
		text->setAlignment(osgText::TextBase::LEFT_BOTTOM_BASE_LINE);
		text->setColor(osg::Vec4(1, 1, 1, 1));
		text->setCharacterSize(20);
		text->setCharacterSizeMode(osgText::TextBase::OBJECT_COORDS);
		text->setAxisAlignment(osgText::TextBase::XY_PLANE);
		text->setFontResolution(256, 256);
		text->setText("FRAME DROPS");
	}

	void createNetworkStatsTimeout()
	{
		_stats = new osg::Group;

		setNetworkStats(_statsOn);

		_ig->getViewer()->getView(0)->getSceneData()->asGroup()->addChild(_stats);

		osg::Camera* camera = new osg::Camera;
		_stats->addChild(camera);

		osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
		if (!wsi)
		{
			osg::notify(osg::NOTICE) << "Error, no WindowSystemInterface available, cannot create windows." << std::endl;
			return;
		}

		unsigned int width, height;
		wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(0), width, height);

		width = 1280;
		height = 1024;
		camera->setProjectionMatrix(osg::Matrix::ortho2D(0, width, 0, height));
		camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
		camera->setViewMatrix(osg::Matrix::identity());
		camera->setClearMask(GL_DEPTH_BUFFER_BIT);
		camera->setRenderOrder(osg::Camera::POST_RENDER);
		camera->setAllowEventFocus(true);
		camera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

		osg::Geode* geode = new osg::Geode;
		camera->addChild(geode);

		geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);

		osg::Geometry* geometry = new osg::Geometry;
		geode->addDrawable(geometry);

		float depth = 0.0f;

		const osg::Vec2 size(600,200);

		osg::Vec3Array* vertices = new osg::Vec3Array;
		vertices->push_back(osg::Vec3(1280-size.x(), 30, depth));
		vertices->push_back(osg::Vec3(1280-10, 30, depth));
		vertices->push_back(osg::Vec3(1280-10, 30+size.y(), depth));
		vertices->push_back(osg::Vec3(1280-size.x(), 30+size.y(), depth));
		geometry->setVertexArray(vertices);

		osg::Vec3Array* normals = new osg::Vec3Array;
		normals->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));
		geometry->setNormalArray(normals);
		geometry->setNormalBinding(osg::Geometry::BIND_OVERALL);

		osg::Vec4Array* colors = new osg::Vec4Array;
		colors->push_back(osg::Vec4(.2f, .2, 0.8f, 0.5f));
		geometry->setColorArray(colors);
		geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

		geometry->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, 4));
		geometry->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
		geometry->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

		geode->getOrCreateStateSet()->setRenderBinDetails(HUD_RENDER_BIN, "RenderBin");

		osg::ref_ptr<osg::Geometry> timeGraphGeometry1 = new osg::Geometry;
		geode->addDrawable(timeGraphGeometry1);

		osg::Vec4 posAndSize(1280 - size.x(), 30, size.x() - 10, size.y());
		osg::Vec2 margin(10, 10);

		osg::ref_ptr<osg::Vec3Array> lines1 = new osg::Vec3Array;
		lines1->push_back(osg::Vec3(posAndSize.x(), posAndSize.y(), depth));
		lines1->push_back(osg::Vec3(posAndSize.x(), posAndSize.y() + size.y(), depth));
		lines1->push_back(osg::Vec3(posAndSize.x(), posAndSize.y(), depth));
		lines1->push_back(osg::Vec3(posAndSize.x() + size.x(), posAndSize.y(), depth));
		timeGraphGeometry1->setVertexArray(lines1);

		timeGraphGeometry1->setNormalArray(normals);
		timeGraphGeometry1->setNormalBinding(osg::Geometry::BIND_OVERALL);

		osg::Vec4Array* colors1 = new osg::Vec4Array;
		colors1->push_back(osg::Vec4(1.f, 1.f, 1.f, 1.f));
		timeGraphGeometry1->setColorArray(colors1);
		timeGraphGeometry1->setColorBinding(osg::Geometry::BIND_OVERALL);

		timeGraphGeometry1->addPrimitiveSet(new osg::DrawArrays(GL_LINES, 0, 4));
		timeGraphGeometry1->getOrCreateStateSet()->setAttributeAndModes(new osg::LineWidth(1.5f), osg::StateAttribute::ON);

		osg::ref_ptr<osg::Geometry> timeGraphGeometry2 = new osg::Geometry;
		geode->addDrawable(timeGraphGeometry2);

		osg::ref_ptr<osg::Vec3Array> lines2 = new osg::Vec3Array;
		timeGraphGeometry2->setVertexArray(lines2);

		_timeGraphSteps = 10.0;
		double dt = _timeout / _timeGraphSteps;
		double inc = posAndSize.w() / _timeGraphSteps;
		double d = 0;
		while (d <= posAndSize.w())
		{
			lines2->push_back(osg::Vec3(posAndSize.x(), posAndSize.y() + d, depth));
			lines2->push_back(osg::Vec3(posAndSize.x() + posAndSize.z(), posAndSize.y() + d, depth));

			d += inc;
		}

		double ar = height*1.0 / width;
		d = inc*ar;
		while (d <= posAndSize.z())
		{
			lines2->push_back(osg::Vec3(posAndSize.x() + d, posAndSize.y(), depth));
			lines2->push_back(osg::Vec3(posAndSize.x() + d, posAndSize.y() + posAndSize.w(), depth));

			d += inc*ar;
		}

		timeGraphGeometry2->addPrimitiveSet(new osg::DrawArrays(GL_LINES, 0, lines2->size()));
		timeGraphGeometry2->getOrCreateStateSet()->setAttributeAndModes(new osg::LineWidth(0.5f), osg::StateAttribute::ON);

		osg::Vec4Array* colors2 = new osg::Vec4Array;
		colors2->push_back(osg::Vec4(.5f, .5f, .5f, 1.0f));
		timeGraphGeometry2->setColorArray(colors2);
		timeGraphGeometry2->setColorBinding(osg::Geometry::BIND_OVERALL);

		timeGraphGeometry2->setNormalArray(normals);
		timeGraphGeometry2->setNormalBinding(osg::Geometry::BIND_OVERALL);

		std::string timesFont("fonts/arial.ttf");

		osg::StateSet* stateset = geode->getOrCreateStateSet();
		stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

		d = 0;
		unsigned int dtinc = 0;
		while (d <= posAndSize.w())
		{
			osgText::Text* text = new  osgText::Text;
			geode->addDrawable(text);

			text->setFont(timesFont);
			text->setPosition(osg::Vec3(posAndSize.x(), posAndSize.y() + d, 1));
			text->setAlignment(osgText::TextBase::LEFT_BOTTOM_BASE_LINE);
			text->setColor(osg::Vec4(1, 1, 1, 1));
			text->setCharacterSize(20);
			text->setCharacterSizeMode(osgText::TextBase::OBJECT_COORDS);
			text->setAxisAlignment(osgText::TextBase::XY_PLANE);
			text->setFontResolution(256, 256);

			std::ostringstream oss;
			if (dtinc < _timeGraphSteps)
				oss << std::fixed << (double)dt*dtinc;
			else
				oss << "TIMEOUT";
			text->setText(oss.str());

			dtinc += 2.0;
			d += inc * 2;
		}

		_timeGraphGeometry = new osg::Geometry;
		geode->addDrawable(_timeGraphGeometry);

		_timeGraphGeometry->setUseDisplayList(false);
		_timeGraphGeometry->setUseVertexBufferObjects(true);
		_timeGraphGeometry->setDataVariance(osg::Object::DYNAMIC);

		osg::Vec4Array* colors3 = new osg::Vec4Array;
		colors3->push_back(osg::Vec4(1.0f, .0f, .0f, 1.0f));
		_timeGraphGeometry->setColorArray(colors3);
		_timeGraphGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

		_timeGraphGeometry->setNormalArray(normals);
		_timeGraphGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);

		_timeGraphVertices = new osg::Vec3Array;
		_timeGraphGeometry->setVertexArray(_timeGraphVertices);

		for (unsigned int i = 0; i < (unsigned int)posAndSize.z(); ++i)
		{
			_timeGraphVertices->push_back(osg::Vec3(posAndSize.x()+i,posAndSize.y(),1));
		}
		_timeGraphGeometry->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP, 0, _timeGraphVertices->size()));
		_timeGraphGeometry->getOrCreateStateSet()->setAttributeAndModes(new osg::LineWidth(1.5f), osg::StateAttribute::ON);

		_timeGraphPosAndSize = posAndSize;

	}
};

HeaderCallback::HeaderCallback(OpenIG::Base::ImageGenerator* ig, NetworkingPlugin* nplugin)
: imageGenerator(ig)
, plugin(nplugin)
, frameNumber(0)
, packetDropsDetected(false)
{

}

void HeaderCallback::process(OpenIG::Library::Networking::Packet& packet)
{
	Header* h = dynamic_cast<Header*>(&packet);
	if (h)
	{
		if (h->masterIsDead == 1) imageGenerator->getViewer()->setDone(true);

		static bool firstHeaderPacket = true;
		if (firstHeaderPacket)
		{
			firstHeaderPacket = false;
		}
		else
		{
			if (frameNumber + 1 != h->frameNumber)
			{
				osg::notify(osg::NOTICE) << "Networking: Detected packet drops. Frame #" << h->frameNumber << std::endl;
				packetDropsDetected = true;

				plugin->updateNetworkStatsFrameDrops(true);
			}
			else
			{
				plugin->updateNetworkStatsFrameDrops(false);
				packetDropsDetected = false;
			}
		}
		frameNumber = h->frameNumber;

	}
}

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
	return new OpenIG::Plugins::NetworkingPlugin;
}

extern "C" EXPORT void DeletePlugin(OpenIG::PluginBase::Plugin* plugin)
{
    osg::ref_ptr<OpenIG::PluginBase::Plugin> p(plugin);
}
