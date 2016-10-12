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

#include <Library-Networking/packet.h>
#include <Library-Networking/network.h>
#include <Library-Networking/udpnetwork.h>
#include <Library-Networking/buffer.h>
#include <Library-Networking/parser.h>
#include <Library-Networking/factory.h>

#include <Library-Protocol/header.h>
#include <Library-Protocol/entitystate.h>
#include <Library-Protocol/camera.h>
#include <Library-Protocol/tod.h>
#include <Library-Protocol/lightstate.h>
#include <Library-Protocol/command.h>

#include <Core-Base/imagegenerator.h>
#include <Core-Base/attributes.h>
#include <Core-Base/mathematics.h>

#include <cstshare/cstshareobject.h>
#include <QtUtil/player_manager.h>
#include <QtUtil/util.h>

#include <Public_Headers/SilverLining.h>
#include <Public_Headers/CloudTypes.h>
#include <Public_Headers/AtmosphericConditions.h>
#include <Public_Headers/CloudLayer.h>
#include <Public_Headers/Version.h>

#include <Plugin-SilverLining/SkyDrawable.h>
#include <Plugin-SilverLining/AtmosphereReference.h>

#include <osg/ref_ptr>
#include <osg/Vec3>

#include <osgDB/XmlParser>

#include <iostream>
#include <iomanip>

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

namespace OpenIG {
    namespace Plugins {

        struct NotifyErrorHandler : public OpenIG::Library::Networking::ErrorHandler
        {
            virtual std::ostream& operator<<(const std::ostringstream& oss)
            {
                std::cout << oss.str();
                return std::cout;
            }
        };

        class MusePlugin : public PluginBase::Plugin
        {
        public:
            MusePlugin()
                : _context(0)
                , _cameraPos(0)
                , _cameraAtt(0)
                , _inUpdate(false)
            {
                _mutex = new boost::mutex;
            }

            std::string getStringType( CloudTypes type )
            {
                if( type == CIRROCUMULUS)
                    return "CIRROCUMULUS";
                else if( type == CIRRUS_FIBRATUS)
                    return "CIRRUS_FIBRATUS";
                else if( type == STRATUS)
                    return "STRATUS";
                else if( type == CUMULUS_MEDIOCRIS)
                    return "CUMULUS_MEDIOCRIS";
                else if( type == CUMULUS_CONGESTUS)
                    return "CUMULUS_CONGESTUS";
            #ifdef SLMIN_VERSION_REQUIRED
              #if (SLMIN_VERSION_REQUIRED(2122))
                else if( type == CUMULUS_CONGESTUS_HI_RES)
                    return "CUMULUS_CONGESTUS_HI_RES";
              #endif
              #if (SLMIN_VERSION_REQUIRED(3000))
                else if( type == TOWERING_CUMULUS)
                    return "TOWERING_CUMULUS";
              #endif
              #if (SLMIN_VERSION_REQUIRED(3227))
                else if( type == SANDSTORM)
                    return "SANDSTORM";
              #endif
            #endif
                else if( type == CUMULONIMBUS_CAPPILATUS)
                    return "CUMULONIMBUS_CAPPILATUS";
                else if( type == STRATOCUMULUS)
                    return "STRATOCUMULUS";
                else if( type == CUMULUS_CONGESTUS)
                    return "CUMULUS_CONGESTUS";
                else
                    return "NUM_CLOUD_TYPES";
            }

            CloudTypes getType( std::string type )
            {
                if( type == "CIRROCUMULUS")
                    return CIRROCUMULUS;
                else if( type == "CIRRUS_FIBRATUS")
                    return CIRRUS_FIBRATUS;
                else if( type == "STRATUS")
                    return STRATUS;
                else if( type == "CUMULUS_MEDIOCRIS")
                    return CUMULUS_MEDIOCRIS;
                else if( type == "CUMULUS_CONGESTUS")
                    return CUMULUS_CONGESTUS;
            #ifdef SLMIN_VERSION_REQUIRED
              #if (SLMIN_VERSION_REQUIRED(2122))
                else if( type == "CUMULUS_CONGESTUS_HI_RES")
                    return CUMULUS_CONGESTUS_HI_RES;
              #endif
              #if (SLMIN_VERSION_REQUIRED(3000))
                else if( type == "TOWERING_CUMULUS")
                    return TOWERING_CUMULUS;
              #endif
              #if (SLMIN_VERSION_REQUIRED(3227))
                else if( type == "SANDSTORM")
                    return SANDSTORM;
              #endif
            #endif
                else if( type == "CUMULONIMBUS_CAPPILATUS")
                    return CUMULONIMBUS_CAPPILATUS;
                else if( type == "STRATOCUMULUS")
                    return STRATOCUMULUS;
                else
                    return CUMULUS_CONGESTUS;
            }

            void cloudLayerType0(void* data)
            {
                // We create the buffer and fill it with packets
                OpenIG::Library::Networking::Buffer buffer_to_ig(BUFFER_SIZE);
                // We create header packet with the frame number
                OpenIG::Library::Protocol::Header header(frameNumber++);
                // Write the header to the buffer
                header.write(buffer_to_ig);

                addCloud0NoUpdate = true;
                _cl0Type = *(CloudTypes*)data;
                osg::notify(osg::NOTICE) << "MusePlugin cloudLayerType0 RMS setting  _cl0Type: " << getStringType(_cl0Type) << std::endl;

                _cl0Type      = (CloudTypes)ShareObj->getShare<int>("muse.effects.clouds.layer_type0");
                _cl0Thickness = ShareObj->getShare<float>("muse.effects.clouds.layer_thickness_0");
                _cl0Altitude  = ShareObj->getShare<float>("muse.effects.clouds.layer_altitude_0");
                _cl0Density   = ShareObj->getShare<float>("muse.effects.clouds.layer_density_0");

                //_cl0Thickness *=  Base::Math::instance()->M_PER_FT;
                //_cl0Altitude *=  Base::Math::instance()->M_PER_FT;

                // Command - all from OpenIG are supported
                OpenIG::Library::Protocol::Command cmd;
                cmd.command = "removeclouds 0";
                cmd.write(buffer_to_ig);
                //_ig->removeCloudLayer(0);
                if(_cl0Type == NUM_CLOUD_TYPES)
                    return;

                osg::notify(osg::NOTICE) << "MusePlugin cloudLayerType0 RMS set  _cl0Altitude: " << _cl0Altitude << std::endl;
                osg::notify(osg::NOTICE) << "MusePlugin cloudLayerType0 RMS set _cl0Thickness: " << _cl0Thickness << std::endl;
                osg::notify(osg::NOTICE) << "MusePlugin cloudLayerType0 RMS set   _cl0Density: " << _cl0Density << std::endl;

                std::ifstream infile;
                if(_cloudConfig.size() != 0)
                {
                    std::string filename = _cloudConfig;
                    filename.append(getStringType(_cl0Type));
                    filename.append(".config");
                    filename.append("1");
                    osg::notify(osg::NOTICE) << "MusePlugin cloudLayerType0 attempting to use file: " << filename << std::endl;
                    infile.open(filename.c_str(), std::ifstream::in);
                    if(infile.is_open())
                    {
                        osg::notify(osg::NOTICE) << "MusePlugin cloudLayerType0 file: " << filename << ", is OPEN" << std::endl;
                        infile.close();
                        cmd.command = "loadcloudfile 0 ";
                        cmd.command += filename;
                        cmd.command += " ";
                        cmd.command += (int)_cl0Type;
                        cmd.write(buffer_to_ig);
                        //_ig->loadCloudLayerFile(0,filename, (int)_cl0Type);
                    }
                    //Try to load layer manualy as a fallback.....
                    else
                    {
                       // _ig->addCloudLayer((int)0,(int)_cl0Type,(double)_cl0Altitude,(double)_cl0Thickness,(double)_cl0Density);
                        cmd.command  = "addclouds 0 ";
                        cmd.command += (int)_cl0Type;
                        cmd.command += " ";
                        cmd.command += (double)_cl0Altitude;
                        cmd.command += " ";
                        cmd.command +=  (double)_cl0Thickness;
                        cmd.command += " ";
                        cmd.command +=  (double)_cl0Density;
                        cmd.write(buffer_to_ig);
                    }
                }
                else
                {
                    //_ig->addCloudLayer((int)0,(int)_cl0Type,(double)_cl0Altitude,(double)_cl0Thickness,(double)_cl0Density);
                    cmd.command  = "addclouds 0 ";
                    cmd.command += (int)_cl0Type;
                    cmd.command += " ";
                    cmd.command += (double)_cl0Altitude;
                    cmd.command += " ";
                    cmd.command +=  (double)_cl0Thickness;
                    cmd.command += " ";
                    cmd.command +=  (double)_cl0Density;
                    cmd.write(buffer_to_ig);
                }
                _network->send(buffer_to_ig);
            }

            void updateLayer0(void *data)
            {
                //If we just add a new cloud, do not update when RMS values are setup for the first time CGR
                if(addCloud0NoUpdate)
                {
                    addCloud0NoUpdate = false;
                    return;
                }

                _cl0Type      = (CloudTypes)ShareObj->getShare<int>("muse.effects.clouds.layer_type0");
                _cl0Thickness = ShareObj->getShare<float>("muse.effects.clouds.layer_thickness_0");
                _cl0Altitude  = ShareObj->getShare<float>("muse.effects.clouds.layer_altitude_0");
                _cl0Density   = ShareObj->getShare<float>("muse.effects.clouds.layer_density_0");

                //_cl0Thickness *= Base::Math::instance()->M_PER_FT;
                //_cl0Altitude *= Base::Math::instance()->M_PER_FT;

                osg::notify(osg::NOTICE) << "MusePlugin updateLayer0 RMS setting  _cl0Type: " << getStringType(_cl0Type) << std::endl;
                osg::notify(osg::NOTICE) << "MusePlugin updateLayer0 RMS set  _cl0Altitude: " << _cl0Altitude << std::endl;
                osg::notify(osg::NOTICE) << "MusePlugin updateLayer0 RMS set _cl0Thickness: " << _cl0Thickness << std::endl;
                osg::notify(osg::NOTICE) << "MusePlugin updateLayer0 RMS set   _cl0Density: " << _cl0Density << std::endl;

                _ig->updateCloudLayer(0,_cl0Altitude, _cl0Thickness, _cl0Density);

            }

            void cloudLayerType1(void *data)
            {
                addCloud1NoUpdate = true;
                _cl1Type = *(CloudTypes*)data;
                osg::notify(osg::NOTICE) << "MusePlugin cloudLayerType0 RMS setting  _cl1Type: " << getStringType(_cl1Type) << std::endl;

                _cl1Type      = (CloudTypes)ShareObj->getShare<int>("muse.effects.clouds.layer_type1");
                _cl1Thickness = ShareObj->getShare<float>("muse.effects.clouds.layer_thickness_1");
                _cl1Altitude  = ShareObj->getShare<float>("muse.effects.clouds.layer_altitude_1");
                _cl1Density   = ShareObj->getShare<float>("muse.effects.clouds.layer_density_1");

                //_cl1Thickness *=  Base::Math::instance()->M_PER_FT;
                //_cl1Altitude *=  Base::Math::instance()->M_PER_FT;

                _ig->removeCloudLayer(1);
                if(_cl1Type == NUM_CLOUD_TYPES)
                    return;

                osg::notify(osg::NOTICE) << "MusePlugin cloudLayerType0 RMS set  _cl1Altitude: " << _cl1Altitude << std::endl;
                osg::notify(osg::NOTICE) << "MusePlugin cloudLayerType0 RMS set _cl1Thickness: " << _cl1Thickness << std::endl;
                osg::notify(osg::NOTICE) << "MusePlugin cloudLayerType0 RMS set   _cl1Density: " << _cl1Density << std::endl;

                std::ifstream infile;
                if(_cloudConfig.size() != 0)
                {
                    std::string filename = _cloudConfig;
                    filename.append(getStringType(_cl1Type));
                    filename.append(".config");
                    filename.append("2");
                    osg::notify(osg::NOTICE) << "MusePlugin cloudLayerType1 attempting to use file: " << filename << std::endl;
                    infile.open(filename.c_str(), std::ifstream::in);
                    if(infile.is_open())
                    {
                        osg::notify(osg::NOTICE) << "MusePlugin cloudLayerType1 file: " << filename << ", is OPEN" << std::endl;
                        infile.close();
                        _ig->loadCloudLayerFile(1,filename, (int)_cl1Type);
                    }
                }
                else
                    _ig->addCloudLayer((int)1,(int)_cl1Type,(double)_cl1Altitude,(double)_cl1Thickness,(double)_cl1Density);
            }

            void updateLayer1(void *data)
            {
                //If we just add a new cloud, do not update when RMS values are setup for the first time CGR
                if(addCloud1NoUpdate)
                {
                    addCloud1NoUpdate = false;
                    return;
                }

                _cl1Type      = (CloudTypes)ShareObj->getShare<int>("muse.effects.clouds.layer_type1");
                _cl1Thickness = ShareObj->getShare<float>("muse.effects.clouds.layer_thickness_1");
                _cl1Altitude  = ShareObj->getShare<float>("muse.effects.clouds.layer_altitude_1");
                _cl1Density   = ShareObj->getShare<float>("muse.effects.clouds.layer_density_1");

                //_cl1Thickness *=  Base::Math::instance()->M_PER_FT;
                //_cl1Altitude *=  Base::Math::instance()->M_PER_FT;

                osg::notify(osg::NOTICE) << "MusePlugin updateLayer1 RMS setting  _cl0Type: " << getStringType(_cl1Type) << std::endl;
                osg::notify(osg::NOTICE) << "MusePlugin updateLayer1 RMS set  _cl1Altitude: " << _cl1Altitude << std::endl;
                osg::notify(osg::NOTICE) << "MusePlugin updateLayer1 RMS set _cl1Thickness: " << _cl1Thickness << std::endl;
                osg::notify(osg::NOTICE) << "MusePlugin updateLayer1 RMS set   _cl1Density: " << _cl1Density << std::endl;

                _ig->updateCloudLayer(1,_cl1Altitude, _cl1Thickness, _cl1Density);
            }

            void setTOD( void *tod )
            {
                double time = ShareObj->getShare<double>( "muse.otw_scene.tod");
                //osg::notify(osg::NOTICE) << "MusePlugin setTimeOfDay time: " << std::setprecision(51) <<  time << std::endl;

                int oig_minutes = 0;

                //check for 1/4 hour increment to match incoming Muse time
                 int hour = int(time);
                 if (hour < time)
                 {
                     float minutes = time - hour;
                     //osg::notify(osg::NOTICE) << "MusePlugin setTimeOfDay minutes: " << minutes << std::endl;
                     if(minutes == 0.25)
                     {
                         oig_minutes = 15;
                         //osg::notify(osg::NOTICE) << "MusePlugin setTimeOfDay oig_minutes: " << oig_minutes << std::endl;
                     }
                     else if(minutes == 0.5)
                     {
                         oig_minutes = 30;
                         //osg::notify(osg::NOTICE) << "MusePlugin setTimeOfDay oig_minutes: " << oig_minutes << std::endl;
                     }
                     else if(minutes == 0.75)
                     {
                         oig_minutes = 45;
                         //osg::notify(osg::NOTICE) << "MusePlugin setTimeOfDay oig_minutes: " << oig_minutes << std::endl;
                     }
                 }
                 else
                     oig_minutes = 0;

                 osg::notify(osg::NOTICE) << "MusePlugin setTimeOfDay hour: " << std::dec << hour << ", minutes: " << oig_minutes << std::endl;
                _ig->setTimeOfDay(hour, oig_minutes);
            }

            void setVisibility( void *nm )
            {
                double _nm = *(float*)nm;
                float meters = _nm * Base::Math::instance()->M_PER_NMI;
                //osg::notify(osg::NOTICE) << "MusePlugin setVisibility incoming nm: " << _nm << ", output meters:" << meters << std::endl;

                _ig->setFog(meters);
            }

            void setRain( void *rain )
            {
                float r = *(float*)rain;
                _ig->setRain(r);
                //osg::notify(osg::NOTICE) << "MusePlugin setRain: " << r << std::endl;
            }

            void setSnow( void *snow )
            {
                float s = *(float*)snow;
                _ig->setSnow(s);
                //osg::notify(osg::NOTICE) << "MusePlugin setSnow: " << s << std::endl;
            }

            void setWindSpeed( void *speed )
            {
                ws = *(float*)speed;
                _ig->setWind(ws,wd);
                //osg::notify(osg::NOTICE) << "MusePlugin setWindSpeed: " << ws << ", wd:" << wd << std::endl;
            }

            void setWindHeading( void *heading )
            {
                wd = *(float*)heading;
                _ig->setWind(ws, wd);
                //osg::notify(osg::NOTICE) << "MusePlugin setWindHeading: " << ws << ", wd:" << wd << std::endl;
            }

            void updatePlayersThread()
            {
                std::cout << "PluginMuse::updatePlayersThread started!!!!!!!!" << std::endl;
                while(1)
                {
                    if(!_mutex->try_lock())
                    {
                        std::cout << "PluginMuse::updatePlayersThread!! try_locked!!" << std::endl;
                        continue;
                    }
                    _threadVpl.clear();
                    //std::cout << "PluginMuse::updatePlayersThread!! lock!!" << std::endl;

                    //_threadLocked = true;
                    if(ShareObj->getShare<bool>("UPDATEPLAYERS") == true)
                    {
                        _vpl = PManager()->getVisualPlayerList();
                        std::vector<VisualPlayer*>::iterator vpIt;
                        VisualEntity* ve;
                        //std::cout << "PluginMuse::updatePlayersThread found visualPlayerList size: " << _vpl.size() << std::endl;

                        for( vpIt=_vpl.begin(); vpIt!=_vpl.end(); vpIt++ )
                        {
                            ve = (*vpIt)->v_entity;

                            if( !ve )// || ve->id <= 1)
                                continue;
                            else if( ve->state == ACTIVE || ve->id == 1 )
                            {
                                osg::Vec3f pos = ve->pos;
                                osg::Vec3  att = ve->att;

                                //add player or update the player...
                                //std::cout << "PluginMuse::updatePlayersThread found active entity id: " << ve->id << ", name: " << (*vpIt)->name << ", ADDING!!!" << std::endl;

                                std::string model;
                                if((*vpIt)->name.compare(0,5,"Truck") == 0)
                                    model = "/usr/local/database/model/nissanTruck.osgb";
                                else if((*vpIt)->name.compare(0,3,"T72") == 0)
                                    model = "/usr/local/database/model/T72a.osgb";
                                else if((*vpIt)->name.compare(0,6,"ZSU-23") == 0)
                                    model = "/usr/local/database/model/zsu23.ive";
                                else if((*vpIt)->name.compare(0,6,"Mig-29") == 0)
                                    model = "/usr/local/database/model/Mig-29_Fulcrum.osgb";
                                else if((*vpIt)->name.compare(0,3,"AMX") == 0)
                                    model = "/usr/local/database/model/AMX_IT.ac";

                                //std::cout << "OIGMuseBridge::updatePlayersThread found active entity id: " << (*vpIt)->id << ", loading file: " << model << std::endl;

                                osg::Matrixd entitymd = OpenIG::Base::Math::instance()->toMuseMatrix(pos.x(),pos.y(),pos.z(),att.x(),att.y(),att.z());

                                if( (ve->id != 1) && (_ig->getEntityName(ve->id) == ""))
                                //if(ve->id == 2)
                                    {
                                        _ig->addEntity((*vpIt)->id, model.c_str(), entitymd);
                                        _ig->setEntityName((*vpIt)->id, model.c_str());
                                        std::cout << "OIGMuseBridge::updatePlayersThread adding active entity id: " << (*vpIt)->id << ", loading file: " << model << std::endl;
                                    }
                                //else
                                //    std::cout << "OIGMuseBridge::updatePlayersThread found active entity id: " << (*vpIt)->id << std::endl;

                                //Save away our copy for updating
                                _threadVpl.push_back((*vpIt));
                                continue;
                            }
                            else if( (ve->state != ACTIVE) && (_ig->getEntityName(ve->id) != "") )
                            {
                                _ig->removeEntity(ve->id);
                                std::cout << "OIGMuseBridge::updatePlayersThread Removing inactive entity id: " << (*vpIt)->id << std::endl;
                            }
                        }

                        _localVpl.clear();

                        osg::Vec3f pos = *_cameraPos;
                        osg::Vec3  att = *_cameraAtt;

                        _ownshipEye = Base::Math::instance()->toViewMatrix(
                                            pos.x(),
                                            pos.y(),
                                            pos.z(),
                                            att.x(),
                                            att.y(),
                                            att.z());

                        for( vpIt = _threadVpl.begin(); vpIt != _threadVpl.end() ; vpIt++ )
                        {
                            //add player or update the player...
//                            VisualEntity* ve = (*vpIt)->v_entity;
//                            if(ve->id == 1)
//                            {
//                                ve->att = att;
//                                ve->pos = pos;
//                            }

                            //Put in our own vector for updating of the scene in our update() method
                            //This way its all there from the same frame to update our OSG scenegraph.
                            _localVpl.push_back((*vpIt));
                            //_threadVpl.erase(vpIt);
                        }

                    }
                    //std::cout << "PluginMuse::updatePlayersThread!! unlocked!!" << std::endl;
                    _mutex->unlock();
                    OpenThreads::Thread::microSleep(1000);
                }
            }

            virtual std::string getName() { return "Muse"; }

            virtual std::string getDescription() { return "Integration sample of OpenIG and MUSE"; }

            virtual std::string getVersion() { return "2.0.0"; }

            virtual std::string getAuthor() { return "ComPro, Nick"; }

            virtual void clean(PluginBase::PluginContext&)
            {
            }

            virtual void init(PluginBase::PluginContext& context)
            {
                std::string      host = "10.5.63.11";
                std::string localhost = "127.0.0.1";
                unsigned int	 port = 8888;
                frameNumber           = 1;

                //OpenIG::Library::Networking::Network::log = boost::shared_ptr<OpenIG::Library::Networking::ErrorHandler>(new NotifyErrorHandler);

                _network = boost::shared_ptr<OpenIG::Library::Networking::UDPNetwork>(new OpenIG::Library::Networking::UDPNetwork(host));
                _network->setPort(port);

                _localhost = boost::shared_ptr<OpenIG::Library::Networking::UDPNetwork>(new OpenIG::Library::Networking::UDPNetwork(localhost));
                _localhost->setPort(port);
                setCloud0 = false;

                _context = context;
                _ig = context.getImageGenerator();
                _view = _ig->getViewer()->getView(0);

                // Time to get the visual player list from the current running Muse scenario...
                // and get it loaded into the OIG
                if( PManager()->getLoader() == NULL )
                    PManager()->loadPlayerDatabase("/usr/local/muse/amx/data/amx_cockpit/continuum/scenario/.scenario");

                ShareObj->addRegionSegment("LOCAL", "AMX");

                _cameraPos = ShareObj->createShare<osg::Vec3d>("otwPos");
                _cameraAtt = ShareObj->createShare<osg::Vec3>("otwAtt");
#if 0
                osg::notify(osg::NOTICE) << "3Camera Position from CstShare: "
                    << _cameraPos->x() << ", "
                    << _cameraPos->y() << ", "
                    << _cameraPos->z() << ", Att: "
                    << _cameraAtt->x() << ", "
                    << _cameraAtt->y() << ", "
                    << _cameraAtt->z() << std::endl;
#endif

/*
                _util.changeState<int>( "muse.effects.clouds.layer_type0", this, &MusePlugin::cloudLayerType0, false);
                _util.changeState<int>( "muse.effects.clouds.layer_type1", this, &MusePlugin::cloudLayerType1, false);
                _util.changeState<float>( "muse.effects.clouds.layer_thickness_0", this, &MusePlugin::updateLayer0, false );
                _util.changeState<float>( "muse.effects.clouds.layer_thickness_1", this, &MusePlugin::updateLayer1, false );
                _util.changeState<int>( "muse.effects.clouds.layer_altitude_0", this, &MusePlugin::updateLayer0, false );
                _util.changeState<int>( "muse.effects.clouds.layer_altitude_1", this, &MusePlugin::updateLayer1, false );
                _util.changeState<int>( "muse.effects.clouds.layer_density_0", this, &MusePlugin::updateLayer0, false );
                _util.changeState<int>( "muse.effects.clouds.layer_density_1", this, &MusePlugin::updateLayer1, false );

                _util.changeState<double>( "muse.otw_scene.tod", this, &MusePlugin::setTOD);
                _util.changeState<float>( "muse.otw_scene.rain_intensity", this, &MusePlugin::setRain );
                _util.changeState<float>( "muse.otw_scene.snow_intensity", this, &MusePlugin::setSnow );
                _util.changeState<float>( "muse.otw_scene.visibility_nm", this, &MusePlugin::setVisibility );

                _util.changeState<float>( "AMX_WIND_SPEED", this, &MusePlugin::setWindSpeed );
                _util.changeState<float>( "AMX_WIND_HEADING", this, &MusePlugin::setWindHeading );
*/
               _thread = boost::shared_ptr<boost::thread>(new boost::thread(&MusePlugin::updatePlayersThread, this));
            }

            virtual void config(const std::string& xmlFileName)
            {
                return;

                int id=0;
                osg::notify(osg::NOTICE) << "MusePlugin config -- using muse's cloud config files!!!" << std::endl;

                //Get location of SL cloud setup file and saved cloud files if in use....
                {
                    osgDB::XmlNode* root = osgDB::readXmlFile(xmlFileName);
                    if (root == 0) return;

                    if (root->children.size() == 0) return;

                    osgDB::XmlNode* config = root->children.at(0);
                    if (config->name != "OpenIG-Plugin-Config") return;


                    osgDB::XmlNode::Children::iterator itr = config->children.begin();
                    for (; itr != config->children.end(); ++itr)
                    {
                        osgDB::XmlNode* child = *itr;
                        if (child->name == "CloudConfig")
                        {
                            _cloudConfig = child->contents;
                        }
                    }
                }

                //Parse and save the Muse SL cloud layer(s) configuraiton information...
                {
                    std::string cloudConfigFile = _cloudConfig + "SilverLiningClouds.xml";
                    osgDB::XmlNode* root = osgDB::readXmlFile(cloudConfigFile);
                    if (root == 0) return;

                    if (root->children.size() == 0) return;

                    osgDB::XmlNode* config = root->children.at(0);
                    if (config->name != "CloudSettings") return;

                    osg::notify(osg::NOTICE) << "MusePlugin CloudSettings.size: " << config->children.size() << std::endl;
                    if ( config->children.size())
                    {
                        osgDB::XmlNode::Children::iterator itr = config->children.begin();
                        for (; itr != config->children.end(); ++itr)
                        {
                            osgDB::XmlNode* _clouds = *itr;
                            if (_clouds->name == "Cloud")
                            {
                                 //osg::notify(osg::NOTICE) << "MusePlugin found a Cloud entry " << std::endl;
                                 //osg::notify(osg::NOTICE) << "MusePlugin found a Cloud entries: " << _clouds->children.size() << std::endl;
                                 if(_clouds->children.size())
                                 {
                                    CloudLayerInfo cloudLayer;
                                    osgDB::XmlNode::Children::iterator cl_itr = _clouds->children.begin();
                                    for (; cl_itr != _clouds->children.end(); ++cl_itr)
                                    {
                                        osgDB::XmlNode* _cloud = *cl_itr;
                                        //osg::notify(osg::NOTICE) << "MusePlugin _cloud->name: " << _cloud->name << std::endl;
                                        if (_cloud->name == "type")
                                        {
                                            cloudLayer._type = (int)getType( _cloud->contents );
                                        }
                                        if (_cloud->name == "density")
                                        {
                                            cloudLayer._density = strtod(_cloud->contents.c_str(), NULL);
                                        }
                                        if (_cloud->name == "base_altitude")
                                        {
                                            cloudLayer._altitude = strtod(_cloud->contents.c_str(), NULL);// * Base::Math::instance()->M_PER_FT);
                                        }
                                        if (_cloud->name == "base_width")
                                        {
                                             cloudLayer._width = strtod(_cloud->contents.c_str(), NULL);
                                        }
                                        if (_cloud->name == "base_length")
                                        {
                                            cloudLayer._length = strtod(_cloud->contents.c_str(), NULL);
                                        }
                                        if (_cloud->name == "thickness")
                                        {
                                            cloudLayer._thickness = strtod(_cloud->contents.c_str(), NULL);// * Base::Math::instance()->M_PER_FT);
                                        }
                                        if (_cloud->name == "enable")
                                        {
                                            if( _cloud->contents.compare(0,4,"true") == 0 )
                                                cloudLayer._enable = true;
                                            else
                                                cloudLayer._enable = false;
                                        }
                                        if (_cloud->name == "infinity")
                                        {
                                            if(_cloud->contents.compare(0,4,"true") == 0)
                                                cloudLayer._infinite = true;
                                            else
                                                cloudLayer._enable = false;
                                        }
                                        if (_cloud->name == "layer")
                                        {
                                            cloudLayer._museLayer = atoi(_cloud->contents.c_str());
                                        }
                                        if (_cloud->name == "pos_offset")
                                        {
                                            if(_cloud->children.size())
                                            {
                                                osg::Vec3 cloud_pos;
                                                osgDB::XmlNode::Children::iterator pos_itr = _cloud->children.begin();
                                                for (; pos_itr != _cloud->children.end(); ++pos_itr)
                                                {
                                                    osgDB::XmlNode* _pos = *pos_itr;
                                                    if(_pos->name == "x")
                                                        cloud_pos.x() = (strtod(_pos->contents.c_str(), NULL));
                                                    if(_pos->name == "y")
                                                        cloud_pos.y() = (strtod(_pos->contents.c_str(), NULL));
                                                    if(_pos->name == "z")
                                                        cloud_pos.z() = (strtod(_pos->contents.c_str(), NULL));
                                                }
                                                cloudLayer._pos_offset = cloud_pos;
                                            }
                                        }

                                    }
                                    cloudLayer._id = id;
#if 0
                                    osg::notify(osg::NOTICE) << "MusePlugin config        Id: " << cloudLayer._id           << std::endl;
                                    osg::notify(osg::NOTICE) << "MusePlugin config      Type: " << cloudLayer._type         << std::endl;
                                    osg::notify(osg::NOTICE) << "MusePlugin config  Altitude: " << cloudLayer._altitude     << std::endl;
                                    osg::notify(osg::NOTICE) << "MusePlugin config Thickness: " << cloudLayer._thickness    << std::endl;
                                    osg::notify(osg::NOTICE) << "MusePlugin config   Density: " << cloudLayer._density      << std::endl;
                                    osg::notify(osg::NOTICE) << "MusePlugin config     Width: " << cloudLayer._width        << std::endl;
                                    osg::notify(osg::NOTICE) << "MusePlugin config    Length: " << cloudLayer._length       << std::endl;
                                    osg::notify(osg::NOTICE) << "MusePlugin config  Infinite: " << cloudLayer._infinite     << std::endl;
                                    osg::notify(osg::NOTICE) << "MusePlugin config MuseLayer: " << cloudLayer._museLayer    << std::endl;
                                    osg::notify(osg::NOTICE) << "MusePlugin config    Enable: " << cloudLayer._enable       << std::endl;
                                    osg::notify(osg::NOTICE) << "MusePlugin config    Offset: " << cloudLayer._pos_offset.x() << ":" <<\
                                                                                                   cloudLayer._pos_offset.y() << ":" <<\
                                                                                                   cloudLayer._pos_offset.z() << std::endl<<std::endl;
#endif
                                    id++;
                                    //save cloud layer information so we can load it when we have the proper context to load it....
                                    if(cloudLayer._museLayer == 1)
                                        _cloudsFromMuseLayer1ToAdd.push_back(cloudLayer);
                                    else
                                        _cloudsFromMuseLayer2ToAdd.push_back(cloudLayer);
                                 }
                             }//if (_clouds->name == "Cloud")

                         }
                     }
                 }

             }

            virtual void update(PluginBase::PluginContext& context)
            {
                //_inUpdate = true;

                bool once=true;
                _context = context;
                std::vector<VisualPlayer*>::iterator vpIt;
                //_util.update();

                _ig = context.getImageGenerator();

                if(!_mutex->try_lock())
                {
                    std::cout << "PluginMuse::update!! try_locked!!" << std::endl;
                    return;
                }
                //std::cout << "PluginMuse::update!! locked!!" << std::endl;
                //_ig->setCameraPosition(_ownshipEye,true);


                {
                    //We have it all loaded, just update the pos and att now with from data stored in our thread reading the data from CstShared
                    for( vpIt = _localVpl.begin(); vpIt != _localVpl.end() ; vpIt++ )
                    {
                        //add player or update the player...
                        //std::cout << "OIGMuseBridge::Update found active entity id: " << (*vpIt)->id << ", name: " << (*vpIt)->name << ", UPDATING!!!" << std::endl;
                        VisualEntity* ve = (*vpIt)->v_entity;

                        osg::Vec3f pos = ve->pos;
                        osg::Vec3  att = ve->att;

                        osg::Matrixd entitymd = OpenIG::Base::Math::instance()->toMuseMatrix(pos.x(),pos.y(),pos.z(),att.x(),att.y(),att.z());

                        if(ve->id == 1)
                        {
                            //osg::Matrixd eye = OpenIG::Base::Math::instance()->toViewMatrix(pos.x(),pos.y(),pos.z(),att.x(),att.y(),att.z());
                            _ig->setCameraPosition(_ownshipEye,true);
                        }
                        else
                        _ig->updateEntity( (*vpIt)->id, entitymd );

                        //_localVpl.erase(vpIt);
                    }
                }
                //std::cout << "PluginMuse::Update mutex unlock" << std::endl;
                _mutex->unlock();
            }


        protected:
            PluginBase::PluginContext _context;
            OpenIG::Base::ImageGenerator*   _ig;
            osg::Matrixd                    _eye;
            osg::Vec3d*                     _cameraPos;
            osg::Vec3*                      _cameraAtt;
            osgViewer::View*                _view;

            std::vector<VisualPlayer*>                     _vpl;
            std::vector<VisualPlayer*>                     _threadVpl;

            std::vector<VisualPlayer*>                  _oigVpl;

            std::vector<VisualPlayer*>                 _localVpl;
            bool                                       _inUpdate;
            bool                                       _threadLocked;
            boost::mutex*							   _mutex;

            // The network
            boost::shared_ptr<OpenIG::Library::Networking::Network> _network;
            boost::shared_ptr<OpenIG::Library::Networking::Network> _localhost;
            boost::shared_ptr<boost::thread> _thread;
            osg::Matrixd _ownshipEye;

            bool playerInitComplete;
            unsigned int frameNumber;

            bool                            setCloud0;

            Util                            _util;
            SilverLining::Atmosphere* _atmosphere;
            CloudTypes _cl0Type, _cl1Type, clType0, clType1;
            double _cl0Altitude, _cl1Altitude, clAltitude0, clAltitude1;
            double _cl0Thickness, _cl1Thickness, clThickness0, clThickness1;
            double _cl0Density, _cl1Density, clDensity0, clDensity1;
            double ws, wd;
            bool addCloud0NoUpdate;
            bool addCloud1NoUpdate;

            std::string _cloudConfig;
            typedef std::map< int, CloudLayerInfo >                     CloudLayers;
            typedef std::vector< CloudLayerInfo >                       CloudLayersQueue;
            typedef std::vector< CloudLayerInfo >::iterator             CloudLayersQueueIterator;
            CloudLayersQueue    _cloudsFromMuseLayer1ToAdd;
            CloudLayersQueue    _cloudsFromMuseLayer2ToAdd;
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
    return new OpenIG::Plugins::MusePlugin;
}

extern "C" EXPORT void DeletePlugin(OpenIG::PluginBase::Plugin* plugin)
{
    osg::ref_ptr<OpenIG::PluginBase::Plugin> p(plugin);
}
