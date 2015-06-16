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
#include <IgPluginCore/plugin.h>
#include <IgPluginCore/plugincontext.h>

#include <IgCore/imagegenerator.h>
#include <IgCore/attributes.h>
#include <IgCore/stringutils.h>
#include <IgCore/configuration.h>
#include <IgCore/commands.h>

#include <osg/ref_ptr>
#include <osg/StateSet>
#include <osg/Texture2D>
#include <osg/Group>
#include <osg/Geometry>
#include <osg/Geode>

#include <osgDB/XmlParser>
#include <osgDB/ReadFile>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include <osgViewer/CompositeViewer>

#include <osgShadow/ShadowedScene>
#include <osgShadow/MinimalShadowMap>

namespace igplugins
{

class GPUVegetationPlugin : public igplugincore::Plugin
{
public:

    GPUVegetationPlugin() {}

    virtual std::string getName() { return "GPUVegetation"; }

    virtual std::string getDescription( ) { return "Vegetation generated on the GPU"; }

    virtual std::string getVersion() { return "1.0.0"; }

    virtual std::string getAuthor() { return "ComPro, Nick"; }

    virtual void databaseRead(const std::string& fileName, osg::Node* node, const osgDB::Options* options)
    {
        // This is a bit hacky code. We really need to
        // remember the master offset if it is provided
        // for the database pager, but also have to
        // take into account loading of other non-offseted
        // entities. Let think more about this later
        if (fileName.find("master") != std::string::npos)
        {
            _options = const_cast<osgDB::Options*>(options);
            _path = osgDB::getFilePath(fileName);

            // Find the VegetationInfo.xml here and parse it
            _xmlFileName = osgDB::getFilePath(fileName)+"/VegetationInfo.xml";
            if (!osgDB::fileExists(_xmlFileName))
            {
                osg::notify(osg::NOTICE) << "GPU Vegetation: failed to read XML file: " << _xmlFileName << std::endl;
            }
            else
            {
                readXML(_xmlFileName);                                
            }
        }

        osg::Vec3 offset;
        if (_options.valid())
        {
            std::string offsetStr = _options.valid() ? _options->getOptionString() : "";

            igcore::StringUtils::Tokens tokensNewOffset = igcore::StringUtils::instance()->tokenize(offsetStr,",");

            if (tokensNewOffset.size()==3)
            {
                offset.x() = atof(tokensNewOffset.at(0).c_str());
                offset.y() = atof(tokensNewOffset.at(1).c_str());
                offset.z() = atof(tokensNewOffset.at(2).c_str());
            }
        }

        std::string simpleFileName = osgDB::getSimpleFileName(fileName);

        if (node && node->asGroup() && simpleFileName.substr(0,_pattern.length())==_pattern)
        {
            osg::ref_ptr<osg::Group> root = new osg::Group;

            bool shadowed = igcore::Configuration::instance()->getConfig("Shadowed-GPU-Vegetation","no") == "yes";
            if (!shadowed)
            {
                root->setNodeMask(0x4);
            }

            node->asGroup()->addChild(root);

            VegetationInfoMapIterator itr = _vegetationInfo.begin();
            for ( ; itr != _vegetationInfo.end(); ++itr )
            {
                std::ostringstream oss;
                oss << fileName << "." << itr->first << ".vegbin";

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

                            osg::notify(osg::NOTICE) << "GPU Vegetation: read " << length << " vegetation bytes from " << treesFileName << std::endl;
                        }
                        file.close();
                    }
                }
                {
                    std::ostringstream oss;
                    oss << fileName << "." << itr->first << ".scales.vegbin";

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

                            osg::notify(osg::NOTICE) << "GPU Vegetation: read " << length << " vegetation (scales) bytes from " << treesFileName << std::endl;
                        }
                        file.close();
                    }
                }

                if (vxs.valid() && vxs->size())
                {
                    if (_options.valid())
                    {
                        osg::Vec3Array::iterator itr = vxs->begin();
                        for ( ; itr != vxs->end(); ++itr )
                        {
                            osg::Vec3& v = *itr;
                            v += offset;
                        }
                    }
#if 1
                    osg::LOD* lod = new osg::LOD;
                    lod->setName("GPU-Vegetation");
                    root->addChild(lod);

                    osg::Geode* geode = new osg::Geode;
                    lod->addChild(geode,0.f,_lodRange);
#else
                    osg::Geode* geode = new osg::Geode;
                    root->addChild(geode);
#endif
                    geode->setNodeMask(0x2);

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

                    osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
                    normals->push_back(osg::Vec3(0,1,0));
                    geometry->setNormalArray(normals);
                    geometry->setNormalBinding(osg::Geometry::BIND_OVERALL);

                    if (!itr->second._ss.valid())
                    {
                        osg::ref_ptr<osg::StateSet> ss = new osg::StateSet;
                        _vegetationInfo[itr->first]._ss = ss;

                        osg::ref_ptr<osg::Texture2D> texture = itr->second._texture;

                        if (!texture.valid())
                        {
                            osg::notify(osg::NOTICE) << "GPU Vegetation: null texture:" << itr->second._textureName << std::endl;
                        }

                        ss->setTextureAttributeAndModes(0, texture.get());
                        ss->setMode(GL_BLEND, osg::StateAttribute::ON);
                        ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

                        if (!_gpuProgram.valid()) _gpuProgram = new osg::Program;
                        osg::ref_ptr<osg::Program> program = _gpuProgram;

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
                            "varying vec3 normal;                                       \n"
                            "varying vec3 eyeVec;                                       \n"
                            "varying vec3 lightDirs[8];                                 \n"
                            "uniform sampler2D color_texture;                           \n"
                            "                                                                       \n"
                            "uniform bool lightsEnabled[8];                                         \n"
                            "                                                                       \n"
                            "const float cos_outer_cone_angle = 0.4; // 36 degrees                  \n"
                            "                                                                       \n"
                            "uniform sampler2D baseTexture;                                         \n"
                            "                                                                       \n"
                            "void computeFogColor(inout vec4 color)                                 \n"
                            "{                                                                      \n"
                            "    if (gl_FragCoord.w > 0.0)                                          \n"
                            "    {                                                                  \n"
                            "        const float LOG2 = 1.442695;									\n"
                            "        float z = gl_FragCoord.z / gl_FragCoord.w;                     \n"
                            "        float fogFactor = exp2( -gl_Fog.density *						\n"
                            "            gl_Fog.density *											\n"
                            "            z *														\n"
                            "            z *														\n"
                            "            LOG2 );													\n"
                            "        fogFactor = clamp(fogFactor, 0.0, 1.0);                        \n"
                            "                                                                       \n"
                            "        vec4 clr = color;                                              \n"
                            "        color = mix(gl_Fog.color, color, fogFactor );                  \n"
                            "        color.a = clr.a;                                               \n"
                            "    }                                                                  \n"
                            "}                                                                      \n"
                            "void computeAmbientColor(inout vec4 color)                             \n"
                            "{                                                                      \n"
                            "	vec4 final_color =                                                  \n"
                            "   (gl_FrontLightModelProduct.sceneColor * gl_FrontMaterial.ambient) + \n"
                            "	(gl_LightSource[0].ambient * gl_FrontMaterial.ambient);             \n"
                            "                                                                       \n"
                            "	vec3 N = normalize(normal);                                         \n"
                            "	vec3 L = normalize(gl_LightSource[0].position.xyz);                 \n"
                            "                                                                       \n"
                            "	float lambertTerm = max(dot(N,L),0.0);                              \n"
                            "                                                                       \n"
                            "	//if(lambertTerm > 0.0)                                             \n"
                            "	{                                                                   \n"
                            "		final_color += gl_LightSource[0].diffuse *                      \n"
                            "		               gl_FrontMaterial.diffuse *                       \n"
                            "					   lambertTerm;                                     \n"
                            "                                                                       \n"
                            "		vec3 E = normalize(eyeVec);                                     \n"
                            "		vec3 R = reflect(-L, N);                                        \n"
                            "		float specular = pow( max(dot(R, E), 0.0),                      \n"
                            "		                 gl_FrontMaterial.shininess );                  \n"
                            "		final_color +=  gl_LightSource[0].specular *                    \n"
                            "                               gl_FrontMaterial.specular *				\n"
                            "					   specular;                                        \n"
                            "	}                                                                   \n"
                            "                                                                       \n"
                            "	color += final_color;                                               \n"
                            "}                                                                      \n"
                            "                                                                       \n"
                            "void computeColorForLightSource(int i, inout vec4 color)				\n"
                            "{                                                                      \n"
                            "   if (!lightsEnabled[i]) return;                                      \n"
                            "                                                                       \n"
                            "	vec4 final_color =                                                  \n"
                            "   (gl_FrontLightModelProduct.sceneColor * gl_FrontMaterial.ambient) +	\n"
                            "   (gl_LightSource[i].ambient * gl_FrontMaterial.ambient);             \n"
                            "                                                                       \n"
                            "		float distSqr = dot(lightDirs[i],lightDirs[i]);                 \n"
                            "		float invRadius = gl_LightSource[i].constantAttenuation;		\n"
                            "		float att = clamp(1.0-invRadius * sqrt(distSqr), 0.0, 1.0);		\n"
                            "		vec3 L = lightDirs[i] * inversesqrt(distSqr);                   \n"
                            "		vec3 D = normalize(gl_LightSource[i].spotDirection);			\n"
                            "                                                                       \n"
                            "		float cos_cur_angle = dot(-L, D);                               \n"
                            "                                                                       \n"
                            "		float cos_inner_cone_angle = gl_LightSource[i].spotCosCutoff;	\n"
                            "                                                                       \n"
                            "		float cos_inner_minus_outer_angle =                             \n"
                            "			cos_inner_cone_angle - cos_outer_cone_angle;                \n"
                            "                                                                       \n"
                            "		float spot = 0.0;                                               \n"
                            "		spot = clamp((cos_cur_angle - cos_outer_cone_angle) /			\n"
                            "			cos_inner_minus_outer_angle, 0.0, 1.0);                     \n"
                            "                                                                       \n"
                            "		vec3 N = normalize(normal);                                     \n"
                            "                                                                       \n"
                            "		float lambertTerm = max( dot(N,L), 0.0);                        \n"
                            "		if(lambertTerm > 0.0)                                           \n"
                            "		{                                                               \n"
                            "			final_color += gl_LightSource[i].diffuse *                  \n"
                            "				gl_FrontMaterial.diffuse *                              \n"
                            "				lambertTerm * spot * att;                               \n"
                            "                                                                       \n"
                            "			vec3 E = normalize(eyeVec);                                 \n"
                            "			vec3 R = reflect(-L, N);                                    \n"
                            "                                                                       \n"
                            "			float specular = pow( max(dot(R, E), 0.0),                  \n"
                            "				gl_FrontMaterial.shininess );                           \n"
                            "                                                                       \n"
                            "			final_color += gl_LightSource[i].specular *                 \n"
                            "				gl_FrontMaterial.specular *                             \n"
                            "				specular * spot * att;                                  \n"
                            "		}                                                               \n"
                            "                                                                       \n"
                            "                                                                       \n"
                            "		color += final_color;                                           \n"
                            "}                                                                      \n"
                            "                                                                       \n"
                            "void lighting( inout vec4 color )                                      \n"
                            "{                                                                      \n"
                            "	vec4 clr = vec4(0.0);                                               \n"
                            "                                                                       \n"
#if 0
                            "	for (int i = 1; i < 8; i++)                                         \n"
                            "	{                                                                   \n"
                            "		computeColorForLightSource(i,clr);                              \n"
                            "	}                                                                   \n"
#endif
                            "	computeAmbientColor(clr);                                           \n"
                            "                                                                       \n"
                            "	color *= clr;                                                       \n"
                            "                                                                       \n"
                            "	computeFogColor(color);                                             \n"
                            "}                                                                      \n"
                            "void main()                                                \n"
                            "{															\n"
                            "   vec4 color = texture2D(baseTexture, gl_TexCoord[0].st); \n"
                            "   if (color.a < 0.6) color.a = 0.0;                       \n"
                            "   lighting(color);                                        \n"
                            "   gl_FragColor = color;                                   \n"
                            "}                                                          \n"
                        );

                        osg::Shader* mainGeometryShader = new osg::Shader( osg::Shader::GEOMETRY,
                            "#version 120                                               \n"
                            "#extension GL_ARB_geometry_shader4 : enable                \n"
                            "                                                           \n"
                            "varying in vec3 scale[];                                   \n"
                            "varying in vec4 uv[];                                      \n"
                            "varying vec3 normal;                                       \n"
                            "varying vec3 eyeVec;                                       \n"
                            "varying vec3 lightDirs[8];                                 \n"
                            "                                                           \n"
                            "void setupVaryings(in vec4 v)                              \n"
                            "{                                                          \n"
                            "   normal = vec3(0.0,1.0,0.0);                             \n"
                            "   vec3 vVertex = vec3(gl_ModelViewMatrix * v);            \n"
                            "                                                           \n"
                            "   lightDirs[0] = gl_LightSource[0].position.xyz;          \n"
                            "   lightDirs[1] = vec3(gl_LightSource[1].position.xyz-vVertex);    \n"
                            "   lightDirs[2] = vec3(gl_LightSource[1].position.xyz-vVertex);    \n"
                            "   lightDirs[3] = vec3(gl_LightSource[1].position.xyz-vVertex);    \n"
                            "   lightDirs[4] = vec3(gl_LightSource[1].position.xyz-vVertex);    \n"
                            "   lightDirs[5] = vec3(gl_LightSource[1].position.xyz-vVertex);    \n"
                            "   lightDirs[6] = vec3(gl_LightSource[1].position.xyz-vVertex);    \n"
                            "   lightDirs[7] = vec3(gl_LightSource[1].position.xyz-vVertex);    \n"
                            "                                                           \n"
                            "                                                             \n"
                            "   eyeVec = -vVertex;                                      \n"
                            "}                                                          \n"
                            "void main()                                                \n"
                            "{															\n"
                            "   float width_half = scale[0].y/2.0;                      \n"
                            "   float height = scale[0].z;                              \n"
                            "                                                           \n"
                            "   vec4 v = gl_PositionIn[0];                              \n"
                            "                                                           \n"
                            "   gl_Position = gl_ModelViewProjectionMatrix * (v + vec4(-width_half,0.0,0.0,0.0));  gl_TexCoord[0].st = vec2(uv[0].x,uv[0].y); setupVaryings(v); EmitVertex();     \n"
                            "   gl_Position = gl_ModelViewProjectionMatrix * (v + vec4(width_half,0.0,0.0,0.0));  gl_TexCoord[0].st = vec2(uv[0].z,uv[0].y); setupVaryings(v); EmitVertex();      \n"
                            "   gl_Position = gl_ModelViewProjectionMatrix * (v + vec4(-width_half,0.0,height,0.0));  gl_TexCoord[0].st = vec2(uv[0].x,uv[0].w); setupVaryings(v); EmitVertex();  \n"
                            "   gl_Position = gl_ModelViewProjectionMatrix * (v + vec4(width_half,0.0, height,0.0));  gl_TexCoord[0].st = vec2(uv[0].z,uv[0].w); setupVaryings(v); EmitVertex();  \n"
                            "   EndPrimitive();                                         \n"
                            "                                                           \n"
                            "   gl_Position = gl_ModelViewProjectionMatrix * (v + vec4(0.0,-width_half,0.0,0.0));  gl_TexCoord[0].st = vec2(uv[0].x,uv[0].y); setupVaryings(v); EmitVertex();     \n"
                            "   gl_Position = gl_ModelViewProjectionMatrix * (v + vec4(0.0,width_half,0.0,0.0));  gl_TexCoord[0].st = vec2(uv[0].z,uv[0].y); setupVaryings(v); EmitVertex();      \n"
                            "   gl_Position = gl_ModelViewProjectionMatrix * (v + vec4(0.0,-width_half,height,0.0));  gl_TexCoord[0].st = vec2(uv[0].x,uv[0].w); setupVaryings(v); EmitVertex();  \n"
                            "   gl_Position = gl_ModelViewProjectionMatrix * (v + vec4(0.0,width_half,height,0.0));  gl_TexCoord[0].st = vec2(uv[0].z,uv[0].w); setupVaryings(v); EmitVertex();   \n"
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
    }    

    virtual void init(igplugincore::PluginContext& context)
    {
        igcore::Commands::instance()->addCommand("gpuveg",new GPUVegetationCommand(context.getImageGenerator(),_lodRange));
    }

protected:
    std::string                                 _pattern;
    std::string                                 _xmlFileName;
    double                                      _lodRange;
    osg::ref_ptr<const osgDB::Options>          _options;
    osg::ref_ptr<osg::Program>                  _gpuProgram;

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
    std::string         _path;

    void readXML(const std::string& fileName)
    {
        osg::ref_ptr<osgDB::XmlNode> root = osgDB::readXmlFile(fileName);
        if (!root.valid())
        {
            osg::notify(osg::NOTICE) << "GPU Vegetation: Failed to read the configuration xml: " << fileName << std::endl;
            return;
        }

        osg::ref_ptr<osgDB::XmlNode> config = root->children.size() ? root->children.at(0) : 0;
        if (!config.valid())
        {
            osg::notify(osg::NOTICE) << "GPU Vegetation: Expecting <OpenIG-Vegetation-Config> tag" << std::endl;
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
            }
            if (child->name == "Pattern")
            {
                _pattern = child->contents;
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
                if (!image.valid())
                {
                    image = osgDB::readImageFile(_path+"/"+info._textureName);
                }

                info._texture = new osg::Texture2D;
                info._texture->setImage(image);

                if (!image.valid())
                {
                    osg::notify(osg::NOTICE) << "GPU Vegetation: failed to load image: " << info._textureName << std::endl;
                }
                else
                {
                    osg::notify(osg::NOTICE) << "GPU Vegetation: read image: " << info._textureName << std::endl;
                }
            }
            if (child->name == "Height")
            {

            }
            if (child->name == "UV")
            {
                igcore::StringUtils::Tokens tokens = igcore::StringUtils::instance()->tokenize(child->contents);
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

    class GPUVegetationCommand : public igcore::Commands::Command
    {
    public:
        GPUVegetationCommand(igcore::ImageGenerator* ig, double& range)
            : _ig(ig)
            , _range(range)
        {

        }

        virtual const std::string getUsage() const
        {
            return "range";
        }

        virtual const std::string getDescription() const
        {
            return  "sets the range of the GPU vegetation LODs\n"
                    "     range - the range in meters for the LOD nodes of the GPU vegetation\n";
        }

        class SetRangeNodeVisitor : public osg::NodeVisitor
        {
        public:
            SetRangeNodeVisitor(double range)
                : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
                , _range(range)
            {

            }

            virtual void apply(osg::LOD& lod)
            {
                if (lod.getName() == "GPU-Vegetation")
                {
                    lod.setRange(0,0,_range);
                }
                traverse(lod);
            }

        protected:
            double  _range;
        };

        virtual int exec(const igcore::StringUtils::Tokens& tokens)
        {
            if (tokens.size() == 1)
            {
                double range = atof(tokens.at(0).c_str());
                _range = range;

                SetRangeNodeVisitor nv(range);
                _ig->getScene()->accept(nv);

                return 0;
            }

            return -1;
        }
    protected:
        igcore::ImageGenerator* _ig;
        double&                 _range;
    };
};

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

extern "C" EXPORT igplugincore::Plugin* CreatePlugin()
{
    return new igplugins::GPUVegetationPlugin;
}

extern "C" EXPORT void DeletePlugin(igplugincore::Plugin* plugin)
{
    osg::ref_ptr<igplugincore::Plugin> p(plugin);
}
