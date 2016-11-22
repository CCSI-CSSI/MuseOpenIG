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
//#*    Please direct any questions or comments to the OpenIG Forums
//#*    Email address: openig@compro.net
//#*
//#*
//#*****************************************************************************
#include <Core-PluginBase/Plugin.h>
#include <Core-PluginBase/PluginContext.h>

#include <Core-Base/ImageGenerator.h>
#include <Core-Base/StringUtils.h>

#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>

#include <OpenThreads/Mutex>
#include <OpenThreads/ScopedLock>

#include <osg/ValueObject>
#include <osg/PolygonOffset>

#include <sstream>

namespace OpenIG {
    namespace Plugins {

        class ApplyOffsetNodeVisitor : public osg::NodeVisitor
        {
        public:
            ApplyOffsetNodeVisitor(const osg::Vec3d& offset)
                : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
                , _offset(offset)
                , _doOffset(true)
            {

            }

            virtual void apply(osg::Node& node)
            {
                if (node.getName().compare(0, 2, "db") == 0 && node.getNumParents())
                {
                    _doOffset = false;
                }

                osg::Transform* transform = node.asTransform();
                if (transform && _doOffset)
                {
                    osg::MatrixTransform* mxt = transform->asMatrixTransform();
                    if (mxt)
                    {
                        osg::Matrixd mx = mxt->getMatrix();
                        osg::Vec3d t = mx.getTrans();

                        t += _offset;

                        osg::Vec3d scale = mx.getScale();
                        osg::Quat  rotate = mx.getRotate();

                        mxt->setMatrix(osg::Matrixd::scale(scale) * osg::Matrixd::rotate(rotate)*osg::Matrixd::translate(t));
                        mxt->dirtyBound();
                    }

                    osg::PositionAttitudeTransform* pat = transform->asPositionAttitudeTransform();
                    if (pat)
                    {
                        osg::notify(osg::NOTICE) << "osg::PositionAttitudeTransform not handled!" << std::endl;
                    }
                }

                osg::Geode* geode = node.asGeode();
                if (geode && _doOffset)
                {
                    for (unsigned int i = 0; i < geode->getNumDrawables(); ++i)
                    {
                        osg::Geometry* geometry = geode->getDrawable(i)->asGeometry();
                        if (!geometry) continue;

                        osg::Vec3Array* vxs = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());
                        if (!vxs) continue;

                        osg::Vec3Array::iterator itr = vxs->begin();
                        for (; itr != vxs->end(); ++itr)
                        {
                            osg::Vec3& vertex = *itr;
                            vertex += _offset;
                        }
                        vxs->dirty();
                        geometry->dirtyBound();
                    }
                }


                osg::LOD* lod = dynamic_cast<osg::LOD*>(&node);
                if (lod && _doOffset)
                {
                    osg::Vec3d center = lod->getCenter();
                    center += _offset;

                    lod->setCenter(center);
                    lod->dirtyBound();
                }

                traverse(node);

                if (node.getName().compare(0, 2, "db") == 0 && node.getNumParents())
                {
                    _doOffset = true;
                }

                node.dirtyBound();
            }

        protected:
            osg::Vec3d          _offset;
            bool                _doOffset;
        };

        class VDBOffsetPlugin : public OpenIG::PluginBase::Plugin
        {
        public:

            VDBOffsetPlugin() {}

            virtual std::string getName() { return "VDBOffset"; }

            virtual std::string getDescription() { return "Offsets a visual database by values passed as osg plugin options"; }

            virtual std::string getVersion() { return "1.0.0"; }

            virtual std::string getAuthor() { return "ComPro, Nick"; }

            virtual void databaseRead(const std::string&, osg::Node* node, const osgDB::Options* options)
            {
                OpenThreads::ScopedLock<OpenThreads::Mutex>     lock(_mutex);

                _options = options;

                if (_options.valid())
                {
                    std::string offsetStr = _options.valid() ? _options->getOptionString() : "";

                    OpenIG::Base::StringUtils::Tokens tokensNewOffset = OpenIG::Base::StringUtils::instance()->tokenize(offsetStr, ",");

                    if (tokensNewOffset.size() == 3)
                    {
                        _offset.x() = atof(tokensNewOffset.at(0).c_str());
                        _offset.y() = atof(tokensNewOffset.at(1).c_str());
                        _offset.z() = atof(tokensNewOffset.at(2).c_str());
                    }

                    ApplyOffsetNodeVisitor nv(_offset);
                    node->accept(nv);
                }
            }


            virtual void update(OpenIG::PluginBase::PluginContext& context)
            {
                bool newOffset = false;

                osg::ref_ptr<osg::Referenced> ref = context.getAttribute("VDBOffset");
                const osgDB::Options *attr = dynamic_cast<const osgDB::Options *>(ref.get());
                if (attr)
                {
                    std::string newOffsetStr = attr->getOptionString();

                    OpenThreads::ScopedLock<OpenThreads::Mutex>     lock(_mutex);

                    OpenIG::Base::StringUtils::Tokens tokensNewOffset = OpenIG::Base::StringUtils::instance()->tokenize(newOffsetStr, ",");

                    if (tokensNewOffset.size() == 3)
                    {
                        _offset.x() = atof(tokensNewOffset.at(0).c_str());
                        _offset.y() = atof(tokensNewOffset.at(1).c_str());
                        _offset.z() = atof(tokensNewOffset.at(2).c_str());
                    }


                    osg::notify(osg::NOTICE) << "new offset = " << _offset.x() << ", " << _offset.y() << ", " << _offset.z() << std::endl;

                    std::ostringstream oss;
                    oss << _offset.x() << "," << _offset.y() << "," << _offset.z() << std::endl;

                    _options = new osgDB::Options(oss.str());

                    newOffset = true;

                }

                if (newOffset && context.getImageGenerator()->getEntityMap().count(0))
                {
                    osg::ref_ptr<osg::MatrixTransform> entity = context.getImageGenerator()->getEntityMap()[0];
                    if (entity.valid())
                    {
                        std::string fileName;
                        entity->getUserValue("fileName", fileName);

                        context.getImageGenerator()->reloadEntity(0, fileName, _options.get());
                    }
                }
            }


        protected:
            osg::ref_ptr<const osgDB::Options>      _options;
            osg::Vec3d                              _offset;
            OpenThreads::Mutex                      _mutex;

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
    return new OpenIG::Plugins::VDBOffsetPlugin;
}

extern "C" EXPORT void DeletePlugin(OpenIG::PluginBase::Plugin* plugin)
{
    osg::ref_ptr<OpenIG::PluginBase::Plugin> p(plugin);
}
