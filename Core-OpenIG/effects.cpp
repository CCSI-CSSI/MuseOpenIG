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
#include "openig.h"

#include <Core-Base/stringutils.h>

#include <iostream>

#include <osg/ValueObject>

using namespace OpenIG;
using namespace OpenIG::Base;

void Engine::initEffects()
{
	if (_effectsRoot.valid()) return;

	_effectsRoot = new osg::Group;
	if (getScene() && getScene()->asGroup()) getScene()->asGroup()->addChild(_effectsRoot);
}

void Engine::addEffect(unsigned int id, const std::string& name, const osg::Matrixd& mx, const std::string& attributes)
{
	if (!_effectsImplementationCallback.valid()) return;

	removeEffect(id);

	osg::ref_ptr<GenericAttribute> attr(new GenericAttribute);
	attr->setUserValue("name", name);

	StringUtils::Tokens tokens = StringUtils::instance()->tokenize(attributes, ";");
	StringUtils::Tokens::iterator itr = tokens.begin();
	for (; itr != tokens.end(); ++itr)
	{
		std::string attribute = *itr;

		StringUtils::Tokens t = StringUtils::instance()->tokenize(attribute, "=");
		if (t.size() != 2) continue;

		std::string attributeName = t.at(0);
		std::string attributeValue = t.at(1);

		std::string::value_type ch = attributeValue.at(0);
		attributeValue.erase(attributeValue.begin());

		switch (ch)
		{
		case 'F':
			attr->setUserValue(attributeName, (float)atof(attributeValue.c_str()));
			break;
		case 'S':
			attr->setUserValue(attributeName, attributeValue);
			break;
		case 'I':
			attr->setUserValue(attributeName, (int)atoi(attributeValue.c_str()));
			break;
		}
	}

	osg::ref_ptr<osg::Node> effectImplementation = _effectsImplementationCallback->create(id, name, attr);
	if (!effectImplementation.valid()) return;
	
	Effect effect = new osg::MatrixTransform;
	effect->setMatrix(mx);
	effect->addChild(effectImplementation);

	_effects[id] = effect;

	_effectsRoot->addChild(effect);
}

void Engine::removeEffect(unsigned int id)
{
	EffectMap::iterator itr = _effects.find(id);
	if (itr == _effects.end()) return;

	_effectsRoot->removeChild(itr->second);
	_effects.erase(itr);

	if (_effectsImplementationCallback.valid())
	{
		_effectsImplementationCallback->destroy(id);
	}
}

void Engine::bindEffect(unsigned int id, unsigned int entityID, const osg::Matrixd& mx)
{
	if (_entities.count(entityID) == 0) return;
	if (_effects.count(id) == 0) return;

	Entity entity = _entities[entityID];
	Effect effect = _effects[id];

	if (!entity.valid() || !effect.valid()) return;

	_effectsRoot->removeChild(effect);
	
	entity->addChild(effect);

	effect->setMatrix(mx);
	effect->setUserValue("boundTo", entityID);
}

void Engine::unbindEffect(unsigned int id)
{
	if (_effects.count(id) == 0) return;
	Effect effect = _effects[id];
	if (!effect.valid()) return;

	unsigned int entityID = 0;
	if (!effect->getUserValue("boundTo", entityID)) return;
	if (entityID == 0) return;

	if (_entities.count(entityID) == 0) return;
	Entity entity = _entities[entityID];
	if (!entity.valid()) return;

	effect->setUserValue("boundTo", 0);

	osg::NodePath np;
	np.push_back(effect);

	osg::ref_ptr<osg::Group> parent = effect->getNumParents() ? effect->getParent(0) : 0;
	while (parent.valid())
	{
		np.insert(np.begin(), parent);
		parent = parent->getNumParents() ? parent->getParent(0) : 0;
	}

	osg::Matrixd wmx = osg::computeLocalToWorld(np);

	const osg::Node::ParentList& pl = effect->getParents();
	for (size_t i = 0; i<pl.size(); ++i)
	{
		pl.at(i)->removeChild(effect);
	}

	effect->setMatrix(wmx);

	_effectsRoot->addChild(effect);
}

void Engine::updateEffect(unsigned int id, const osg::Matrixd& mx)
{
	EffectMap::iterator itr = _effects.find(id);
	if (itr == _effects.end()) return;

	itr->second->setMatrix(mx);
}

void Engine::setEffectImplementationCallback(GenericImplementationCallback* cb)
{
	_effectsImplementationCallback = cb;
}