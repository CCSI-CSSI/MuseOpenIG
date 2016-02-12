#include "shaderutils.h"
#include "glerrorutils.h"

#include <Library-Graphics/OIGAssert.h>

namespace osg
{
	GLint ShaderUtils::compileShader(const std::string& strSource, osg::Shader::Type shaderType, osg::GLExtensions* ext)
	{
		if (ext==0)
		{
			osg::notify(osg::ALWAYS) << "Shader compilation: error: Extensions are Null" << std::endl;
			return 0;
		}

		GLint shaderID = ext->glCreateShader(shaderType);
		GLenum errorCode = OpenIG::Base::GLErrorUtils::checkAndLogError();
		if (shaderID==0 || errorCode!=GL_NO_ERROR)
		{
			osg::notify(osg::ALWAYS) << "Shader compilation: error Could not create a Shader Object" << std::endl;
			return 0;
		}
		const GLchar* src = static_cast<const GLchar*>(strSource.c_str());
		ext->glShaderSource(shaderID, 1, &(src), NULL);
		ext->glCompileShader(shaderID);

		GLint bCompiled;
		ext->glGetShaderiv(shaderID, GL_COMPILE_STATUS, &bCompiled);
		ASSERT_PREDICATE(bCompiled==GL_TRUE);
		if (bCompiled!=GL_TRUE)
		{
			GLsizei logLength = 0;
			GLchar message[2048];
			ext->glGetShaderInfoLog(shaderID, 1024, &logLength, message);
			osg::notify(osg::ALWAYS)<<"Shader compilation error: shader compilation error: "<<std::endl;
			osg::notify(osg::ALWAYS)<<message<<std::endl;
			return 0;
		}
		else
		{
			osg::notify(osg::ALWAYS) << "Shader compilation: Shader compiled successfully..." << std::endl;
		}
		return shaderID;
	}
}

