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
//#*	author    Trajce Nikolov Nick openig@compro.net
//#*	copyright(c)Compro Computer Services, Inc.
//#*
//#*    Please direct any questions or comments to the OpenIG Forums
//#*    Email address: openig@compro.net
//#*

#include <osgDB/XmlParser>
#include <osgDB/ReadFile>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>

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

    Tokens tokenize(const std::string& str, const std::string& delimiters = " ")
    {
        Tokens tokens;
        std::string::size_type delimPos = 0, tokenPos = 0, pos = 0;

        if(str.length()<1)  return tokens;
        while(1)
        {
            delimPos = str.find_first_of(delimiters, pos);
            tokenPos = str.find_first_not_of(delimiters, pos);
            if (tokenPos != std::string::npos && str[tokenPos]=='\"')
            {
                delimPos = str.find_first_of("\"", tokenPos+1);
                pos++;
            }

            if(std::string::npos != delimPos)
            {
                if(std::string::npos != tokenPos)
                {
                    if(tokenPos<delimPos)
                    {
                        std::string token = str.substr(pos,delimPos-pos);
                        if (token.length()) tokens.push_back(token);
                    }
                }
                pos = delimPos+1;
            }
            else
            {
                if(std::string::npos != tokenPos)
                {
                    std::string token = str.substr(pos);
                    if (token.length()) tokens.push_back(token);
                }
                break;
            }
        }
        return tokens;
    }


    template<typename T>
    inline std::string &ltrim(std::string &s, T istestchar = std::isspace)
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(istestchar)));
        return s;
    }

    template<typename T>
    inline std::string &rtrim(std::string &s, T istestchar = std::isspace)
    {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(istestchar)).base(), s.end());
        return s;
    }

    template<typename T>
    inline std::string &trim(std::string &s, T istestchar = std::isspace)
    {
        return ltrim(rtrim(s, istestchar), istestchar);
    }
};

StringUtils* StringUtils::instance()
{
    static StringUtils s_StringUtils;
    return &s_StringUtils;
}

class DatabaseReadCallback : public osgDB::Registry::ReadFileCallback
{
public:
    DatabaseReadCallback(const std::string& pattern, const std::string& xmlFileName)
        : _pattern(pattern)
        , _xmlFileName(xmlFileName)
        , _lodRange(0.0)
        , _vegetationInfoId(0)
    {
        readXML(_xmlFileName);
    }

    virtual osgDB::ReaderWriter::ReadResult readNode(const std::string& filename, const osgDB::Options* options)
    {
        osgDB::ReaderWriter::ReadResult result = osgDB::Registry::instance()->readNodeImplementation(filename,options);
        if (result.getNode() && filename.substr(0,_pattern.length())==_pattern)
        {
            osg::ref_ptr<osg::Group> root = new osg::Group;

#if 0
            osgDB::ReaderWriter::ReadResult newResult(root);
            result = newResult;
#else
            result.getNode()->asGroup()->addChild(root);
#endif

            VegetationInfoMapIterator itr = _vegetationInfo.begin();
            for ( ; itr != _vegetationInfo.end(); ++itr )
            {
                std::ostringstream oss;
                oss << filename << "." << itr->first << ".vegbin";

                osg::ref_ptr<osg::Vec3Array> vxs;
                osg::ref_ptr<osg::Vec3Array> scale;

                {
                    std::string treesFileName = oss.str();
                    std::ifstream file;
                    file.open(treesFileName.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
                    if (file.is_open())
                    {
                        file.seekg(0, std::ios::end);
                        long length = file.tellg();
                        file.seekg(0, std::ios::beg);

                        if (length > 0)
                        {
                            vxs = new osg::Vec3Array;
                            vxs->resize(length / sizeof(osg::Vec3));

                            file.read((char*)vxs->getDataPointer(), length);

                            osg::notify(osg::NOTICE) << "read " << length << " vegetation bytes from " << treesFileName << std::endl;
                        }
                        file.close();
                    }
                }
                {
                    std::ostringstream oss;
                    oss << filename << "." << itr->first << ".scales.vegbin";

                    std::string treesFileName = oss.str();
                    std::ifstream file;
                    file.open(treesFileName.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
                    if (file.is_open())
                    {
                        file.seekg(0, std::ios::end);
                        long length = file.tellg();
                        file.seekg(0, std::ios::beg);

                        if (length > 0)
                        {
                            scale = new osg::Vec3Array;
                            scale->resize(length / sizeof(osg::Vec3));

                            file.read((char*)scale->getDataPointer(), length);

                            osg::notify(osg::NOTICE) << "read " << length << " vegetation (scales) bytes from " << treesFileName << std::endl;
                        }
                        file.close();
                    }
                }

                if (vxs.valid() && vxs->size())
                {
#if 1
                    osg::LOD* lod = new osg::LOD;
                    root->addChild(lod);

                    osg::Geode* geode = new osg::Geode;
                    lod->addChild(geode,0.f,_lodRange);
#else
                    osg::Geode* geode = new osg::Geode;
                    root->addChild(geode);
#endif

                    osg::Geometry* geometry = new osg::Geometry;
                    geode->addDrawable(geometry);

                    geometry->setVertexArray(vxs);
                    geometry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, vxs->size()));

                    if (scale.valid() && scale->size())
                    {
                        geometry->setVertexAttribArray(7,scale);
                        geometry->setVertexAttribBinding(7, osg::Geometry::BIND_PER_VERTEX);
                    }
                    else
                    {
                        osg::ref_ptr<osg::Vec3Array> scales = new osg::Vec3Array;
                        scales->push_back(osg::Vec3(1.f,1.f,1.f));
                        geometry->setVertexAttribArray(7,scale);
                        geometry->setVertexAttribBinding(7, osg::Geometry::BIND_OVERALL);
                    }

                    osg::ref_ptr<osg::Vec4Array> uv = new osg::Vec4Array;
                    uv->push_back(itr->second._uv);
                    geometry->setVertexAttribArray(6,uv);
                    geometry->setVertexAttribBinding(6, osg::Geometry::BIND_OVERALL);

                    if (!itr->second._ss.valid())
                    {
                        osg::ref_ptr<osg::StateSet> ss = new osg::StateSet;
                        _vegetationInfo[itr->first]._ss = ss;

                        osg::ref_ptr<osg::Texture2D> texture = itr->second._texture;

                        if (!texture.valid())
                        {
                            osg::notify(osg::NOTICE) << "null texture:" << itr->second._textureName << std::endl;
                        }

                        VegetationInfo info = itr->second;

                        ss->setTextureAttributeAndModes(0, texture);
                        ss->setMode(GL_BLEND, osg::StateAttribute::ON);
                        ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

                        osg::ref_ptr<osg::Program> program = new osg::Program;
                        program->setName("microshader");

                        osg::Shader* mainVertexShader = new osg::Shader( osg::Shader::VERTEX,
                            "#version 120                                               \n"
                            "#extension GL_ARB_geometry_shader4 : enable                \n"
                            "                                                           \n"
                            "varying vec4 uv;                                           \n"
                            "attribute vec4 UV;                                         \n"
                            "                                                           \n"
                            "varying vec3 scale;                                        \n"
                            "attribute vec3 Scale;                                      \n"
                            "                                                           \n"
                            "void main()                                                \n"
                            "{															\n"
                            "   scale = Scale;                                          \n"
                            "   uv = UV;                                                \n"
                            "   gl_Position = gl_Vertex;                                \n"
                            "}                                                          \n"
                        );
                        osg::Shader* mainFragmentShader = new osg::Shader( osg::Shader::FRAGMENT,
                            "#version 120                                               \n"
                            "#extension GL_ARB_geometry_shader4 : enable                \n"
                            "                                                           \n"
                            "uniform sampler2D color_texture;                           \n"
                            "                                                           \n"
                            "void main()                                                \n"
                            "{															\n"
                            "   gl_FragColor = mix(vec4(1.0,0.0,0.0,1.5),texture2D(color_texture, gl_TexCoord[0].st),0.5);\n"
                            "}                                                          \n"
                        );

                        osg::Shader* mainGeometryShader = new osg::Shader( osg::Shader::GEOMETRY,
                            "#version 120                                               \n"
                            "#extension GL_ARB_geometry_shader4 : enable                \n"
                            "                                                           \n"
                            "varying in vec3 scale[];                                   \n"
                            "varying in vec4 uv[];                                      \n"
                            "                                                           \n"
                            "void main()                                                \n"
                            "{															\n"
                            "   float width_half = scale[0].y/2.0;                      \n"
                            "   float height = scale[0].z;                              \n"
                            "                                                           \n"
                            "   vec4 v = gl_PositionIn[0];                              \n"
                            "                                                           \n"
                            "   gl_Position = gl_ModelViewProjectionMatrix * (v + vec4(-width_half,0.0,0.0,0.0));  gl_TexCoord[0].st = vec2(uv[0].x,uv[0].y); EmitVertex();     \n"
                            "   gl_Position = gl_ModelViewProjectionMatrix * (v + vec4(width_half,0.0,0.0,0.0));  gl_TexCoord[0].st = vec2(uv[0].z,uv[0].y); EmitVertex();      \n"
                            "   gl_Position = gl_ModelViewProjectionMatrix * (v + vec4(-width_half,0.0,height,0.0));  gl_TexCoord[0].st = vec2(uv[0].x,uv[0].w); EmitVertex();  \n"
                            "   gl_Position = gl_ModelViewProjectionMatrix * (v + vec4(width_half,0.0, height,0.0));  gl_TexCoord[0].st = vec2(uv[0].z,uv[0].w); EmitVertex();  \n"
                            "   EndPrimitive();                                         \n"
                            "                                                           \n"
                            "   gl_Position = gl_ModelViewProjectionMatrix * (v + vec4(0.0,-width_half,0.0,0.0));  gl_TexCoord[0].st = vec2(uv[0].x,uv[0].y); EmitVertex();     \n"
                            "   gl_Position = gl_ModelViewProjectionMatrix * (v + vec4(0.0,width_half,0.0,0.0));  gl_TexCoord[0].st = vec2(uv[0].z,uv[0].y); EmitVertex();      \n"
                            "   gl_Position = gl_ModelViewProjectionMatrix * (v + vec4(0.0,-width_half,height,0.0));  gl_TexCoord[0].st = vec2(uv[0].x,uv[0].w); EmitVertex();  \n"
                            "   gl_Position = gl_ModelViewProjectionMatrix * (v + vec4(0.0,width_half,height,0.0));  gl_TexCoord[0].st = vec2(uv[0].z,uv[0].w); EmitVertex();   \n"
                            "   EndPrimitive();                                         \n"
                            "}                                                          \n"
                        );

                        program->addShader(mainVertexShader);
                        program->addShader(mainFragmentShader);
                        program->addShader(mainGeometryShader);

                        program->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 8);
                        program->setParameter(GL_GEOMETRY_INPUT_TYPE_EXT, GL_POINTS);
                        program->setParameter(GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);

                        program->addBindAttribLocation("UV",6);
                        program->addBindAttribLocation("Scale",7);

                        ss->setAttributeAndModes(program, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
                    }

                    geometry->setStateSet(itr->second._ss.get());

                }
            }
        }
        return result;
    }
protected:
    std::string         _pattern;
    std::string         _xmlFileName;
    double              _lodRange;

    struct VegetationInfo
    {
        std::string                         _textureName;
        osg::ref_ptr<osg::Texture2D>        _texture;
        osg::Vec3                           _scale;
        double                              _height;
        double                              _width;
        osg::ref_ptr<osg::StateSet>         _ss;
        size_t                              _id;
        osg::Vec4                           _uv;

        VegetationInfo()
            : _scale(osg::Vec3(1.f,1.f,1.f))
            , _height(0.0)
            , _width(0.0)
            , _id(0)
            , _uv(osg::Vec4(0.f,0.f,1.f,1.f))
        {

        }
    };

    typedef std::map< size_t, VegetationInfo>               VegetationInfoMap;
    typedef std::map< size_t, VegetationInfo>::iterator     VegetationInfoMapIterator;

    VegetationInfoMap   _vegetationInfo;
    size_t              _vegetationInfoId;

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

        size_t id = 0;
        osgDB::XmlNode::Children::iterator itr = config->children.begin();
        for ( ; itr != config->children.end(); ++itr)
        {
            osg::ref_ptr<osgDB::XmlNode> child = *itr;
            if (child->name == "Vegetation-Info")
            {
                readVegetationInfo(id++,child.get());
            }
            if (child->name == "LOD-Range")
            {
                _lodRange = atof(child->contents.c_str());
                if(_lodRange == 0)
                    _lodRange = 10000;
            }
        }
    }

    void readVegetationInfo(size_t id, osgDB::XmlNode* node)
    {
        if (node == NULL) return;

        VegetationInfo info;

        osgDB::XmlNode::Children::iterator itr = node->children.begin();
        for ( ; itr != node->children.end(); ++itr)
        {
            osg::ref_ptr<osgDB::XmlNode> child = *itr;

            if (child->name == "Texture")
            {
                info._textureName = child->contents;

                osg::ref_ptr<osg::Image> image = osgDB::readImageFile(info._textureName);

                info._texture = new osg::Texture2D;
                info._texture->setImage(image);

                if (!image.valid())
                {
                    osg::notify(osg::NOTICE) << "failed to load image: " << info._textureName << std::endl;
                }
                else
                {
                    osg::notify(osg::NOTICE) << "read image: " << info._textureName << std::endl;
                }
            }
            if (child->name == "Height")
            {

            }
            if (child->name == "UV")
            {
                StringUtils::Tokens tokens = StringUtils::instance()->tokenize(child->contents);
                if (tokens.size() == 4)
                {
                    info._uv.x() = atof(tokens.at(0).c_str());
                    info._uv.y() = atof(tokens.at(1).c_str());
                    info._uv.z() = atof(tokens.at(2).c_str());
                    info._uv.w() = atof(tokens.at(3).c_str());
                }
            }
            if (child->name == "Width")
            {
            }
            if (child->name == "Scale")
            {
            }

        }
        info._id = id;
        _vegetationInfo[id] = info;
    }
};

int main(int argc, char** argv)
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is viewer for OpenIG databases including GPU vegetation.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");

    osgViewer::Viewer viewer(arguments);

    unsigned int helpType = 0;
    if ((helpType = arguments.readHelpType()))
    {
        arguments.getApplicationUsage()->write(osg::notify(osg::NOTICE), helpType);
        return 1;
    }

    // report any errors if they have occurred when parsing the program arguments.
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

    viewer.setCameraManipulator( new osgGA::TrackballManipulator );
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler(new osgViewer::StatsHandler);

    std::string pattern;
    while (arguments.read("--pattern",pattern)) {}

    std::string xml = "VegetationInfo.xml";
    while (arguments.read("--config", xml)) {}

    //If we pass in just the database name, look for
    //the VegetationInfo.xml file inside that VDB's directory...CGR
    if(xml == "VegetationInfo.xml")
    {
        std::string dbnode;
        std::string configPath;
        if( arguments.isString(1) )
        {
            dbnode = arguments[1];
            configPath = osgDB::getFilePath(dbnode);
        }
        configPath += "/";
        configPath += xml;
        //If the VegetationInfo.xml file is present in the VDB directory we use it
        //otherwise we just try to find it in the current directory as usual...CGR
        if(osgDB::fileExists(configPath))
            xml = configPath;
        //osg::notify(osg::NOTICE) << "Attempting to use: " << xml << ", for VegetationInfo.xml control file..." << std::endl;
    }

    osgDB::Registry::instance()->setReadFileCallback( new DatabaseReadCallback(pattern, xml) );

    // load the data
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);
    if (!loadedModel)
    {
        osg::notify(osg::NOTICE) << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occurred when parsing the program arguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(osg::notify(osg::NOTICE));
        return 1;
    }


    viewer.setSceneData( loadedModel.get() );

    viewer.realize();

    const osg::BoundingSphere& bs = loadedModel->getBound();

    osg::notify(osg::NOTICE) << bs.center().x() << ", " << bs.center().y() << ", " << bs.center().z() << std::endl;

    return viewer.run();
}

