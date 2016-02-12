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
#include <Library-Bullet/configreader.h>

#include <boost/filesystem.hpp>

#include <osg/Notify>

using namespace OpenIG::Library::Bullet;

ConfigReader::ConfigReader() :
    _debug(false)
{
}

void
ConfigReader::
readXML( std::string datapath, std::string fileName )
{
    data_path = datapath;

    osg::ref_ptr<osgDB::XmlNode> root = osgDB::readXmlFile(datapath+fileName);

    if (!root.valid())
    {
        osg::notify(osg::NOTICE) << "Config: Failed to read the xml configuration: " << fileName << std::endl;
        return;
    }

    osg::ref_ptr<osgDB::XmlNode> config = root->children.size() ? root->children.at(0) : 0;
    if (!config.valid())
    {
        osg::notify(osg::NOTICE) << "Config: Expecting <OpenIG-Bullets-Config> tag" << std::endl;
        return;
    }

    osgDB::XmlNode::Children::iterator itr = config->children.begin();
    for ( ; itr != config->children.end(); ++itr)
    {
        osg::ref_ptr<osgDB::XmlNode> child = *itr;

        if (child->name == "View")
        {
            readViewInfo( child.get() );
        }
        else if (child->name == "Terrain")
        {
            readTerrainInfo( child.get() );
        }
        else if( child->name == "Vehicle" )
        {
            readVehicleInfo( child.get() );
        }
        else if( child->name == "debug" )
        {
            if( child->contents == "true" )
                _debug = true;
        }
    }
}

void
ConfigReader::
readTerrainInfo( osgDB::XmlNode* node )
{
    if (node == NULL)
        return;

    osgDB::XmlNode::Children::iterator itr = node->children.begin();
    for ( ; itr != node->children.end(); ++itr)
    {
        osg::ref_ptr<osgDB::XmlNode> child = *itr;

        if( child->name == "model" )
        {
            terrain_data.terrain = buildPath( child->contents );
        }
        else if( child->name == "texture" )
        {
            terrain_data.texture = buildPath( child->contents );
        }
        else if( child->name == "local_db_size" )
        {
            terrain_data.local_db_size = getVector3( child->properties );
        }
    }
}

std::string
ConfigReader::
buildPath( std::string name )
{
    std::string fileName;

    if( !boost::filesystem::exists(name) )
    {
        fileName = data_path+name;

        if( !boost::filesystem::exists(fileName) )
        {
             osg::notify(osg::FATAL) << "This is not a valid file name = " << fileName << std::endl;
        }
    }
    else
        fileName = name;

    return fileName;
}

void
ConfigReader::
readViewInfo( osgDB::XmlNode* node )
{
    if (node == NULL)
        return;

    osgDB::XmlNode::Children::iterator itr = node->children.begin();
    for ( ; itr != node->children.end(); ++itr)
    {
        osg::ref_ptr<osgDB::XmlNode> child = *itr;

        if( child->name == "viewport" )
        {
            osgDB::XmlNode::Properties::iterator pitr = child->properties.begin();
             for ( ; pitr != child->properties.end(); ++pitr )
             {
                 if (pitr->first == "x")
                     view_data.x = atof( pitr->second.c_str() );
                 else if (pitr->first == "y")
                     view_data.y = atof( pitr->second.c_str() );
                 else if (pitr->first == "w")
                     view_data.width = atof( pitr->second.c_str() );
                 else if (pitr->first == "h")
                    view_data.height = atof( pitr->second.c_str() );
             }
        }
        else if( child->name == "name" )
        {
            view_data.name = child->contents;
        }
        else if( child->name == "window_decoration" )
        {
            if( child->contents == "true" || child->contents == "TRUE" || child->contents == "True" )
                view_data.windowDecoration = true;
            else
                view_data.windowDecoration = false;
        }
    }
}

void
ConfigReader::
readVehicleInfo( osgDB::XmlNode* node )
{
    if (node == NULL)
        return;

    VehicleData* vd = new VehicleData;

    osgDB::XmlNode::Children::iterator itr = node->children.begin();
    for ( ; itr != node->children.end(); ++itr)
    {
        osg::ref_ptr<osgDB::XmlNode> child = *itr;

        if ( child->name == "id" )
        {
            vd->id = atoi( child->contents.c_str() );
        }
        else if ( child->name == "mass" )
        {
            vd->mass = atof( child->contents.c_str() );
        }
        else if ( child->name == "model" )
        {
            vd->model = buildPath( child->contents );
        }
        else if( child->name == "tire_model" )
            vd->tire_model = buildPath( child->contents );
        else if( child->name == "pos" )
        {
            vd->pos = getVector3( child->properties );
        }
        else if ( child->name == "heading" )
        {
            vd->heading = atof( child->contents.c_str() );
        }
        else if( child->name == "front_wheel_pos"  )
        {
            vd->front_wheel_pos = getVector3( child->properties );
        }
        else if( child->name == "rear_wheel_pos"  )
        {
            vd->rear_wheel_pos = getVector3( child->properties );
        }
        else if( child->name == "wheel_radius"  )
        {
            vd->wheel_radius = atof( child->contents.c_str() );
        }
        else if( child->name == "right_index"  )
        {
            vd->right_index = atoi( child->contents.c_str() );
        }
        else if( child->name == "up_index"  )
        {
            vd->up_index = atoi( child->contents.c_str() );
        }
        else if( child->name == "forward_index"  )
        {
            vd->forward_index = atoi( child->contents.c_str() );
        }
        else if( child->name == "wheel_friction"  )
        {
            vd->wheel_friction = atof( child->contents.c_str() );
        }
        else if( child->name == "suspension_rest_length"  )
        {
            vd->suspension_rest_length = atof( child->contents.c_str() );
        }
        else if( child->name == "suspension_stiffness"  )
        {
            vd->suspension_stiffness = atof( child->contents.c_str() );
        }
        else if( child->name == "suspension_damping"  )
        {
            vd->suspension_damping = atof( child->contents.c_str() );
        }
        else if( child->name == "suspension_compression"  )
        {
            vd->suspension_compression = atof( child->contents.c_str() );
        }
        else if( child->name == "roll_influence"  )
        {
            vd->roll_influence = atof( child->contents.c_str() );
        }
        else if( child->name == "steering_increment"  )
        {
            vd->steering_increment = atof( child->contents.c_str() );
        }
        else if( child->name == "steering_clamp"  )
        {
            vd->steering_clamp = atof( child->contents.c_str() );
        }
        else if( child->name == "wheel_direction_CS0"  )
        {
            vd->wheel_direction_CS0 = getVector3( child->properties );
        }
        else if( child->name == "wheel_axle_CS"  )
        {
            vd->wheel_axle_CS = getVector3( child->properties );
        }
        else if( child->name == "engine_force"  )
        {
            vd->engine_force = atof( child->contents.c_str() );
        }
        else if( child->name == "brakes"  )
        {
            vd->brakes = atof( child->contents.c_str() );
        }
    }

    vehicle_data_list.push_back( vd );
}

osg::Vec3
ConfigReader::
getVector3( osgDB::XmlNode::Properties properties )
{
    osg::Vec3 pos;

    osgDB::XmlNode::Properties::iterator pitr = properties.begin();

    for ( ; pitr != properties.end(); ++pitr )
    {
        if (pitr->first == "x")
            pos.x() = atof( pitr->second.c_str() );
        else if (pitr->first == "y")
            pos.y() = atof( pitr->second.c_str() );
        else if (pitr->first == "z")
            pos.z() = atof( pitr->second.c_str() );
    }

    return pos;
}
