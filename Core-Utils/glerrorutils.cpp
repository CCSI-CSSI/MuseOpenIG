#include "glerrorutils.h"

#include <osg/Notify>

namespace OpenIG {
	namespace Utils {
		GLenum GLErrorUtils::checkAndLogError(void)
		{
			GLenum errCode = glGetError();
			switch (errCode)
			{
			case GL_NO_ERROR:
				break;
			case GL_INVALID_ENUM:
                osg::notify(osg::ALWAYS) << "OpenGL error: [0x"<<std::hex<<errCode<<std::dec<<"] GL_INVALID_ENUM" << std::endl;
				break;
			case GL_INVALID_VALUE:
                osg::notify(osg::ALWAYS) << "OpenGL error: [0x"<<std::hex<<errCode<<std::dec<<"] GL_INVALID_VALUE" << std::endl;
				break;
			case GL_INVALID_OPERATION:
                osg::notify(osg::ALWAYS) << "OpenGL error: [0x"<<std::hex<<errCode<<std::dec<<"] GL_INVALID_OPERATION" << std::endl;
				break;
			case GL_OUT_OF_MEMORY:
                osg::notify(osg::ALWAYS) << "OpenGL error: [0x"<<std::hex<<errCode<<std::dec<<"] GL_OUT_OF_MEMORY" << std::endl;
				break;
			case GL_STACK_UNDERFLOW:
                osg::notify(osg::ALWAYS) << "OpenGL error: [0x"<<std::hex<<errCode<<std::dec<<"] GL_STACK_UNDERFLOW" << std::endl;
				break;
			case GL_STACK_OVERFLOW:
                osg::notify(osg::ALWAYS) << "OpenGL error: [0x"<<std::hex<<errCode<<std::dec<<"] GL_STACK_OVERFLOW" << std::endl;
				break;
            default:
                osg::notify(osg::ALWAYS) << "OpenGL error: [0x"<<std::hex<<errCode<<std::dec<<"] UNKNOWN" << std::endl;
                break;
            }
			return errCode;
		}
	}
}
