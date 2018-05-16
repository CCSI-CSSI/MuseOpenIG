//#******************************************************************************
//#*
//#*      Copyright (C) 2015  Compro Computer Services
//#*      http://openig.compro.net
//#*
//#*      Source available at: https://github.com/CCSI-CSSI/MuseOpenIG
//#*i
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
//#*    Please direct any questions or comments to the OpenIG Forums
//#*    Email address: openig@compro.net
//#*
//#*
//#*****************************************************************************
#include "Engine.h"

#include <Core-PluginBase/PluginOperation.h>
#include <Core-PluginBase/PluginContext.h>

#include <Core-Base/Mathematics.h>
#include <Core-Base/Types.h>
#include <Core-Base/Commands.h>
#include <Core-Base/Configuration.h>
#include <Core-Base/Animation.h>
#include <Core-Base/FileSystem.h>

#include <osgDB/ReadFile>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include <osgGA/TrackballManipulator>

#include <osg/Depth>
#include <osg/ValueObject>
#include <osg/Vec3d>
#include <osg/Quat>
#include <osg/Notify>
#include <osg/io_utils>

#include <osgShadow/ShadowedScene>
#include <osgShadow/LightSpacePerspectiveShadowMap>
#include <osgShadow/ViewDependentShadowMap>
#include <osgShadow/StandardShadowMap>

#include <Library-Graphics/OIGMath.h>

#include <sstream>

using namespace OpenIG;
using namespace OpenIG::Base;
using namespace OpenIG::PluginBase;

static int ReceivesShadowTraversalMask = 0x10;
static int CastsShadowTraversalMask = 0x20;

Engine::Engine()
    : _sceneCreatedByOpenIG(false)
    , _updateViewerCameraMainpulator(false)
    , _splashOn(true)
    , _setupMask(Standard)
{
}

Engine::~Engine()
{
    cleanup();
}

std::string Engine::version()
{
    return "2.0.7";
}

class InitPluginOperation : public PluginOperation
{
public:
    InitPluginOperation(OpenIG::Engine* ig)
        : PluginOperation()
        , _ig(ig)
    {

    }

    virtual void apply(OpenIG::PluginBase::Plugin* plugin)
    {
        if (plugin) plugin->init(_ig->getPluginContext());
    }

protected:
    OpenIG::Engine* _ig;
};



class ConfigPluginOperation : public PluginOperation
{
public:
    virtual void apply(OpenIG::PluginBase::Plugin* plugin)
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
    PreFramePluginOperation(OpenIG::Engine* ig, double dt)
        : _ig(ig)
        , _dt(dt)
    {

    }

    virtual void apply(OpenIG::PluginBase::Plugin* plugin)
    {
        if (plugin)
        {
            plugin->preFrame(_ig->getPluginContext(),_dt);
        }
    }
protected:
    OpenIG::Engine* _ig;
    double          _dt;
};

class PostFramePluginOperation : public PluginOperation
{
public:
    PostFramePluginOperation(OpenIG::Engine* ig, double dt)
        : _ig(ig)
        , _dt(dt)
    {

    }

    virtual void apply(OpenIG::PluginBase::Plugin* plugin)
    {
        if (plugin)
        {
            plugin->postFrame(_ig->getPluginContext(),_dt);
        }
    }
protected:
    OpenIG::Engine* _ig;
    double          _dt;
};

class UpdatePluginOperation : public PluginOperation
{
public:
    UpdatePluginOperation(OpenIG::Engine* ig)
        : PluginOperation()
        , _ig(ig)
    {

    }

    virtual void apply(OpenIG::PluginBase::Plugin* plugin)
    {
        if (plugin) plugin->update(_ig->getPluginContext());
    }

protected:
    OpenIG::Engine* _ig;
};

class BeginningOfFramePluginOperation : public PluginOperation
{
public:
    BeginningOfFramePluginOperation(OpenIG::Engine* ig)
        : PluginOperation()
        , _ig(ig)
    {

    }

    virtual void apply(OpenIG::PluginBase::Plugin* plugin)
    {
        if (plugin) plugin->beginningOfFrame(_ig->getPluginContext());
    }

protected:
    OpenIG::Engine* _ig;
};

class EndOfFramePluginOperation : public PluginOperation
{
public:
    EndOfFramePluginOperation(OpenIG::Engine* ig)
        : PluginOperation()
        , _ig(ig)
    {

    }

    virtual void apply(OpenIG::PluginBase::Plugin* plugin)
    {
        if (plugin) plugin->endOfFrame(_ig->getPluginContext());
    }

protected:
    OpenIG::Engine* _ig;
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

    virtual void apply(OpenIG::PluginBase::Plugin* plugin)
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

    virtual void apply(OpenIG::PluginBase::Plugin* plugin)
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

    virtual void apply(OpenIG::PluginBase::Plugin* plugin)
    {
        if (plugin && _node.valid()) plugin->databaseReadInVisitorAfterTraverse(*_node);
    }

protected:
    osg::ref_ptr<osg::Node>         _node;
};

class AddEntityPluginOperation : public PluginOperation
{
public:
    AddEntityPluginOperation(OpenIG::Engine* ig, osg::MatrixTransform* entity, unsigned int id, const std::string& fileName)
        : PluginOperation()
        , _entity(entity)
        , _id(id)
        , _fileName(fileName)
        , _ig(ig)
    {

    }

    virtual void apply(OpenIG::PluginBase::Plugin* plugin)
    {
        if (plugin && _entity.valid()) plugin->entityAdded(_ig->getPluginContext(),_id,*_entity,_fileName);
    }

protected:
    osg::ref_ptr<osg::MatrixTransform>  _entity;
    unsigned int                        _id;
    std::string                         _fileName;
    OpenIG::Engine*                     _ig;
};

class DatabaseReadNodeVisitor : public osg::NodeVisitor
{
public:
    DatabaseReadNodeVisitor(OpenIG::Base::ImageGenerator* ig, osgDB::Options* options)
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

        Engine* openIG = dynamic_cast<Engine*>(_ig);
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
    OpenIG::Base::ImageGenerator*             _ig;
    osg::ref_ptr<osgDB::Options>        _options;
};


class DatabaseReadCallback : public osgDB::Registry::ReadFileCallback
{
public:
    DatabaseReadCallback(Engine* ig)
        : _ig(ig)
    {

    }

    virtual osgDB::ReaderWriter::ReadResult readNode(const std::string& filename, const osgDB::Options* options)
    {
        osgDB::ReaderWriter::ReadResult result;

        if (_ig->getReadNodeImplementationCallback())
        {
            osg::ref_ptr<osg::Node> node = _ig->getReadNodeImplementationCallback()->readNode(filename, options);
            if (node.valid())
            {
                result = osgDB::ReaderWriter::ReadResult(node, osgDB::ReaderWriter::ReadResult::FILE_LOADED);
            }
        }
        else
        {
            if (_ig->getReadFileCallback())
            {
                result = _ig->getReadFileCallback()->readNode(filename, options);
            }
            else
            {
                result = osgDB::Registry::instance()->readNodeImplementation(filename, options);
            }
        }
        if (result.getNode())
        {
            osg::ref_ptr<DatabaseReadPluginOperation> po(
                new DatabaseReadPluginOperation(filename,result.getNode(),options)
            );

            Engine* openIG = dynamic_cast<Engine*>(_ig);
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
    Engine* _ig;
};

class PrintLoadedPluginsPluginOperation : public OpenIG::PluginBase::PluginOperation
{
public:
    virtual void apply(OpenIG::PluginBase::Plugin* plugin)
    {
        osg::notify(osg::NOTICE) << "OpenIG: Plugin loaded: " << plugin->getName() << std::endl;
        osg::notify(osg::NOTICE) << "\tauthor: " << plugin->getAuthor() << std::endl;
        osg::notify(osg::NOTICE) << "\tversion: " << plugin->getVersion() << std::endl;
        osg::notify(osg::NOTICE) << "\tdescription: " << plugin->getDescription() << std::endl;
    }
};
void Engine::setReadNodeImplementationCallback(OpenIG::Base::ImageGenerator::ReadNodeImplementationCallback* cb)
{
    _readFileCallback = cb;
}

OpenIG::Base::ImageGenerator::ReadNodeImplementationCallback* Engine::getReadNodeImplementationCallback()
{
    return _readFileCallback;
}

struct SetFarPlaneUniformCallback : public osg::Camera::DrawCallback
{
    osg::ref_ptr<osg::Uniform>              _uniform;

    SetFarPlaneUniformCallback(osg::Uniform* uniform)
    {
        _uniform = uniform;
    }

    void operator () (osg::RenderInfo& renderInfo) const
    {
        const osg::Matrix& proj = renderInfo.getCurrentCamera()->getProjectionMatrix();

        double vfov, ar, n, f;
        proj.getPerspective(vfov, ar, n, f);
        double fc = (double)(2.0 / OpenIG::Library::Graphics::Math::Log2(f + 1.0));
        _uniform->set((float)fc);

       // std::cout << "(SetFarPlaneUniformCallback) Fcoef = " << fc << std::endl;
    }
};

void Engine::setReadFileCallback(osgDB::Registry::ReadFileCallback* cb)
{
    _userReadFileCallback = cb;
}


osgDB::Registry::ReadFileCallback* Engine::getReadFileCallback()
{
    return _userReadFileCallback.get();
}

void Engine::init(osgViewer::CompositeViewer* viewer, const std::string& xmlFileName, const ViewIdentifiers& ids)
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

    if (configPath.empty()) configPath = std::string("igdata/");
    else configPath += std::string("/");

    Configuration::instance()->readFromXML(configPath + configFileName,"OpenIG-Config");

    initScene();
    initViewer(viewer,ids);
    if (_setupMask & WithTerminal) initTerminal();
    initCommands();
    initPluginContext();
    if (_setupMask & WithOnscreenHelp) initOnScreenHelp();
    if (_setupMask & WithSplashScreen) initSplashScreen();
    if (_setupMask & WithScreenMessages) initOnScreenMessages();
    initEffects();

    osgDB::Registry::instance()->setReadFileCallback( new DatabaseReadCallback(this) );

    std::string pluginsPath = FileSystem::path(FileSystem::Plugins);
    osg::notify(osg::NOTICE) << "OpenIG: Loading Plugins from path: " << pluginsPath << std::endl;
    osg::notify(osg::NOTICE) << "OpenIG: Loading Plugins config: " << configPath + configFileName << std::endl;

    PluginHost::loadPlugins(pluginsPath, configPath + configFileName);

    osg::ref_ptr<PrintLoadedPluginsPluginOperation> printPluginOperation(new PrintLoadedPluginsPluginOperation);
    PluginHost::applyPluginOperation(printPluginOperation.get());

    osg::ref_ptr<ConfigPluginOperation> configPluginOperation(new ConfigPluginOperation);
    PluginHost::applyPluginOperation(configPluginOperation.get());

    osg::ref_ptr<InitPluginOperation> initPluginOperation(new InitPluginOperation(this));
    PluginHost::applyPluginOperation(initPluginOperation.get());

    createSunMoonLight();

}

void Engine::loadScript(const std::string& fileName)
{
    OpenIG::Base::Commands::instance()->loadScript(fileName);
}

class CleanPluginOperation : public OpenIG::PluginBase::PluginOperation
{
public:
    CleanPluginOperation(OpenIG::Engine* ig)
        : _ig(ig)
    {
    }
    virtual void apply(OpenIG::PluginBase::Plugin* plugin)
    {
        if (plugin && _ig) plugin->clean(_ig->getPluginContext());
    }

protected:
    OpenIG::Engine* _ig;
};

void Engine::cleanup()
{
    if (_viewer.valid())
    {
        _viewer->stopThreading();
        while (_viewer->areThreadsRunning());

        osgViewer::ViewerBase::Views views;
        _viewer->getViews(views);

        osgViewer::ViewerBase::Views::iterator itr = views.begin();
        for (; itr != views.end(); ++itr)
        {
            osgViewer::View* view = *itr;

            bool openIGScene = false;
            if (view->getUserValue("OpenIG-Scene", openIGScene) && openIGScene)
            {
                view->setSceneData(0);
            }
        }
    }

    _context.setValueObject(0);
    _context.getAttributes().clear();
    _entities.clear();
    _lights.clear();
    _effects.clear();
    _lightAttributes.clear();
    _entityCache.clear();

    _sunOrMoonLight					= NULL;
    _fog							= NULL;
    _scene							= NULL;
    _lightImplementationCallback	= NULL;
    _lightsGroup					= NULL;
    _keypad							= NULL;
    _keypadCameraManipulator		= NULL;
    _viewerCameraManipulator		= NULL;
    _splashCamera					= NULL;
    _effectsRoot					= NULL;
    _effectsImplementationCallback	= NULL;
    _readFileCallback				= NULL;
    _userReadFileCallback			= NULL;

    osgDB::Registry::instance()->setReadFileCallback(0);

    Commands::instance()->clear();

    osg::ref_ptr<CleanPluginOperation> operation(new CleanPluginOperation(this));
    PluginHost::applyPluginOperation(operation.get());

    PluginHost::unloadPlugins();

    if (_viewer.valid())
    {
        _viewer->startThreading();
        _viewer = NULL;
    }
}

void Engine::frame(bool usePlugins)
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
            if (usePlugins)
            {
                osg::ref_ptr<BeginningOfFramePluginOperation> pluginOperation(new BeginningOfFramePluginOperation(this));
                PluginHost::applyPluginOperation(pluginOperation.get());
            }

            _viewer->advance();
            _viewer->eventTraversal();
            _viewer->updateTraversal();

            if (usePlugins)
            {
                osg::ref_ptr<UpdatePluginOperation> updatePluginOperation(new UpdatePluginOperation(this));
                PluginHost::applyPluginOperation(updatePluginOperation.get());
            }

            preRender();

            if (usePlugins)
            {
                osg::ref_ptr<PreFramePluginOperation> preFramePluginOperation(
                    new PreFramePluginOperation(this, _viewer->getFrameStamp()->getSimulationTime())
                    );
                PluginHost::applyPluginOperation(preFramePluginOperation.get());
            }

            _viewer->renderingTraversals();

            if (usePlugins)
            {
                osg::ref_ptr<PostFramePluginOperation> postFramePluginOperation(
                    new PostFramePluginOperation(this, _viewer->getFrameStamp()->getSimulationTime())
                    );
                PluginHost::applyPluginOperation(postFramePluginOperation.get());

                osg::ref_ptr<EndOfFramePluginOperation> endOfFramePluginOperation(
                    new EndOfFramePluginOperation(this)
                    );
                PluginHost::applyPluginOperation(endOfFramePluginOperation.get());
            }

            postRender();
        }

    }

    osg::Timer_t now = osg::Timer::instance()->tick();
    if (_splashOn && osg::Timer::instance()->delta_s(firstFrimeTimeTick,now) > 5.0)
    {
        _splashOn = false;
        for (size_t i = 0; i < _viewer->getNumViews(); ++i)
        {
            if (_viewer->getView(i)->getSceneData() && _viewer->getView(i)->getSceneData()->asGroup())
            {
                _viewer->getView(i)->getSceneData()->asGroup()->removeChild(_splashCamera);
            }
        }
        _splashCamera = 0;
    }
}

void Engine::preRender()
{
    if (_viewer->getNumViews() != 0)
    {
        for (unsigned int cameraID = 0; cameraID < _viewer->getNumViews(); ++cameraID)
        {
            osg::ref_ptr<osg::Camera> camera = _viewer->getView(cameraID)->getCamera();

            unsigned int    id = 0;
            osg::Matrixd    offset;
            bool            bind = false;

            if (camera->getUserValue("bindOffset", offset) &&
                camera->getUserValue("bindTo", id) &&
                camera->getUserValue("bindToEntity", bind))
            {
                if (bind)
                {
                    EntityMapIterator itr = _entities.find(id);
                    if (itr == _entities.end())
                        return;

                    bool freezeOrientation = false;
                    camera->getUserValue("freeze", freezeOrientation);

                    osg::NodePath np;

                    osg::ref_ptr<osg::MatrixTransform> e = new osg::MatrixTransform;
                    if (freezeOrientation)
                    {
                        e->setMatrix(osg::Matrixd::translate(itr->second->getMatrix().getTrans()));
                        np.push_back(e);
                    }
                    else
                    {
                        np.push_back(itr->second);
                    }

                    osg::ref_ptr<osg::Group> parent = itr->second->getNumParents() ? itr->second->getParent(0) : 0;
                    while (parent)
                    {
                        np.insert(np.begin(), parent);
                        parent = parent->getNumParents() ? parent->getParent(0) : 0;
                    }

                    osg::Matrixd wmx = osg::computeLocalToWorld(np);

                    osg::Matrixd final = offset * wmx;

                    bool fixedUp = false;
                    if (camera->getUserValue("fixedUp", fixedUp) && fixedUp)
                    {
                        osg::Vec3d  scale = wmx.getScale();
                        osg::Quat   rotation = wmx.getRotate();
                        osg::Vec3d  translate = wmx.getTrans();

                        OpenIG::Base::Math::instance()->fixVerticalAxis(translate, rotation, false);

                        wmx = osg::Matrixd::scale(scale)*osg::Matrixd::rotate(rotation)*osg::Matrixd::translate(translate);
                        final = offset * wmx;
                    }

                    setCameraPosition(final,false,cameraID);

                }
            }

            osg::ref_ptr<osg::UserDataContainer> container = camera->getUserDataContainer();
            if (!container.valid()) continue;

            for (size_t i = 0; i < container->getNumUserObjects(); ++i)
            {
                osg::ref_ptr<osg::MatrixTransform> entity = dynamic_cast<osg::MatrixTransform*>(container->getUserObject(i));
                if (!entity.valid()) continue;

                osg::Matrixd mx;
                entity->getUserValue("BindToCameraOffset", mx);

                osg::Matrixd wmx = mx * camera->getInverseViewMatrix();

                entity->setMatrix(wmx);
            }
        }
    }
}

void Engine::postRender()
{
    OpenIG::PluginBase::PluginContext::AttributeMap& attrs = _context.getAttributes();
    attrs.clear();
}


void Engine::addEntity(unsigned int id, const std::string& fileName, const osg::Matrixd& mx, const osgDB::Options* options)
{
    osgDB::getDataFilePathList().push_back(osgDB::getFilePath(fileName));

    if (options != 0 && !options->getOptionString().empty())
    {
        osgDB::Registry::instance()->setOptions(const_cast<osgDB::Options*>(options));
    }
    else
    {
        osgDB::Registry::instance()->setOptions(0);
    }

    osg::ref_ptr<osg::Node> model;

	std::string simpleFileName = fileName;// osgDB::getSimpleFileName(fileName);
    if (isFileCached(simpleFileName))
    {
        EntityCache::iterator itr = _entityCache.find(simpleFileName);
        if (itr != _entityCache.end())
        {
            model = itr->second;
            osg::notify(osg::NOTICE) << "OpenIG: Model " << simpleFileName << " added from the cache." << std::endl;
        }
        else
        {
            model = osgDB::readNodeFile(fileName, options);
            if (model.valid())
            {
                _entityCache[simpleFileName] = model;
                osg::notify(osg::NOTICE) << "OpenIG: Model " << simpleFileName << " added to the cache." << std::endl;
            }
        }
    }
    else
    {
        model = osgDB::readNodeFile(fileName, options);
    }

    if (!model.valid())
    {
        osg::notify(osg::NOTICE) << "OpenIG: failed to add entity: " << fileName << std::endl;
        return;
    }

    osg::ref_ptr<osg::MatrixTransform> mxt = new osg::MatrixTransform;
    mxt->addChild(model);
    mxt->setMatrix(mx);

    std::ostringstream oss;
    oss << id;
    mxt->setName(oss.str());
    mxt->setUserValue("fileName",fileName);
    mxt->setUserValue("ID",id);

    _entities[id] = mxt;
    getScene()->asGroup()->addChild(mxt);

    osg::ref_ptr<AddEntityPluginOperation> pluginOperation(new AddEntityPluginOperation(this,mxt,id,fileName));
    this->applyPluginOperation(pluginOperation.get());
}

void Engine::addEntity(unsigned int id, const osg::Node* node, const osg::Matrixd& mx, const osgDB::Options* options)
{
    osg::ref_ptr<osg::Node> model = const_cast<osg::Node*>(node);

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
        osg::notify(osg::NOTICE) << "OpenIG: failed to add entity from Node" << std::endl;
        return;
    }

    osg::ref_ptr<osg::MatrixTransform> mxt = new osg::MatrixTransform;
    mxt->addChild(model);
    mxt->setMatrix(mx);

    std::ostringstream oss;
    oss << id;
    mxt->setName(oss.str());
    mxt->setUserValue("ID", id);

    _entities[id] = mxt;
    getScene()->asGroup()->addChild(mxt);

    osg::ref_ptr<AddEntityPluginOperation> pluginOperation(new AddEntityPluginOperation(this, mxt, id, "fromNode"));
    this->applyPluginOperation(pluginOperation.get());
}

void Engine::reloadEntity(unsigned int id, const std::string& fileName, const osgDB::Options* options)
{
    EntityMapIterator itr = _entities.find(id);
    if (itr == _entities.end())
        return;

    itr->second->setUserValue("fileName",fileName);
    itr->second->getChild(0)->setUserData(const_cast<osgDB::Options*>(options));

    osg::ref_ptr<osg::Node> model = osgDB::readNodeFile(fileName, options);

    if (!model.valid())
    {
        osg::notify(osg::NOTICE) << "OpenIG: failed to reload entity: " << fileName << std::endl;
        return;
    }

    osg::notify(osg::NOTICE) << "OpenIG: reloading :" << fileName << std::endl;

    itr->second->setUserValue("fileName", fileName);
    itr->second->replaceChild(itr->second->getChild(0), model);

    osg::notify(osg::NOTICE) << "OpenIG: reloading done:" << fileName << std::endl;
}

void Engine::setEntityName(unsigned int id, const std::string& name)
{
    EntityMapIterator itr = _entities.find(id);
    if (itr == _entities.end()) return;

    itr->second->setUserValue("Name",name);
}

std::string Engine::getEntityName(unsigned int id)
{
    EntityMapIterator itr = _entities.find(id);
    if (itr == _entities.end()) return "";

    std::string name;
    itr->second->getUserValue("Name",name);

    return name;
}

struct FindSubEntityNodeVisitor : public osg::NodeVisitor
{
    FindSubEntityNodeVisitor(const std::string& name)
        : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
        , _name(name)
    {

    }

    virtual void apply(osg::Node& node)
    {
        std::string name;
        if (node.getUserValue("Name", name) && name == _name)
        {
            subEntity = dynamic_cast<osg::MatrixTransform*>(&node);
        }

        if (!subEntity.valid()) traverse(node);
    }
    Engine::Entity	subEntity;
protected:
    std::string		_name;
};

unsigned int Engine::getEntityId(unsigned int parentEntityId, const std::string& subEntityName)
{
    EntityMapIterator itr = _entities.find(parentEntityId);
    if (itr == _entities.end()) return 0;

    FindSubEntityNodeVisitor nv(subEntityName);
    itr->second->accept(nv);

    unsigned int ID = 0;
    if (nv.subEntity.valid())
        nv.subEntity->getUserValue("ID", ID);

    return ID;
}


void Engine::removeEntity(unsigned int id)
{
    EntityMapIterator itr = _entities.find(id);
    if (itr == _entities.end())
        return;

    getScene()->asGroup()->removeChild(itr->second);
    _entities.erase(itr);
}

void Engine::updateEntity(unsigned int id, const osg::Matrixd& mx)
{
    EntityMapIterator itr = _entities.find(id);
    if (itr == _entities.end())
        return;

    if (itr->second.valid())
    {
        itr->second->setMatrix(mx);
    }
}

void Engine::showEntity(unsigned int id, bool show)
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

void Engine::bindToEntity(unsigned int id, unsigned int toEntityId)
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

    titr->second->addChild(entity);

}

void Engine::unbindFromEntity(unsigned int id)
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
	getScene()->asGroup()->addChild(entity);
}

void Engine::bindEntityToCamera(unsigned int id, const osg::Matrixd& mx, unsigned int cameraID)
{
    if (cameraID >= _viewer->getNumViews()) return;

    EntityMapIterator itr = _entities.find(id);
    if (itr == _entities.end()) return;

    osg::ref_ptr<osg::MatrixTransform> entity = itr->second;
    if (!entity.valid()) return;

    osg::ref_ptr<osg::Camera> camera = _viewer->getView(cameraID)->getCamera();

    osg::ref_ptr<osg::UserDataContainer> container = camera->getOrCreateUserDataContainer();
    if (!container.valid()) return;

    container->addUserObject(entity);
    entity->setUserValue("BindToCameraOffset", mx);

    std::string cameras;
    entity->getUserValue("BindCameras", cameras);

    std::ostringstream oss;
    oss << cameras << ":" << cameraID;
    entity->setUserValue("BindCameras", oss.str());
}

void Engine::bindEntityToCameraUpdate(unsigned int id, const osg::Matrixd& mx)
{
    EntityMapIterator itr = _entities.find(id);
    if (itr == _entities.end()) return;

    osg::ref_ptr<osg::MatrixTransform> entity = itr->second;
    if (!entity.valid()) return;

    entity->setUserValue("BindToCameraOffset", mx);
}

void Engine::unbindEntityFromCamera(unsigned int id)
{
    EntityMapIterator itr = _entities.find(id);
    if (itr == _entities.end()) return;

    osg::ref_ptr<osg::MatrixTransform> entity = itr->second;
    if (!entity.valid()) return;

    std::string cameras;
    entity->getUserValue("BindCameras", cameras);

    unsigned int cameraID = 0;

    StringUtils::Tokens tokens = StringUtils::instance()->tokenize(cameras, ":");
    StringUtils::TokensIterator titr = tokens.begin();
    for (; titr != tokens.end(); ++titr)
    {
        cameraID = atoi(titr->c_str());

        if (cameraID >= _viewer->getNumViews()) continue;

        osg::ref_ptr<osg::Camera> camera = _viewer->getView(cameraID)->getCamera();

        osg::ref_ptr<osg::UserDataContainer> container = camera->getUserDataContainer();
        if (!container.valid()) continue;

        bool removed = false;
        for (size_t i = 0; i < container->getNumUserObjects(); ++i)
        {
            osg::ref_ptr<osg::MatrixTransform> object = dynamic_cast<osg::MatrixTransform*>(container->getUserObject(i));
            if (!object.valid()) continue;

            if (object == entity)
            {
                container->removeUserObject(i);
                removed = true;
                break;
            }
        }
        if (removed) break;
    }

    std::ostringstream oss;

    titr = tokens.begin();
    for (; titr != tokens.end(); ++titr)
    {
        if (atoi(titr->c_str()) != cameraID)
        {
            oss << ":" << *titr;
        }
    }

    entity->setUserValue("BindCameras", oss.str());
}


void Engine::initScene()
{
    if (!_scene.valid())
    {
        _sunOrMoonLight = new osg::LightSource;
        _sunOrMoonLight->getLight()->setLightNum(0);
        _sunOrMoonLight->setName("SunOrMoon");
        _sunOrMoonLight->setCullingActive(false);

        float minLightMargin = Configuration::instance()->getConfig("ShadowedScene-minLightMargin", 10.0);
        float maxFarPlane = Configuration::instance()->getConfig("ShadowedScene-maxFarPlane", 1000.0);
        unsigned int texSize = Configuration::instance()->getConfig("ShadowedScene-texSize", 4096);
        unsigned int baseTexUnit = Configuration::instance()->getConfig("ShadowedScene-baseTexUnit", 0);
        unsigned int shadowTexUnit = Configuration::instance()->getConfig("ShadowedScene-shadowTexUnit", 1);
        bool useShadowedScene = Configuration::instance()->getConfig("ShadowedScene", "yes") == "yes";

		osg::ref_ptr<osgShadow::ViewDependentShadowMap> vdsm = new osgShadow::ViewDependentShadowMap;


        if (useShadowedScene)
        {
            osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene = new osgShadow::ShadowedScene;
			shadowedScene->setReceivesShadowTraversalMask(ReceivesShadowTraversalMask);
			shadowedScene->setCastsShadowTraversalMask(CastsShadowTraversalMask);
#if 0
            osg::ref_ptr<osgShadow::MinimalShadowMap> msm = new osgShadow::LightSpacePerspectiveShadowMapDB;
            shadowedScene->setShadowTechnique(msm.get());
            
            msm->setMinLightMargin(minLightMargin);
            msm->setMaxFarPlane(maxFarPlane);
            msm->setTextureSize(osg::Vec2s(texSize, texSize));
            msm->setShadowTextureCoordIndex(shadowTexUnit);
            msm->setShadowTextureUnit(shadowTexUnit);
            msm->setBaseTextureCoordIndex(baseTexUnit);
            msm->setBaseTextureUnit(baseTexUnit);
            msm->setLight(_sunOrMoonLight->getLight());

            osg::Shader* mainFragmentShader = new osg::Shader(osg::Shader::FRAGMENT,
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
                "} \n");

            msm->setMainFragmentShader(mainFragmentShader);
#else 
			osgShadow::ShadowSettings* settings = shadowedScene->getShadowSettings();
			settings->setNumShadowMapsPerLight(1);
			settings->setShaderHint(osgShadow::ShadowSettings::NO_SHADERS);
			settings->setBaseShadowTextureUnit(1);
			settings->setLightNum(0);
			settings->setMaximumShadowMapDistance(200);
			settings->setTextureSize(osg::Vec2s(8192, 8192));
			settings->setReceivesShadowTraversalMask(ReceivesShadowTraversalMask);
			settings->setCastsShadowTraversalMask(CastsShadowTraversalMask);

			osg::ref_ptr<osgShadow::ViewDependentShadowMap> vdsm = new osgShadow::ViewDependentShadowMap;
			shadowedScene->setShadowTechnique(vdsm);

			static const char fragmentShaderSource_withBaseTexture[] =
				"uniform sampler2D baseTexture;                                          \n"
				"uniform int baseTextureUnit;                                            \n"
				"uniform sampler2DShadow shadowTexture0;                                 \n"
				"uniform int shadowTextureUnit0;                                         \n"
				"                                                                        \n"
				"void main(void)                                                         \n"
				"{                                                                       \n"
				"  vec4 colorAmbientEmissive = gl_FrontLightModelProduct.sceneColor;     \n"
				"  vec4 color = texture2D( baseTexture, gl_TexCoord[baseTextureUnit].xy );                                              \n"
				"  color *= mix( colorAmbientEmissive, gl_Color, shadow2DProj( shadowTexture0, gl_TexCoord[shadowTextureUnit0] ).r );     \n"
				"  gl_FragColor = color;                                                                                                \n"
				"} \n";

			osg::ref_ptr<osg::Program> program = new osg::Program;
			program->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource_withBaseTexture));

			//shadowedScene->getOrCreateStateSet()->setAttributeAndModes(program, osg::StateAttribute::ON);
#endif

            _scene = shadowedScene;
        }
        else
        {
            _scene = new osg::Group;
        }

        _lightsGroup = new osg::Group;

        _fog = new osg::Fog;
        _fog->setDensity(0);
        _scene->getOrCreateStateSet()->setAttributeAndModes(_fog.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		_scene->getOrCreateStateSet()->setDefine("FOG", osg::StateAttribute::ON);

    }
}

void Engine::setupInitFlags(unsigned int mask)
{
    _setupMask = mask;
}

void Engine::setViewType(osgViewer::View* view, ViewType type)
{
    // This is really experimental we are not after
    // doing full sensor support
    switch (type)
    {
    case OTW:
        view->getSceneData()->getOrCreateStateSet()->addUniform(new osg::Uniform("ViewOptions_EO", (bool)false));
        view->getSceneData()->getOrCreateStateSet()->addUniform(new osg::Uniform("ViewOptions_IR", (bool)false));
        break;
    case EO:
        view->getSceneData()->getOrCreateStateSet()->addUniform(new osg::Uniform("ViewOptions_EO", (bool)true));
        view->getSceneData()->getOrCreateStateSet()->addUniform(new osg::Uniform("ViewOptions_IR", (bool)false));
        break;
    case IR:
        view->getSceneData()->getOrCreateStateSet()->addUniform(new osg::Uniform("ViewOptions_EO", (bool)false));
        view->getSceneData()->getOrCreateStateSet()->addUniform(new osg::Uniform("ViewOptions_IR", (bool)true));
        break;
    default:
        view->getSceneData()->getOrCreateStateSet()->addUniform(new osg::Uniform("ViewOptions_EO", (bool)false));
        view->getSceneData()->getOrCreateStateSet()->addUniform(new osg::Uniform("ViewOptions_IR", (bool)false));
    }
    view->setUserValue("Options", (unsigned int)type);
}

void Engine::initView(osgViewer::View* view, ViewType type)
{
    initScene();

    osg::Group* root = new osg::Group;
    root->addChild(_sunOrMoonLight);
    root->addChild(_scene);
    root->addChild(_lightsGroup);
    view->setSceneData(root);

    view->setLight(_sunOrMoonLight->getLight());
    view->setUserValue("OpenIG-Scene", (bool)true);

    osg::ref_ptr<osg::Uniform> fCoefUniform = new osg::Uniform("Fcoef", 0.0f);
    view->getSceneData()->getOrCreateStateSet()->addUniform(fCoefUniform);
    view->getCamera()->setPreDrawCallback(new SetFarPlaneUniformCallback(fCoefUniform.get()));

    osg::StateSet *stateSet = view->getSceneData()->getOrCreateStateSet();
    stateSet->setMode(GL_DEPTH_CLAMP, osg::StateAttribute::ON);
    osg::Depth* depth = new osg::Depth(osg::Depth::LEQUAL);
    stateSet->setAttribute(depth);

    setViewType(view, type);

    createSunMoonLight();
}

void Engine::initViewer(osgViewer::CompositeViewer *viewer, const ViewIdentifiers& ids)
{
    _viewer = viewer;

    if (_viewer->getNumViews() != 0)
    {
        osgViewer::ViewerBase::Views views;
        _viewer->getViews(views);

        osgViewer::ViewerBase::Views::iterator itr = views.begin();
        for (unsigned int viewID = 0; itr != views.end(); ++itr, ++viewID)
        {
            bool processThisView = false;
            if (ids.size())
            {
                ViewIdentifiers::const_iterator vitr = ids.begin();
                for (; vitr != ids.end(); ++vitr)
                {
                    if (*vitr == viewID)
                    {
                        processThisView = true;
                        break;
                    }
                }
            }
            else
            {
                processThisView = true;
            }
            if (processThisView && !(**itr).getSceneData())
            {
                initView(*itr);
            }
        }
    }

}

void Engine::initPluginContext()
{
    _context.setImageGenerator(this);
}

void Engine::setCameraPosition(const osg::Matrixd& mx, bool viewMatrix, unsigned int cameraID)
{
    if (_viewer->getNumViews()==0) return;
    if (cameraID >= _viewer->getNumViews()) return;

    switch (viewMatrix)
    {
    case true:
         _viewer->getView(cameraID)->getCamera()->setViewMatrix(mx);
         _viewer->getView(cameraID)->getCamera()->setUserValue("ViewMatrix", mx);
        break;
    case false:
         _viewer->getView(cameraID)->getCamera()->setViewMatrix(osg::Matrixd::inverse(mx));
         _viewer->getView(cameraID)->getCamera()->setUserValue("ViewMatrix", osg::Matrixd::inverse(mx));
        break;
    }
}

void Engine::bindCameraToEntity(unsigned int id, const osg::Matrixd& mx, unsigned int cameraID)
{
    EntityMapIterator itr = _entities.find(id);
    if (itr == _entities.end())
        return;

    if (itr->second.valid() && _viewer.valid() && (_viewer->getNumViews() != 0) && (cameraID < _viewer->getNumViews()))
    {
        osg::ref_ptr<osg::Camera> camera = _viewer->getView(cameraID)->getCamera();
        camera->setUserValue("bindOffset",mx);
        camera->setUserValue("bindTo",id);
        camera->setUserValue("bindToEntity",(bool)true);
    }
}

void Engine::bindCameraSetFixedUp(bool fixedUp, bool freezeOrientation, unsigned int cameraID)
{
    if (_viewer.valid() && (_viewer->getNumViews() != 0) && (cameraID < _viewer->getNumViews()))
    {
        osg::ref_ptr<osg::Camera> camera = _viewer->getView(cameraID)->getCamera();
        camera->setUserValue("fixedUp",(bool)fixedUp);
        camera->setUserValue("freeze", (bool)freezeOrientation);

    }
}

void Engine::bindCameraUpdate(const osg::Matrixd& mx, unsigned int cameraID)
{
    if (_viewer.valid() && (_viewer->getNumViews() != 0) && (cameraID < _viewer->getNumViews()))
    {
        osg::ref_ptr<osg::Camera> camera = _viewer->getView(cameraID)->getCamera();
        camera->setUserValue("bindOffset",mx);
    }
}

bool Engine::isCameraBoundToEntity(unsigned int cameraID)
{
    bool bound = false;
    if (_viewer.valid() && _viewer->getNumViews() != 0 && cameraID < _viewer->getNumViews())
    {
        osg::ref_ptr<osg::Camera> camera = _viewer->getView(cameraID)->getCamera();
        camera->getUserValue("bindToEntity",bound);
    }

    return bound;
}

void Engine::unbindCameraFromEntity(unsigned int cameraID)
{
    if (_viewer.valid() && _viewer->getNumViews() != 0 && cameraID < _viewer->getNumViews())
    {
        osg::ref_ptr<osg::Camera> camera = _viewer->getView(cameraID)->getCamera();

        camera->setUserValue("bindToEntity",(bool)false);
    }
}

void Engine::setFog(double visibility)
{
    if (_fog.valid())
    {
        _fog->setDensity(3.912/visibility);
        _fog->setColor(osg::Vec4(0.9,0.9,0.9,1.0));
    }

    OpenIG::Base::FogAttributes fog(visibility);
    _context.addAttribute("Fog", new PluginContext::Attribute<OpenIG::Base::FogAttributes>(fog));
}

void Engine::setWind(float speed, float direction)
{
    OpenIG::Base::WindAttributes attr(speed,direction);
    _context.addAttribute("Wind", new PluginContext::Attribute<OpenIG::Base::WindAttributes>(attr));
}

void Engine::setTimeOfDay(unsigned int hour, unsigned int minutes)
{
    if (hour < 24)
    {
        OpenIG::Base::TimeOfDayAttributes tod((hour ? hour : 1),minutes);
        _context.addAttribute("TOD", new PluginContext::Attribute<OpenIG::Base::TimeOfDayAttributes>(tod));
    }
}

void Engine::setDate(unsigned int month, int day, int year)
{
    OpenIG::Base::DateAttributes date(month,day,year);
    _context.addAttribute("Date", new PluginContext::Attribute<OpenIG::Base::DateAttributes>(date));
}

void Engine::setRain(double factor)
{
    OpenIG::Base::RainSnowAttributes rain(factor);
    _context.addAttribute("Rain", new PluginContext::Attribute<OpenIG::Base::RainSnowAttributes>(rain));
}

void Engine::setSnow(double factor)
{
    OpenIG::Base::RainSnowAttributes snow(factor);
    _context.addAttribute("Snow", new PluginContext::Attribute<OpenIG::Base::RainSnowAttributes>(snow));
}

void Engine::addCloudLayer(unsigned int id, int type, double altitude, double thickness, double density, bool enable)
{
    OpenIG::Base::CLoudLayerAttributes attr;
    attr.setId(id);
    attr.setType(type);
    attr.setAltitude(altitude);
    attr.setDensity(density);
    attr.setEnable(enable);
    attr.setFlags(true);
    attr.setIsDirty(true);
    attr.setThickness(thickness);

    _context.addAttribute("CloudLayer", new PluginContext::Attribute<OpenIG::Base::CLoudLayerAttributes>(attr));
}

void Engine::enableCloudLayer(unsigned int id, bool enableIn)
{
    OpenIG::Base::CLoudLayerAttributes attr;
    attr.setId(id);
    attr.setFlags(false, false, enableIn);

    //osg::notify(osg::NOTICE) << "Engine::enableCloudLayerFile( " << id << ", " << enableIn << ")" << std::endl;
    _context.addAttribute("EnableCloudLayer", new PluginContext::Attribute<OpenIG::Base::CLoudLayerAttributes>(attr));
}

void Engine::removeCloudLayer(unsigned int id)
{
    OpenIG::Base::CLoudLayerAttributes attr;
    attr.setId(id);
    attr.setFlags(false,true);
    attr.setIsDirty(true);

    _context.addAttribute("CloudLayer", new PluginContext::Attribute<OpenIG::Base::CLoudLayerAttributes>(attr));
}

void Engine::removeAllCloudlayers()
{
    _context.addAttribute("RemoveAllCloudLayers", new osg::Referenced);
}

void Engine::updateCloudLayer(unsigned int id, double altitude, double thickness, double density)
{
    OpenIG::Base::CLoudLayerAttributes attr;
    attr.setId(id);
    attr.setAltitude(altitude);
    attr.setDensity(density);
    attr.setThickness(thickness);
    attr.setIsDirty(true);

    _context.addAttribute("CloudLayer", new PluginContext::Attribute<OpenIG::Base::CLoudLayerAttributes>(attr));
}

void Engine::createCloudLayerFile(unsigned int id, int type, double altitude, double thickness, double density, bool enable, const std::string& filename)
{
    OpenIG::Base::CLoudLayerFileAttributes attr;
    attr.setId(id);
    attr.setType(type);
    attr.setAltitude(altitude);
    attr.setDensity(density);
    attr.setFlags(true, false);
    attr.setIsDirty(true);
    attr.setThickness(thickness);
    attr.setEnable(enable);
    attr.setFilename(filename);

    _context.addAttribute("CloudLayerFile", new PluginContext::Attribute<OpenIG::Base::CLoudLayerFileAttributes>(attr));
}

void Engine::removeCloudLayerFile(unsigned int id)
{
    OpenIG::Base::CLoudLayerFileAttributes attr;
    attr.setId(id);
    attr.setFlags(false, true);

    //osg::notify(osg::NOTICE) << "Engine::loadCloudLayerFile( " << id << ", " << filename << ")" << std::endl;
    _context.addAttribute("CloudLayerFile", new PluginContext::Attribute<OpenIG::Base::CLoudLayerFileAttributes>(attr));
}

void Engine::loadCloudLayerFile(unsigned int id, int type, const std::string& filename)
{
    OpenIG::Base::CLoudLayerFileAttributes attr;
    attr.setId(id);
    attr.setType(type);
    attr.setFilename(filename);
    attr.setFlags(false, false);
    //attr.setIsDirty(true);

    osg::notify(osg::NOTICE) << "Engine::loadCloudLayerFile( " << id << ", type: " << type << ", filename: " << filename << ")" << std::endl;
    _context.addAttribute("CloudLayerFile", new PluginContext::Attribute<OpenIG::Base::CLoudLayerFileAttributes>(attr));
}

void Engine::resetAnimation(unsigned int entityId, const std::string& animationName)
{
    OpenIG::Base::AnimationAttributes attr;
    attr.entityId = entityId;
    attr.animationName = animationName;
    attr.playback = false;
    attr.reset = true;

    _context.addAttribute("Animation", new PluginContext::Attribute<OpenIG::Base::AnimationAttributes>(attr));
}

void Engine::resetAnimation(unsigned int entityId, const StringUtils::StringList& animations)
{
    StringUtils::StringListConstIterator itr = animations.begin();
    for ( ; itr != animations.end(); ++itr )
    {
        stopAnimation(entityId,*itr);
    }
}

void Engine::stopAnimation(unsigned int entityId, const std::string& animationName)
{
    OpenIG::Base::AnimationAttributes attr;
    attr.entityId = entityId;
    attr.animationName = animationName;
    attr.playback = false;

    _context.addAttribute("Animation", new PluginContext::Attribute<OpenIG::Base::AnimationAttributes>(attr));
}

void Engine::stopAnimation(unsigned int entityId, const StringUtils::StringList& animations)
{
    StringUtils::StringListConstIterator itr = animations.begin();
    for ( ; itr != animations.end(); ++itr )
    {
        stopAnimation(entityId,*itr);
    }
}

void Engine::playAnimation(unsigned int entityId, const std::string& animationName)
{
    OpenIG::Base::AnimationAttributes attr;
    attr.entityId = entityId;
    attr.animationName = animationName;

    _context.addAttribute("Animation", new PluginContext::Attribute<OpenIG::Base::AnimationAttributes>(attr));

}

void Engine::changeAnimationStatus(unsigned int entityId, ImageGenerator::AnimationStatus status, const OpenIG::Base::StringUtils::StringList& animations)
{
    switch (status)
    {
    case Play:
        playAnimation(entityId, animations);
        break;
    case Stop:
        stopAnimation(entityId, animations);
        break;
    case Reset:
        resetAnimation(entityId, animations);
        break;
    case Pause:
        {
            StringUtils::StringListConstIterator itr = animations.begin();
            for (; itr != animations.end(); ++itr)
            {
                OpenIG::Base::AnimationAttributes attr;
                attr.entityId = entityId;
                attr.animationName = *itr;
                attr.pause = true;

                _context.addAttribute("Animation", new PluginContext::Attribute<OpenIG::Base::AnimationAttributes>(attr));
            }
        }
        break;
    case Restore:
    {
        StringUtils::StringListConstIterator itr = animations.begin();
        for (; itr != animations.end(); ++itr)
        {
            OpenIG::Base::AnimationAttributes attr;
            attr.entityId = entityId;
            attr.animationName = *itr;
            attr.restore = true;

            _context.addAttribute("Animation", new PluginContext::Attribute<OpenIG::Base::AnimationAttributes>(attr));
        }
    }
        break;
    }
}

void Engine::playAnimation(unsigned int entityId, const StringUtils::StringList& animations)
{
    StringUtils::StringListConstIterator itr = animations.begin();
    for ( ; itr != animations.end(); ++itr )
    {
        playAnimation(entityId,*itr);
    }
}

void Engine::playAnimation(unsigned int entityId, const std::string& animationName, RefAnimationSequenceCallbacks* cbs )
{
    OpenIG::Base::AnimationAttributes attr;
    attr.entityId = entityId;
    attr.animationName = animationName;
    attr.sequenceCallbacks = cbs;

    _context.addAttribute("Animation", new PluginContext::Attribute<OpenIG::Base::AnimationAttributes>(attr));
}

void Engine::setUpdateViewerCameraManipulator(bool update)
{
    _updateViewerCameraMainpulator = update;
}

OpenIG::Base::LightImplementationCallback* Engine::getLightImplementationCallback()
{
    return _lightImplementationCallback.get();
}

void Engine::setLightImplementationCallback( OpenIG::Base::LightImplementationCallback* cb)
{
    _lightImplementationCallback = cb;
}

osgViewer::CompositeViewer* Engine::getViewer()
{
    return _viewer.get();
}

osg::Node* Engine::getScene()
{
    return _scene.get();
}

OpenIG::Base::ImageGenerator::EntityMap& Engine::getEntityMap()
{
    return _entities;
}

osg::LightSource* Engine::getSunOrMoonLight()
{
    return _sunOrMoonLight.get();
}

osg::Fog* Engine::getFog()
{
    return _fog.get();
}

OpenIG::PluginBase::PluginContext& Engine::getPluginContext()
{
    return _context;
}

Engine::LightsMap& Engine::getLightsMap()
{
    return _lights;
}

const OpenIG::Base::StringUtils::StringList& Engine::getFilesToBeCached() const
{
    return _filesToBeCached;
}

void Engine::addFilesToBeCached(const OpenIG::Base::StringUtils::StringList& files)
{
    _filesToBeCached.insert(_filesToBeCached.end(), files.begin(), files.end());
}

bool Engine::isFileCached(const std::string& fileName)
{
    OpenIG::Base::StringUtils::StringList::iterator itr = _filesToBeCached.begin();
    for (; itr != _filesToBeCached.end(); ++itr)
    {
        if (*itr == fileName) return true;
    }
    return false;
}

void Engine::setIntersectionCallback(IntersectionCallback* cb)
{
	_intersectionCallback = cb;
}

bool Engine::intersect(const osg::Vec3d& start, const osg::Vec3d& end, osg::Vec3d& intersectionPoint, unsigned mask)
{
	if (_intersectionCallback.valid())
		return _intersectionCallback->intersect(start, end, intersectionPoint, mask);
	return false;
}
