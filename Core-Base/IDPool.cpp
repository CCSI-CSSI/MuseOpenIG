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

#include "IDPool.h"

using namespace OpenIG::Base;

IDPool*  IDPool::instance()
{
    static IDPool s_generator;
    return &s_generator;
}

void IDPool::initIdGroup(const std::string& group, unsigned int base, unsigned int size)
{
    IdGroup& idGroup = _groups[group];
    idGroup._base = base;

    if (size != 0)
    {
        IdGroup::IDs& ids = idGroup._ids;
        ids.clear();

        for (size_t i = 0; i < size; ++i)
        {
            ids.push_back(base+i);
        }
    }
}

bool IDPool::getNextId(const std::string& group, unsigned int& id)
{
    IdGroup& idGroup = _groups[group];

    if (idGroup._ids.size())
    {
        id = idGroup._ids.front();
        idGroup._ids.erase(idGroup._ids.begin());

        return true;
    }

    return false;
}

void IDPool::setAvailableIds(const std::string& group, const std::vector<unsigned int>& ids)
{
    IdGroup& idGroup = _groups[group];
    idGroup._ids.insert(idGroup._ids.begin(),ids.begin(),ids.end());
}

IDPool::IDPool()
{

}

IDPool::~IDPool()
{

}
