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
#include "forwardpluscullvisitor.h"

using namespace OpenIG::Plugins;

ForwardPlusCullVisitor::ForwardPlusCullVisitor()
	: osgUtil::CullVisitor()
	, _searchedProgram(false)
{

}

void ForwardPlusCullVisitor::addShadersToLinkWithMain(osg::ref_ptr<osg::Shader> shader)
{
	if (shader.valid()==false)
	{
		return;
	}
	_shadersToLink.push_back(shader);
}

void ForwardPlusCullVisitor::findProgramIfNotFound(void)
{
	osg::ref_ptr<osgUtil::StateGraph> sg = getCurrentStateGraph();
	if (sg.valid()==false)
	{
		return;
	}

	const osg::StateSet* ss = sg->getStateSet();
	if (ss==0)
	{
		return;
	}
	const osg::StateAttribute* attr = ss->getAttribute(osg::StateAttribute::PROGRAM);

	if (attr==0)
	{
		return;
	}

	const osg::Program* program = dynamic_cast<const osg::Program*>(attr);
	if (program==0)
	{
		return;
	}
	osg::Program* nonConstProgram = const_cast<osg::Program*>(program);
	_program = nonConstProgram;

	// Hack to mark successful processing
	if (program->getNumShaders()!=4)
	{
		return;
	}

	for(VectorShaders::const_iterator it = _shadersToLink.begin(); it != _shadersToLink.end(); ++it)
	{
		osg::Shader* shader = (*it).get();
		if (shader==0)
		{
			continue;
		}
		_program->addShader(shader);
	}

	_program->dirtyProgram();

	_searchedProgram = true;
}

void ForwardPlusCullVisitor::apply(osg::Group& node)
{
	//findProgramIfNotFound();
	osgUtil::CullVisitor::apply(node);
}