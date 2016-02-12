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

#include "configuration.h"

#include <osgDB/XmlParser>

using namespace OpenIG::Base;

Configuration* Configuration::instance()
{
    static Configuration s_configuration;
    return &s_configuration;
}

bool Configuration::readFromXML(const std::string& fileName, const std::string& section)
{
    osg::ref_ptr<osgDB::XmlNode> root = osgDB::readXmlFile(fileName);
    if (!root.valid()) return false;

    if (root->children.size() == 0) return false;

    osgDB::XmlNode* config = root->children.at(0);
    if (config->name != section) return false;

    osgDB::XmlNode::Children::iterator itr = config->children.begin();
    for ( ; itr != config->children.end(); ++itr)
    {
        osgDB::XmlNode* child = *itr;
        _configuration[child->name] = child->contents;
    }

    return true;
}

const std::string Configuration::getConfig(const std::string& token, const std::string value)
{
    ConfigMapIterator itr = _configuration.find(token);
    if (itr != _configuration.end() && !itr->second.empty())
        return itr->second;

    return value;
}

double Configuration::getConfig(const std::string& token, double value)
{
    ConfigMapIterator itr = _configuration.find(token);
    if (itr != _configuration.end())
        return atof(itr->second.c_str());

    return value;
}

int Configuration::getConfig(const std::string& token, int value)
{
    ConfigMapIterator itr = _configuration.find(token);
    if (itr != _configuration.end())
        return atoi(itr->second.c_str());

    return value;
}
