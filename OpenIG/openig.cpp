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
#include "openig.h"

#include <IgPluginCore/pluginoperation.h>
#include <IgPluginCore/plugincontext.h>

#include <IgCore/mathematics.h>
#include <IgCore/attributes.h>
#include <IgCore/commands.h>
#include <IgCore/configuration.h>
#include <IgCore/animation.h>

#include <osgDB/ReadFile>
#include <osgDB/FileNameUtils>

#include <osgGA/TrackballManipulator>

#include <osg/ValueObject>
#include <osg/Vec3d>
#include <osg/Quat>
#include <osg/Notify>

#include <osgShadow/ShadowedScene>
#include <osgShadow/LightSpacePerspectiveShadowMap>
#include <osgShadow/StandardShadowMap>

#include <sstream>

#include <limits.h>
#if ( __WORDSIZE == 64 )
#define BUILD_64   1
#endif

using namespace openig;
using namespace igcore;
using namespace igplugincore;

static int ReceivesShadowTraversalMask = 0x1;
static int CastsShadowTraversalMask = 0x2;

OpenIG::OpenIG()       
    : _sceneCreatedByOpenIG(false)
    , _updateViewerCameraMainpulator(false)
    , _splashOn(true)
{
}

OpenIG::~OpenIG()
{
    cleanup();
}

std::string OpenIG::version()
{
    return "1.0.1";
}

class InitPluginOperation : public PluginOperation
{
public:
    InitPluginOperation(openig::OpenIG* ig)
        : PluginOperation()
        , _ig(ig)
    {

    }

    virtual void apply(igplugincore::Plugin* plugin)
    {
        if (plugin) plugin->init(_ig->getPluginContext());
    }

protected:
    openig::OpenIG* _ig;
};



class ConfigPluginOperation : public PluginOperation
{
public:
    virtual void apply(igplugincore::Plugin* plugin)
    {
        if (plugin)
        {
            std::string configFileName = plugin->getLibrary()+".xml";
            plugin->config(configFileName);
        }
    }
};

class PreFramePluginOperation : public PluginOperation
{
public:
    PreFramePluginOperation(openig::OpenIG* ig, double dt)
        : _ig(ig)
        , _dt(dt)
    {

    }

    virtual void apply(igplugincore::Plugin* plugin)
    {
        if (plugin)
        {
            plugin->preFrame(_ig->getPluginContext(),_dt);
        }
    }
protected:
    openig::OpenIG* _ig;
    double          _dt;
};

class PostFramePluginOperation : public PluginOperation
{
public:
    PostFramePluginOperation(openig::OpenIG* ig, double dt)
        : _ig(ig)
        , _dt(dt)
    {

    }

    virtual void apply(igplugincore::Plugin* plugin)
    {
        if (plugin)
        {
            plugin->postFrame(_ig->getPluginContext(),_dt);
        }
    }
protected:
    openig::OpenIG* _ig;
    double          _dt;
};

class UpdatePluginOperation : public PluginOperation
{
public:
    UpdatePluginOperation(openig::OpenIG* ig)
        : PluginOperation()
        , _ig(ig)
    {

    }

    virtual void apply(igplugincore::Plugin* plugin)
    {
        if (plugin) plugin->update(_ig->getPluginContext());
    }

protected:
    openig::OpenIG* _ig;
};

class DatabaseReadPluginOperation : public PluginOperation
{
public:
    DatabaseReadPluginOperation(const std::string& fileName, osg::Node* node, const osgDB::Options* options)
        : PluginOperation()
        , _fileName(fileName)
        , _node(node)
        , _options(options)
    {

    }

    virtual void apply(igplugincore::Plugin* plugin)
    {
        if (plugin && _node.valid()) plugin->databaseRead(_fileName,_node,_options);
    }

protected:
    std::string                         _fileName;
    osg::ref_ptr<osg::Node>             _node;
    osg::ref_ptr<const osgDB::Options>  _options;
};

class DatabaseReadInVisitorBeforeTraversePluginOperation : public PluginOperation
{
public:
    DatabaseReadInVisitorBeforeTraversePluginOperation(osg::Node* node, osgDB::Options* options)
        : PluginOperation()
        , _node(node)
        , _options(options)
    {

    }

    virtual void apply(igplugincore::Plugin* plugin)
    {
        if (plugin && _node.valid()) plugin->databaseReadInVisitorBeforeTraverse(*_node,_options.get());
    }

protected:
    osg::ref_ptr<osg::Node>         _node;
    osg::ref_ptr<osgDB::Options>    _options;
};

class DatabaseReadInVisitorAfterTraversePluginOperation : public PluginOperation
{
public:
    DatabaseReadInVisitorAfterTraversePluginOperation(osg::Node* node)
        : PluginOperation()
        , _node(node)
    {

    }

    virtual void apply(igplugincore::Plugin* plugin)
    {
        if (plugin && _node.valid()) plugin->databaseReadInVisitorAfterTraverse(*_node);
    }

protected:
    osg::ref_ptr<osg::Node>         _node;
};

class AddEntityPluginOperation : public PluginOperation
{
public:
    AddEntityPluginOperation(openig::OpenIG* ig, osg::MatrixTransform* entity, unsigned int id, const std::string& fileName)
        : PluginOperation()
        , _entity(entity)
        , _id(id)
        , _fileName(fileName)
        , _ig(ig)
    {

    }

    virtual void apply(igplugincore::Plugin* plugin)
    {
        if (plugin && _entity.valid()) plugin->entityAdded(_ig->getPluginContext(),_id,*_entity,_fileName);
    }

protected:
    osg::ref_ptr<osg::MatrixTransform>  _entity;
    unsigned int                        _id;
    std::string                         _fileName;
    openig::OpenIG*                     _ig;
};

class DatabaseReadNodeVisitor : public osg::NodeVisitor
{
public:
    DatabaseReadNodeVisitor(igcore::ImageGenerator* ig, osgDB::Options* options)
        : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
        , _ig(ig)
        , _options(options)
    {

    }

    virtual void apply(osg::Node& node)
    {
        osg::ref_ptr<DatabaseReadInVisitorBeforeTraversePluginOperation> pobefore(
            new DatabaseReadInVisitorBeforeTraversePluginOperation(&node,_options.get())
        );

        OpenIG* openIG = dynamic_cast<OpenIG*>(_ig);
        if (openIG)
        {
            openIG->applyPluginOperation(pobefore.get());
        }

        traverse(node);

        osg::ref_ptr<DatabaseReadInVisitorAfterTraversePluginOperation> poafter(
            new DatabaseReadInVisitorAfterTraversePluginOperation(&node)
        );

        if (openIG)
        {
            openIG->applyPluginOperation(poafter.get());
        }
    }
protected:
    igcore::ImageGenerator*             _ig;
    osg::ref_ptr<osgDB::Options>        _options;
};


class DatabaseReadCallback : public osgDB::Registry::ReadFileCallback
{
public:
    DatabaseReadCallback(igcore::ImageGenerator* ig)
        : _ig(ig)
    {

    }

    virtual osgDB::ReaderWriter::ReadResult readNode(const std::string& filename, const osgDB::Options* options)
    {
        osgDB::ReaderWriter::ReadResult result = osgDB::Registry::instance()->readNodeImplementation(filename,options);
        if (result.getNode())
        {
            osg::ref_ptr<DatabaseReadPluginOperation> po(
                new DatabaseReadPluginOperation(filename,result.getNode(),options)
            );

            OpenIG* openIG = dynamic_cast<OpenIG*>(_ig);
            if (openIG)
            {
                openIG->applyPluginOperation(po.get());
            }

            DatabaseReadNodeVisitor nv(_ig, const_cast<osgDB::Options*>(options));
            result.getNode()->accept(nv);
        }
        return result;
    }

protected:
    igcore::ImageGenerator* _ig;
};

class PrintLoadedPluginsPluginOperation : public igplugincore::PluginOperation
{
public:
    virtual void apply(igplugincore::Plugin* plugin)
    {
        osg::notify(osg::NOTICE) << "Plugin loaded: " << plugin->getName() << ", " << plugin->getDescription() << std::endl;
    }
};

void OpenIG::init(osgViewer::CompositeViewer* viewer, const std::string& xmlFileName)
{
    const char* env = getenv("OSGNOTIFYLEVEL");
    if (env == 0)
    {
        env = getenv("OSG_NOTIFY_LEVEL");
        if (env == 0)
        {
            osg::setNotifyLevel(osg::FATAL);
        }
    }

	std::string configPath = osgDB::getFilePath(xmlFileName);	
	std::string configFileName = osgDB::getSimpleFileName(xmlFileName);

#if     defined (__linux) || defined (__APPLE__)
	if (configPath.empty()) configPath = std::string("/usr/local/bin/igdata/");	
	else configPath += std::string("/");
#elif   defined (_WIN32)
	if (configPath.empty()) configPath = std::string("igdata/");
	else configPath += std::string("/");
#endif

	Configuration::instance()->readFromXML(configPath + configFileName,"OpenIG-Config");

    initViewer(viewer);
    initTerminal();
    initCommands();
    initPluginContext();
    initOnScreenHelp();
    initSplashScreen();

    osgDB::Registry::instance()->setReadFileCallback( new DatabaseReadCallback(this) );

#if     defined (__linux) 
	#if defined(BUILD_64)
        PluginHost::loadPlugins("/usr/local/lib64/igplugins", configPath + configFileName);
    #else
        PluginHost::loadPlugins("/usr/local/lib/igplugins", configPath + configFileName);
    #endif
#elif	defined (__APPLE__)	
	PluginHost::loadPlugins("/usr/local/lib/igplugins", configPath + configFileName);
#elif   defined (_WIN32)
	PluginHost::loadPlugins("igplugins", configPath + configFileName );
#endif

    osg::ref_ptr<PrintLoadedPluginsPluginOperation> printPluginOperation(new PrintLoadedPluginsPluginOperation);
    PluginHost::applyPluginOperation(printPluginOperation.get());

    osg::ref_ptr<ConfigPluginOperation> configPluginOperation(new ConfigPluginOperation);
    PluginHost::applyPluginOperation(configPluginOperation.get());

    osg::ref_ptr<InitPluginOperation> initPluginOperation(new InitPluginOperation(this));
    PluginHost::applyPluginOperation(initPluginOperation.get());


}

void OpenIG::loadScript(const std::string& fileName)
{
#if 0
    _defaultScriptFileName = fileName;
#else
    igcore::Commands::instance()->loadScript(fileName);
#endif
}

struct LoadDefaultScriptViewerOperation : public osg::Operation
{
    LoadDefaultScriptViewerOperation(const std::string& fileName, openig::OpenIG* ig)
        : _ig(ig)
        , _fileName(fileName)
    {
        setKeep(false);
        setName("LoadDefaultScriptViewerOperation");
    }

    virtual void operator () (osg::Object*)
    {
        Commands::instance()->loadScript(_fileName);
    }

protected:
    openig::OpenIG* _ig;
    std::string     _fileName;
};

class CleanPluginOperation : public igplugincore::PluginOperation
{
public:
	CleanPluginOperation(openig::OpenIG* ig)
		: _ig(ig)
	{
	}
	virtual void apply(igplugincore::Plugin* plugin)
    {
		if (plugin && _ig) plugin->clean(_ig->getPluginContext());
    }

protected:
	openig::OpenIG* _ig;
};

void OpenIG::cleanup()
{
    _entities.clear();
    _entitiesToAdd.clear();
    _entitiesToRemove.clear();
    _lights.clear();

    Commands::instance()->clear();

	osg::ref_ptr<CleanPluginOperation> operation(new CleanPluginOperation(this));
	PluginHost::applyPluginOperation(operation.get());

    PluginHost::unloadPlugins();       

#if defined (__linux) || defined(__APPLE__)
    if (_sceneCreatedByOpenIG)
    {
        _viewer->stopThreading();

        while (_viewer->areThreadsRunning());

        if (_viewer->getView(0))
            _viewer->getView(0)->setSceneData(0);
    }
#endif
}

void OpenIG::frame()
{
    static bool         firstFrame = true;
    static osg::Timer_t firstFrimeTimeTick = 0;

    if (_viewer.valid())
    {
        if (firstFrame)
        {
            _viewer->frame();

            firstFrame = false;

            firstFrimeTimeTick = osg::Timer::instance()->tick();
        }
        else
        {
            _viewer->advance();
            _viewer->eventTraversal();
            _viewer->updateTraversal();

            osg::ref_ptr<UpdatePluginOperation> updatePluginOperation(new UpdatePluginOperation(this));
            PluginHost::applyPluginOperation(updatePluginOperation.get());

            preRender();

            osg::ref_ptr<PreFramePluginOperation> preFramePluginOperation(
                new PreFramePluginOperation(this,_viewer->getFrameStamp()->getSimulationTime())
            );
            PluginHost::applyPluginOperation(preFramePluginOperation.get());

            _viewer->renderingTraversals();

            osg::ref_ptr<PostFramePluginOperation> postFramePluginOperation(
                new PostFramePluginOperation(this,_viewer->getFrameStamp()->getSimulationTime())
            );
            PluginHost::applyPluginOperation(postFramePluginOperation.get());

            postRender();
        }

    }

    osg::Timer_t now = osg::Timer::instance()->tick();
    if (_splashOn && osg::Timer::instance()->delta_s(firstFrimeTimeTick,now) > 5.0)
    {
        _splashOn = false;
        _viewer->getView(0)->getSceneData()->asGroup()->removeChild(_splashCamera);
        _splashCamera = 0;
    }
}

void OpenIG::preRender()
{
    if (_viewer->getNumViews() != 0)
    {
        osg::ref_ptr<osg::Camera> camera = _viewer->getView(0)->getCamera();

        unsigned int    id = 0;
        osg::Matrixd    offset;
        bool            bind = false;

        if ( camera->getUserValue("bindOffset",offset) &&
             camera->getUserValue("bindTo",id) &&
             camera->getUserValue("bindToEntity",bind))
        {
            if (bind)
            {
                EntityMapIterator itr = _entities.find(id);
                if (itr == _entities.end())
                    return;

                osg::NodePath np;
                np.push_back(itr->second);

                osg::ref_ptr<osg::Group> parent = itr->second->getNumParents() ? itr->second->getParent(0) : 0;
                while (parent)
                {
                    np.insert(np.begin(),parent);
                    parent = parent->getNumParents() ? parent->getParent(0) : 0;
                }

                osg::Matrixd wmx = osg::computeLocalToWorld(np);

                osg::Matrixd final = offset * wmx;

                bool fixedUp = false;
                if (camera->getUserValue("fixedUp",fixedUp) && fixedUp)
                {
                    osg::Vec3d  scale = wmx.getScale();
                    osg::Quat   rotation = wmx.getRotate();
                    osg::Vec3d  translate = wmx.getTrans();

                    igcore::Math::instance()->fixVerticalAxis(translate,rotation,false);

                    wmx = osg::Matrixd::scale(scale)*osg::Matrixd::rotate(rotation)*osg::Matrixd::translate(translate);
                    final = offset * wmx;
                }
                setCameraPosition(final);

                if (_updateViewerCameraMainpulator && _viewerCameraManipulator.valid())
                {
                    _viewerCameraManipulator->setByMatrix(final);
                }
            }
        }

    }
}

void OpenIG::postRender()
{
    igplugincore::PluginContext::AttributeMap& attrs = _context.getAttributes();
    attrs.clear();

    if (_defaultScriptFileName.length())
    {
        _viewer->addUpdateOperation(new LoadDefaultScriptViewerOperation(
            _defaultScriptFileName,
            this));
        _defaultScriptFileName.clear();
    }
}


void OpenIG::addEntity(unsigned int id, const std::string& fileName, const osg::Matrixd& mx, const osgDB::Options* options)
{
    osg::ref_ptr<osg::Node> model = osgDB::readNodeFile(fileName,options);

    if (options != 0 && !options->getOptionString().empty())
    {
        osgDB::Registry::instance()->setOptions(const_cast<osgDB::Options*>(options));
    }
    else
    {
        osgDB::Registry::instance()->setOptions(0);
    }

    if (!model.valid())
    {
        osg::notify(osg::NOTICE) << "failed to add entity: " << fileName << std::endl;
        return;
    }

    model->getOrCreateStateSet()->setRenderBinDetails(8, "RenderBin");

    osg::ref_ptr<osg::MatrixTransform> mxt = new osg::MatrixTransform;
    mxt->addChild(model);
    mxt->setMatrix(mx);

    std::ostringstream oss;
    oss << id;
    mxt->setName(oss.str());
    mxt->setUserValue("fileName",fileName);
    mxt->setUserValue("ID",id);

    _entities[id] = mxt;
    _entitiesToAdd[id] = mxt;

    osg::ref_ptr<AddEntityPluginOperation> pluginOperation(new AddEntityPluginOperation(this,mxt,id,fileName));
    this->applyPluginOperation(pluginOperation.get());
}

void OpenIG::reloadEntity(unsigned int id, const std::string& fileName, const osgDB::Options* options)
{
    EntityMapIterator itr = _entities.find(id);
    if (itr == _entities.end())
        return;

    itr->second->setUserValue("fileName",fileName);
    itr->second->getChild(0)->setUserData(const_cast<osgDB::Options*>(options));

    _entitiesToReload[itr->first] = itr->second;
}

void OpenIG::setEntityName(unsigned int id, const std::string& name)
{
    EntityMapIterator itr = _entities.find(id);
    if (itr == _entities.end()) return;

    itr->second->setUserValue("Name",name);
}

std::string OpenIG::getEntityName(unsigned int id)
{
    EntityMapIterator itr = _entities.find(id);
    if (itr == _entities.end()) return "";

    std::string name;
    itr->second->getUserValue("Name",name);

    return name;
}

void OpenIG::removeEntity(unsigned int id)
{
    EntityMapIterator itr = _entities.find(id);
    if (itr == _entities.end())
        return;

    _entitiesToRemove[id] = itr->second;
    _entities.erase(itr);
}

void OpenIG::updateEntity(unsigned int id, const osg::Matrixd& mx)
{
    EntityMapIterator itr = _entities.find(id);
    if (itr == _entities.end())
        return;

    if (itr->second.valid())
    {
        itr->second->setMatrix(mx);
    }
}

void OpenIG::showEntity(unsigned int id, bool show)
{
    EntityMapIterator itr = _entities.find(id);
    if (itr == _entities.end())
        return;

    if (itr->second.valid() && itr->second->getNumChildren() != 0)
    {
        switch (show)
        {
        case true:
            itr->second->getChild(0)->setNodeMask(0xFFFFFFFF);
            break;
        case false:
            itr->second->getChild(0)->setNodeMask(0x0);
            break;
        }
    }
}

void OpenIG::bindToEntity(unsigned int id, unsigned int toEntityId)
{
    EntityMapIterator itr = _entities.find(id);
    if (itr == _entities.end())
        return;

    EntityMapIterator titr = _entities.find(toEntityId);
    if (titr == _entities.end())
        return;

    osg::ref_ptr<osg::MatrixTransform> entity = itr->second;

    const osg::Node::ParentList& pl = itr->second->getParents();
    for (size_t i=0; i<pl.size(); ++i)
    {
        pl.at(i)->removeChild(entity);
    }

    EntityMapIterator eaitr = _entitiesToAdd.find(id);
    if (eaitr != _entitiesToAdd.end())
    {
        _entitiesToAdd.erase(eaitr);
    }

    titr->second->addChild(entity);

}

void OpenIG::unbindFromEntity(unsigned int id)
{
    EntityMapIterator itr = _entities.find(id);
    if (itr == _entities.end())
        return;

    osg::ref_ptr<osg::MatrixTransform> entity = itr->second;

    osg::NodePath np;
    np.push_back(entity);

    osg::ref_ptr<osg::Group> parent = entity->getNumParents() ? entity->getParent(0) : 0;
    while (parent.valid())
    {
        np.insert(np.begin(),parent);
        parent = parent->getNumParents() ? parent->getParent(0) : 0;
    }

    osg::Matrixd wmx = osg::computeLocalToWorld(np);

    const osg::Node::ParentList& pl = itr->second->getParents();
    for (size_t i=0; i<pl.size(); ++i)
    {
        pl.at(i)->removeChild(entity);
    }

    entity->setMatrix(wmx);

    _entitiesToAdd[id] = entity;
}

struct UpdateEntitiesViewerOperation : public osg::Operation
{
    UpdateEntitiesViewerOperation(osgViewer::CompositeViewer* viewer, igcore::ImageGenerator* ig)
        : _viewer(viewer)
        , _ig(ig)
    {
        setKeep(true);
        setName("UpdateEntitiesViewerOperation");
    }

    virtual void operator () (osg::Object*)
    {
        if (_viewer && _ig && _viewer->getNumViews()!=0 && _viewer->getView(0)->getSceneData())
        {
            igcore::ImageGenerator::EntityMap& emr = _ig->getEntitiesToRemoveMap();
            igcore::ImageGenerator::EntityMap& ema = _ig->getEntitesToAddMap();
            igcore::ImageGenerator::EntityMap& emrl = _ig->getEntitiesToReloadMap();

            igcore::ImageGenerator::EntityMapIterator itr = emr.begin();
            for ( ; itr != emr.end(); ++itr )
            {
                _ig->getScene()->asGroup()->removeChild(itr->second);
            }
            emr.clear();

            itr = ema.begin();
            for ( ; itr != ema.end(); ++itr )
            {
                _ig->getScene()->asGroup()->addChild(itr->second);
            }
            ema.clear();


            itr = emrl.begin();
            for ( ; itr != emrl.end(); ++itr )
            {
                std::string fileName;
                itr->second->getUserValue("fileName",fileName);

                osg::ref_ptr<osgDB::Options> options = dynamic_cast<osgDB::Options*>(itr->second->getChild(0)->getUserData());

                osg::ref_ptr<osg::Node> model = osgDB::readNodeFile(fileName,options);

                if (!model.valid())
                {
                    osg::notify(osg::NOTICE) << "failed to reload entity: " << fileName << std::endl;
                    continue;
                }

                model->getOrCreateStateSet()->setRenderBinDetails(8, "RenderBin");

                osg::notify(osg::NOTICE) << "reloading :" << fileName << std::endl;

                itr->second->setUserValue("fileName",fileName);
                itr->second->replaceChild(itr->second->getChild(0),model);

                osg::notify(osg::NOTICE) << "reloading done:" << fileName << std::endl;
            }
            emrl.clear();
        }
    }

    osgViewer::CompositeViewer*     _viewer;
    igcore::ImageGenerator*         _ig;
};

void OpenIG::initScene()
{
    if (_viewer->getNumViews())
    {
        if (!_scene.valid())
        {
            osg::ref_ptr<osg::Group> root = new osg::Group;

            _sunOrMoonLight = new osg::LightSource;
            _sunOrMoonLight->getLight()->setLightNum(0);
            _sunOrMoonLight->setName("SunOrMoon");
            _sunOrMoonLight->setCullingActive(false);
            _viewer->getView(0)->setLight(_sunOrMoonLight->getLight());
            root->addChild(_sunOrMoonLight.get());

            osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene = new osgShadow::ShadowedScene;
            osg::ref_ptr<osgShadow::MinimalShadowMap> msm = new osgShadow::LightSpacePerspectiveShadowMapDB;
            shadowedScene->setShadowTechnique( msm.get() );

            shadowedScene->setReceivesShadowTraversalMask(ReceivesShadowTraversalMask);
            shadowedScene->setCastsShadowTraversalMask(CastsShadowTraversalMask);

            float minLightMargin        = Configuration::instance()->getConfig("ShadowedScene-minLightMargin",10.0);
            float maxFarPlane           = Configuration::instance()->getConfig("ShadowedScene-maxFarPlane",1000.0);
            unsigned int texSize        = Configuration::instance()->getConfig("ShadowedScene-texSize",4096);
            unsigned int baseTexUnit    = Configuration::instance()->getConfig("ShadowedScene-baseTexUnit",0);
            unsigned int shadowTexUnit  = Configuration::instance()->getConfig("ShadowedScene-shadowTexUnit",1);

            msm->setMinLightMargin( minLightMargin );
            msm->setMaxFarPlane( maxFarPlane );
            msm->setTextureSize( osg::Vec2s( texSize, texSize ) );
            msm->setShadowTextureCoordIndex( shadowTexUnit );
            msm->setShadowTextureUnit( shadowTexUnit );
            msm->setBaseTextureCoordIndex( baseTexUnit );
            msm->setBaseTextureUnit( baseTexUnit );
            msm->setLight(_sunOrMoonLight->getLight());

            _fog = new osg::Fog;
            _fog->setDensity(0);
            shadowedScene->getOrCreateStateSet()->setAttributeAndModes(_fog.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

            osg::Shader* mainFragmentShader = new osg::Shader( osg::Shader::FRAGMENT,
                " // following expressions are auto modified - do not change them:       \n"
                " // gl_TexCoord[0]  0 - can be subsituted with other index              \n"
                "                                                                        \n"
                "float DynamicShadow( );                                                 \n"
                "                                                                        \n"
                "uniform sampler2D baseTexture;                                          \n"
                "                                                                        \n"
                "void computeFogColor(inout vec4 color)                                  \n"
                "{                                                                       \n"
                "    if (gl_FragCoord.w > 0.0)                                           \n"
                "    {                                                                   \n"
                "        const float LOG2 = 1.442695;									 \n"
                "        float z = gl_FragCoord.z / gl_FragCoord.w;                      \n"
                "        float fogFactor = exp2( -gl_Fog.density *						 \n"
                "            gl_Fog.density *											 \n"
                "            z *														 \n"
                "            z *														 \n"
                "            LOG2 );													 \n"
                "        fogFactor = clamp(fogFactor, 0.0, 1.0);                         \n"
                "                                                                        \n"
                "        vec4 clr = color;                                               \n"
                "        color = mix(gl_Fog.color, color, fogFactor );                   \n"
                "        color.a = clr.a;                                                \n"
                "    }                                                                   \n"
                "}	                                                                     \n"
                "void main(void)                                                         \n"
                "{                                                                       \n"
                "  vec4 colorAmbientEmissive = gl_FrontLightModelProduct.sceneColor;     \n"
                "  vec4 color = texture2D( baseTexture, gl_TexCoord[0].xy );             \n"
                "  color *= mix( colorAmbientEmissive, gl_Color, DynamicShadow() );      \n"
                "  computeFogColor(color.rgba);                                          \n"
                "  gl_FragColor = color;                                                 \n"
                "} \n" );

            msm->setMainFragmentShader(mainFragmentShader);

            root->addChild(_scene = shadowedScene);
            root->addChild(_lightsGroup = new osg::Group);

            _viewer->getView(0)->setSceneData(root);

            _sceneCreatedByOpenIG = true;
        }
    }
}

void OpenIG::initViewer(osgViewer::CompositeViewer *viewer)
{
    _viewer = viewer;

    if (_viewer->getNumViews() != 0)
    {
        if (!_viewer->getView(0)->getSceneData())
        {
           initScene();
        }
        _viewer->addUpdateOperation(new UpdateEntitiesViewerOperation(_viewer,this));
    }

}

void OpenIG::initPluginContext()
{
    _context.setImageGenerator(this);
}

void OpenIG::setCameraPosition(const osg::Matrixd& mx, bool viewMatrix)
{
    if (_viewer->getNumViews()==0) return;

    osg::ref_ptr<osgGA::GUIEventAdapter> ea = new osgGA::GUIEventAdapter;
    osg::ref_ptr<DymmuGUIActionAdapter> aa = new DymmuGUIActionAdapter;


    switch (viewMatrix)
    {
    case true:
         _viewer->getView(0)->getCamera()->setViewMatrix(mx);

         if (_updateViewerCameraMainpulator && _viewer->getView(0)->getCameraManipulator())
         {
             _viewer->getView(0)->getCameraManipulator()->setByInverseMatrix(mx);
             _viewer->getView(0)->getCameraManipulator()->init(*ea,*aa);
         }
        break;
    case false:
         _viewer->getView(0)->getCamera()->setViewMatrix(osg::Matrixd::inverse(mx));

         if (_updateViewerCameraMainpulator && _viewer->getView(0)->getCameraManipulator())
         {
             _viewer->getView(0)->getCameraManipulator()->setByMatrix(mx);
             _viewer->getView(0)->getCameraManipulator()->init(*ea,*aa);
         }
        break;
    }


}

void OpenIG::bindCameraToEntity(unsigned int id, const osg::Matrixd& mx)
{
    EntityMapIterator itr = _entities.find(id);
    if (itr == _entities.end())
        return;

    if (itr->second.valid() && _viewer.valid() && _viewer->getNumViews() != 0)
    {
        osg::ref_ptr<osg::Camera> camera = _viewer->getView(0)->getCamera();
        camera->setUserValue("bindOffset",mx);
        camera->setUserValue("bindTo",id);
        camera->setUserValue("bindToEntity",(bool)true);

        if (_updateViewerCameraMainpulator && _viewer->getView(0)->getCameraManipulator())
        {
            _viewerCameraManipulator = _viewer->getView(0)->getCameraManipulator();
            _viewer->getView(0)->setCameraManipulator(0,false);
        }
    }
}

void OpenIG::bindCameraSetFixedUp(bool fixedUp)
{
    if (_viewer.valid() && _viewer->getNumViews() != 0)
    {
        osg::ref_ptr<osg::Camera> camera = _viewer->getView(0)->getCamera();
        camera->setUserValue("fixedUp",(bool)fixedUp);
    }
}

void OpenIG::bindCameraUpdate(const osg::Matrixd& mx)
{
    if (_viewer.valid() && _viewer->getNumViews() != 0)
    {
        osg::ref_ptr<osg::Camera> camera = _viewer->getView(0)->getCamera();
        camera->setUserValue("bindOffset",mx);
    }
}

bool OpenIG::isCameraBoundToEntity()
{
    bool bound = false;
    if (_viewer.valid() && _viewer->getNumViews() != 0)
    {
        osg::ref_ptr<osg::Camera> camera = _viewer->getView(0)->getCamera();
        camera->getUserValue("bindToEntity",bound);
    }

    return bound;
}

void OpenIG::unbindCameraFromEntity()
{
    if (_viewer.valid() && _viewer->getNumViews() != 0)
    {
        osg::ref_ptr<osg::Camera> camera = _viewer->getView(0)->getCamera();

        camera->setUserValue("bindToEntity",(bool)false);

        if (_updateViewerCameraMainpulator && _viewerCameraManipulator.valid())
        {
            osg::ref_ptr<osgGA::GUIEventAdapter> ea = new osgGA::GUIEventAdapter;
            osg::ref_ptr<DymmuGUIActionAdapter> aa = new DymmuGUIActionAdapter;

            _viewer->getView(0)->setCameraManipulator(_viewerCameraManipulator,false);
            _viewerCameraManipulator->setByInverseMatrix(_viewer->getView(0)->getCamera()->getViewMatrix());
            _viewerCameraManipulator->setNode(0);
            _viewerCameraManipulator->init(*ea,*aa);

            _viewerCameraManipulator = 0;
        }

    }
}

void OpenIG::setFog(double visibility)
{
    if (_fog.valid())
    {
        _fog->setDensity(1.0/visibility);
        _fog->setColor(osg::Vec4(0.9,0.9,0.9,1.0));
    }

    igcore::FogAttributes fog(visibility);
    _context.addAttribute("Fog", new PluginContext::Attribute<igcore::FogAttributes>(fog));
}

void OpenIG::setWind(float speed, float direction)
{
    igcore::WindAttributes attr(speed,direction);
    _context.addAttribute("Wind", new PluginContext::Attribute<igcore::WindAttributes>(attr));
}

void OpenIG::setTimeOfDay(unsigned int hour, unsigned int minutes)
{
    if (hour < 24)
    {
        igcore::TimeOfDayAttributes tod(hour,minutes);
        _context.addAttribute("TOD", new PluginContext::Attribute<igcore::TimeOfDayAttributes>(tod));
    }
}

void OpenIG::setRain(double factor)
{
    igcore::RainSnowAttributes rain(factor);
    _context.addAttribute("Rain", new PluginContext::Attribute<igcore::RainSnowAttributes>(rain));
}

void OpenIG::setSnow(double factor)
{
    igcore::RainSnowAttributes snow(factor);
    _context.addAttribute("Snow", new PluginContext::Attribute<igcore::RainSnowAttributes>(snow));
}

void OpenIG::addCloudLayer(unsigned int id, int type, double altitude, double thickness, double density)
{
    igcore::CLoudLayerAttributes attr;
    attr.setId(id);
    attr.setType(type);
    attr.setAltitude(altitude);
    attr.setDensity(density);
    attr.setFlags(true);
    attr.setIsDirty(true);
    attr.setThickness(thickness);

    _context.addAttribute("CloudLayer", new PluginContext::Attribute<igcore::CLoudLayerAttributes>(attr));
}

void OpenIG::removeCloudLayer(unsigned int id)
{
    igcore::CLoudLayerAttributes attr;
    attr.setId(id);
    attr.setFlags(false,true);
    attr.setIsDirty(true);

    _context.addAttribute("CloudLayer", new PluginContext::Attribute<igcore::CLoudLayerAttributes>(attr));
}

void OpenIG::removeAllCloudlayers()
{
    _context.addAttribute("RemoveAllCloudLayers", new osg::Referenced);
}

void OpenIG::updateCloudLayer(unsigned int id, double altitude, double thickness, double density)
{
    igcore::CLoudLayerAttributes attr;
    attr.setId(id);
    attr.setAltitude(altitude);
    attr.setDensity(density);
    attr.setThickness(thickness);
    attr.setIsDirty(true);

    _context.addAttribute("CloudLayer", new PluginContext::Attribute<igcore::CLoudLayerAttributes>(attr));
}

void OpenIG::resetAnimation(unsigned int entityId, const std::string& animationName)
{
    igcore::AnimationAttributes attr;
    attr._entityId = entityId;
    attr._animationName = animationName;
    attr._playback = false;
    attr._reset = true;

    _context.addAttribute("Animation", new PluginContext::Attribute<igcore::AnimationAttributes>(attr));
}

void OpenIG::resetAnimation(unsigned int entityId, const StringUtils::StringList& animations)
{
    StringUtils::StringListConstIterator itr = animations.begin();
    for ( ; itr != animations.end(); ++itr )
    {
        stopAnimation(entityId,*itr);
    }
}

void OpenIG::stopAnimation(unsigned int entityId, const std::string& animationName)
{
    igcore::AnimationAttributes attr;
    attr._entityId = entityId;
    attr._animationName = animationName;
    attr._playback = false;

    _context.addAttribute("Animation", new PluginContext::Attribute<igcore::AnimationAttributes>(attr));
}

void OpenIG::stopAnimation(unsigned int entityId, const StringUtils::StringList& animations)
{
    StringUtils::StringListConstIterator itr = animations.begin();
    for ( ; itr != animations.end(); ++itr )
    {
        stopAnimation(entityId,*itr);
    }
}

void OpenIG::playAnimation(unsigned int entityId, const std::string& animationName)
{
    igcore::AnimationAttributes attr;
    attr._entityId = entityId;
    attr._animationName = animationName;

    _context.addAttribute("Animation", new PluginContext::Attribute<igcore::AnimationAttributes>(attr));

}

void OpenIG::playAnimation(unsigned int entityId, const StringUtils::StringList& animations)
{
    StringUtils::StringListConstIterator itr = animations.begin();
    for ( ; itr != animations.end(); ++itr )
    {
        playAnimation(entityId,*itr);
    }
}

void OpenIG::playAnimation(unsigned int entityId, const std::string& animationName, RefAnimationSequenceCallbacks* cbs )
{
    igcore::AnimationAttributes attr;
    attr._entityId = entityId;
    attr._animationName = animationName;
    attr._sequenceCallbacks = cbs;

    _context.addAttribute("Animation", new PluginContext::Attribute<igcore::AnimationAttributes>(attr));
}

void OpenIG::setUpdateViewerCameraManipulator(bool update)
{
    _updateViewerCameraMainpulator = update;
}

igcore::LightImplementationCallback* OpenIG::getLightImplementationCallback()
{
    return _lightImplementationCallback.get();
}

void OpenIG::setLightImplementationCallback( igcore::LightImplementationCallback* cb)
{
    _lightImplementationCallback = cb;
}

osgViewer::CompositeViewer* OpenIG::getViewer()
{
    return _viewer;
}

osg::Node* OpenIG::getScene()
{
    return _scene.get();
}

igcore::ImageGenerator::EntityMap& OpenIG::getEntityMap()
{
    return _entities;
}

igcore::ImageGenerator::EntityMap& OpenIG::getEntitesToAddMap()
{
    return _entitiesToAdd;
}

igcore::ImageGenerator::EntityMap& OpenIG::getEntitiesToRemoveMap()
{
    return _entitiesToRemove;
}

igcore::ImageGenerator::EntityMap& OpenIG::getEntitiesToReloadMap()
{
    return _entitiesToReload;
}

osg::LightSource* OpenIG::getSunOrMoonLight()
{
    return _sunOrMoonLight.get();
}

osg::Fog* OpenIG::getFog()
{
    return _fog.get();
}

igplugincore::PluginContext& OpenIG::getPluginContext()
{
    return _context;
}

OpenIG::LightsMap& OpenIG::getLightsMap()
{
    return _lights;
}

