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

#include <osgDB/ReadFile>
#include <osgDB/XmlParser>
#include <osgDB/FileNameUtils>

#include <osg/NodeVisitor>
#include <osg/Texture2D>

#include <limits>
#include <cctype>

class StringUtils
{
protected:
    StringUtils() {}
    ~StringUtils() {}

public:
    static StringUtils* instance();

    typedef std::vector<std::string>					Tokens;
    typedef std::vector<std::string>::iterator			TokensIterator;
    typedef std::vector<std::string>::const_iterator	TokensConstIterator;

    Tokens tokenize(const std::string& str, const std::string& delimiters = " ");

    template<typename T>
    static inline std::string &ltrim(std::string &s, T istestchar = std::isspace)
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(istestchar)));
        return s;
    }

    template<typename T>
    static inline std::string &rtrim(std::string &s, T istestchar = std::isspace)
    {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(istestchar)).base(), s.end());
        return s;
    }

    template<typename T>
    static inline std::string &trim(std::string &s, T istestchar = std::isspace)
    {
        return ltrim(rtrim(s, istestchar), istestchar);
    }
};

StringUtils* StringUtils::instance()
{
    static StringUtils s_StringUtils;
    return &s_StringUtils;
}

StringUtils::Tokens StringUtils::tokenize(const std::string& str, const std::string& delimiters)
{
    Tokens tokens;
    std::string::size_type delimPos = 0, tokenPos = 0, pos = 0;

    if (str.length()<1)  return tokens;
    while (1)
    {
        delimPos = str.find_first_of(delimiters, pos);
        tokenPos = str.find_first_not_of(delimiters, pos);
        if (tokenPos != std::string::npos && str[tokenPos] == '\"')
        {
            delimPos = str.find_first_of("\"", tokenPos + 1);
            pos++;
        }

        if (std::string::npos != delimPos)
        {
            if (std::string::npos != tokenPos)
            {
                if (tokenPos<delimPos)
                {
                    std::string token = str.substr(pos, delimPos - pos);
                    if (token.length()) tokens.push_back(token);
                }
            }
            pos = delimPos + 1;
        }
        else
        {
            if (std::string::npos != tokenPos)
            {
                std::string token = str.substr(pos);
                if (token.length()) tokens.push_back(token);
            }
            break;
        }
    }
    return tokens;
}

int istrimchar(int ch)
{
    switch (ch)
    {
    case '\"':
        return 1;
    default:
        return 0;
    }
}

class GenerateVegetationFileBasedOnTexture : public osg::NodeVisitor
{
public:
    GenerateVegetationFileBasedOnTexture(bool print, bool printDetails)
        : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
        , _print(print)
        , _printDetails(printDetails)
        , _vegetationType(CrossedTriangleFans)
    {

    }

    enum VegetationType
    {
        CrossedTriangleFans,
        Billboards,
        CrossedQuads
    };

    void readXML(const std::string& fileName)
    {
        osg::ref_ptr<osgDB::XmlNode> root = osgDB::readXmlFile(fileName);
        if (!root.valid())
        {
            osg::notify(osg::NOTICE) << "Failed to read the configuration xml: " << fileName << std::endl;
            return;
        }

        osg::ref_ptr<osgDB::XmlNode> config = root->children.size() ? root->children.at(0) : 0;
        if (!config.valid())
        {
            osg::notify(osg::NOTICE) << "Expecting <OpenIG-Vegetation-Config> tag" << std::endl;
            return;
        }

        osgDB::XmlNode::Children::iterator itr = config->children.begin();
        for ( ; itr != config->children.end(); ++itr)
        {
            osg::ref_ptr<osgDB::XmlNode> child = *itr;
            if (child->name == "Vegetation-Info")
            {
                readVegetationInfo(child.get());
            }
            if (child->name == "Vegetation-Type")
            {
                if (child->contents == "Crossed-TriangleFans")
                    _vegetationType = CrossedTriangleFans;
                else
                if (child->contents == "Billboards")
                    _vegetationType = Billboards;
                else
                if (child->contents == "Crossed-Quads")
                    _vegetationType = CrossedQuads;
            }
        }
    }

    void readVegetationInfo(osgDB::XmlNode* node)
    {
        if (!node) return;
        if (node->children.size() == 0) return;

        osgDB::XmlNode::Children::iterator itr = node->children.begin();
        for ( ; itr != node->children.end(); ++itr)
        {
            osg::ref_ptr<osgDB::XmlNode> child = *itr;
            if (child->name == "Objects")
            {
                std::string objects = child->contents;

                StringUtils::Tokens tokens = StringUtils::instance()->tokenize(objects);
                StringUtils::TokensIterator titr = tokens.begin();

                ObjectFileNames& ofn = _objects[_objectId++];

                osg::ref_ptr<PosScaleArrays> psa = new PosScaleArrays;
                psa->_pos = new osg::Vec3Array;
                psa->_scale = new osg::Vec3Array;

                for ( ; titr != tokens.end(); ++titr)
                {
                    _images[*titr] = psa;

                    ofn.push_back(_images[*titr]);
                }
            }
        }
    }

    virtual void apply(osg::Node& node)
    {
        osg::ref_ptr<osg::StateSet> ss = node.getOrCreateStateSet();
        osg::ref_ptr<osg::StateAttribute> sa = ss->getTextureAttribute(0,osg::StateAttribute::TEXTURE);
        if (sa.valid())
        {
            osg::ref_ptr<osg::Texture2D> texture = dynamic_cast<osg::Texture2D*>(sa.get());
            if (texture.valid())
            {
                osg::ref_ptr<osg::Image> image = texture->getImage();
                if (image.valid())
                {
                    std::string simpleFileName = osgDB::getSimpleFileName(image->getFileName());

                    if (_print)
                    {
                        PrintFileNamesMapIterator itr = _printImageFileNames.find(simpleFileName);
                        if ( itr == _printImageFileNames.end())
                        {
                            osg::notify(osg::NOTICE) << "Image: " << simpleFileName << std::endl;
                            _printImageFileNames[simpleFileName] = true;
                        }
                    }

                    FileNamesMapIterator itr = _images.find(simpleFileName);
                    //osg::notify(osg::NOTICE) << "image to lookup: " << simpleFileName << std::endl;
                    if ( itr == _images.end())
                    {
                        //osg::notify(osg::NOTICE) << "\t not found!" << std::endl;
                    }
                    else
                    {
                        //osg::notify(osg::NOTICE) << "\t found!" << std::endl;

                        osg::ref_ptr<osg::Vec3Array>& positions= itr->second->_pos;
                        osg::ref_ptr<osg::Vec3Array>& scales = itr->second->_scale;

                        osg::ref_ptr<osg::Geode> geode = dynamic_cast<osg::Geode*>(&node);
                        if (geode.valid())
                        {
                            for ( size_t i=0; i<geode->getNumDrawables(); ++i)
                            {
                                osg::ref_ptr<osg::Geometry> geometry = geode->getDrawable(i)->asGeometry();
                                if (geometry.valid())
                                {
                                    osg::ref_ptr<osg::Vec3Array> verts = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());

                                    for (size_t j=0; j<geometry->getNumPrimitiveSets(); ++j)
                                    {
                                        osg::ref_ptr<osg::PrimitiveSet> ps = geometry->getPrimitiveSet(j);
                                        if (!ps.valid()) continue;

                                        if (_printDetails)
                                            osg::notify(osg::NOTICE) << itr->first << ", mode: " << ps->getMode() << ", type: " << ps->getType() << std::endl;

                                        if ((_vegetationType==CrossedTriangleFans && (j%2==0)) ||
                                            (_vegetationType==Billboards) ||
                                            (_vegetationType==CrossedQuads))
                                        {
                                            if (ps->getType()==osg::PrimitiveSet::DrawArraysPrimitiveType)
                                            {
                                                std::vector<osg::Vec3> pos;
                                                std::vector<osg::Vec3> scale;
                                                if (extractPositionScale(verts,ps,pos,scale))
                                                {
                                                    positions->insert(positions->end(), pos.begin(), pos.end());
                                                    scales->insert(scales->end(), scale.begin(), scale.end());
                                                }
                                            }
                                            else
                                            {
                                                osg::Vec3 pos;
                                                osg::Vec3 scale;
                                                if (extractPositionScale(verts,ps,pos,scale))
                                                {
                                                    positions->push_back(pos);
                                                    scales->push_back(scale);
                                                }
                                            }
                                        }
                                    }

                                    if (_printDetails)
                                        osg::notify(osg::NOTICE) << itr->first << ", Verts #:" << verts->size() << ", PrimitiveSets #:" << geometry->getNumPrimitiveSets() << std::endl;
                                }
                            }
                        }
                    }
                }
            }
        }
        traverse(node);
    }

    void generateVegetationFiles(const std::string& fileName, const std::string& substitute, const std::string& with)
    {
        ObjectsMapIterator itr = _objects.begin();
        for ( ; itr != _objects.end(); ++itr)
        {
            std::ostringstream oss;
            oss << itr->first;

            std::string vegetationfileName = fileName;
            if ( substitute.length() )
            {
                std::size_t pos = vegetationfileName.find(substitute);
                if ( pos != std::string::npos )
                {
                    if(with.length())
                        vegetationfileName.replace(pos,substitute.length(),with);
                    else
                        vegetationfileName.erase(pos,substitute.length());
                }
            }

            std::string binFileName = vegetationfileName + "." + oss.str() + ".vegbin";

            std::ofstream file;
            file.open(binFileName.c_str(),std::ios::out|std::ios::binary);
            if (file.is_open())
            {
                ObjectFileNames& arrays = itr->second;
                ObjectFileNamesIterator oitr = arrays.begin();
                for ( ; oitr != arrays.end(); ++oitr)
                {
                    osg::ref_ptr<osg::Vec3Array>& verts = (*oitr)->_pos;
                    if (!verts.valid()) continue;

                    file.write(reinterpret_cast<const char*>(&verts->front()), sizeof(float)*3*verts->size());
                }


                file.close();

                osg::notify(osg::NOTICE) << "vegetation bin file written: " << binFileName << std::endl;
            }

            std::string binScaleFileName = vegetationfileName + "." + oss.str() + ".scales.vegbin";

            std::ofstream fileScales;
            fileScales.open(binScaleFileName.c_str(),std::ios::out|std::ios::binary);
            if (fileScales.is_open())
            {
                ObjectFileNames& arrays = itr->second;
                ObjectFileNamesIterator oitr = arrays.begin();
                for ( ; oitr != arrays.end(); ++oitr)
                {
                    osg::ref_ptr<osg::Vec3Array>& scales = (*oitr)->_scale;
                    if (!scales.valid()) continue;

                    fileScales.write(reinterpret_cast<const char*>(&scales->front()), sizeof(float)*3*scales->size());
                }


                fileScales.close();

                osg::notify(osg::NOTICE) << "vegetation scales bin file written: " << binScaleFileName << std::endl;
            }

        }
    }

protected:
    bool                _print;
    bool                _printDetails;
    VegetationType      _vegetationType;
    static size_t       _objectId;

    struct PosScaleArrays : osg::Referenced
    {
        osg::ref_ptr<osg::Vec3Array>    _pos;
        osg::ref_ptr<osg::Vec3Array>    _scale;
    };


    typedef std::map< std::string, osg::ref_ptr<PosScaleArrays> >               FileNamesMap;
    typedef std::map< std::string, osg::ref_ptr<PosScaleArrays> >::iterator     FileNamesMapIterator;

    FileNamesMap        _images;

    typedef std::map< std::string, bool>                                        PrintFileNamesMap;
    typedef std::map< std::string, bool>::iterator                              PrintFileNamesMapIterator;

    PrintFileNamesMap   _printImageFileNames;

    typedef std::vector< osg::ref_ptr<PosScaleArrays> >                         ObjectFileNames;
    typedef std::vector< osg::ref_ptr<PosScaleArrays> >::iterator               ObjectFileNamesIterator;
    typedef std::map< size_t, ObjectFileNames >                                 ObjectsMap;
    typedef std::map< size_t, ObjectFileNames >::iterator                       ObjectsMapIterator;

    ObjectsMap          _objects;

    bool extractPositionScale(
            osg::ref_ptr<osg::Vec3Array>& verts,
            osg::ref_ptr<osg::PrimitiveSet>& ps,
            std::vector<osg::Vec3>& pos,
            std::vector<osg::Vec3>& scale)
    {
        if (!verts.valid() || !ps.valid()) return false;

        switch (ps->getMode())
        {
        case osg::PrimitiveSet::QUADS:
            switch (ps->getType())
            {
                case osg::PrimitiveSet::DrawArraysPrimitiveType:
                {
                    osg::ref_ptr<osg::DrawArrays> da = dynamic_cast<osg::DrawArrays*>(ps.get());
                    if (da.valid())
                    {
                        osg::BoundingBox bb;
                        size_t quadsCount = 1;
                        size_t counter = 0;
                        for (int i=da->getFirst(); i<da->getFirst()+da->getCount(); ++i)
                        {
                            unsigned int idx = da->index(i);
                            bb.expandBy(verts->at(idx));

                            if ((++counter % 4 == 0))
                            {
                                if ( ++quadsCount % 2 == 0)
                                {
                                    pos.push_back(osg::Vec3(bb.center().x(),bb.center().y(),bb.zMin()));

                                    osg::BoundingSphere bs(bb);

                                    float scaleXY = bs.radius();
                                    scale.push_back(osg::Vec3(scaleXY,scaleXY,bb.zMax()-bb.zMin()));
                                }

                                bb = osg::BoundingBox();
                            }
                        }

                        return true;
                    }
                }
                break;
                default:
                    osg::notify(osg::NOTICE) << "Primitive set type: " << ps->getType() << " not implemented yet" << std::endl;
                break;
            }
            break;
        default:
            osg::notify(osg::NOTICE) << "Primitive set mode: " << ps->getMode() << " not implemented yet" << std::endl;
        }

        return false;
    }

    bool extractPositionScale(osg::ref_ptr<osg::Vec3Array>& verts,osg::ref_ptr<osg::PrimitiveSet>& ps, osg::Vec3& pos, osg::Vec3& scale)
    {
        if (!verts.valid() || !ps.valid()) return false;

        switch (ps->getMode())
        {
        case osg::PrimitiveSet::TRIANGLE_STRIP:
            switch (ps->getType())
            {
                case osg::PrimitiveSet::DrawElementsUShortPrimitiveType:
                {
                    osg::ref_ptr<osg::DrawElementsUShort> de = dynamic_cast<osg::DrawElementsUShort*>(ps->getDrawElements());
                    if (de.valid())
                    {
                        osg::BoundingBox bb;
                        for (size_t i=0; i<de->getNumIndices(); ++i)
                        {
                            unsigned int idx = de->index(i);
                            bb.expandBy(verts->at(idx));
                        }

                        pos = osg::Vec3(bb.center().x(),bb.center().y(),bb.zMin());

                        osg::BoundingSphere bs(bb);

                        float scaleXY = bs.radius();
                        scale = osg::Vec3(scaleXY,scaleXY,bb.zMax()-bb.zMin());

                        return true;
                    }
                }
                break;
                default:
                    osg::notify(osg::NOTICE) << "Primitive set type: " << ps->getType() << " not implemented yet" << std::endl;
                break;
            }
            break;
        case osg::PrimitiveSet::QUADS:
            switch (ps->getType())
            {
                case osg::PrimitiveSet::DrawElementsUShortPrimitiveType:
                {
                    osg::ref_ptr<osg::DrawElementsUShort> de = dynamic_cast<osg::DrawElementsUShort*>(ps->getDrawElements());
                    if (de.valid())
                    {
                        osg::BoundingBox bb;
                        for (size_t i=0; i<de->getNumIndices(); ++i)
                        {
                            unsigned int idx = de->index(i);
                            bb.expandBy(verts->at(idx));
                        }

                        pos = osg::Vec3(bb.center().x(),bb.center().y(),bb.zMin());

                        osg::BoundingSphere bs(bb);

                        float scaleXY = bs.radius();
                        scale = osg::Vec3(scaleXY,scaleXY,bb.zMax()-bb.zMin());

                        return true;
                    }
                }
                break;
                default:
                    osg::notify(osg::NOTICE) << "Primitive set type: " << ps->getType() << " not implemented yet" << std::endl;
                break;
            }
            break;
        default:
            osg::notify(osg::NOTICE) << "Primitive set mode: " << ps->getMode() << " not implemented yet" << std::endl;
        }

        return false;
    }
};

size_t GenerateVegetationFileBasedOnTexture::_objectId = 0;

class ParseDatabaseReadCallback : public osgDB::Registry::ReadFileCallback
{
public:
    ParseDatabaseReadCallback(
                const std::string& xmlFileName,
                bool print=false,
                bool printDetails=false,
                const std::string& substitute = "",
                const std::string& with = "")
        : _print(print)
        , _printDetails(printDetails)
        , _xmlFileName(xmlFileName)
        , _substitite(substitute)
        , _with(with)
    {
        _nv = new GenerateVegetationFileBasedOnTexture(_print,_printDetails);
        _nv->readXML(_xmlFileName);
    }
    virtual osgDB::ReaderWriter::ReadResult readNode(const std::string& filename, const osgDB::Options* options)
    {
        osgDB::ReaderWriter::ReadResult result = osgDB::Registry::instance()->readNodeImplementation(filename,options);
        if (result.getNode())
        {
            result.getNode()->accept(*_nv.get());
            _nv->generateVegetationFiles(filename,_substitite,_with);
        }
        return result;
    }

protected:
    bool                                                _print;
    bool                                                _printDetails;
    std::string                                         _xmlFileName;
    osg::ref_ptr<GenerateVegetationFileBasedOnTexture>  _nv;
    std::string                                         _substitite;
    std::string                                         _with;

};

int main(int argc, char** argv)
{
    osg::ArgumentParser arguments(&argc,argv);

    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is vegetation files generator for OpenIG.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("--print","Prints all the textures used in the file");
    arguments.getApplicationUsage()->addCommandLineOption("--printDetails","Prints details about the vegetation geometry");
    arguments.getApplicationUsage()->addCommandLineOption("--config <filename>","Use this config xml");
    arguments.getApplicationUsage()->addCommandLineOption("--substitute string","substitute the string in the output vegetation file with the replacement string");
    arguments.getApplicationUsage()->addCommandLineOption("--with string","the replacement string");

    unsigned int helpType = 0;
    if ((helpType = arguments.readHelpType()))
    {
        arguments.getApplicationUsage()->write(osg::notify(osg::NOTICE), helpType);
        return 1;
    }

    if (arguments.errors())
    {
        arguments.writeErrorMessages(osg::notify(osg::NOTICE));
        return 1;
    }


    if (arguments.argc()<=1)
    {
        arguments.getApplicationUsage()->write(osg::notify(osg::NOTICE),osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }

    bool print = arguments.read("--print");
    bool printDetails = arguments.read("--printdetails");

    std::string xml = "VegetationInfo.xml";
    while (arguments.read("--config",xml)) {}

    std::string substitute;
    std::string with;

    while (arguments.read("--substitute",substitute)) {}
    while (arguments.read("--with",with)) {}

    osgDB::Registry::instance()->setReadFileCallback(
        new ParseDatabaseReadCallback(xml,print,printDetails,substitute,with)
    );

    osg::ref_ptr<osg::Node> model = osgDB::readNodeFiles(arguments);
    model = 0;

    return 0;
}

