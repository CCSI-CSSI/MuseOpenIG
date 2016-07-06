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
//#*	author    Roni Zanoli <roni@compro.net>
//#*	copyright(c)Compro Computer Services, Inc.

#ifndef CONFIGREADER_H
#define CONFIGREADER_H

#include <osgDB/XmlParser>

#if defined(OPENIG_SDK)
	#include <OpenIG-Bullet/export.h>
#else
	#include <Library-Bullet/export.h>
#endif

namespace OpenIG {
	namespace Library {
		namespace Bullet {

			class IGLIBBULLET_EXPORT VehicleData
			{
			public:
				std::string		model;
				std::string		tire_model;
				osg::Vec3		pos;
				osg::Vec3		front_wheel_pos;
				osg::Vec3		rear_wheel_pos;
				float			wheel_radius;
				int				right_index;
				int				up_index;
				int				forward_index;
				float			wheel_friction;
				float			suspension_rest_length;
				float			suspension_stiffness;
				float			suspension_damping;
				float			suspension_compression;
				float			steering_clamp;
				float			steering_increment;
				float			roll_influence;
				osg::Vec3		wheel_direction_CS0;
				osg::Vec3		wheel_axle_CS;
				float			engine_force;
				float			brakes;
				int				id;
				float			mass;
				float			heading;
			};

			class IGLIBBULLET_EXPORT ViewData
			{
			public:
				ViewData() : x(0), y(0), width(640), height(480), windowDecoration(true) {}
				float		x;
				float		y;
				float		width;
				float		height;
				bool		windowDecoration;
				std::string name;
			};

			class IGLIBBULLET_EXPORT TerrainData
			{
			public:
				std::string		terrain;
				std::string		texture;
				osg::Vec3		local_db_size;
			};

			class IGLIBBULLET_EXPORT ConfigReader
			{
			public:
				ConfigReader();

				void								readXML(std::string datapath, std::string fileName);
				inline std::vector<VehicleData*>	getVehicleDataList(void) { return vehicle_data_list; }
				inline ViewData						getViewData(void) { return view_data; }
				inline bool							Debug(void) { return _debug; }
				inline TerrainData					getTerrainData(void) { return terrain_data; }

			private:
				void		readVehicleInfo(osgDB::XmlNode* node);
				void		readViewInfo(osgDB::XmlNode* node);
				void		readTerrainInfo(osgDB::XmlNode* node);
				osg::Vec3	getVector3(osgDB::XmlNode::Properties properties);
				std::string buildPath(std::string);

				std::vector<VehicleData*>	vehicle_data_list;
				ViewData					view_data;
				bool						_debug;
				TerrainData					terrain_data;
				std::string					data_path;
			};
		}
	}
} // namespace

#endif // CONFIGREADER_H
