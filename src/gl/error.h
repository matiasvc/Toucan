#pragma once

#include <string>
#include <sstream>

#include <glad/glad.h>

inline void post_call_callback(const char *name, void* funcptr, int len_args, ...) {
	
	if (GLenum errorCode = glad_glGetError(); errorCode != GL_NO_ERROR) {
		std::ostringstream ss;
		ss << "OpenGL Error in call to " << name << " : ";
		
		switch (errorCode) {
			case GL_INVALID_ENUM:                  ss << "INVALID_ENUM"; break;
			case GL_INVALID_VALUE:                 ss << "INVALID_VALUE"; break;
			case GL_INVALID_OPERATION:             ss << "INVALID_OPERATION"; break;
			case GL_INVALID_FRAMEBUFFER_OPERATION: ss << "INVALID_FRAMEBUFFER_OPERATION"; break;
			case GL_OUT_OF_MEMORY:                 ss << "OUT_OF_MEMORY"; break;
			default:                               ss << "UNKNOWN_ERROR"; break;
		}
		
		throw std::runtime_error(ss.str());
	}
}
