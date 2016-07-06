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
//#*	author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
//#*	copyright(c)Compro Computer Services, Inc.

#include <OpenIG-Protocol/header.h>
#include <OpenIG-Protocol/entitystate.h>
#include <OpenIG-Protocol/hot.h>
#include <OpenIG-Protocol/los.h>
#include <OpenIG-Protocol/camera.h>
#include <OpenIG-Protocol/hotresponse.h>
#include <OpenIG-Protocol/losresponse.h>
#include <OpenIG-Protocol/camera.h>
#include <OpenIG-Protocol/tod.h>
#include <OpenIG-Protocol/command.h>
#include <OpenIG-Protocol/lightstate.h>

#include <OpenIG-Networking/udpnetwork.h>
#include <OpenIG-Networking/tcpclient.h>
#include <OpenIG-Networking/buffer.h>
#include <OpenIG-Networking/factory.h>
#include <OpenIG-Networking/parser.h>

#include <OpenIG-Base/mathematics.h>

#include <osg/ArgumentParser>
#include <osg/Matrix>
#include <osg/Quat>
#include <osg/Vec3>
#include <osg/io_utils>
#include <osg/Timer>

#include <OpenThreads/Thread>

#include <boost/shared_ptr.hpp>

#include <map>

typedef std::map<unsigned int, osg::Vec3d>		ResponsesMap;
ResponsesMap									hotResponses;
ResponsesMap									losResponsesPosition;
ResponsesMap									losResponsesNormal;

struct NotifyErrorHandler : public OpenIG::Library::Networking::ErrorHandler
{
	virtual std::ostream& operator<<(const std::ostringstream& oss)
	{
		std::cout << oss.str();
		return std::cout;
	}
};

struct Parser : public OpenIG::Library::Networking::Parser
{
	Parser()
	{
		OpenIG::Library::Networking::Factory::instance()->addTemplate(new OpenIG::Library::Protocol::Header);
		OpenIG::Library::Networking::Factory::instance()->addTemplate(new OpenIG::Library::Protocol::HOTResponse);		
		OpenIG::Library::Networking::Factory::instance()->addTemplate(new OpenIG::Library::Protocol::LOSResponse);
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
		}
	}
};

struct HOTResponse : public OpenIG::Library::Networking::Packet::Callback
{
	HOTResponse()		
	{
	}

	virtual void process(OpenIG::Library::Networking::Packet& packet)
	{
		OpenIG::Library::Protocol::HOTResponse* response = dynamic_cast<OpenIG::Library::Protocol::HOTResponse*>(&packet);
		if (response)
		{
			//std::cout << "HOT response recieved. Request ID:" << response->id << ", " << response->position << std::endl;

			hotResponses[response->id] = response->position;
		}
	}	
};

struct LOSResponse : public OpenIG::Library::Networking::Packet::Callback
{
	LOSResponse()
	{
	}

	virtual void process(OpenIG::Library::Networking::Packet& packet)
	{
		OpenIG::Library::Protocol::LOSResponse* response = dynamic_cast<OpenIG::Library::Protocol::LOSResponse*>(&packet);
		if (response)
		{
			//std::cout << "LOS response recieved. Request ID:" << response->id << ", " << response->position << ", " << response->normal << std::endl;

			losResponsesPosition[response->id] = response->position;
			losResponsesNormal[response->id] = response->normal;
		}
	}
};

int main(int argc, char** argv)
{
	osg::ArgumentParser arguments(&argc, argv);

	arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
	arguments.getApplicationUsage()->setDescription(arguments.getApplicationName() + " is a sample Host for OpenIG");
	arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName() + " [options]");
	arguments.getApplicationUsage()->addCommandLineOption("--host <host ip address>", "The IP of the host");
	arguments.getApplicationUsage()->addCommandLineOption("--port <port>", "The port to be used");
	arguments.getApplicationUsage()->addCommandLineOption("--server <server ip address>", "The IP of the Terrain Query Server");
	arguments.getApplicationUsage()->addCommandLineOption("--server_port <port>", "The port to be used for communication with the Terrain Query Server");

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
	std::string port = "8888";
	std::string server = "127.0.0.1";
	std::string serverport = "8889";

	while (arguments.read("--host", host));
	while (arguments.read("--port", port));
	while (arguments.read("--server", server));
	while (arguments.read("--server_port", serverport));

	OpenIG::Library::Networking::Network::log = boost::shared_ptr<OpenIG::Library::Networking::ErrorHandler>(new NotifyErrorHandler);

	// This is the UDP network that drives the IG
	boost::shared_ptr<OpenIG::Library::Networking::Network>	network = boost::shared_ptr<OpenIG::Library::Networking::UDPNetwork>(new OpenIG::Library::Networking::UDPNetwork(host));
	network->setPort(atoi(port.c_str()));

	// This is the TCP Client network that talks to the Terrain Query Server	
	boost::shared_ptr<OpenIG::Library::Networking::Network>	client = boost::shared_ptr<OpenIG::Library::Networking::TCPClient>(new OpenIG::Library::Networking::TCPClient(host,server));	
	client->setPort(atoi(serverport.c_str()));

	client->addCallback((OpenIG::Library::Networking::Packet::Opcode)OPCODE_HEADER, new HeaderCallback);
	client->addCallback((OpenIG::Library::Networking::Packet::Opcode)OPCODE_HOTRESPONSE, new HOTResponse);	
	client->addCallback((OpenIG::Library::Networking::Packet::Opcode)OPCODE_LOSRESPONSE, new LOSResponse);
	client->setParser(new Parser);

	// Initial values we are going to
	// change in runtime
	double x = 0.0;
	double y = 0.0;
	double z = 0.0; 	

	unsigned int frameNumber = 1;
	while (1)
	{		
		// We update the model
		// We are moving it forward along the runway
		static double dy = 0.0;
		dy += 1.5;		

		// The id of the HOT/LOS request/response
		unsigned int HOTRequestID = 1;
		unsigned int LOSRequestID = 1;		

		// We create header packet with the frame number
		OpenIG::Library::Protocol::Header header(frameNumber++);

		// This is to talk to the Terrain Query Server
		//-----------------------------------------------------------------------------------------
		OpenIG::Library::Networking::Buffer buffer_to_terrain_server(BUFFER_SIZE);

		// Write the header to the buffer
		header.write(buffer_to_terrain_server);			

		// write the entity update to the buffer
		// We create a packet for the entity update
		OpenIG::Library::Protocol::EntityState estate;
		estate.entityID = 1;
		estate.mx = OpenIG::Base::Math::instance()->toMatrix(x, y + dy, hotResponses[HOTRequestID].z() + 1.35, 0.0, 0.0, 0.0);
		estate.write(buffer_to_terrain_server);
		
		// HOT packet
		OpenIG::Library::Protocol::HOT hot;
		hot.id = HOTRequestID;
		hot.position = osg::Vec3d(x, y + dy, hotResponses[HOTRequestID].z()); 
		hot.write(buffer_to_terrain_server);

		// LOS packet
		OpenIG::Library::Protocol::LOS los;
		los.id = LOSRequestID;
		los.start= osg::Vec3d(x, y + dy, hotResponses[HOTRequestID].z());
		los.end = osg::Vec3d(x, y + dy + 500.0, hotResponses[HOTRequestID].z());
		los.write(buffer_to_terrain_server);		

		// Send to the network
		client->send(buffer_to_terrain_server);

		// Process resonses
		client->process();
		//-----------------------------------------------------------------------------------------

		// This is to talk to the IG
		//-----------------------------------------------------------------------------------------

		// We create the buffer and fill it with packets
		OpenIG::Library::Networking::Buffer buffer_to_ig(BUFFER_SIZE);

		// Write the header to the buffer
		header.write(buffer_to_ig);		

		// write the entity update to the buffer
		// We create a packet for the entity update		
		estate.mx = OpenIG::Base::Math::instance()->toMatrix(x, y + dy, hotResponses[HOTRequestID].z(), 0.0, 0.0, 0.0);
		estate.write(buffer_to_ig);

		// Turn the front wheels a bit
		estate.entityID = 10000;
		estate.mx = OpenIG::Base::Math::instance()->toMatrix(0.75, 1.45, 0.3, 70.0, 0.0, 0.0);
		estate.write(buffer_to_ig);

		estate.entityID = 10001;
		estate.mx = OpenIG::Base::Math::instance()->toMatrix(-0.75, 1.45, 0.3, 70.0, 0.0, 0.0);
		estate.write(buffer_to_ig);

#if 0
		// Command - all from OpenIG are supported
		OpenIG::Library::Protocol::Command cmd;
		cmd.command = "rain 1 & snow 1 & fog 1000";
		cmd.write(buffer_to_ig);
#endif

#if 0
		// Time of day
		// This is not working well with the SkyDome plugin,
		// - it shows some bright and dark scene. With the
		// SilverLining it works just great
		static unsigned int hours = 12;
		static unsigned int minutes = 0;

		OpenIG::Library::Protocol::TOD tod;
		tod.hour = hours;
		tod.minutes = minutes;
		tod.write(buffer_to_ig);

		++minutes;
		if (minutes == 60)
		{
			minutes = 0;
			++hours;
			if (hours > 20)
			{
				hours = 12;
			}
		}
#endif

#if 1
		// Let turn on/off the lights
		// each second
		static osg::Timer_t lastTime = 0;
		static bool			lightsOn = true;

		osg::Timer_t now = osg::Timer::instance()->tick();
		if (lastTime == 0) lastTime = now;

		if (osg::Timer::instance()->delta_s(lastTime, now) > 1.0)
		{
			lightsOn = !lightsOn;

			OpenIG::Library::Protocol::LightState ls;

			ls.id = 10000;
			ls.enabled = lightsOn;
			ls.write(buffer_to_ig);

			ls.id = 10001;
			ls.enabled = lightsOn;
			ls.write(buffer_to_ig);

			lastTime = now;

			//std::cout << "Lights on: " << lightsOn << std::endl;
		}

#endif

		// Send to the network
		network->send(buffer_to_ig);
		//-----------------------------------------------------------------------------------------

		// Breath a bit
		OpenThreads::Thread::microSleep(16000);

	}

	return 0;
}
